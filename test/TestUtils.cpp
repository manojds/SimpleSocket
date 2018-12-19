#include "TestUtils.h"

using namespace simple_socket_test;

std::string TestUtils::server_ip = "127.0.0.1";
std::string TestUtils::server_port = "9093";

void TestUtils::fillBufferWithRandomData(char* buffer, int buffer_size)
{
    //Note:: srand and rand are not thread safe. 
    //here it is not a problem those functions will not be called rfmo multiple threads.
    srand (time(NULL));
    
    for (int i=0; i < buffer_size; ++i)
    {
        buffer[i] = rand() % 256;        
    }    
}

