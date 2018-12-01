#include "SimpleSocket.h"
#include "Logger.h"
#include <string.h>
#include <iostream>
#include <stdio.h>
//includes for socket programming
#include <errno.h>
#include <sys/select.h>
#include <netdb.h>

#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <fcntl.h>
//for ubuntu close(fd)
#include <unistd.h>

#include "Logger.h"


using namespace network_utils;

#define HORNET_SOCKET_FAMILY        AF_INET

SimpleSocket::SimpleSocket():
        socket_fd(-1),
        socket_desc("Un-named Socket")
{
    
}

SimpleSocket::~SimpleSocket() 
{

}


/*!
 * Connects to the specified server
 * @param [in] _sRemotePort - host name of the remote host
 * @param [in] _sRemotePort - the port of the remote host
 * @return 
 * <ul>
 * - returns the error code
 * - 0 -On Success 
 * - E_IP_PORT_RESOLVE_ERROR- Unable to resolve the specified host name and port pair
 * - E_RESOURCE_LIMIT - no resources available in the system
 * - E_FAILED_TO_CONNECT- failed to connect to the remote host
 * </ul>
 * 
 */
SimpleSocketErrCodes SimpleSocket::connectToServer(const std::string & server_name,const std::string & port)
{
    const char* log_token="SimpleSocket::Connect";
    
    struct addrinfo aihints, *pai_servinfo, *pai_Itr;

    //formatting aihints
    memset(&aihints,0,sizeof aihints);
    aihints.ai_family=HORNET_SOCKET_FAMILY;
    aihints.ai_socktype=SOCK_STREAM;

    int ret = getaddrinfo(server_name.c_str(),port.c_str(),&aihints,&pai_servinfo);

    if(ret != 0)
    {      
        Logger::logWarnning(log_token,"Error while  resolving the Combination of <RemoteHost ,Port> System reports error<%d:%s>. Socket object <%p>",ret, gai_strerror(ret),this);
        return SimpleSocketErrCodes::IpPortResolveError;
    }

    //check whether pai_Itr is null that means cannot resolve the hostname
    if(pai_servinfo == NULL)
    {
        Logger::logWarnning(log_token, "Bad Combination of Remote Host and Remote Port. Socket object <%p>",this);
        return SimpleSocketErrCodes::IpPortResolveError;
    }

    int local_sock_fd=socket(HORNET_SOCKET_FAMILY,SOCK_STREAM,0);

    if(local_sock_fd==-1)
    {
        Logger::logWarnning(log_token,"Error while creating the socket system reports error<%d:%s>. Socket object <%p>",errno, getErrnoAsString().c_str(),this);
        freeaddrinfo(pai_servinfo);
        return SimpleSocketErrCodes::ResourceLimitReached;
    }

    for(pai_Itr=pai_servinfo;pai_Itr!=NULL;pai_Itr=pai_Itr->ai_next)
    {
        if(connect(local_sock_fd,pai_Itr->ai_addr,pai_Itr->ai_addrlen)==-1)
        {
            continue;
        }
        break;
    }
    if(pai_Itr==NULL)
    {//failed to connect
        Logger::logError(log_token,"Failed To connect to <%s> Port<%s> System reports Error<%d:%s>",server_name.c_str(),port.c_str(),errno, getErrnoAsString().c_str());
        close(local_sock_fd);
        freeaddrinfo(pai_servinfo);
        return SimpleSocketErrCodes::FailedToConnect;
    }

    socket_fd = local_sock_fd;
    //getting the remote host name
    char remote_host_name[INET6_ADDRSTRLEN];
    inet_ntop(pai_Itr->ai_family,get_in_addr((struct sockaddr *)pai_Itr->ai_addr),remote_host_name, sizeof remote_host_name);
    //we need to free the addrinfo structure linked list
    freeaddrinfo(pai_servinfo); 
    
    //we are here means, we are connected.
    Logger::logInfo(log_token,"Successfully connected to Remote Host<%s> and Port <%s>. Socket object <%p>. Socket FD <%d>",remote_host_name, port.c_str(),this, socket_fd);
    
    //TODO:: loginfo the local port and IP too
    
    return SimpleSocketErrCodes::Success;     
}

/*!
 *  Start to Listen for incoming connection on a specified port
 * @param [in] _sPort - the port to be listen for incoming conenction
 * @return 
 * <ul>
 * - returns the error code
 * - 0 -On Success 
 * - E_IP_PORT_RESOLVE_ERROR- the specified port is invalid
 * - E_PORT_ALREADY_INUSE - specified port is already in use
 * </ul>
 * 
 */
SimpleSocketErrCodes SimpleSocket::listen(const std::string& port, int back_log_size)
{
    const char * log_token="SimpleSocket::Listen";
    int port_reuse=1;                                  //for port reuse
    struct addrinfo hints, *ai, *pItr;
    

    //initializing hints structure which we pass to getaddrinfo
    memset(&hints,0 ,sizeof hints);
    hints.ai_family=HORNET_SOCKET_FAMILY;
    hints.ai_socktype=SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // fill in my IP for me    

    int ret = -1;
    ret = getaddrinfo(NULL,port.c_str(),&hints,&ai);
    if(ret!=0)
    {        
        Logger::logWarnning(log_token, "Failed to resolve interface address and the port combination. System reports error <%d:%s>. Socket Object <%p>",ret,gai_strerror(ret),this);
        return SimpleSocketErrCodes::IpPortResolveError;
    }
    if(ai == NULL)
    {        
        Logger::logWarnning(log_token, "Bad combination of interface address and the port. Socket Object <%p>",this);
        return SimpleSocketErrCodes::IpPortResolveError;
    }
    
    for(pItr=ai; pItr!=NULL; pItr=pItr->ai_next)
    {
        socket_fd = socket(pItr->ai_family,pItr->ai_socktype,pItr->ai_protocol);
        if(socket_fd<0)
        {  
            Logger::logWarnning(log_token, "Socket system call failed. System reports error<%d:%s>. Socket Object<%p>",errno,getErrnoAsString().c_str(), this);
            continue;
        }
        //Make the port reusable
        setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &port_reuse, sizeof(int));

        if (bind(socket_fd, pItr->ai_addr, pItr->ai_addrlen) < 0)
        {
            Logger::logWarnning(log_token, "Bind system call failed. System reports error<%d:%s>.Socket Object<%p>",errno, getErrnoAsString().c_str(), this);
            close(socket_fd);
            continue;
        }
        //we are here means we could bind. So let's go out of the loop
        break;
    }

    if(pItr==NULL)
    {
        Logger::logError(log_token, "Failed to bind  on port [%s]. System reports error <%d:%s>.Socket Object<%p>",port.c_str(),errno, getErrnoAsString().c_str(),this);
        freeaddrinfo(ai);
        return SimpleSocketErrCodes::PortAlreadyInUse;
    }
    //we should free the address info structure now.
    freeaddrinfo(ai);

    if(::listen(socket_fd, back_log_size) == -1)                
    {
        Logger::logError(log_token, "Failed to Listen on port [%s]. System reports error <%d:%s>. Socket Object<%p>",port.c_str(),errno, getErrnoAsString().c_str(),this);
        close(socket_fd);
        return SimpleSocketErrCodes::PortAlreadyInUse;
    }   
    return SimpleSocketErrCodes::Success;
}


/*!
 *  Accepts a socket connection. if there is no connections the call would get block, in socket is blocking mode.
 * In non blocking mode this call would not block, if there are no connections.
 * @param [in,out] _NewConnection - New Connection will be stored in this argument if successfully accepted the connection.
 * @return 
 * <ul>
 * - returns the error code
 * - 0 -On Success 
 * - E_WOULD_BLOCK- operation is blocking
 * - E_ACCEPT_FAILED -accept call failed due to system error
 * </ul>
 * 
 */
SimpleSocketErrCodes SimpleSocket::accept(SimpleSocket & new_conn)
{
    const char * log_token="SimpleSocket::accept";
    char remoteIP[INET6_ADDRSTRLEN];
    struct sockaddr_storage remoteaddr; // client address

    socklen_t addrlen;
    int client_fd;
    addrlen=sizeof remoteaddr;

    client_fd = ::accept(socket_fd,(struct sockaddr *)&remoteaddr, &addrlen);

    if(client_fd<0)
    {//error
        if((errno==EAGAIN)||(errno==EWOULDBLOCK)||(errno==ECONNABORTED))
        {
            //this means that after select return and before we call accept the 
            //socket has been closed due to network error. So the client is not available to us.
            //there is nothing to do here.
            return SimpleSocketErrCodes::WouldBlock;
        }
        else
        {
            Logger::logWarnning(log_token, "Error while Accepting a new Client. System reports error <%d:%s>.Socket object <%p>",errno,getErrnoAsString().c_str(),this);
            return SimpleSocketErrCodes::FailedToAcceptConn;
        }
    }
    else
    {
        //We got a new Client        
        Logger::logInfo(log_token, "Accepted a new Client Connection from [%s]. Socket FD <%d>",inet_ntop(remoteaddr.ss_family,get_in_addr((struct sockaddr*)&remoteaddr),remoteIP, INET6_ADDRSTRLEN), socket_fd);
        new_conn.socket_fd = client_fd;
        return SimpleSocketErrCodes::Success;
    }    
}

/*!
 *  Sends data in the specified buffer. There is no guarantee that this function will send all the data.
 * if the socket is non blocking mode or, if _bNonBlocking is set to true, this call would not block, if the send buffer is full in the system.
 * otherwise this call would get blocked if the send buffer is full on the system
 * @param [in] _pBuffer - pointer to the buffer which holds the data to send
 * @param [in] _iByteCount - number of bytes to be sent
 * @param [in] _bNonBlocking - if set to to true , this function will send the data in non blocking mode, for this operation. default value for this argument is false.
 * @return 
 * <ul>
 * - returns the error code, on error or number of bytes actually sent on success
 * - positive value -number of bytes sent in this call
 * - E_WOULD_BLOCK- operation is blocking
 * - E_RETRY - operation is interrupted by the system, user should retry 
 * - E_SOCKET_ERROR - send failed due to socket error. user should close the Socket in case of this error
 * </ul>
 * 
 */
SimpleSocketErrCodes SimpleSocket::sendData(const char * buffer, int byte_count, bool non_blocking /*= false*/)
{
    const char * log_token="SimpleSocket::sendData";
    int ret_send(-1);
    SimpleSocketErrCodes ret_val(SimpleSocketErrCodes::GenericError);
    int send_flags = MSG_NOSIGNAL;
    
    if(non_blocking)
    {
        send_flags = send_flags | MSG_DONTWAIT;
    }
    
    ret_send=send(socket_fd,buffer,byte_count,send_flags);

    if(ret_send<0)
    {
        if((errno==EAGAIN)||(errno==EWOULDBLOCK))
        {
            Logger::logDebug(log_token, "Sending is blocking. send returned <%d:%s>. Socket Object <%p>",errno,getErrnoAsString().c_str(),this);
            ret_val= SimpleSocketErrCodes::WouldBlock;
        }
        else if(errno == EINTR)
        {//signal is caught before send any data. retry..
            Logger::logDebug(log_token, "Signal is caught while sending data. retrying. Socket Object t <%p>",this);
            ret_val=SimpleSocketErrCodes::Retry;
        }
        else
        {
            //error while sending
            Logger::logWarnning(log_token, "Error while writing to the network socket. System reports error <%d:%s>. Socket Object <%p>>", errno,getErrnoAsString().c_str(),this);
            ret_val=SimpleSocketErrCodes::SocketError;
        }
    }
    else if(ret_send==0)
    {
        Logger::logDebug(log_token, "Zero number of bytes have been specified to send. Socket <%p>", this);
    }
    //else some bytes have been sent

    return ret_val;
}


/*!
 *  Sends data in the buffer. This function will try to send the all the data in the buffer, 
 * it would not return until either all the data has been sent or error has occurred on the socket.
 * if the socket is non blocking mode and the send buffer is full, the thread call 
 * this function will stuck in a busy loop, till the the send buffer get available. 
 * otherwise this call would get blocked if the send buffer is full on the system
 * So it is not wise to use this function if socket has been set to non blocking mode.
 * @param [in] _pBuffer - pointer to the buffer which holds the data to send
 * @param [in,out] _iByteCount - This argument is a value result argument. 
 * this argument is used to specify the number of bytes to be sent, 
 * when this function returns it holds number of bytes actually sent, if no error occurred this will hold same value as the input value
 * @return 
 * <ul>
 * - returns the error code
 * - 0 -on success
 * - E_SOCKET_ERROR - receive failed due to socket error. user should close the Socket in case of this error
 * </ul>
 * 
 */
SimpleSocketErrCodes SimpleSocket::sendAllData(const char * buffer, int & byte_count)
{

    int iSentCount(0);                  //number bytes sent so far
    int iRes(E_SOCKET_ERROR);                        //response from the 
    //send all. stop if error occurred or all the data have been transmitted.
    while(iSentCount<byte_count)
    {
        
        iRes=SendData(buffer+iSentCount,byte_count-iSentCount);
        if(iRes>=0)
        {
            iSentCount=iSentCount+iRes;
        }  
        else if( (iRes == E_WOULD_BLOCK ) ||iRes==E_RETRY)
        {//if operation is blocking, or the return code is retry we should retry
            continue;
        }
        else
        {
            break;
        }
    }
    if(byte_count==iSentCount)
    {//if we sent all the data set the return code to success
        iRes=0;
    }

    //update the sent byte count in the argument
    byte_count=iSentCount;
    return iRes;
}


void SimpleSocket::testLog()
{
    Logger::logDebug("SomeKey", "Manoj De Silva");
}

/*!
 * Returns the error description of the error code specified in the argument.
 */
//string SimpleSocket::GetErrorDescription(SimpleSocketErrCodes error_code)
//{
//    switch(error_code)
//    {
//        case IpPortResolveError:
//            return "IpPortResolveError";
//            break;
//            
//        case ResourceLimitReached:
//            return "ResourceLimitReached";
//            break;
//            
//        case FailedToConnect:
//            return "FailedToConnect";
//            break;
//            
//        case PortAlreadyInUse:
//            return "PortAlreadyInUse";
//            break;
//            
//        case WouldBlock:
//            return "WouldBlock";
//            break;
//            
//        case FailedToAcceptConn:
//            return "FailedToAcceptConn";
//            break;
//            
//        case Retry:
//            return "Retry";
//            break;
//            
//        case SocketError:
//            return "SocketError";
//            break;
//            
//        case ReceiveFailed:
//            return "ReceiveFailed";
//            break;
//            
//        case NotConnected:
//            return "NotConnected";
//            break;
//            
//        case GenericError:
//            return "GenericError";
//            break;
//            
//        case NotListening:
//            return "NotListening";
//            break;
//            
//        default:            
//            char pError[64];
//            snprintf(pError,64,"%s:%d","UNKNOWN_ERROR_CODE", error_code);
//            return pError;
//            break;
//    }
//    
//}

std::string SimpleSocket::getErrnoAsString()
{
    char buffer[128];
    strerror_r(errno, buffer, 128);
    
    return buffer;    
}

void* SimpleSocket::get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in*) sa)->sin_addr);
    }
    else
    {
        return &(((struct sockaddr_in6*) sa)->sin6_addr);
    }
}



