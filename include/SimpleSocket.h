
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
        
        void setSocketOption(SimpleSocketOptions option);
        SimpleSocketReadyStates waitTillSocketIsReady(bool wait_for_read, bool wait_for_write, long wait_time_ms);
        
        std::string getConnDescription();
        
        SimpleSocketErrCodes    getLastError();
        std::string             getLastErrorString();
        
        
    private:   
        int         getCurrentFDStutusFlags();
        void        setFDStutusFlags(int flags, const std::string& flag_name);
        void *      get_in_addr(struct sockaddr *sa);  
        void        setLastError(SimpleSocketErrCodes error, const std::string& error_str);
        void        clearLastError();
        std::string getErrnoAsString();

        int         socket_fd;
        std::string socket_desc;        
        
        thread_local    SimpleSocketErrCodes    last_error;
        thread_local    std::string             last_error_str;
        
    };
}

#endif /* SIMPLESOCKET_H */

