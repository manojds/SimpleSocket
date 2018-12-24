/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   TestUtils.h
 * Author: manojd
 *
 * Created on December 19, 2018, 7:55 AM
 */

#ifndef TESTUTILS_H
#define TESTUTILS_H
#include <iostream>
#include <gtest/gtest.h>
#include "EchoServer.h"
#include <SimpleSocket.h>

namespace simple_socket_test
{
using namespace network_utils;


class TestUtils
{
public:
    static void fillBufferWithRandomData(char* buffer, int buffer_size);
    
    static  std::string server_ip;
    static  std::string server_port;
    
};





}

#endif /* TESTUTILS_H */

