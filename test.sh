#!/usr/bin/env bash

# Enhanced IRC Community Simulation Script with Communication Metrics
# Usage: ./irc_community.sh <server_ip> <port> <password>

SERVER_IP=${1:-"127.0.0.1"}
PORT=${2:-6666}
PASSWORD=${3:-"Password123!"}

# Colors for better output readability
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
PURPLE='\033[0;35m'
ORANGE='\033[0;33m'
NC='\033[0m' # No Color

# Message delay to make conversation look natural
MSG_DELAY=2

# Set up temp directory for socket files
TEMP_DIR=$(mktemp -d)
trap 'rm -rf "$TEMP_DIR"; exit' EXIT INT TERM

echo -e "${YELLOW}Starting Enhanced IRC Community Simulation...${NC}"
echo -e "${YELLOW}Connecting to $SERVER_IP:$PORT${NC}"

# Function to set up a client with a socket
setup_client() {
    local nick=$1
    local username=$2
    local socket="$TEMP_DIR/$nick.sock"
    
    echo -e "${GREEN}Connecting user $nick...${NC}"
    
    # Start netcat with a Unix socket for communication
    mkfifo "$socket"
    nc "$SERVER_IP" "$PORT" < "$socket" > "/tmp/irc_${nick}.log" 2>&1 &
    local pid=$!
    
    # Store the PID and socket
    eval "${nick}_pid=$pid"
    eval "${nick}_socket=$socket"
    
    # Send authentication commands
    echo "PASS $PASSWORD" > "$socket"
    sleep 0.5
    echo "NICK $nick" > "$socket"
    sleep 0.5
    echo "USER $username 0 * :$username" > "$socket"
    
    # Wait for authentication to complete
    sleep 3
    
    echo -e "${GREEN}User $nick authenticated${NC}"
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
}

# Connect all users
setup_client "AdminUser" "admin"
setup_client "Moderator1" "mod1"
setup_client "Moderator2" "mod2"
setup_client "JohnDoe" "johnd"
setup_client "JaneSmith" "janes"
setup_client "NewGuy" "newguy"
setup_client "Developer1" "dev1"
setup_client "Developer2" "dev2"
setup_client "MarketingUser" "marketing"
setup_client "SupportAgent" "support"

# Verify authentication with a PING
echo -e "${YELLOW}Verifying authentication...${NC}"
for user in AdminUser Moderator1 Moderator2 JohnDoe JaneSmith NewGuy Developer1 Developer2 MarketingUser SupportAgent; do
    send_command "$user" "PING :${user}_auth_check"
done
sleep 2

# Now proceed with the scenario...
echo -e "${ORANGE}===== Creating main channel =====${NC}"
join_channel "AdminUser" "#main"
send_command "AdminUser" "TOPIC #main :Welcome to the IRC Community!"

# Continue with your script...
# ...