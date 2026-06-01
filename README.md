*This project has been created as part of the 42 curriculum by malena-b, luialvar and manuelfe.*

# ft_irc
Implementation of an IRC (Internet Relay Chat) server in C++98 as part of the 42 cursus.
---
### Description
This project is a from-scratch implementation of an IRC server. The server is capable of handling multiple concurrent client connections using poll(), and follows the RFC 2812 guidelines for client-server communication. It supports core IRC functionalities such as channels, private messages, and both user and channel mode management.

IRC (Internet Relay Chat) is a text-based, real-time communication protocol originally designed by Jarkko Oikarinen in 1988 and standardized through various RFC (Request for Comments) documents.

Unlike modern messaging platforms based on HTTP or WebSockets, the classic IRC standard is an application layer protocol that runs directly over TCP, utilizing a Client-Server architecture structured around the following rules:

Plain-Text Messaging: Every command sent or received is a text string strictly terminated by \r\n (CRLF) characters, with a maximum limit of 512 bytes.

Dynamic Channels: Channels (chat rooms starting with #) are created dynamically when the first user joins them and are automatically destroyed when the last member leaves.

Standardized Numeric Replies: The server communicates with clients through both direct commands and 3-digit "numeric replies" (codes from 001 to 599) to notify events, states, or syntax errors (e.g., 461 for missing parameters or 482 for channel operator privileges denied).

###Instructions
1. **Compile the project:**

```bash
make
```
2. **Run the server:**

```bash
./ircserv <port> <password>
```
* **`<port>`**: The port on which the server will listen for connections (e.g., 6667).

* **`<password>`**: The password required for clients to connect to the server.

3. **Connect with an IRC client:**

You can use any IRC client like HexChat, irssi, or even nc to test the connection.

Using `nc`:

```bash
nc localhost 6667
```
Once connected, you must register by sending the following commands in order:

```
PASS <password>
NICK <your_nickname>
USER <your_username> 0 * :<your_real_name>
```
### Resources

*   **RFC 2812**: Internet Relay Chat: Client Protocol

*   **Modern IRC Documentation**: ircdocs.horse

*   https://medium.com/@afatir.ahmedfatir/small-irc-server-ft-irc-42-network-7cee848de6f9

*   https://modern.ircdocs.horse/#join-message

*   **AI Usage Declaration**:
 AI was used to assist in debugging (such as remembering to capture error codes when calling functions), verifying permissions (checking whether a client is authorized to perform a specific action), generating repetitive boilerplate code like getters and setters, locating precise error codes within the official documentation, and filtering external documentation to check its relevance for specific edge cases.

### Features List and Architecture Design
The server implements a robust architecture for command processing and state management.

#### Implemented Commands
* **Connection and Registration**: `PASS`, `NICK`, `USER`, `QUIT`

* **Channel Operations:** `JOIN`, `PART`, `TOPIC`, `INVITE`, `KICK`

* **Messaging**: `PRIVMSG`

* **Mode Management**: `MODE`

* **Queries**: `WHO`

* **Miscellaneous**: `PING`, `PONG`

#### Command Architecture
The server utilizes the **Command Design Pattern** to encapsulate each action into an object.

1. **Parsing (`parser.cpp`)**: Every message received from a client is processed by the `parseMessage` function. This function tokenizes the message into its three main components: prefix (optional), command, and arguments, properly handling middle parameters and the final trailing parameter. The result is stored in a `CommandParts` structure.

2. **Execution (`Server::executeCommand`)**: This function acts as a dispatcher. It uses a map (`_commandHandlers`) to associate the command string (e.g., "MODE") with a function pointer that handles it (e.g., `&Server::handleMode`).

3. **Command Classes (e.g., `ModeCommand.hpp`)**: For complex commands, the handler function (e.g., `handleMode`) instantiates a specific class (e.g., `ModeCommand`). This class receives references to the server, the client, and the arguments, and contains the entire validation and execution logic inside its `execute()` method.

#### The Channel Class
The `Channel` class is a core component for managing IRC channels.

* **Member Management**: Stores pointers to the `Client` objects currently in the channel.

* **Channel Properties**: Keeps track of the topic, channel key (`+k`), user limit (`+l`), etc.

* **Mode Management**: Uses a `std::set<char>` to store active flags (`i`, `t`, `k`, `o`, `l`). This allows for highly efficient lookups.

* **Roles**: Maintains separate sets (`std::set`) for operators (`+o`) and invited clients (`+i`), enabling clear and efficient permission management.

* **Communication**: Provides a `broadcastMessage` method to relay messages to all members of the channel.

#### The MODE Command
The `MODE` command is one of the most complex elements, making it an excellent example of the server's architectural capabilities.

1. **Mode Querying**: If `MODE #channel` is invoked without further arguments, the server replies with the channel's current mode status (RPL_CHANNELMODEIS).

2. **Mode Modification**:

* **Parsing (`ModeCommand::_parse`)**: The parsing logic iterates through the mode string (e.g., `+ik-l <key> <limit>`). It is capable of grouping multiple mode changes under a single sign (+ or -) and correctly matching parameters with the specific modes that require them (`o`, `k`, `l`).

* **Permissions (`_checkPermissions`)**: Before applying any changes, the server verifies that the client executing the command has channel operator privileges.

* **Execution (`_applyChanges`): This method sequentially applies the validated changes. It calls the corresponding methods of the `Channel` class to toggle a mode flag (`setMode`/`unsetMode`) or to set a specific value (e.g., `setKey`, `setUserLimit`).

* **Notification (`_broadcastChanges`)**: Once all modifications are applied, a single `MODE` message summarizing all successful changes is constructed and broadcast to all channel members, keeping them perfectly synchronized.
