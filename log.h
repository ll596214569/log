////////////////////////////////////////////////////////////////////
/// 版权所有(c) ll596214569
///                    
/// @file  log.h                                   
/// @brief 日志
///                                                       
/// 
///                                                       
/// @version 1.0 
/// @author  ll
/// @date    2023.9.2
///                                                       
/// 修订说明：最初版本                                              
/////////////////////////////////////////////////////////////////////

#pragma once

#ifndef LOG_LL_AF4SG45GDS2G545DS45G_H_
#define LOG_LL_AF4SG45GDS2G545DS45G_H_

#include <string>
#include <functional>
#include <mutex>
#include <queue>
#include <atomic>
#include <condition_variable>
#include <thread> 
#include <fstream>

namespace LLLOG{
    enum class LogLevel : size_t 
    { 
        DEBUG_LOG,
        INFO_LOG,
        WARNING_LOG,
        ERROR_LOG
    };
    
    using LogMsg = std::string;
    using LogFunc = std::function<void(LogMsg&)>;

    class Logger
    {            
        public:
            Logger(LogFunc func = nullptr);
            ~Logger();

            Logger(const Logger&) = delete; 
            Logger& operator=(const Logger&) = delete;
            
            void Log(LogLevel level,const LogMsg& msg);

        private:

            void LogWriting();

        private:

            std::string m_strPath = "LLLog";
            std::fstream m_fs;
            LogFunc m_pLogFunc = nullptr;
            size_t  m_nQueueLimit = 1000;

            std::mutex m_mutex;
            std::condition_variable m_cv;
            std::atomic<bool> m_bEnd = false;
            std::queue<LogMsg> m_queue;
            std::thread m_pThread;
    };
};

static LLLOG::Logger gLogger;
#define LOG_DEBUG(x) gLogger.Log(LLLOG::LogLevel::DEBUG_LOG, (x));
#define LOG_INFO(x) gLogger.Log(LLLOG::LogLevel::INFO_LOG, (x));
#define LOG_WARNING(x) gLogger.Log(LLLOG::LogLevel::WARNING_LOG, (x));
#define LOG_ERROR(x) gLogger.Log(LLLOG::LogLevel::ERROR_LOG, (x));

#endif