
# Mini Redis

A Redis-like in-memory database built from scratch in  **pure C++20** .

The goal of this project is not simply to recreate Redis commands. The project is being built incrementally to understand how a high-performance network server works internally.

We are implementing the networking, protocol parsing, data structures, concurrency, event loop, persistence, and testing ourselves rather than hiding the important concepts behind libraries.

---

## Current Status

### Milestone 1 — TCP Server

Current functionality:

* Creates an IPv4 TCP socket
* Binds to `0.0.0.0:6379`
* Starts listening for connections
* Accepts TCP clients
* Receives raw bytes using `recv()`
* Sends responses using `send()`
* Implements a basic `PING` command
* Handles unknown commands
* Handles client disconnection
* Runs continuously

Current protocol:

```text
PING
```

Response:

```text
PONG
```

This is  **not Redis-compatible yet** .

The real Redis protocol, RESP, will be implemented in a later milestone.

---

# Project Structure

```text
mini-redis/
│
├── Dockerfile
├── README.md
├── .gitignore
│
└── src/
    └── main.cpp
```

The project will become more modular as functionality is added.

Eventually we expect something closer to:

```text
mini-redis/
│
├── Dockerfile
├── README.md
├── CMakeLists.txt
│
├── include/
│   ├── server.hpp
│   ├── client.hpp
│   ├── resp.hpp
│   └── database.hpp
│
├── src/
│   ├── main.cpp
│   ├── server.cpp
│   ├── client.cpp
│   ├── resp.cpp
│   └── database.cpp
│
└── tests/
```

---

# Building

The project is currently built using `g++` with C++20.

```bash
g++ -std=c++20 -Wall -Wextra -pedantic src/main.cpp -o mini-redis
```

Run:

```bash
./mini-redis
```

Expected output:

```text
Socket created. fd = 3
Socket bound to port 6379
Server listening on port 6379
```

---

# Running With Docker

The project uses Docker so the development environment does not depend on the host operating system.

Build the image:

```bash
docker build -t mini-redis .
```

Run the container:

```bash
docker run --rm -it -p 6379:6379 mini-redis
```

The port mapping:

```text
6379:6379
```

means:

```text
Host port 6379
       │
       ▼
Container port 6379
```

---

# Testing With Netcat

With the server running, connect from another terminal:

```bash
nc 127.0.0.1 6379
```

Type:

```text
PING
```

The server should respond:

```text
PONG
```

For an unknown command:

```text
HELLO
```

the server responds:

```text
ERR unknown command
```

---

# Networking Architecture

The current server follows this sequence:

```text
socket()
   │
   ▼
bind()
   │
   ▼
listen()
   │
   ▼
accept()
   │
   ▼
recv()
   │
   ▼
command handling
   │
   ▼
send()
   │
   ▼
close()
```

The listening socket and client socket are different.

```text
                server_fd
                   │
                   ▼
            ┌──────────────┐
            │ LISTEN socket│
            └──────┬───────┘
                   │
                accept()
                   │
                   ▼
               client_fd
                   │
                   ▼
            ┌──────────────┐
            │ ESTABLISHED  │
            │ TCP socket   │
            └──────────────┘
```

The listening socket accepts connections.

The client socket communicates with a specific client.

---

# Important Concepts Learned

## File Descriptors

Linux represents resources such as sockets using integer file descriptors.

For example:

```text
server_fd = 3
client_fd = 4
```

The number itself is simply an identifier used by the process to refer to the kernel-managed resource.

---

## `socket()`

```cpp
socket(AF_INET, SOCK_STREAM, 0);
```

Creates an IPv4 TCP socket.

```text
AF_INET
    ↓
IPv4

SOCK_STREAM
    ↓
TCP-style byte stream
```

---

## `bind()`

```cpp
bind(...)
```

Associates the socket with a local IP address and port.

Our server uses:

```text
0.0.0.0:6379
```

`0.0.0.0` means the socket is bound to all local IPv4 interfaces.

---

## `listen()`

```cpp
listen(server_fd, SOMAXCONN);
```

Changes the socket into a listening socket.

It allows the kernel to maintain incoming connection state for the server.

---

## `accept()`

```cpp
int client_fd = accept(...);
```

Retrieves a pending client connection and gives the application a new socket file descriptor.

Important:

```text
server_fd ≠ client_fd
```

The listening socket remains available for additional connections.

---

## `recv()`

```cpp
recv(client_fd, buffer, size, 0);
```

Reads bytes from the TCP connection.

Its return value is important:

```text
> 0
    number of bytes received

0
    peer closed the connection

-1
    error
```

---

## `send()`

```cpp
send(client_fd, data, size, 0);
```

Writes bytes to the TCP connection.

Later we will handle an important issue:

**`send()` is not guaranteed to send all requested bytes in one call.**

---

# TCP Is a Byte Stream

This is one of the most important networking concepts in this project.

If a client performs:

```text
send("PING")
send("GET key")
```

the server is not guaranteed to receive:

```text
PING
GET key
```

as two separate `recv()` calls.

It could receive:

```text
PINGGET key
```

or:

```text
PI
```

followed by:

```text
NGGET key
```

TCP guarantees ordered bytes, not application-level messages.

Therefore we eventually need:

```text
TCP byte stream
       │
       ▼
Input buffer
       │
       ▼
Protocol parser
       │
       ▼
Complete command
```

This becomes especially important when implementing RESP.

---

# Current Limitations

This version is deliberately simple.

### 1. One client at a time

The server currently uses blocking I/O:

```text
accept()
   ↓
recv()
   ↓
send()
   ↓
close()
   ↓
accept()
```

If one client connects and does not send anything, the server can remain blocked in `recv()`.

Other clients cannot be processed concurrently.

---

### 2. No RESP

The current server accepts:

```text
PING
```

rather than Redis's actual wire protocol.

We will implement RESP ourselves.

---

### 3. No persistent database

There is currently no:

```text
SET
GET
DEL
EXISTS
```

and no in-memory key-value store.

---

### 4. Fixed receive buffer

The current implementation uses:

```cpp
char buffer[1024];
```

This is intentionally simple.

A production server needs dynamically managed input buffers and proper message framing.

---

### 5. No concurrency

There are currently no:

* threads
* thread pools
* non-blocking sockets
* `epoll`
* event loop

These will be introduced later after understanding why they are necessary.

---

# Development Roadmap

## Phase 1 — Networking

### Milestone 1

* [X] `socket()`
* [X] `bind()`
* [X] `listen()`
* [X] `accept()`
* [X] `recv()`
* [X] `send()`
* [X] Basic `PING`

### Milestone 2

* [ ] Persistent client connections
* [ ] Multiple requests per connection
* [ ] Proper send handling
* [ ] Input buffering
* [ ] Graceful disconnect handling

### Milestone 3

* [ ] Multiple clients
* [ ] Threads
* [ ] Thread safety
* [ ] Connection lifecycle

### Milestone 4

* [ ] Non-blocking sockets
* [ ] `epoll`
* [ ] Event loop
* [ ] Event-driven server

---

# Phase 2 — Redis Protocol

* [ ] Understand RESP
* [ ] RESP simple strings
* [ ] RESP errors
* [ ] RESP integers
* [ ] RESP bulk strings
* [ ] RESP arrays
* [ ] RESP parser
* [ ] RESP serializer
* [ ] Streaming/partial input parsing

---

# Phase 3 — Database

Implement:

```text
SET
GET
DEL
EXISTS
```

Then:

```text
INCR
DECR
MGET
MSET
```

Eventually:

```text
LIST
SET
HASH
SORTED SET
```

---

# Phase 4 — Memory Management

Study and implement:

* Memory ownership
* Object lifetime
* Allocations
* Deallocations
* Hash tables
* Load factors
* Rehashing
* Memory usage tracking

---

# Phase 5 — Expiration

Implement:

```text
SET key value EX 10
```

and:

```text
TTL key
```

This will introduce:

* Time handling
* Expiration metadata
* Lazy expiration
* Active expiration

---

# Phase 6 — Persistence

Investigate and implement simplified versions of:

```text
RDB
AOF
```

Topics:

* Serialization
* File I/O
* Durability
* Recovery
* Crash consistency

---

# Phase 7 — Production-Oriented Improvements

* [ ] Configuration
* [ ] Logging
* [ ] Error handling
* [ ] Signals
* [ ] Graceful shutdown
* [ ] Metrics
* [ ] Connection limits
* [ ] Memory limits
* [ ] Benchmarks
* [ ] Stress testing

---

# Phase 8 — Testing

We will eventually create:

```text
tests/
```

with tests for:

* TCP connection handling
* RESP parsing
* Command parsing
* `SET`
* `GET`
* `DEL`
* expiration
* concurrent clients
* malformed requests
* partial TCP packets
* large requests

---

# Design Goal

The goal is not to produce a copy of Redis line-by-line.

The goal is to understand the systems underneath a Redis-like server:

```text
                 ┌─────────────────┐
                 │      Client     │
                 └────────┬────────┘
                          │
                       TCP/IP
                          │
                 ┌────────▼────────┐
                 │  Network Layer  │
                 └────────┬────────┘
                          │
                 ┌────────▼────────┐
                 │   Event Loop    │
                 └────────┬────────┘
                          │
                 ┌────────▼────────┐
                 │  RESP Parser    │
                 └────────┬────────┘
                          │
                 ┌────────▼────────┐
                 │ Command Handler │
                 └────────┬────────┘
                          │
                 ┌────────▼────────┐
                 │   Data Store    │
                 └────────┬────────┘
                          │
                 ┌────────▼────────┐
                 │   Persistence   │
                 └─────────────────┘
```

The final project should demonstrate understanding of  **C++, Linux networking, TCP/IP, operating-system interfaces, concurrency, event-driven architecture, protocol design, data structures, memory management, persistence, and performance engineering** .

---

## Current Git checkpoint

After confirming:

```text
PING → PONG
```

commit this milestone:

```bash
git add .
git commit -m "feat: implement basic TCP server with ping"
```

This commit becomes our clean baseline before we introduce  **persistent connections and proper request buffering** .
