#ifndef KIWIXLOG_H
#define KIWIXLOG_H

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

#include <iostream>
#include <string>
#include <cstdarg>

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

        void info(const char *fmt, ...);

    private:
        //  Facilitates the use of printf-style format string
        //  I.e. %s %d %f -- etc..
        //  Use this in favour of spdlog's own non-standard format string syntax
        //  I.e. {0:s} {1:d} {2:3.2f} -- etc
        std::string format(const char *fmt, va_list &args);

        spdlog::sinks::stdout_sink_st *terminalSink;
        spdlog::sinks::basic_file_sink_st *fileSink;
        
        spdlog::logger *terminalHandle;
        spdlog::logger *fileHandle;

        std::string logfilePath;
        Level maxLevel;
};

#endif
