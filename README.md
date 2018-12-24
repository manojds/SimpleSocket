# SimpleSocket
SimpleSocket is a set of classes that makes network programming in unix environment easier. Creating a server with SimpleSocket is easy as;
```
SimpleSocket server_socket;
server_socket.listen("9092"); //9092 is the listening port

SimpleSocket client_socket;
server_socket.accept(client_socket); //newly connected client will be represented by client_socket
```

Connecting to a server is easy as;

```
SimpleSocket client;
client.connectToServer("127.0.0.1", "9092");
```

