# IRC Server (ft_ircserv)

![IRC Server](https://img.shields.io/badge/IRC-Server-blue)
![C++98](https://img.shields.io/badge/C%2B%2B-98-orange)
![Version](https://img.shields.io/badge/Version-1.0-green)

**Last Updated**: 2025-04-05

## Table of Contents
1. [Introduction](#introduction)
2. [Project Overview](#project-overview)
3. [Features](#features)
4. [Implementation Details](#implementation-details)
5. [Network Programming Concepts](#network-programming-concepts)
6. [Project Constraints](#project-constraints)
7. [Building and Running](#building-and-running)
8. [Commands](#commands)
9. [Architecture](#architecture)
10. [Future Improvements](#future-improvements)
11. [Credits](#credits)

## Introduction

This project implements an IRC (Internet Relay Chat) server from scratch in C++. IRC is a text-based communication protocol that facilitates communication in the form of text messages between clients in a client-server networking model.

The server handles multiple clients simultaneously, manages channels, and implements the core IRC commands according to RFC standards.

## Project Overview

IRC (Internet Relay Chat) is one of the first chat protocols on the internet, dating back to 1988. This implementation follows the core principles of IRC while focusing on a C++98 compliant codebase with robust socket programming and concurrent client handling.

The server is built to handle:
- Multiple simultaneous client connections
- Channel creation and management
- Private messaging between users
- Channel operator privileges
- Various IRC commands as specified in the RFC standards

## Features

- **User Authentication**: Password-protected server access
- **Nickname Management**: Unique nickname assignment and validation
- **Channel Operations**: Create, join, leave, and manage channels
- **Private Messaging**: Direct communication between users
- **Channel Modes**: Support for various channel modes including:
  - Invite-only channels
  - Topic restrictions
  - Channel password protection
  - User limits
  - Operator privileges
- **Network Communication**: Robust socket programming with polling mechanisms
- **Command Processing**: Full parsing and execution of IRC commands
- **Error Handling**: RFC-compliant error responses

## Implementation Details

### Core Components

1. **Server Class**
   - Manages the main server loop
   - Handles client connections using poll()
   - Processes incoming messages and dispatches commands

2. **Client Class**
   - Represents a connected user
   - Stores user information (nickname, username, etc.)
   - Tracks authentication state

3. **Channel Class**
   - Manages channel properties and members
   - Handles channel modes and operator privileges
   - Controls access to channels based on various restrictions

### Socket Programming

The server uses Berkeley sockets API for network communication:
- Socket creation and binding
- Setting up listening sockets
- Accepting new connections
- Non-blocking I/O with poll() for handling multiple clients

### Command Handling

Commands are processed using a command dispatcher pattern:
- Each IRC command maps to a specific handler method
- Command arguments are parsed and validated
- Appropriate responses are generated according to RFC specifications

## Network Programming Concepts

### Socket Communication
The server utilizes TCP/IP sockets to establish reliable, connection-oriented communication with clients. Key socket-related functions used include:
- `socket()`: Creates an endpoint for communication
- `bind()`: Assigns a local address to a socket
- `listen()`: Marks a socket as passive, ready to accept connections
- `accept()`: Accepts a connection on a socket
- `send()/recv()`: Transmits and receives data

### Multiplexing with poll()
Rather than creating a thread for each client (which would violate the project constraints), the server uses `poll()` to multiplex I/O operations:
- Monitors multiple file descriptors simultaneously
- Efficiently handles many connections with a single thread
- Avoids blocking operations that would halt the entire server

## Project Constraints

This implementation adheres to strict project requirements:
- **C++ Standard**: Limited to C++98 standard
- **External Libraries**: No external libraries beyond standard C/C++ libraries and system calls
- **Threading**: No threads or fork (single process using poll() for concurrency)
- **Memory Management**: No memory leaks or undefined behaviors
- **Error Handling**: Robust error handling for all network operations
- **RFC Compliance**: Implementation follows RFC 1459 and related RFC documents for IRC protocol

## Building and Running

### Prerequisites
- c++/g++ GCC/Clang compiler with C++98 support
- Linux/Unix environment (for socket APIs)

### Compilation
```bash
make
# IRC Server

## Running the Server
./ircserv <port> <password>
```

Where:
* `<port>`: The port number the server will listen on
* `<password>`: Password required for clients to connect to the server

## Connecting with a Client
You can connect using any standard IRC client:
```bash
nc <server_ip> <port>
# Then authenticate with:
PASS <password>
NICK <your_nickname>
USER <username> 0 * :<realname>
```

## Commands
The server supports the following IRC commands:

| Command | Format | Description |
|---------|--------|-------------|
| PASS | `PASS <password>` | Set connection password |
| NICK | `NICK <nickname>` | Set or change nickname |
| USER | `USER <username> <mode> <unused> :<realname>` | Set user information |
| JOIN | `JOIN <channel>{,<channel>} [<key>{,<key>}]` | Join channel(s) with optional key(s) |
| PART | `PART <channel>{,<channel>} [<message>]` | Leave channel(s) |
| PRIVMSG | `PRIVMSG <target> :<message>` | Send message to user or channel |
| QUIT | `QUIT [<message>]` | Disconnect from server |
| KICK | `KICK <channel> <user> [<comment>]` | Remove user from channel |
| INVITE | `INVITE <nickname> <channel>` | Invite user to channel |
| TOPIC | `TOPIC <channel> [:<topic>]` | Set or view channel topic |
| MODE | `MODE <target> <modes> [<mode-params>]` | Change channel or user mode |
| PING | `PING <server>` | Test connection to server |
| PONG | `PONG <server>` | Reply to PING message |

## Channel Modes
| Mode | Parameter | Description |
|------|-----------|-------------|
| i | None | Set/remove Invite-only channel |
| t | None | Set/remove restrictions of TOPIC command |
| k | Key | Set/remove channel key (password) |
| o | Nickname | Give/take channel operator privileges |
| l | Limit | Set/remove user limit to channel |

## Architecture
### Class Relationships
```
┌────────────┐       ┌────────────┐
│   Server   │◄─────►│   Client   │
└────────────┘       └────────────┘
      ▲                    ▲
      │                    │
      │                    │
      ▼                    │
┌────────────┐             │
│  Channel   │◄────────────┘
└────────────┘
```

* **Server**: Central class managing connections and dispatching commands
* **Client**: Represents a connected user with their state
* **Channel**: Handles channel operations and membership

### Data Flow
1. Client connects to server
2. Server authenticates client (PASS, NICK, USER)
3. Client joins channels or sends messages
4. Server routes messages to appropriate clients
5. Server handles administrative commands and channel operations

## Future Improvements
* File transfer capabilities
* SSL/TLS support for secure connections
* More extensive channel modes
* Bot support for automated operations
* Web interface for server administration

## Credits
Developed by @saifeddineelhanoune @Da-ghost42

Project completed as part of advanced networking programming studies.

© 2025 - IRC Server Implementation - Version 1.0
