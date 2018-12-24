/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   EchoServer.h
 * Author: Manoj De Silva
 *
 * Created on December 18, 2018, 7:14 AM
 */

#ifndef ECHOSERVER_H
#define ECHOSERVER_H
#include <string>
#include <thread>
#include <mutex>

namespace network_utils
{
    class EchoServer
    {
    public:
        EchoServer();
        ~EchoServer();
        
        
        bool startThread(const std::string& port);
        void disconnectClient();  
        void startServer(const std::string& port);
        void stopServer();
        
        static int getRcvBufferSize();
        
    private:
        
        bool shouldStop();
        bool IsDisconenctClientRequested();

        std::thread runner_thread;
        
        std::mutex  mtx;
        bool        stop_requested = false;   
        bool        disonnect_client_requested = false; 
        static int  rcv_buffer_size;
        
    };    
}


#endif /* ECHOSERVER_H */

