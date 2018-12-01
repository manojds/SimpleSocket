/* 
 * File:   LogFactory.h
 * Author: Manoj De Silva
 *
 * Created on November 25, 2018, 7:51 AM
 */

#ifndef LOGFACTORY_H
#define LOGFACTORY_H
#include <memory>
#include "LogSinkInterface.h"

namespace network_utils
{
    class LogFactory
    {
    public:
        LogFactory()  = delete;
        ~LogFactory()= delete;
        
        static void setLogSink(std::shared_ptr<LogSinkInterface> sink);
        
        static std::shared_ptr<LogSinkInterface> getLogSink();        
        
    private:
        static std::shared_ptr<LogSinkInterface> sink;
        
        
    };
}


#endif /* LOGFACTORY_H */

