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

## Using irssi

[irssi](https://irssi.org/) is the reference client used to test this server.

### Connecting

Start the server, then launch irssi and connect from inside it:

```sh
./ircserv 6667 secret
irssi
```

```
/set nick alice
/connect 127.0.0.1 6667 secret
```

Or in one line from the shell: `irssi -c 127.0.0.1 -p 6667 -w secret -n alice`.

To test interactions between users, open a second terminal and connect a
second irssi with another nick (e.g. `bob`).

### Base commands

| irssi command                   | Sent to the server        | Description                               |
|---------------------------------|---------------------------|-------------------------------------------|
| `/nick <newnick>`               | `NICK`                    | Change your nickname                      |
| `/join #chan [key]`             | `JOIN`                    | Join (or create) a channel                |
| `/part [#chan] [reason]`        | `PART`                    | Leave a channel                           |
| *(just type text)*              | `PRIVMSG #chan`           | Talk in the current channel               |
| `/msg <nick> <text>`            | `PRIVMSG nick`            | Private message to a user                 |
| `/query <nick>`                 | —                         | Open a private conversation window        |
| `/topic [new topic]`            | `TOPIC`                   | Show or set the channel topic             |
| `/kick <nick> [reason]`         | `KICK`                    | Kick a user from the channel (op only)    |
| `/invite <nick> [#chan]`        | `INVITE`                  | Invite a user to a channel (op only)      |
| `/mode #chan +i` / `-i`         | `MODE`                    | Set / unset invite-only                   |
| `/mode #chan +t` / `-t`         | `MODE`                    | Restrict topic changes to operators       |
| `/mode #chan +k <key>` / `-k`   | `MODE`                    | Set / remove the channel key              |
| `/mode #chan +o <nick>` / `-o`  | `MODE`                    | Give / take operator privilege            |
| `/mode #chan +l <n>` / `-l`     | `MODE`                    | Set / remove the user limit               |
| `/quote <RAW LINE>`             | the raw line as-is        | Send any raw IRC command (useful to test) |

Example session as the channel creator (automatically operator):

```
/join #test
/topic Welcome to #test
/mode #test +k hunter2
/mode #test +o bob
/kick bob bye
```

## File transfer (DCC)

File transfer uses **DCC** (Direct Client-to-Client). The server only relays the
initial offer; the file itself travels over a **direct TCP connection between
the two clients**, never through `ircserv`.

How it works:

1. The sender's client sends a CTCP message through a normal `PRIVMSG`:
   `PRIVMSG bob :\x01DCC SEND file.txt <ip> <port> <size>\x01`
2. `ircserv` forwards that `PRIVMSG` to `bob` unchanged, like any other message.
3. Bob's client opens a TCP connection to `<ip>:<port>` and downloads the file.

### Sending a file with irssi

In alice's irssi:

```
/dcc send bob /path/to/file.txt
```

In bob's irssi, a notice appears announcing the offer. Accept it with:

```
/dcc get alice
```

The file is saved in irssi's download directory (your home by default). Other
useful commands:

```
/dcc list                    # show pending / active transfers
/dcc close get alice         # refuse or cancel a transfer
/set dcc_download_path ~/Downloads
```

### Troubleshooting

- If both clients run on the same machine and the transfer never starts, force
  the address irssi advertises: `/set dcc_own_ip 127.0.0.1` (on the sender).
- Across machines, the sender's DCC port must be reachable by the receiver
  (no NAT/firewall in between); you can pin a range with
  `/set dcc_port 5000 5010`.

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


