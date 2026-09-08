*This project has been created as part of the 42 curriculum by Anthony, Ellabiad.*

# ft_irc

## Description

`ircserv` is a small Internet Relay Chat server written in ISO C++98. It accepts
multiple TCP/IPv4 clients in one non-blocking `poll()` event loop. Clients can
authenticate, choose a nickname and username, join channels, exchange direct or
channel messages, and use the required channel-operator commands.

Implemented IRC features include:

- Registration with `CAP`, `PASS`, `NICK`, and `USER`
- `PING`/`PONG`, `QUIT`, nickname changes, and basic discovery commands
- `JOIN`, `PART`, `PRIVMSG`, `NOTICE`, `NAMES`, `LIST`, `WHO`, and `WHOIS`
- Operator commands `KICK`, `INVITE`, `TOPIC`, and `MODE`
- Channel modes `i` (invite only), `t` (operator-only topic), `k` (key),
  `o` (channel operator), and `l` (user limit)
- Fragmented input aggregation and buffered partial writes

The server deliberately bounds per-client input and output queues so one slow or
malicious peer cannot consume unbounded memory. No process forking, threads, or
external libraries are used.

## Instructions

Build the executable:

```sh
make
```

Run it with a port and connection password:

```sh
./ircserv 6667 secret
```

Connect with an IRC client such as HexChat or irssi, using `127.0.0.1`, port
`6667`, and server password `secret`. A minimal netcat session is also possible:

```text
PASS secret
NICK alice
USER alice 0 * :Alice Example
JOIN #general
PRIVMSG #general :hello
```

Use `make clean` to remove object files, `make fclean` to also remove the
executable, and `make re` for a complete rebuild.

## Technical choices

All sockets are non-blocking. The listening socket and every connected client are
included in the same `poll()` call. `recv()` is called only after `POLLIN`, and
`send()` only after `POLLOUT`; unsent data remains queued for a later event.
Channels refer to clients by file descriptor, keeping ownership centralized in the
server and making disconnect cleanup deterministic.

## Resources

- [RFC 1459 — Internet Relay Chat Protocol](https://www.rfc-editor.org/rfc/rfc1459)
- [RFC 2812 — Internet Relay Chat: Client Protocol](https://www.rfc-editor.org/rfc/rfc2812)
- [Modern IRC Client Protocol](https://modern.ircdocs.horse/)
- [`poll(2)` manual](https://man7.org/linux/man-pages/man2/poll.2.html)
- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/)

AI assistance was used to plan the class boundaries, draft implementation code,
review error paths, and design integration tests. The networking model, protocol
behavior, generated source, and test results were checked against the project
requirements and the references above.
