#pragma once

#include "timer.h"
#include "LogHelper.h"

#if defined (LOGGER)

#define LOGGER_START_WATCH(watch_point) TimerDebugProtocol __tmr(watch_point, __FILE__, __FUNCTION__,__LINE__)
#define LOGGER_START_WATCH_OBJ(obj,watch_point) TimerDebugProtocol __tmr(static_cast<std::ostringstream&>(std::ostringstream().flush() << watch_point  ).str().c_str(), __FILE__, __FUNCTION__,__LINE__)
#define LOGGER_START() TimerDebugProtocol __tmr("START", __FILE__, __FUNCTION__,__LINE__)
#define LOGGER_START_OBJ(obj) TimerDebugProtocol __tmr("START", __FILE__, __FUNCTION__,__LINE__)
#define LOGGER_SS(message) (LogHelper::instance())->WriteLine(static_cast<std::ostringstream&>(std::ostringstream().flush() << __FUNCTION__ <<":" <<__LINE__ << ", " << message << std::endl).str().c_str(),0,false) 
#define LOGGER_WARNING(message) (LogHelper::instance())->WriteLine(static_cast<std::ostringstream&>(std::ostringstream().flush() << __FUNCTION__ <<":" <<__LINE__ << ", " << message << std::endl).str().c_str(),8,true) 
#define LOGGER_ERROR(message) (LogHelper::instance())->WriteLine(static_cast<std::ostringstream&>(std::ostringstream().flush() << __FUNCTION__ <<":" <<__LINE__ << ", " << message << std::endl).str().c_str(),128,true) 
#define LOGGER_RAW(direct,string_buffer) (LogHelper::instance())->Direct(direct).PrintRAW(string_buffer).Direct(0)
#define LOGGER_RAW_SIZE(direct,string_buffer,_size) (LogHelper::instance())->Direct(direct).PrintRAW((const char*)string_buffer,(size_t)_size).Direct(0)
#define LOGGER_STOP() __tmr.reset()
#else
#define LOGGER_START_WATCH(watch_point)
#define LOGGER_START_WATCH_OBJ(obj,watch_point) 
#define LOGGER_START() 
#define LOGGER_START_OBJ(obj) 
#define LOGGER_SS(message)
#define LOGGER_WARNING(message)
#define LOGGER_ERROR(message)
#define LOGGER_RAW(direct,string_buffer)
#define LOGGER_RAW_SIZE(direct,string_buffer,_size)
#define LOGGER_STOP()
#endif