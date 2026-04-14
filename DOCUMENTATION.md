# Documentation

## Overview

This project implements a minimal Redis-like database in C. It provides a hash table-backed key/value store, a command parser, and a TCP server that accepts simple text commands over port `8080`.

## Architecture

### Entry Point

`main.c` creates the database instance and starts the TCP server.

### Database Layer

`miniredis.c` contains:

- Hash table creation and cleanup
- Key insertion, lookup, and deletion
- Command parsing
- Command execution for both console and TCP server usage
- File persistence helpers

### Networking Layer

`tcpserver.c` sets up a TCP socket, listens on `0.0.0.0:8080`, accepts clients, receives commands, and sends back formatted responses.

## Data Model

The database stores string keys and string values.

- Each entry is stored in a chained hash table bucket
- Collisions are handled with linked lists
- The current implementation supports one value type: `TYPE_STRING`

## Supported Commands

### `SET <key> <value>`

Stores or updates a key/value pair.

Example:

```text
SET user Alice
```

### `GET <key>`

Returns the stored value for a key.

Example:

```text
GET user
```

### `DEL <key>`

Deletes a key/value pair.

Example:

```text
DEL user
```

### `EXIT`

Recognized by the parser, but the TCP server does not treat it as a normal shutdown command.

## TCP Protocol

Client requests are plain text, terminated by newline or whitespace.

Typical interaction:

```text
Client: SET name Alice
Server: +Done
Client: GET name
Server: +Result: Alice
Client: DEL name
Server: +Deleted
```

Error responses begin with `-`.

## Persistence

The code includes `db_save` and `db_load` functions that read and write a binary `database.db` file.

Important detail: persistence is currently optional and not wired into the main startup/shutdown flow. If persistence is needed, `db_load` should be called at startup and `db_save` before exit.

## Limitations

- Single-threaded server
- One client handled at a time
- Fixed-size receive buffer
- No authentication
- No command pipelining or RESP compatibility

## Suggested Improvements

- Enable persistence in the startup/shutdown path
- Add input validation for missing arguments
- Support multiple clients concurrently
- Replace the custom protocol with RESP for better Redis compatibility
