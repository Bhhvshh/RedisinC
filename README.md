# Redis-Inc With TCP Server

A small Redis-like key/value store written in C. It keeps data in memory, supports a few basic commands, and can be used either from the terminal loop or over a TCP socket.

## Features

- In-memory hash table-backed database
- Commands: `SET`, `GET`, `DEL`, `EXIT`
- TCP server on port `8080`
- Simple text protocol for remote clients
- Persistence helpers are implemented in the codebase, but not enabled by default

## Build

Compile the project with `gcc` on Linux:

```bash
gcc main.c miniredis.c tcpserver.c -o main
```

## Run

Start the server:

```bash
./main
```

The TCP server listens on `0.0.0.0:8080`.

## Connect

Use `nc` or another TCP client:

```bash
nc localhost 8080
```

## Commands

The server accepts plain text commands separated by spaces:

```text
SET name Alice
GET name
DEL name
EXIT
```

Responses are prefixed with `+` for success and `-` for errors.

Examples:

```text
SET name Alice
+Done

GET name
+Result: Alice

DEL name
+Deleted
```

## Project Files

- `main.c` starts the database and TCP server
- `miniredis.c` contains the hash table, command parser, and database logic
- `tcpserver.c` handles socket setup and client communication
- `redislikeinc.h` defines the shared data structures and function declarations

## Notes

- The database is stored in memory while the process is running.
- Persistence functions (`db_save` and `db_load`) are present in the source, but the main loop does not currently call them.
- The server handles one client connection at a time.
