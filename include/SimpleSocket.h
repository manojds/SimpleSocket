
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
#include <thread>
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
        bool accept(SimpleSocket & _NewConnection);
        int  sendData(const char * buffer, int byte_count, bool non_blocking = false);
        void sendAllData(const char * buffer, int byte_count, int& sent_count);
        int receiveData(char * buffer, int buffer_size, bool non_blocking = false); 
        void receiveAllData(char * buffer, int buffer_size, int& byte_count_received);
        SimpleSocketErrCodes close();
        
        void setSocketOption(SimpleSocketOptions option);
        SimpleSocketReadyStates waitTillSocketIsReady(bool wait_for_read, bool wait_for_write, long wait_time_ms);
        
        
        SimpleSocketErrCodes    getLastError();
        std::string             getLastErrorString();
        
        
    private:   
        int         getCurrentFDStutusFlags();
        void        setFDStutusFlags(int flags, const std::string& flag_name);
        void *      get_in_addr(struct sockaddr *sa);  
        void        setLastError(SimpleSocketErrCodes error, const std::string& error_str);
        void        clearLastError();
        void        throwIfNotInConnectedState();
        void        throwIfInConnectedState();
        void        throwIfNotInListeningState();
        void        throwIfInNotConnectedState();
        std::string getErrnoAsString();

        int                 socket_fd;
        SimpleSocketState   state;
        
    };
}

#endif /* SIMPLESOCKET_H */

