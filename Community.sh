#!/usr/bin/env bash

# Comprehensive IRC Testing Script
# Usage: ./test_irc.sh <server_ip> <port> <password>

SERVER_IP=${1:-"0.0.0.0"}
PORT=${2:-6666}
PASSWORD=${3:-"Password1!"}

# Colors for better readability
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
PURPLE='\033[0;35m'
NC='\033[0m' # No Color

# Test results tracking
TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0

# Log directory
LOG_DIR="irc_test_logs"
mkdir -p "$LOG_DIR"
TEST_SUMMARY="$LOG_DIR/test_summary.log"
rm -f "$TEST_SUMMARY"
touch "$TEST_SUMMARY"

echo -e "${YELLOW}=================================${NC}" | tee -a "$TEST_SUMMARY"
echo -e "${YELLOW}   IRC SERVER TEST SUITE        ${NC}" | tee -a "$TEST_SUMMARY"
echo -e "${YELLOW}=================================${NC}" | tee -a "$TEST_SUMMARY"
echo -e "${YELLOW}Server: $SERVER_IP:$PORT${NC}" | tee -a "$TEST_SUMMARY"
echo -e "${YELLOW}$(date)${NC}" | tee -a "$TEST_SUMMARY"
echo -e "${YELLOW}=================================${NC}" | tee -a "$TEST_SUMMARY"

# Set up temp directory for socket files
TEMP_DIR=$(mktemp -d)
trap 'rm -rf "$TEMP_DIR"; exit' EXIT INT TERM

# Message delay to prevent flooding
MSG_DELAY=0.5
# Longer delay for operations that need time to complete
OP_DELAY=1

# Track user status
declare -A USER_STATUS

# Log a test result
log_test() {
    local test_name=$1
    local status=$2
    local details=${3:-""}
    
    TOTAL_TESTS=$((TOTAL_TESTS + 1))
    
    if [[ "$status" == "PASS" ]]; then
        PASSED_TESTS=$((PASSED_TESTS + 1))
        echo -e "${GREEN}[PASS]${NC} $test_name" | tee -a "$TEST_SUMMARY"
    else
        FAILED_TESTS=$((FAILED_TESTS + 1))
        echo -e "${RED}[FAIL]${NC} $test_name: $details" | tee -a "$TEST_SUMMARY"
    fi
    
    if [[ -n "$details" && "$status" == "PASS" ]]; then
        echo -e "       ${BLUE}$details${NC}" | tee -a "$TEST_SUMMARY"
    fi
}

# Function to set up a client with a socket
setup_client() {
    local nick=$1
    local username=$2
    local socket="$TEMP_DIR/$nick.sock"
    local log_file="$LOG_DIR/${nick}.log"
    
    echo -e "${GREEN}Connecting user $nick...${NC}"
    
    # Start netcat with a Unix socket for communication
    mkfifo "$socket"
    nc "$SERVER_IP" "$PORT" < "$socket" > "$log_file" 2>&1 &
    local pid=$!
    
    # Store the PID and socket
    eval "${nick}_pid=$pid"
    eval "${nick}_socket=$socket"
    eval "${nick}_log=$log_file"
    
    # Send authentication commands
    echo "PASS $PASSWORD" > "$socket"
    sleep $MSG_DELAY
    echo "NICK $nick" > "$socket"
    sleep $MSG_DELAY
    echo "USER $username 0 * :$username" > "$socket"
    
    # Wait for authentication to complete
    sleep $OP_DELAY
    
    # Check if login was successful by looking for welcome messages
    if grep -q "001" "$log_file"; then
        USER_STATUS["$nick"]="connected"
        echo -e "${GREEN}User $nick authenticated successfully${NC}"
        return 0
    else
        USER_STATUS["$nick"]="failed"
        echo -e "${RED}User $nick authentication failed${NC}"
        return 1
    fi
}

# Function to send a command through a user's socket
send_command() {
    local nick=$1
    local command=$2
    local socket_var="${nick}_socket"
    local socket="${!socket_var}"
    
    echo -e "${BLUE}[$nick] Sending: $command${NC}"
    echo "$command" > "$socket"
    sleep $MSG_DELAY
}

# Function to send a message
send_message() {
    local nick=$1
    local target=$2
    local message=$3
    
    echo -e "${CYAN}[$nick -> $target]: $message${NC}"
    send_command "$nick" "PRIVMSG $target :$message"
    sleep $MSG_DELAY
}

# Function to join a channel
join_channel() {
    local nick=$1
    local channel=$2
    local key=${3:-""}
    
    echo -e "${PURPLE}[$nick] Joining channel $channel${NC}"
    if [ -z "$key" ]; then
        send_command "$nick" "JOIN $channel"
    else
        send_command "$nick" "JOIN $channel $key"
    fi
    sleep $OP_DELAY
    
    # Check if join was successful
    local log_var="${nick}_log"
    local log="${!log_var}"
    
    if grep -q "JOIN.*$channel" "$log"; then
        log_test "$nick joined $channel" "PASS"
        return 0
    else
        log_test "$nick joined $channel" "FAIL" "No JOIN confirmation found in logs"
        return 1
    fi
}

# Function to part (leave) a channel
part_channel() {
    local nick=$1
    local channel=$2
    local reason=${3:-"Leaving channel"}
    
    echo -e "${PURPLE}[$nick] Leaving channel $channel: $reason${NC}"
    send_command "$nick" "PART $channel :$reason"
    sleep $OP_DELAY
    
    # Check if part was successful
    local log_var="${nick}_log"
    local log="${!log_var}"
    
    if grep -q "PART.*$channel" "$log"; then
        log_test "$nick left $channel" "PASS"
        return 0
    else
        log_test "$nick left $channel" "FAIL" "No PART confirmation found in logs"
        return 1
    fi
}

# Function to kick a user from a channel
kick_user() {
    local kicker=$1
    local channel=$2
    local target=$3
    local reason=${4:-"Kicked from channel"}
    
    echo -e "${RED}[$kicker] Kicking $target from $channel: $reason${NC}"
    send_command "$kicker" "KICK $channel $target :$reason"
    sleep $OP_DELAY
    
    # Check if kick was successful
    local log_var="${kicker}_log"
    local log="${!log_var}"
    
    if grep -q "KICK.*$channel.*$target" "$log"; then
        log_test "$kicker kicked $target from $channel" "PASS"
        return 0
    else
        log_test "$kicker kicked $target from $channel" "FAIL" "No KICK confirmation found in logs"
        return 1
    fi
}

# Function to invite a user to a channel
invite_user() {
    local inviter=$1
    local target=$2
    local channel=$3
    
    echo -e "${PURPLE}[$inviter] Inviting $target to $channel${NC}"
    send_command "$inviter" "INVITE $target $channel"
    sleep $OP_DELAY
    
    # Check if invite was successful
    local log_var="${inviter}_log"
    local log="${!log_var}"
    
    if grep -q "341.*$target.*$channel" "$log" || grep -q "INVITE.*$target.*$channel" "$log"; then
        log_test "$inviter invited $target to $channel" "PASS"
        return 0
    else
        log_test "$inviter invited $target to $channel" "FAIL" "No invite confirmation found in logs"
        return 1
    fi
}

# Function to set channel mode
set_channel_mode() {
    local nick=$1
    local channel=$2
    local mode=$3
    local param=${4:-""}
    
    echo -e "${YELLOW}[$nick] Setting mode $mode on $channel ${param:+with param $param}${NC}"
    if [ -z "$param" ]; then
        send_command "$nick" "MODE $channel $mode"
    else
        send_command "$nick" "MODE $channel $mode $param"
    fi
    sleep $OP_DELAY
    
    # Check if mode change was successful
    local log_var="${nick}_log"
    local log="${!log_var}"
    
    # Remove +/- for grep
    local mode_stripped=$(echo "$mode" | sed 's/[+-]//g')
    
    if grep -q "MODE.*$channel.*$mode" "$log" || grep -q "324.*$channel.*$mode_stripped" "$log"; then
        log_test "$nick set mode $mode on $channel" "PASS"
        return 0
    else
        log_test "$nick set mode $mode on $channel" "FAIL" "No mode confirmation found in logs"
        return 1
    fi
}

# Function to check if private message was received
check_privmsg_received() {
    local sender=$1
    local receiver=$2
    local keyword=$3
    
    local log_var="${receiver}_log"
    local log="${!log_var}"
    
    if grep -q "PRIVMSG.*$keyword" "$log"; then
        log_test "Message from $sender to $receiver was received" "PASS" "Message contained '$keyword'"
        return 0
    else
        log_test "Message from $sender to $receiver was received" "FAIL" "No message containing '$keyword' found"
        return 1
    fi
}

# Connect all users
echo -e "${YELLOW}===== Connecting Users =====${NC}" | tee -a "$TEST_SUMMARY"
setup_client "Owner" "owner"
setup_client "Admin1" "admin1"
setup_client "Admin2" "admin2"
setup_client "User1" "user1"
setup_client "User2" "user2"
setup_client "User3" "user3"
setup_client "Guest1" "guest1"
setup_client "Guest2" "guest2"

# PING test to verify connections
echo -e "${YELLOW}===== Testing User Connections =====${NC}" | tee -a "$TEST_SUMMARY"
for user in Owner Admin1 Admin2 User1 User2 User3 Guest1 Guest2; do
    if [[ "${USER_STATUS[$user]}" == "connected" ]]; then
        send_command "$user" "PING :connection_test"
        sleep $MSG_DELAY
        
        local log_var="${user}_log"
        local log="${!log_var}"
        
        if grep -q "PONG" "$log"; then
            log_test "$user connection test" "PASS"
        else
            log_test "$user connection test" "FAIL" "No PONG response"
        fi
    else
        log_test "$user connection test" "FAIL" "User not connected"
    fi
done

# Create and test a standard channel
echo -e "${YELLOW}===== Testing Channel Creation and Join =====${NC}" | tee -a "$TEST_SUMMARY"
join_channel "Owner" "#general"
join_channel "Admin1" "#general"
join_channel "Admin2" "#general"
join_channel "User1" "#general"
join_channel "User2" "#general"

# Test PRIVMSG to channel
echo -e "${YELLOW}===== Testing Channel Messaging =====${NC}" | tee -a "$TEST_SUMMARY"
send_message "Owner" "#general" "Hello everyone in #general!"
sleep $OP_DELAY

# Check if other users received the message
for user in Admin1 Admin2 User1 User2; do
    check_privmsg_received "Owner" "$user" "Hello everyone in #general"
done

# Test MODE command for channel
echo -e "${YELLOW}===== Testing Channel Modes =====${NC}" | tee -a "$TEST_SUMMARY"
set_channel_mode "Owner" "#general" "+t" # Only ops can change topic
set_channel_mode "Owner" "#general" "+o" "Admin1" # Make Admin1 an operator

# Test operator privileges
echo -e "${YELLOW}===== Testing Operator Commands =====${NC}" | tee -a "$TEST_SUMMARY"
send_command "Admin1" "TOPIC #general :Welcome to the general discussion channel"
sleep $OP_DELAY

log_var="Admin1_log"
log="${!log_var}"
if grep -q "TOPIC.*#general.*Welcome" "$log"; then
    log_test "Admin1 set topic on #general" "PASS"
else
    log_test "Admin1 set topic on #general" "FAIL" "No topic confirmation found"
fi

# Test non-operator trying to set topic (should fail)
send_command "User1" "TOPIC #general :This topic should not be set"
sleep $OP_DELAY

log_var="User1_log"
log="${!log_var}"
if grep -q "482.*#general.*not channel operator" "$log"; then
    log_test "User1 blocked from setting topic" "PASS" "Correctly denied permission"
else
    log_test "User1 blocked from setting topic" "FAIL" "No permission denial found or topic was set"
fi

# Test KICK command
kick_user "Owner" "#general" "User2" "Test kick, please rejoin"

# Test INVITE and channel mode +i
echo -e "${YELLOW}===== Testing Invite-Only Channel =====${NC}" | tee -a "$TEST_SUMMARY"
join_channel "Owner" "#private"
set_channel_mode "Owner" "#private" "+i" # Set channel to invite-only

# Test join without invite (should fail)
send_command "User3" "JOIN #private"
sleep $OP_DELAY

log_var="User3_log"
log="${!log_var}"
if grep -q "473.*#private" "$log" || ! grep -q "JOIN.*#private" "$log"; then
    log_test "User3 blocked from joining invite-only channel" "PASS" "Correctly denied access"
else
    log_test "User3 blocked from joining invite-only channel" "FAIL" "User was able to join without invite"
fi

# Test INVITE command
invite_user "Owner" "User3" "#private"
join_channel "User3" "#private"

# Test channel with key
echo -e "${YELLOW}===== Testing Password Protected Channel =====${NC}" | tee -a "$TEST_SUMMARY"
join_channel "Admin2" "#secure"
set_channel_mode "Admin2" "#secure" "+k" "secretpass"

# Test join without key (should fail)
send_command "Guest1" "JOIN #secure"
sleep $OP_DELAY

log_var="Guest1_log"
log="${!log_var}"
if grep -q "475.*#secure" "$log" || ! grep -q "JOIN.*#secure" "$log"; then
    log_test "Guest1 blocked from joining without password" "PASS" "Correctly denied access"
else
    log_test "Guest1 blocked from joining without password" "FAIL" "User was able to join without password"
fi

# Test join with key
join_channel "Guest1" "#secure" "secretpass"

# Test direct messaging
echo -e "${YELLOW}===== Testing Direct Messaging =====${NC}" | tee -a "$TEST_SUMMARY"
send_message "Guest2" "Guest1" "Hello, this is a private message"
sleep $OP_DELAY
check_privmsg_received "Guest2" "Guest1" "private message"

# Test PART command
echo -e "${YELLOW}===== Testing Channel Part =====${NC}" | tee -a "$TEST_SUMMARY"
part_channel "User1" "#general" "Testing PART command"

# Test messaging after leaving (should fail)
send_message "User1" "#general" "Can I still talk here?"
sleep $OP_DELAY

log_var="User1_log"
log="${!log_var}"
if grep -q "442.*#general" "$log" || grep -q "403.*#general" "$log"; then
    log_test "User1 blocked from messaging after leaving" "PASS" "Correctly denied access"
else
    log_test "User1 blocked from messaging after leaving" "FAIL" "User was able to message after leaving"
fi

# Test user limit
echo -e "${YELLOW}===== Testing User Limit =====${NC}" | tee -a "$TEST_SUMMARY"
join_channel "Admin1" "#limited"
set_channel_mode "Admin1" "#limited" "+l" "2"
join_channel "Guest1" "#limited"

# Third user should fail to join
send_command "Guest2" "JOIN #limited"
sleep $OP_DELAY

log_var="Guest2_log"
log="${!log_var}"
if grep -q "471.*#limited" "$log" || ! grep -q "JOIN.*#limited" "$log"; then
    log_test "Guest2 blocked from joining full channel" "PASS" "Correctly denied access"
else
    log_test "Guest2 blocked from joining full channel" "FAIL" "User was able to join despite limit"
fi

# Test summary
echo -e "${YELLOW}===== Test Results =====${NC}" | tee -a "$TEST_SUMMARY"
echo -e "Total tests: ${TOTAL_TESTS}" | tee -a "$TEST_SUMMARY"
echo -e "${GREEN}Passed: ${PASSED_TESTS}${NC}" | tee -a "$TEST_SUMMARY"
echo -e "${RED}Failed: ${FAILED_TESTS}${NC}" | tee -a "$TEST_SUMMARY"

if [ $FAILED_TESTS -eq 0 ]; then
    echo -e "${GREEN}All tests passed successfully!${NC}" | tee -a "$TEST_SUMMARY"
    exit 0
else
    echo -e "${RED}Some tests failed. Check the logs for details.${NC}" | tee -a "$TEST_SUMMARY"
    exit 1
fi