# Network-UDP

Implimentation of UDP to allow file transmission from client to server.

A file may be transferred using the client command to the server, provided the server is running. This program has support for corrupted ack's (and will corrupt them randomly for demonstration purposes. 

client.c should be called as follows: 

./client [port number] [ip address] [input filename] [output filename] 

server.c should be called as follows: 

./server [port number]

