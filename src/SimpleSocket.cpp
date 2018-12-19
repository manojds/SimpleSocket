#include "SimpleSocket.h"
#include "Logger.h"
#include <sstream> 
#include <cassert>
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
#include "SimpleSocketExceptions.h"

namespace network_utils
{
    thread_local    SimpleSocketErrCodes    last_error      = SimpleSocketErrCodes::NoError;
    thread_local    std::string             last_error_str  = "No Error";
}

using namespace network_utils;

#define SOCKET_FAMILY        AF_INET

SimpleSocket::SimpleSocket():
        socket_fd(-1),
        state(SimpleSocketState::NotConnected),
        socket_desc("Un-named Socket")
{
    
}

SimpleSocket::~SimpleSocket() 
{

}

void SimpleSocket::throwIfInNotConnectedState()
{
    if (state == SimpleSocketState::NotConnected)
    {
        throw SimpleSocketException(SimpleSocketErrCodes::NotConnected, "Socket is in not connected state!");
    }    
}

void SimpleSocket::throwIfNotInConnectedState()
{
    if (state != SimpleSocketState::Connected)
    {
        throw SimpleSocketException(SimpleSocketErrCodes::NotConnected, "Socket is not connected!");
    }
}

void SimpleSocket::throwIfInConnectedState()
{
    if (state == SimpleSocketState::Connected)
    {
        throw SimpleSocketException(SimpleSocketErrCodes::NotConnected, "Socket is already connected!");
    }
}

void SimpleSocket::throwIfNotInListeningState()
{
    if (state != SimpleSocketState::Listening)
    {
        throw SimpleSocketException(SimpleSocketErrCodes::NotListening, "Socket is not in listening state!");
    }
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
void SimpleSocket::connectToServer(const std::string & server_name, const std::string & port)
{
    clearLastError();
    throwIfInConnectedState();
    
    struct addrinfo aihints, *pai_servinfo, *pai_Itr;

    //formatting aihints
    memset(&aihints, 0, sizeof aihints);
    aihints.ai_family = SOCKET_FAMILY;
    aihints.ai_socktype = SOCK_STREAM;

    int ret = getaddrinfo(server_name.c_str(), port.c_str(), &aihints, &pai_servinfo);

    if(ret != 0)
    {
        std::stringstream strm;
        strm<<"Failed to Resolve IP Port combination of ["<<server_name <<":"<< port <<"]. System Error["<< ret <<":"<< gai_strerror(ret) <<"]";
        throw ConnectionFailedException( SimpleSocketErrCodes::IpPortResolveError, strm.str().c_str() );
    }

    //check whether pai_Itr is null that means cannot resolve the hostname
    if(pai_servinfo == NULL)
    {               
        std::stringstream strm;
        strm<<"Bad Combination of Remote Host and Remote Port ["<< server_name <<":"<< port <<"]";
        throw ConnectionFailedException( SimpleSocketErrCodes::IpPortResolveError, strm.str().c_str() );        
    }

    int local_sock_fd = socket(SOCKET_FAMILY,SOCK_STREAM,0);

    if(local_sock_fd == -1)
    {        
        freeaddrinfo(pai_servinfo);
        
        std::stringstream strm;
        strm<<"Error while creating the socket . System Error["<< errno <<":"<< getErrnoAsString() <<"]";
        throw ConnectionFailedException( SimpleSocketErrCodes::ResourceLimitReached, strm.str().c_str() );         
    }

    for(pai_Itr = pai_servinfo; pai_Itr != NULL; pai_Itr = pai_Itr->ai_next)
    {
        if(connect(local_sock_fd, pai_Itr->ai_addr, pai_Itr->ai_addrlen) == -1)
        {
            continue;
        }
        break;
    }
    if(pai_Itr == NULL)
    {//failed to connect        
        ::close(local_sock_fd);
        freeaddrinfo(pai_servinfo);
        
        std::stringstream strm;
        strm<<"Failed to Connect to ["<< server_name <<":"<< port <<"]. System Error["<< errno <<":"<< getErrnoAsString() <<"]";
        throw ConnectionFailedException( SimpleSocketErrCodes::FailedToConnect, strm.str().c_str() );         
    }

    socket_fd = local_sock_fd;
    state = SimpleSocketState::Connected;
    //getting the remote host name
    char remote_host_name[INET6_ADDRSTRLEN];
    inet_ntop(pai_Itr->ai_family,get_in_addr((struct sockaddr *)pai_Itr->ai_addr),remote_host_name, sizeof remote_host_name);
    //we need to free the addrinfo structure linked list
    freeaddrinfo(pai_servinfo);
   
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
void SimpleSocket::listen(const std::string& port, int back_log_size)
{
    clearLastError();    
    
    int port_reuse=1;                                  //for port reuse
    struct addrinfo hints, *ai, *pItr;    

    //initializing hints structure which we pass to getaddrinfo
    memset(&hints,0 ,sizeof hints);
    hints.ai_family=SOCKET_FAMILY;
    hints.ai_socktype=SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // fill in my IP for me    

    int ret = -1;
    ret = getaddrinfo(NULL,port.c_str(),&hints,&ai);
    if(ret!=0)
    {        
        std::stringstream strm;
        strm<<"Failed to resolve interface address and the port combination. Port ["<<port<<"]. System Error["<<ret<<":"<< gai_strerror(ret) <<"]";
        throw SimpleSocketException( SimpleSocketErrCodes::IpPortResolveError, strm.str().c_str() );        
    }
    if(ai == NULL)
    {        
        std::stringstream strm;
        strm<<"Bad combination of interface address and the port. Port ["<<port<<"]";
        throw SimpleSocketException( SimpleSocketErrCodes::IpPortResolveError, strm.str().c_str() );          
    }
    
    for(pItr=ai; pItr!=NULL; pItr=pItr->ai_next)
    {
        socket_fd = socket(pItr->ai_family,pItr->ai_socktype,pItr->ai_protocol);
        if(socket_fd<0)
        { 
            continue;
        }
        //Make the port reusable
        setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &port_reuse, sizeof(int));

        if (bind(socket_fd, pItr->ai_addr, pItr->ai_addrlen) < 0)
        {
            ::close(socket_fd);
            continue;
        }
        //we are here means we could bind. So let's go out of the loop
        break;
    }

    if(pItr==NULL)
    {
        freeaddrinfo(ai);
        
        std::stringstream strm;
        strm<<"Failed to bind  on port  ["<<port<<"]. System Error["<<errno<<":"<< getErrnoAsString() <<"]";
        throw SimpleSocketException( SimpleSocketErrCodes::PortAlreadyInUse, strm.str().c_str() );         
    }
    //we should free the address info structure now.
    freeaddrinfo(ai);

    if(::listen(socket_fd, back_log_size) == -1)                
    {
        ::close(socket_fd);        
        
        std::stringstream strm;
        strm<<"Failed to listen  on port  ["<<port<<"]. System Error["<<errno<<":"<< getErrnoAsString() <<"]";
        throw SimpleSocketException( SimpleSocketErrCodes::PortAlreadyInUse, strm.str().c_str() );          
    } 
    //now server socket is connected.
    state = SimpleSocketState::Listening;
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
bool SimpleSocket::accept(SimpleSocket & new_conn)
{
    clearLastError();
    
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
            setLastError(SimpleSocketErrCodes::WouldBlock, "Accept operation would block" );
            return false;
        }
        else
        {            
            std::stringstream strm;
            strm<<"Failed to accept the client. System Error["<<errno<<":"<< getErrnoAsString() <<"]";
            throw SimpleSocketException( SimpleSocketErrCodes::FailedToAcceptConn, strm.str().c_str() );             
        }
    }
    else
    {
        new_conn.socket_fd = client_fd;
        new_conn.state = SimpleSocketState::Connected;
    }  
    
    return true;
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
int SimpleSocket::sendData(const char * buffer, int byte_count, bool non_blocking /*= false*/)
{
    clearLastError();
    throwIfNotInConnectedState();
    
    int bytes_sent(-1);
    int send_flags = MSG_NOSIGNAL;
    
    if(non_blocking)
    {
        send_flags = send_flags | MSG_DONTWAIT;
    }
    
    bytes_sent = send(socket_fd,buffer,byte_count,send_flags);

    if(bytes_sent<0)
    {
        if((errno==EAGAIN)||(errno==EWOULDBLOCK))
        {
            //send operation is blocking. This happens only in non-blocking mode            
            std::stringstream strm;
            strm<<"Sending is blocking. System Error["<<errno<<":"<< getErrnoAsString() <<"]";
            setLastError( SimpleSocketErrCodes::WouldBlock, strm.str()); 
            return -1;
        
        }
        else if(errno == EINTR)
        {//signal is caught before send any data. retry..
            
            setLastError( SimpleSocketErrCodes::Interrupted, "Signal is caught while sending data" ); 
            return -1;
        }
        else
        {
            //error while sending
            
            std::stringstream strm;
            strm<<"Error while writing to the network socket. System Error["<<errno<<":"<< getErrnoAsString() <<"]";
            throw SimpleSocketException( SimpleSocketErrCodes::SocketError, strm.str().c_str() );             
        }
    }
    else if(bytes_sent==0)
    {
        //Zero number of bytes have been specified to send. 
    }
    //else some bytes have been sent

    return bytes_sent;
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
void SimpleSocket::sendAllData(const char * buffer, int byte_count, int& sent_count)
{
    sent_count = 0;
    
    clearLastError();
    throwIfNotInConnectedState();    
                      //number bytes sent so far
    int res_val(-1);                        //response from the send
    //stop if error occurred or all the data have been transmitted.
    while(sent_count < byte_count)
    {
        try
        {
            res_val = sendData(buffer+sent_count, byte_count-sent_count);
            if(res_val >= 0)
            {
                sent_count = sent_count+res_val;
            } 
            else
            {
                assert(false);
            }
        
        }
        catch (SimpleSocketException& ex)
        {
            if( (ex.getErrorCode() == SimpleSocketErrCodes::WouldBlock ) ||
                    ex.getErrorCode() == SimpleSocketErrCodes::Interrupted )
            {//if operation is blocking, or the return code is retry we should retry
                continue;
            }
            else
            {
                throw;
            }            
        }
    }
}

/*!
 * This function can be used to receive the data from the socket. if there is no data to receive, 
 * and socket is non blocking mode this function would return immediately with error code E_WOULD_BLOCK.
 * if there is no data to receive and socket is on blocking mode this call would get blocked until the data arrives from the socket
 * or other end closes socket or error occurrs in the socket.
 * @param [in] _pBuffer - pointer to the buffer where data to be stored
 * @param [in] _iBufferSize - size of the specified buffer or , maximum number of bytes this function can receive.
 * @param [in] _bNonBlocking - if set to to true , this function will send the data in non blocking mode, for this operation. default value for this argument is false.
 * @return 
 * <ul>
 * - returns the error code
 * - 0 - Connection is closed by the other party
 * - Positive number - number of bytes actually received
 * - E_WOULD_BLOCK- operation is blocking
 * - E_RETRY - operation is interrupted by the system, user should retry 
 * - E_SOCKET_ERROR - send failed due to socket error. user should close the Socket in case of this error
 * </ul>
 * 
 */
int SimpleSocket::receiveData(char * buffer, int buffer_size, bool non_blocking /* = false*/)
{    
    clearLastError();
    throwIfNotInConnectedState();
    
    int rcvd_byte_count(-1);          //Number of bytes we could receive    
    int iFlags(0);              //Flags for recv function
    
    if(non_blocking)
    {//if need to receive in non-blocking mode set flags accordingly
        iFlags=MSG_DONTWAIT;
    }

    rcvd_byte_count = recv(socket_fd, buffer, buffer_size, iFlags);

    if(rcvd_byte_count < 0)
    {
        if((errno == EAGAIN)|| (errno == EWOULDBLOCK))
        {
            //recv operation is blocking. This happens only in non-blocking mode
            std::stringstream strm;
            strm<<"Read operation is blocking. System Error["<< errno <<":"<< getErrnoAsString() <<"]";  
            setLastError(SimpleSocketErrCodes::WouldBlock, strm.str()); 
            return -1;
        }
        else if(errno == EINTR)
        {//signal was caught before receive any data, 
            //just return with  return code "Retry" so that caller will try again.
            std::stringstream strm;
            strm<<"Read operation is interrupted by a signal. System Error["<< errno <<":"<< getErrnoAsString() <<"]";  
            setLastError(SimpleSocketErrCodes::Interrupted, strm.str());
            return -1;
        }
        else
        {//Something bad with socket
            std::stringstream strm;
            strm<<"Error while receiving data. System Error["<< errno <<":"<< getErrnoAsString() <<"]";
            throw SimpleSocketException( SimpleSocketErrCodes::SocketError, strm.str().c_str() );             
        }		
    		
    }
    else if (rcvd_byte_count == 0)
    {
        throw ConnClosedByRemoteHostException( SimpleSocketErrCodes::ConnectionClosedByOtherParty, "Connection Closed by remote host" );         
    }

    return rcvd_byte_count;      
}

/*!
 * Receive the data from the socket. This function will try to receive exact number of bytes which is specified in the _iByteCount argument, 
 * it would not return until either _iByteCount of data has been received or error has occurred on the socket.
 * if the socket is non blocking mode and there i no data to receive, the thread call 
 * this function will stuck in a busy loop, till _iByteCount amount of bytes received. 
 * otherwise this call would get blocked if the send buffer is full on the system
 * So it is not wise to use this function if socket has been set to non blocking mode.
 * @param [in] _pBuffer - pointer to the buffer which holds the data to send
 * @param [in,out] _iByteCount - This argument is a value result argument. 
 * this argument is used to specify the number of bytes to be received, 
 * when this function returns it holds number of bytes actually received, if no error occurred this will hold same value as the input value
 * @return 
 * <ul>
 * - returns the error code
 * - 1 - on success
 * - 0 - Connection closed by other party
 * - E_SOCKET_ERROR - receive failed due to socket error. user should close the Socket in case of this error
 * </ul>
 * 
 */
int SimpleSocket::receiveAllData(char * buffer, int buffer_size, int & byte_count)
{
    clearLastError();
    throwIfNotInConnectedState();
    
    if (buffer_size < byte_count)
    {
        setLastError(SimpleSocketErrCodes::InsufficientBufferSize, "Buffer Size is not sufficient to receive all data");
        return -1;        
    }
    
    int iRes= 0;
    int iBytesReceived(0);
    
    while(iBytesReceived < byte_count)
    {
        try
        {
            iRes = receiveData( buffer + iBytesReceived, byte_count - iBytesReceived);
            iBytesReceived = iBytesReceived + iRes;
        }
        catch (SimpleSocketException& ex)
        {
            if( (ex.getErrorCode() == SimpleSocketErrCodes::WouldBlock ) ||
                    ex.getErrorCode() == SimpleSocketErrCodes::Interrupted )
            {//if operation is blocking, or the return code is retry we should retry
                continue;
            }
            else
            {
                byte_count=iBytesReceived;
                throw;
            }            
        }
    }
    //update the number of bytes received.
    byte_count = iBytesReceived;
    return iRes;
}

/*!
 * Closes the network connection.
 * @return 
 * <ul>
 * - returns the error code
 * - 0 - on success
 * - E_RETRY - operation is interrupted by the system, user should retry 
 * - E_NOT_CONNECTED - socket is not connected.
 * </ul>
 * 
 */
SimpleSocketErrCodes SimpleSocket::close()
{
    clearLastError();
    
    SimpleSocketErrCodes error = SimpleSocketErrCodes::SocketError;
    
    int iRet = ::close(socket_fd);
    if(iRet < 0)
    {
        if(errno == EINTR)
        {
            error = SimpleSocketErrCodes::Interrupted;
        }
        else
        {
            error = SimpleSocketErrCodes::NotConnected;
        }
    }
    else
    {
        state = SimpleSocketState::NotConnected;
        error = SimpleSocketErrCodes::Success;
    }
    
    return error;    
}


/*!
 * Wait till the socket is get ready for receiving or sending. 
 * this function can wait for either reading or writing or for both. also time out can be specified, 
 * if neither write or read available with in the specified function will return. 
 * And this function will return immediately if one of specified scenario is available.
 * @param [in] wait_for_read - should set to true if need to wait for read(receive)
 * @param [in] wait_for_write - should set to true if need to wait for write(send)
 * @param [in] wait_time_ms - maximum time to wait in milli seconds
 * @return 
 * <ul>
 * - returns the error code
 * - 0 - timeout occurred
 * - 1 - read is ready
 * - 2 - write is ready
 * - 3 - both read and write is ready
 * - E_RETRY - operation is interrupted by the system, user should retry 
 * - E_NOT_CONNECTED - socket is not connected.
 * - E_RESOURCE_LIMIT - no resources available in the system
 * - E_GENERIC - system error occurred waiting
 * </ul>
 * 
 */
SimpleSocketReadyStates SimpleSocket::waitTillSocketIsReady(bool wait_for_read, bool wait_for_write, long wait_time_ms)
{   
    clearLastError();
    throwIfInNotConnectedState();
    
    SimpleSocketReadyStates iRet = SimpleSocketReadyStates::InterruptedWhileWaiting;        //return value of the function
    fd_set read_fd_set;   //read FD set
    fd_set write_fd_set;  //write FD set
    struct timeval * wait_timeval_struct(NULL); //time value structure to pass to select
    
    FD_ZERO(&read_fd_set);
    FD_ZERO(&write_fd_set);
    
    if(wait_for_read)
    {//if we need to wait for read,set the socket FD in the appropriate FD set
        FD_SET(socket_fd, &read_fd_set);
    }
    if(wait_for_write)
    {//if we need to wait for write,set the socket FD in the appropriate FD set
        FD_SET(socket_fd, &write_fd_set);        
    }
    if(wait_time_ms >= 0)
    {//if timeout has been specified
        wait_timeval_struct = new timeval; 
        wait_timeval_struct->tv_sec = wait_time_ms/1000;
        wait_timeval_struct->tv_usec = (wait_time_ms % 1000) * 1000;   
    }
    //OK then, let's wait        
    int iRes = select(socket_fd+1, &read_fd_set, &write_fd_set, NULL,  wait_timeval_struct);

    if(iRes<0)
    {//select returned an error   
        SimpleSocketErrCodes error_code = SimpleSocketErrCodes::GenericError;
        if (errno == EINTR)
        {
            setLastError(SimpleSocketErrCodes::Interrupted, "Interrupted while waiting for socket");            
            iRet = SimpleSocketReadyStates::InterruptedWhileWaiting;
        }
        else
        {
            switch(errno)
            {
                case ENOMEM:
                    error_code = SimpleSocketErrCodes::ResourceLimitReached;
                    break;

                case EBADF:
                    error_code = SimpleSocketErrCodes::NotConnected;
                    break;   
                default:
                    error_code = SimpleSocketErrCodes::GenericError;
                    break;
            }
            std::stringstream strm;
            strm<<"Error waiting till socket becomes ready. System Error["<< errno <<":"<< getErrnoAsString() <<"]";
            throw SimpleSocketException( error_code, strm.str().c_str() );
        }
    }
    else if(iRes>0)
    {//select returned, positive value, that means Socket FD should be ready, for read or write    
        if(wait_for_read)
        {
            if(FD_ISSET(socket_fd,&read_fd_set))
            {//if read is ready
                iRet = SimpleSocketReadyStates::ReadyForRead;
            }
        }
        if(wait_for_write)
        {
            if(FD_ISSET(socket_fd,&write_fd_set))
            {//if write is ready
                if(iRet == SimpleSocketReadyStates::ReadyForRead)
                {//if read is already ready
                    iRet = SimpleSocketReadyStates::ReadForReadAndWrite;
                }
                else
                {
                    iRet = SimpleSocketReadyStates::ReadyForWrite;
                }
            } 
        }      
    }
    else
    {//this means a timeout
         iRet = SimpleSocketReadyStates::TimeOutOccured;
    }
    return iRet;    
}
/*!
 * Returns a description of the socket. if the socket is connected description contains IP:Port combination of the local and remote ends.
 * @return 
 * <ul>
 * - returns the error code the description
 * </ul>
 * 
 */
std::string SimpleSocket::getConnDescription()
{
    return socket_desc;
}
/*!
 * Sets the options for socket. Options should be set after socket is connected.
 * @param [in] _tOption - option to be set. 
 * 
 * <ul>
 * - following are the options supported
 * - NON_BLOCKING - if this is passed as argument socket will be set to non blocking. all the send and receive operations after that will not be blocked for I/O.
 * In non blocking mode, send or receive operation cannot be performed because I/O is not ready the send and received functions will return with error code E_WOULD_BLOCK
 * - BLOCKING - if this is passed as argument socket will be set to blocking. this is the default behaviour of the socket. this option is complementary to NON_BLOCKING option
 * In non blocking mode, send or receive operation cannot be performed because I/O is not ready, send and received functions will get blocked.
 * - FLUSH_IMMEDIATELY - this option is used to flush the data in the send buffer immediately after a data send. by default this option is disabled.
 * That means TCP may hold data for some time with out sending even after Send function returns
 * </ul>
 * 
 * @return 
 * true on success , false on failure
 */
void SimpleSocket::setSocketOption(SimpleSocketOptions option)
{
    clearLastError();
    throwIfNotInConnectedState();
    
    int flags(0);    
    int result = -1;
    
    switch(option)
    {
        case SimpleSocketOptions::NonBlocking:
            
            flags = getCurrentFDStutusFlags();
            
            flags |= O_NONBLOCK;
            
            setFDStutusFlags(flags, "NonBlocking");
            
            break;
            
        case SimpleSocketOptions::Blocking:
            
            flags = getCurrentFDStutusFlags();
            flags &= ~O_NONBLOCK;
            setFDStutusFlags(flags, "Blocking");
                       
            break;
            
        case SimpleSocketOptions::FlushImmediately:            
            //set the socket option to flush the data in send buffer immediately.           
            flags = 1;
            
            result = setsockopt(socket_fd, IPPROTO_TCP,  TCP_NODELAY, (char *) &flags, sizeof(int));

            if (result < 0)
            {             
                throw SimpleSocketException( SimpleSocketErrCodes::FailedToSetSocketOption, "Failed to set socket option [FlushImmediately]"); 
            }
            break;
            
        default:
            assert(false);
            break;
    }
    
}

int SimpleSocket::getCurrentFDStutusFlags()
{
    int flags = fcntl (socket_fd, F_GETFL, 0);
    if (flags < 0)
    {
        throw SimpleSocketException( SimpleSocketErrCodes::FailedToGetCurrentSocketOptions, "Failed to retrieve current socket options");         
    }
    
    return flags;    
}

void SimpleSocket::setFDStutusFlags(int flags, const std::string& flag_name)
{
    if((fcntl(socket_fd, F_SETFL, flags) )==-1)
    {
        std::stringstream strm;
        strm<<"Failed to set socket option ["<< flag_name <<"]";
        throw SimpleSocketException( SimpleSocketErrCodes::FailedToSetSocketOption, strm.str().c_str()); 
    }  
}

void SimpleSocket::setLastError(SimpleSocketErrCodes error, const std::string& error_str)
{
    last_error = error;
    last_error_str = error_str;
    
}

void SimpleSocket::clearLastError()
{
    last_error = SimpleSocketErrCodes::NoError;
    last_error_str = "No Error";    
}

SimpleSocketErrCodes SimpleSocket::getLastError()
{
    return last_error;
}

std::string SimpleSocket::getLastErrorString()
{
    return last_error_str;
}

std::string SimpleSocket::getErrnoAsString()
{
    char buffer[128];
    
    return strerror_r(errno, buffer, 128);
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



