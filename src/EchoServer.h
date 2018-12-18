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

namespace network_utils
{
    class EchoServer
    {
    public:
        EchoServer();
        ~EchoServer();
        
        
        bool startThread(const std::string& port);
        bool stop();
        bool disconnectClient();  
        void startServer(const std::string& port);
        
    private:
        
        
        std::thread runner_thread;
        
    };    
}


#endif /* ECHOSERVER_H */

