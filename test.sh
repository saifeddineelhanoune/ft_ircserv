#!/bin/bash

# IRC Server details
SERVER="localhost"    # Change this to your IRC server address
PORT="6667"           # Change this to your IRC server port

# Number of clients to simulate
NUM_CLIENTS=300

# Channels to join
CHANNELS=("#general" "#dev" "#random")

# Message content
MESSAGE="Hello from test client"

# Function to simulate a single IRC client
simulate_client() {
    local client_id=$1
    local nick="User$client_id"
    local user="User$client_id 0 * :Test Client $client_id"
    
    {
        echo -ne "PASS Password1!\r\n"
        echo -ne "NICK $nick\r\n"
        echo -ne "USER $user\r\n"
        sleep 1

        # Join multiple channels
        for channel in "${CHANNELS[@]}"; do
            echo -ne "JOIN $channel\r\n"
            sleep 1
        done

        # Send messages to channels
        for channel in "${CHANNELS[@]}"; do
            echo -ne "PRIVMSG $channel :$MESSAGE from $nick\r\n"
            sleep 1
        done

        # Set user mode
        echo -ne "MODE $nick +i\r\n"
        sleep 1

        # Leave channels
        for channel in "${CHANNELS[@]}"; do
            echo -ne "PART $channel :Leaving\r\n"
            sleep 1
        done

        # Quit the server
        echo -ne "QUIT :Goodbye from $nick\r\n"

    } | nc $SERVER $PORT
}

# Run multiple clients in parallel
for i in $(seq 1 $NUM_CLIENTS); do
    simulate_client $i &
done

# Wait for all clients to finish
wait

echo "Test completed with $NUM_CLIENTS clients."
