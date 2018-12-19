#include "EchoServer.h"
#include "SimpleSocket.h"
#include "SimpleSocketExceptions.h"
#include <iostream>
#include <string.h>

using namespace network_utils;

EchoServer::EchoServer()
{}

EchoServer::~EchoServer()
{}

bool EchoServer::startThread(const std::string& port)
{
    
    runner_thread = std::thread(&EchoServer::startServer, this, port);
    return true;    
}

void EchoServer::startServer(const std::string& port)
{
    std::cout<<"EchoServer - starting "<<std::endl;
    
    SimpleSocket server_socket;
    server_socket.listen(port);
    
    std::cout<<"EchoServer - started on port:"<<port<<std::endl;
    
    while(true)
    {
    
        SimpleSocket client_socket;

        server_socket.accept(client_socket);
        std::cout<<"EchoServer - Client connected !"<<std::endl;

        int buffer_size = 1024;
        char buffer[buffer_size];

        try
        {    
            while (true) 
            {
                int byte_count = client_socket.receiveData(buffer, buffer_size);
                if (byte_count <= 0)
                    continue;
                
                int sent_count = 0;
                client_socket.sendAllData(buffer, byte_count,sent_count);
                memset(buffer,'\0',buffer_size);
            }
        }
        catch (SimpleSocketException& ex)
        {
            std::cout<<"EchoServer - caught exception:"<<ex.what()<<std::endl;
            client_socket.close();
        }
    }    
}