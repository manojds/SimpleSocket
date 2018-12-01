
/* 
 * File:   DefaultLogSink.h
 * Author: Manoj De Silva
 *
 * Created on November 25, 2018, 7:48 AM
 */

#ifndef DEFAULTLOGSINK_H
#define DEFAULTLOGSINK_H
#include <iostream>

#include "LogSinkInterface.h"

namespace network_utils
{
    class DefaultLogSink:public LogSinkInterface
    {
    public:
        virtual void log(LogLevel level, const std::string& logToken, const std::string& logString)
        {
            const char* log_level_str = nullptr;
            
            switch (level)
            {
                case LogLevel::LogDebug:
                    log_level_str = "DEBUG";
                    break;
                    
                case LogLevel::LogInfo:
                    log_level_str = "INFO";
                    break;
                                       
                case LogLevel::LogWarnning:
                    log_level_str = "WARN";
                    break;  
                    
                case LogLevel::LogError:
                    log_level_str = "ERROR";
                    break; 
                default:
                    log_level_str = "UNKNOWN";
                    break;                    
                    
            }
            std::cout<<log_level_str<<"\t"<<logToken<<"\t\t"<<logString<<std::endl;
        }
        
    };
}


#endif /* DEFAULTLOGSINK_H */

