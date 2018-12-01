#include "LogFactory.h"
#include "DefaultLogSink.h"

using namespace network_utils;

std::shared_ptr<LogSinkInterface> LogFactory::sink = std::make_shared<DefaultLogSink>();

void LogFactory::setLogSink(std::shared_ptr<LogSinkInterface> new_sink)
{
    sink = new_sink;
}
        
std::shared_ptr<LogSinkInterface> LogFactory::getLogSink()
{
    return sink;    
}