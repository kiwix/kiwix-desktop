#ifndef KIWIXLOG_H
#define KIWIXLOG_H

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

#include <string>

class Log
{
    public:
        Log();
        virtual ~Log();

        void info(std::string str);

    private:
        std::shared_ptr<spdlog::sinks::stdout_sink_st> terminal_sink;
        // std::shared_ptr<spdlog::sinks::basic_file_sink_st> file_sink;
        
        std::unique_ptr<spdlog::logger> terminal_handle;
};

#endif
