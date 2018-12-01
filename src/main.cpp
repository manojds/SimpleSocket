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

#include "SimpleSocket.h"

using namespace std;
using namespace network_utils;

/*
 * 
 */
int main(int argc, char** argv) {
    
    SimpleSocket sock;
    sock.testLog();
    sock.listen("9095",10);
    sleep(5);
    
    const char* buffer = "Sample Data";
    
    SimpleSocket newSock;
    sock.accept(newSock);
    newSock.sendData(buffer, 5);
    sleep(1000);

    return 0;
}

