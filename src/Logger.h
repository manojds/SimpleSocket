/* 
 * File:   Logger.h
 * Author: Manoj De Silva
 *
 * Created on November 25, 2018, 9:49 AM
 */

#ifndef LOGGER_H
#define LOGGER_H

#include <stdarg.h>

#include "SimpleScoketDefs.h"
#include "LogFactory.h"

#define FORMAT_VAR_ARGS(buffer) \
            va_list args;\
            va_start (args, format);\
            vsnprintf (buffer,1024,format, args);\
            va_end (args);


namespace network_utils
{
    class Logger
    {
    public:
        Logger() = delete;
        ~Logger() = delete;
        
        static void logDebug(const char* token, const char* format, ...)
        {
            char buffer[1024];
            FORMAT_VAR_ARGS(buffer);
            buffer[1024] = '\0';
                    
            LogFactory::getLogSink()->log(LogLevel::LogDebug, token, buffer);
        }
        
        static void logInfo(const char* token, const char* format, ...)
        {
            char buffer[1024];
            FORMAT_VAR_ARGS(buffer);
            buffer[1024] = '\0';
                    
            LogFactory::getLogSink()->log(LogLevel::LogInfo, token, buffer);
        }       
        
        static void logWarnning(const char* token, const char* format, ...)
        {
            char buffer[1024];
            FORMAT_VAR_ARGS(buffer);
            buffer[1024] = '\0';
                    
            LogFactory::getLogSink()->log(LogLevel::LogWarnning, token, buffer);
        }     
        
        static void logError(const char* token, const char* format, ...)
        {
            char buffer[1024];
            FORMAT_VAR_ARGS(buffer);
            buffer[1024] = '\0';
                    
            LogFactory::getLogSink()->log(LogLevel::LogError, token, buffer);
        }          
        
        static void log(LogLevel level, const char* token, const char* format, ...)
        {
            char buffer[1024];            
            FORMAT_VAR_ARGS(buffer);
            buffer[1024] = '\0';
            
            LogFactory::getLogSink()->log(level, token, buffer);
        }
        
    };
}

#endif /* LOGGER_H */

