#include "EchoServer.h"
#include "SimpleSocket.h"
#include "SimpleSocketExceptions.h"
#include <iostream>
#include <string.h>
#include <unistd.h>

using namespace network_utils;

int EchoServer::rcv_buffer_size = 1024;

EchoServer::EchoServer()        
{}

EchoServer::~EchoServer()
{    
    stopServer();
}

int EchoServer::getRcvBufferSize()
{
    return rcv_buffer_size;
}

bool EchoServer::startThread(const std::string& port)
{    
    runner_thread = std::move(std::thread(&EchoServer::startServer, this, port));
    return true;    
}

void EchoServer::startServer(const std::string& port)
{
    std::cout<<"EchoServer - starting "<<std::endl;
    
    SimpleSocket server_socket;
    server_socket.listen(port);
    
    std::cout<<"EchoServer - started on port:"<<port<<std::endl;
    
    while( !shouldStop() )
    {   
        SimpleSocketReadyStates server_sck_ready_state = server_socket.waitTillSocketIsReady(true, false, 500);
        
        if (server_sck_ready_state != SimpleSocketReadyStates::ReadyForRead )
            continue;
                
        SimpleSocket client_socket;
        

        server_socket.accept(client_socket);
        std::cout<<"EchoServer - Client connected !"<<std::endl;

        char buffer[rcv_buffer_size];

        try
        {    
            while ( !shouldStop() ) 
            {
                if (IsDisconenctClientRequested())
                    break;
                
                //we wait for 500ms till we can read
                SimpleSocketReadyStates ready_state = client_socket.waitTillSocketIsReady(true, false, 500);
                
                if (ready_state == SimpleSocketReadyStates::ReadyForRead )
                {                
                    int byte_count = client_socket.receiveData(buffer, rcv_buffer_size);
                    if (byte_count <= 0)
                        continue;

                    int sent_count = 0;
                    client_socket.sendAllData(buffer, byte_count,sent_count);
                    memset(buffer,'\0',rcv_buffer_size);
                }
            }
            
            client_socket.close();
        }
        catch (SimpleSocketException& ex)
        {
            std::cout<<"EchoServer - caught exception:"<<ex.what()<<std::endl;
            client_socket.close();
        }
    }   
    
    server_socket.close();
    
    std::cout<<"EchoServer - stopped.. "<<std::endl;
    
}

void EchoServer::disconnectClient()
{
    mtx.lock();
    disonnect_client_requested = true;
    mtx.unlock();
}

bool EchoServer::IsDisconenctClientRequested()
{
    bool disonnect_client = false;
    mtx.lock();
    disonnect_client = disonnect_client_requested;
    
    disonnect_client_requested= false;
    mtx.unlock();
    
    return disonnect_client;
}

void EchoServer::stopServer()
{
    mtx.lock();
    stop_requested = true;
    mtx.unlock();
    
    if(runner_thread.joinable())
    {
        runner_thread.join();
    }
}

bool EchoServer::shouldStop()
{
    bool should_stop = false;
    mtx.lock();
    should_stop = stop_requested;
    mtx.unlock();
    
    return should_stop;
}