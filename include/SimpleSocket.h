
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
        
        
        SimpleSocketErrCodes connectToServer(const std::string& server_name,const std::string& port);
        SimpleSocketErrCodes listen(const std::string& port, int back_log_size = 10);
        SimpleSocketErrCodes accept(SimpleSocket & _NewConnection);
        SimpleSocketErrCodes sendData(const char * buffer, int byte_count, bool non_blocking = false);
        SimpleSocketErrCodes sendAllData(const char * buffer, int & byte_count);
        
        
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

