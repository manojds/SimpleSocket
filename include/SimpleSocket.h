
/* 
 * File:   SimpleSocket.h
 * Author: Manoj  De Silva
 *
 * Created on November 25, 2018, 6:07 AM
 */

#ifndef SIMPLESOCKET_H
#define SIMPLESOCKET_H
#include <string>
#include <sys/socket.h>
#include "SimpleScoketDefs.h"

namespace network_utils
{
    class SimpleSocket
    {
    public:
        SimpleSocket();
        ~SimpleSocket();
        
        
        void connectToServer(const std::string& server_name,const std::string& port);
        void listen(const std::string& port, int back_log_size = 10);
        void accept(SimpleSocket & _NewConnection);
        int  sendData(const char * buffer, int byte_count, bool non_blocking = false);
        void sendAllData(const char * buffer, int byte_count, int& sent_count);
        int receiveData(char * buffer, int buffer_size, bool non_blocking = false); 
        int receiveAllData(char * buffer, int & byte_count);
        SimpleSocketErrCodes close();
        
        bool setSocketOption(SimpleSocketOptions option);
        SimpleSocketReadyStates waitTillSocketIsReady(bool wait_for_read, bool wait_for_write, long wait_time_ms);
        
        std::string getConnDescription();
        
        
//        string GetErrorDescription(SimpleSocketErrCodes error_code);
        
        std::string getErrnoAsString();

        void testLog();
        
    private:      
        void *      get_in_addr(struct sockaddr *sa);    

        int         socket_fd;
        //TODO::set the description at necessary places.
        std::string socket_desc;        
        
    };
}

#endif /* SIMPLESOCKET_H */

