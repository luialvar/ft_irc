# ft_irc

Implementación de un servidor de IRC (Internet Relay Chat) en C++98 como parte del cursus de 42.

---

## 🇪🇸 Versión en Castellano

### Descripción

Este proyecto es una implementación desde cero de un servidor de IRC. El servidor es capaz de gestionar múltiples conexiones de clientes de forma concurrente utilizando `poll()`, y sigue las directrices del RFC 2812 para la comunicación entre cliente y servidor.

Soporta funcionalidades básicas de IRC como canales, mensajes privados, y la gestión de modos de usuario y de canal.

### Instrucciones

1.  **Compilar el proyecto:**

    ```bash
    make
    ```

2.  **Ejecutar el servidor:**

    ```bash
    ./ircserv <puerto> <contraseña>
    ```

    *   **`<puerto>`**: El puerto en el que el servidor escuchará las conexiones (e.g., 6667).
    *   **`<contraseña>`**: La contraseña requerida para que los clientes se conecten al servidor.

3.  **Conectar con un cliente IRC:**

    Puedes usar cualquier cliente de IRC como HexChat, irssi, o incluso `nc` para probar la conexión.

    Con `nc`:
    ```bash
    nc localhost 6667
    ```

    Una vez conectado, debes registrarte enviando los siguientes comandos en orden:
    ```
    PASS <contraseña>
    NICK <tu_nickname>
    USER <tu_usuario> 0 * :<tu_nombre_real>
    ```

### Recursos

*   **RFC 2812**: Internet Relay Chat: Client Protocol
*   **Documentación moderna de IRC**: ircdocs.horse

### Lista de Funcionalidades y Diseño

El servidor implementa una arquitectura robusta para el manejo de comandos y la gestión de estado.

#### Comandos Implementados

*   **Conexión y Registro**: `PASS`, `NICK`, `USER`, `QUIT`
*   **Operaciones de Canal**: `JOIN`, `PART`, `TOPIC`, `INVITE`, `KICK`
*   **Mensajería**: `PRIVMSG`
*   **Gestión de Modos**: `MODE`
*   **Consultas**: `WHO`
*   **Misceláneos**: `PING`, `PONG`

#### Arquitectura de Comandos

El servidor utiliza el **patrón de diseño Command** para encapsular cada acción en un objeto.

1.  **Parseo (`parser.cpp`)**: Cada mensaje recibido del cliente es procesado por la función `parseMessage`. Esta función descompone el mensaje en sus tres partes principales: prefijo (opcional), comando y argumentos, manejando correctamente los parámetros intermedios y el parámetro final (trailing). El resultado se almacena en una estructura `CommandParts`.

2.  **Ejecución (`Server::executeCommand`)**: Esta función actúa como un despachador. Utiliza un mapa (`_commandHandlers`) para asociar el string del comando (e.g., "MODE") con un puntero a la función que lo maneja (e.g., `&Server::handleMode`).

3.  **Clases de Comando (e.g., `ModeCommand.hpp`)**: Para comandos complejos, la función manejadora (e.g., `handleMode`) instancia una clase específica (e.g., `ModeCommand`). Esta clase recibe las referencias al servidor, al cliente y los argumentos, y contiene toda la lógica para validar y ejecutar el comando en su método `execute()`.

#### Clase `Channel`

La clase `Channel` es fundamental para la gestión de los canales de IRC.

*   **Gestión de Miembros**: Almacena punteros a los `Client` que están en el canal.
*   **Propiedades del Canal**: Guarda el topic, la clave (`+k`), el límite de usuarios (`+l`), etc.
*   **Gestión de Modos**: Utiliza un `std::set<char>` para almacenar los modos activos (`i`, `t`, `k`, `o`, `l`). Esto permite una comprobación muy eficiente.
*   **Roles**: Mantiene conjuntos (`std::set`) separados para operadores (`+o`) y clientes invitados (`+i`), permitiendo una gestión de permisos clara y eficiente.
*   **Comunicación**: Proporciona un método `broadcastMessage` para enviar un mensaje a todos los miembros del canal.

#### Foco en el Comando `MODE`

El comando `MODE` es uno de los más complejos y su implementación es un buen ejemplo de la arquitectura del servidor.

1.  **Consulta de Modos**: Si se invoca `MODE #canal` sin más argumentos, el servidor responde con el estado actual de los modos del canal (RPL\_CHANNELMODEIS).

2.  **Modificación de Modos**:
    *   **Parseo (`ModeCommand::_parse`)**: La lógica de parseo itera sobre la cadena de modos (e.g., `+ik-l <clave> <limite>`). Es capaz de agrupar múltiples cambios de modo bajo un solo signo (`+` o `-`) y de asociar correctamente los parámetros con los modos que los requieren (`o`, `k`, `l`).
    *   **Permisos (`_checkPermissions`)**: Antes de aplicar cualquier cambio, se verifica que el cliente que ejecuta el comando sea un operador del canal.
    *   **Aplicación (`_applyChanges`)**: Este método aplica secuencialmente los cambios validados. Llama a los métodos correspondientes de la clase `Channel` para activar/desactivar un modo (`setMode`/`unsetMode`) o para establecer un valor (e.g., `setKey`, `setUserLimit`).
    *   **Notificación (`_broadcastChanges`)**: Una vez aplicados los cambios, se construye un único mensaje `MODE` que resume todas las modificaciones exitosas y se envía a todos los miembros del canal, manteniéndolos sincronizados.

---

## 🇬🇧 English Version

### Description

This project is a from-scratch implementation of an IRC server. The server is capable of handling multiple client connections concurrently using `poll()` and follows the guidelines of RFC 2812 for client-server communication.

It supports basic IRC features such as channels, private messages, and user/channel mode management.

### Instructions

1.  **Compile the project:**

    ```bash
    make
    ```

2.  **Run the server:**

    ```bash
    ./ircserv <port> <password>
    ```

    *   **`<port>`**: The port on which the server will listen for connections (e.g., 6667).
    *   **`<password>`**: The password required for clients to connect to the server.

3.  **Connect with an IRC client:**

    You can use any IRC client like HexChat, irssi, or even `nc` to test the connection.

    With `nc`:
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

### Features and Design

The server implements a robust architecture for command handling and state management.

#### Implemented Commands

*   **Connection & Registration**: `PASS`, `NICK`, `USER`, `QUIT`
*   **Channel Operations**: `JOIN`, `PART`, `TOPIC`, `INVITE`, `KICK`
*   **Messaging**: `PRIVMSG`
*   **Mode Management**: `MODE`
*   **Queries**: `WHO`
*   **Miscellaneous**: `PING`, `PONG`

#### Command Architecture

The server uses the **Command design pattern** to encapsulate each action into an object.

1.  **Parsing (`parser.cpp`)**: Each message received from the client is processed by the `parseMessage` function. This function breaks down the message into its three main parts: prefix (optional), command, and arguments, correctly handling middle and trailing parameters. The result is stored in a `CommandParts` struct.

2.  **Execution (`Server::executeCommand`)**: This function acts as a dispatcher. It uses a map (`_commandHandlers`) to associate the command string (e.g., "MODE") with a pointer to its handler function (e.g., `&Server::handleMode`).

3.  **Command Classes (e.g., `ModeCommand.hpp`)**: For complex commands, the handler function (e.g., `handleMode`) instantiates a specific class (e.g., `ModeCommand`). This class receives references to the server, client, and arguments, and contains all the logic to validate and execute the command in its `execute()` method.

#### `Channel` Class

The `Channel` class is central to managing IRC channels.

*   **Member Management**: Stores pointers to the `Client`s in the channel.
*   **Channel Properties**: Holds the topic, key (`+k`), user limit (`+l`), etc.
*   **Mode Management**: Uses a `std::set<char>` to store active modes (`i`, `t`, `k`, `o`, `l`), allowing for very efficient checks.
*   **Roles**: Maintains separate sets (`std::set`) for operators (`+o`) and invited clients (`+i`), enabling clean and efficient permission management.
*   **Communication**: Provides a `broadcastMessage` method to send a message to all channel members.

#### Focus on the `MODE` Command

The `MODE` command is one of the most complex, and its implementation is a good showcase of the server's architecture.

1.  **Mode Query**: If `MODE #channel` is invoked with no other arguments, the server replies with the channel's current mode status (RPL\_CHANNELMODEIS).

2.  **Mode Modification**:
    *   **Parsing (`ModeCommand::_parse`)**: The parsing logic iterates over the mode string (e.g., `+ik-l <key> <limit>`). It can group multiple mode changes under a single sign (`+` or `-`) and correctly associate parameters with the modes that require them (`o`, `k`, `l`).
    *   **Permissions (`_checkPermissions`)**: Before applying any changes, it verifies that the client executing the command is a channel operator.
    *   **Application (`_applyChanges`)**: This method sequentially applies the validated changes. It calls the corresponding methods of the `Channel` class to enable/disable a mode (`setMode`/`unsetMode`) or to set a value (e.g., `setKey`, `setUserLimit`).
    *   **Notification (`_broadcastChanges`)**: Once the changes are applied, a single `MODE` message summarizing all successful modifications is built and sent to all channel members, keeping them in sync.
