/* 
 * File:   SimpleScoketDefs.h
 * Author: Manoj De Silva
 *
 * Created on November 25, 2018, 6:18 AM
 */

#ifndef SIMPLESCOKETDEFS_H
#define SIMPLESCOKETDEFS_H

enum class SimpleSocketState
{
    NotConnected,
    Connected,
    Listening
};

enum class SimpleSocketOptions
{
    NonBlocking,
    Blocking,
    FlushImmediately
};

enum class SimpleSocketReadyStates
{
    TimeOutOccured                  = 0,
    ReadyForRead                    = 1,
    ReadyForWrite                   = 2,
    ReadForReadAndWrite             = 3,
    InterruptedWhileWaiting         = 4 
};

enum class SimpleSocketErrCodes
{
    NoError                         = 2,
    Success                         = 1,
    ConnectionClosedByOtherParty    = 0,
    IpPortResolveError              = -1,
    ResourceLimitReached            = -2,
    FailedToConnect                 = -3,
    PortAlreadyInUse                = -4,
    WouldBlock                      = -5,
    FailedToAcceptConn              = -6,
    Interrupted                     = -7,
    SocketError                     = -8,
    ReceiveFailed                   = -9,
    NotConnected                    = -10,
    GenericError                    = -11,
    NotListening                    = -12,
    FailedToGetCurrentSocketOptions = -13,
    FailedToSetSocketOption         = -14,
    InsufficientBufferSize          = -15,
    AlreadyConnected                = -16
    
};


enum class LogLevel
{
    LogDebug,
    LogInfo,
    LogWarnning ,
    LogError
    
};
//#define E_IP_PORT_RESOLVE_ERROR -1
//#define E_RESOURCE_LIMIT        -2
//#define E_FAILED_TO_CONNECT     -3
//#define E_PORT_ALREADY_INUSE    -4
//#define E_WOULD_BLOCK           -5
//#define E_ACCEPT_FAILED         -6
//#define E_RETRY                 -7
//#define E_SOCKET_ERROR           -8
//#define E_RECEIVE_FAILED        -9
//#define E_NOT_CONNECTED         -10
//#define E_GENERIC               -11
//#define E_NOT_LISTENING         -12

#endif /* SIMPLESCOKETDEFS_H */

