# netchat
Client-Server TCP stream plaintext chat

netchat_server takes one argument, port number
netchat_client takes two arguments, host address and port number
host address can be an IPv4 or an IPv6 address

If the address is not valid program returns 1
The server opens an IPv6 socket but with socket option it accepts both IPv4 and IPv6 connections

Both the programs use a set of send and recv message functions run on two threads
Both the programs disconnect with a "bye..." message sent or received
