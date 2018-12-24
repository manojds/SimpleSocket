/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "TestUtils.h"
#include "SimpleSocketExceptions.h"

namespace simple_socket_test
{
using namespace network_utils;

TEST(ErrorConditionTests, DisonnectWhileReceiving) 
{
    EchoServer echo_server;

    echo_server.startThread(TestUtils::server_port);
    
    SimpleSocket client;
    client.connectToServer(TestUtils::server_ip, TestUtils::server_port);
    
    echo_server.disconnectClient();
    
    int buffer_size = 1024;
    char buffer[buffer_size];
    
    ASSERT_THROW(client.receiveData(buffer, buffer_size), ConnClosedByRemoteHostException);
    
    echo_server.stopServer();    
    
}

TEST(ErrorConditionTests, TryingToConnectAlreadyConenctedSocket) 
{    
    EchoServer echo_server;

    echo_server.startThread(TestUtils::server_port);
    
    SimpleSocket client;
    client.connectToServer(TestUtils::server_ip, TestUtils::server_port);   
    
    try
    {
        client.connectToServer(TestUtils::server_ip, TestUtils::server_port);
        FAIL() << "Expected SimpleSocketException exception..but not received.";
    }
    catch (SimpleSocketException& ex)
    {
        EXPECT_EQ(ex.getErrorCode(), SimpleSocketErrCodes::AlreadyConnected);        
    }
    catch(...)
    {
        FAIL() << "Expected SimpleSocketException exception..but received another";
    }
      
    
    echo_server.stopServer();
}

TEST(ErrorConditionTests, TryingToReceiveOnNotConnectedSocket) 
{    
    SimpleSocket client;
    int buffer_size = 1024;
    char buffer[buffer_size];    
    
    try
    {
        client.receiveData(buffer, buffer_size);
        FAIL() << "Expected SimpleSocketException exception..but not received.";
    }
    catch (SimpleSocketException& ex)
    {
        EXPECT_EQ(ex.getErrorCode(), SimpleSocketErrCodes::NotConnected);        
    }
    catch(...)
    {
        FAIL() << "Expected SimpleSocketException exception..but received another";
    }    
}



TEST(ErrorConditionTests, ConnectionFailedTest) 
{    
    SimpleSocket client;
    
    ASSERT_THROW(client.connectToServer(TestUtils::server_ip, TestUtils::server_port), ConnectionFailedException);    
}


}