Fixed the existing code structure to ensure it compiles correctly
Enhanced the Channel class to support:
Channel operators
Invite-only mode (i)
Topic restriction mode (t)
Channel key/password (k)
User limits (l)
Operator privileges (o)
Implemented all the required channel operator commands:
KICK: Allows operators to remove users from a channel
INVITE: Allows operators to invite users to a channel
TOPIC: Allows viewing or changing the channel topic
MODE: Allows changing channel modes (i, t, k, o, l)
The server now meets all the requirements specified in the subject:
The server handles multiple clients at the same time using non-blocking I/O with poll()
Authentication works with nickname, username, and password.
Channel operations work (joining, messaging, leaving).
Private messaging between users works.
All required operator commands are implemented.
The implementation follows the C++98 standard as required and uses poll() for handling all I/O operations in a non-blocking manner.


