
# Mini Redis

A Redis-like in-memory database server built from scratch in C++.

The goal of this project is to understand:

- TCP/IP networking
- Linux socket APIs
- Client-server architecture
- Network protocols
- Event-driven I/O
- Concurrency
- In-memory data structures
- Persistence
- Systems programming
- Performance engineering

## Current Progress

### Stage 1 — TCP Socket

Implemented:

- IPv4 TCP socket creation
- Socket binding

Not implemented yet:

- Listening
- Accepting connections
- Reading/writing data
- RESP protocol
- Redis commands

### Stage 2 -- TCP Accept 

* implemented Accept
* Application started accepting connections
* Till only one connection is accepted
* Gradully writing it eventually build the event loop and trying to apply threading
