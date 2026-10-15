#ifndef KIWIXLOG_H
#define KIWIXLOG_H

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

#include <string>

class Log
{
    public:
        enum Level
        {
            NONE,
            INFO,
            DEBUG,
            WARN,
            ERROR,
            CRITICAL
        };

        Log(Level maxLevel = Level::NONE, std::string outfile = "");
        virtual ~Log();

        void info(std::string str);
        void debug(std::string str);
        void warn(std::string str);
        void error(std::string str);
        void critical(std::string str);

    private:
        void set_levels();
        std::shared_ptr<spdlog::sinks::stdout_sink_st> terminalSink;
        std::shared_ptr<spdlog::sinks::basic_file_sink_st> fileSink;
        
        std::unique_ptr<spdlog::logger> terminalHandle;
        std::unique_ptr<spdlog::logger> fileHandle;
};

#endif
