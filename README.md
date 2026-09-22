*This project has been created as part of the 42 curriculum by ellabiad, anlamber.*

# ft_irc

## Description

`ft_irc` is a from-scratch implementation of an IRC (Internet Relay Chat) server,
written in C++98. The goal of the project is to understand and implement a real
network protocol: handling multiple simultaneous TCP clients with a single
`poll()` loop, parsing the IRC message format, and reproducing the core behavior
of a real IRC server closely enough that a standard IRC client can connect to it,
authenticate, join channels, and exchange messages.

The server does **not** implement an IRC client, and does **not** implement
server-to-server communication (no network of servers, only clients talking to
one server).

### Implemented features

- Connection registration: `PASS`, `NICK`, `USER`
- Channels: `JOIN`, `PART`, `TOPIC`
- Messaging: `PRIVMSG` (to a channel or to a user), `PING` / `PONG`
- Channel operators: `KICK`, `INVITE`
- Channel modes (`MODE`):
  - `i` — invite-only channel
  - `t` — restrict `TOPIC` to operators
  - `k` — channel key (password)
  - `o` — give/take operator privilege
  - `l` — user limit
- Graceful disconnect on `QUIT` or on connection loss, with cleanup from every
  channel the client was in.

## Instructions

### Build

```sh
make
```

This produces the `ircserv` executable. Other standard targets are available:
`make clean`, `make fclean`, `make re`.

### Run

```sh
./ircserv <port> <password>
```

- `port`: the TCP port the server listens on (e.g. `6667`).
- `password`: the password an IRC client must provide (via the `PASS` command)
  before registering.

### Connect

Connect with any standard IRC client, for example [irssi](https://irssi.org/):

```sh
irssi -c 127.0.0.1 -p <port> -w <password>
```

or with `nc` for quick manual testing:

```sh
nc 127.0.0.1 <port>
PASS <password>
NICK me
USER me 0 * :me
JOIN #test
```

## Resources

- [RFC 1459 — Internet Relay Chat Protocol](https://datatracker.ietf.org/doc/html/rfc1459)
- [RFC 2812 — IRC Client Protocol](https://datatracker.ietf.org/doc/html/rfc2812)
- [modern.ircdocs.info](https://modern.ircdocs.info/) — up-to-date, readable reference for IRC message formats and numeric replies
- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/) — sockets, `poll()`, non-blocking I/O
- `man` pages: `socket(2)`, `poll(2)`, `fcntl(2)`, `accept(2)`, `recv(2)`, `send(2)`

### Use of AI

Claude (Anthropic) was used throughout the project as a coding assistant in the
terminal (Claude Code), in a peer-programming style: explaining IRC protocol
details and RFC behavior on request, helping debug specific issues (e.g. socket
handling, message parsing edge cases), and making small, targeted, reviewed
edits (e.g. the `MODE` and `JOIN` command handlers). No large unreviewed blocks
of code were accepted; every change was read, understood, and could be
explained by the authors before being kept.
