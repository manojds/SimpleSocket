# SimpleSocket
SimpleSocket is a set of classes that makes network programming in unix environment easier. 
Creating a server with SimpleSocket is easy as;
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


## Getting Started
### Prerequisites
None. You only need C++11 in unix environment to use SimpleSocket.

### Using SimpleSocket
SimpleSocket is meant to be used as directly placing source file in your project. You can import following files to your project to get started.

- SimpleScoketDefs.h
- SimpleSocketExceptions.h
- SimpleSocket.h
- SimpleSocket.cpp


Project on github contains above files and tests for the SimpleSocket class and make files to build those tests. Poject directory contains following structure.
- include - constains .h include files
- src - contains .cpp source files
- test - contains test files
- nbproject - cotains make files for the project
- MakeFile - main makefile for the project

### Building and Running Tests
To build the tests just type "make" in the project root directory. Executable that contains the tests will be created in a directory called "dist".

## Examples
test directory on the project contains many examples that demonstrate the usage of SimpleSocket class. Following few examples shows that are drawn from the test directory.

### A Simple echo server
```
SimpleSocket server_socket;

try
{
	server_socket.listen("9091"); //listen on the port 9091

	server_socket.accept(client_socket); //accept will return when a client is connected..
	std::cout<<"EchoServer - Client connected !"<<std::endl;

	int rcv_buffer_size = 1024;

	try
	{    
		while ( !shouldStop() ) //we will go until we should stop
		{
			
			//we wait for 500ms till we can read from the client socket.
			SimpleSocketReadyStates ready_state = client_socket.waitTillSocketIsReady(true, false, 500);
			
			if (ready_state == SimpleSocketReadyStates::ReadyForRead )
			{
				char buffer[rcv_buffer_size];
				memset(buffer,'\0',rcv_buffer_size);
				
				int byte_count = client_socket.receiveData(buffer, rcv_buffer_size);
				if (byte_count <= 0) // in case of a recoverable error we just continue to read
					continue;

				int sent_count = 0;
				client_socket.sendAllData(buffer, byte_count,sent_count);
				
			}
		}
		
		client_socket.close();
	}
	catch (SimpleSocketException& ex)
	{
		std::cout<<"EchoServer - exception caught from client. exception:"<<ex.what()<<std::endl;
		client_socket.close();
	}

catch (SimpleSocketException& ex)
{
	std::cout<<"EchoServer - exception caught from the server. exception:"<<ex.what()<<std::endl;
	server_socket.close();
}
```
### Connecting to a server

```
try
{
	SimpleSocket client;
	client.connectToServer("127.0.0.1", "9092");
}
catch(ConnectionFailedException& ex)
{
	std::cout<<"Failed to connect to 127.0.0.1:9092. Reason:"<<	ex.what()<<std::endl;
}
```
## Documentation
Usage of every public method is described in SimpleSocket.cpp file.

## Error Handling
There are two types of errors reported by SimpleSocket. That are;
- Recoverable errors
- Non-recoverable errors

Recoverable errors are errors that are only temporary and we can continue to use the socket even if we encounter a recoverable error. Examples for this kind of errors are a signal is caught while we are trying to send or receving data. In such a case we can try again to send or receive data. Recoverable errors are signalled from SimpleSocket as a failure return value. that is -1 or SimpleSocketErrCodes depending on the return type of the method. If the rerurn type is an int, to get the exact details of the error, user should call SimpleSocket::getLastError() method. It will return an enum that contains the exact error code. Reason for using an indirect mechanism to report the exact error is that methods such as send and receive need to return the eaxct number of bytes that they have sent in the return value. 

Non-recoverable errors are reported via exceptions. Documentation of each method lists that which exceptions are thrown from the method. All Exceptons thrown by SimpleSocket is derived from SimpleSocketException. In case of a Non-recoverable error users can no longer use the socket. So user will have to close the socket if a non-recoverable exception is thrown.
