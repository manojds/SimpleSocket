/* 
 * File:   Logger.h
 * Author: Manoj De Silva
 *
 * Created on November 25, 2018, 7:10 AM
 */

#ifndef LOGSINKINTERFACE_H
#define LOGSINKINTERFACE_H

#include "SimpleScoketDefs.h"

namespace network_utils
{
    class LogSinkInterface
    {
    public:
        LogSinkInterface(){}
        virtual ~LogSinkInterface(){}
        
        virtual void log(LogLevel level, const std::string& logToken, const std::string& logString) = 0;
        
        
    };
}

#endif /* LOGSINKINTERFACE_H */

