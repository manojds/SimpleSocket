/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include <gtest/gtest.h>
#include <EchoServer.h>
#include <SimpleSocket.h>

namespace simple_socket_test
{
using namespace network_utils;

TEST(TestCaseName, TestName) 
{
    EchoServer echo_server;

    echo_server.startServer("9095");
    
}

}

