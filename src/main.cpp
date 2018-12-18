/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.cpp
 * Author: manojd
 *
 * Created on November 25, 2018, 6:01 AM
 */

#include <cstdlib>
#include <unistd.h>
#include <gtest/gtest.h>

#include "SimpleSocket.h"

using namespace std;
using namespace network_utils;

/*
 * 
 */
int main(int argc, char** argv) 
{
    
    ::testing::InitGoogleTest(&argc, argv);

    

    return RUN_ALL_TESTS();
}

