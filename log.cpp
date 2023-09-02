#include "log.h"
#include <sstream>

namespace LLLOG{

    const char* log_level_names[] =
    {
        "DEBUG",
        "INFO",
        "WARNING",
        "ERROR"
    };

std::string GetCurrentDate() 
{
    std::time_t t = std::time(nullptr);
    std::tm* local_time = std::localtime(&t);
    char buf[32] = { 0 };
    sprintf(buf, "%02d/%02d/%02d %02d:%02d:%02d", 
        local_time->tm_year + 1900, local_time->tm_mon + 1, local_time->tm_mday, 
        local_time->tm_hour, local_time->tm_min, local_time->tm_sec);
	return std::string(buf);
}

Logger::Logger(LogFunc func) 
:m_pLogFunc(func)
{
    if(m_pLogFunc == nullptr)
    {
        m_fs.open(m_strPath, std::fstream::out | std::fstream::trunc);

        m_pLogFunc = [this](LogMsg& l) -> void
        {   
            m_fs << l;
            m_fs.flush();
        };
    }
    m_pThread = std::thread(&Logger::LogWriting, this);
}

Logger::~Logger() {
    m_bEnd = true;
    m_cv.notify_one();
    if (m_pThread.joinable()) 
    {
        m_pThread.join();
    }
}

void Logger::Log(LogLevel level,const LogMsg& msg)
{
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::ostringstream stream;
    stream << GetCurrentDate() << "[" << log_level_names[static_cast<int>(level)] << "] " << msg << std::endl;
    auto l = stream.str();

    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_queue.push(std::move(l));
        if (m_queue.size() > 3)
        {
            m_cv.notify_all();
        }
    }
}


void Logger::LogWriting() 
{
    std::queue<LogMsg> writer_queue;
    while (true) 
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_cv.wait(lock, [this, &writer_queue]() 
        {
            return this->m_bEnd || this->m_queue.size() > this->m_nQueueLimit 
            || (writer_queue.size() <= 0 && !this->m_queue.empty());
        });

        if (!this->m_queue.empty()) 
        {
            writer_queue.swap(this->m_queue);
            lock.unlock(); 
            while(!writer_queue.empty())
            {
                auto& l = writer_queue.front();
                m_pLogFunc(l);
                writer_queue.pop();
            }
        }
        if (m_bEnd) 
        {            
            return;
        }
    }
}
};