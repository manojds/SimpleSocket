/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include "TestUtils.h"

namespace simple_socket_test
{
using namespace network_utils;



void pingPongTestForDataSize(int send_block_size)
{
    EchoServer echo_server;

    echo_server.startThread(TestUtils::server_port);
    
    SimpleSocket client;
    client.connectToServer(TestUtils::server_ip, TestUtils::server_port);

    char send_buffer[send_block_size];
    TestUtils::fillBufferWithRandomData(send_buffer, send_block_size);
    
    int sent_count;
    client.sendAllData(send_buffer, send_block_size, sent_count );
    
    char read_buffer[send_block_size];
    memset(read_buffer, '\0', send_block_size);
    
    int bytes_to_be_received = send_block_size;
    int ret_val = client.receiveAllData(read_buffer, send_block_size, bytes_to_be_received);
    
    if (ret_val > 0)
    {
        ASSERT_EQ(memcmp(send_buffer, read_buffer, send_block_size), 0);
    }    
    
    client.close();  
    
    echo_server.stopServer(); 
}

TEST(PingPongTests, SmallBuffer) 
{    
    pingPongTestForDataSize(100);       
    
}

TEST(PingPongTests, BufferLenghtIsOneByteLesserThanTheServerBuffer) 
{
    pingPongTestForDataSize(EchoServer::getRcvBufferSize() -1 );       
}

TEST(PingPongTests, BufferLenghtSimilarToTheServerBuffer) 
{
    pingPongTestForDataSize(EchoServer::getRcvBufferSize()); 
}

TEST(PingPongTests, BufferLenghtIsOneByteLagerThanTheServerBuffer) 
{
     pingPongTestForDataSize(EchoServer::getRcvBufferSize() + 1 ); 
    
}

TEST(PingPongTests, LargeBuffer) 
{
    pingPongTestForDataSize(EchoServer::getRcvBufferSize()*100 );     
}

}

