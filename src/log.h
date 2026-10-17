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

        Log();
        virtual ~Log();

        void info(const char *fmt, ...);
        void debug(const char *fmt, ...);
        void warn(const char *fmt, ...);
        void error(const char *fmt, ...);
        void critical(const char *fmt, ...);

    private:
        //  Facilitates the use of printf-style format string
        //  I.e. %s %d %f -- etc..
        //  Use this in favour of spdlog's own non-standard formatting syntax
        //  I.e. {0:s} {1:d} {2:3.2f} -- etc
        std::string format(const char *fmt, va_list &args);
        
        //  Setup log level and output pattern for a spdlog::sink
        template<typename T> void configureDestination(T &sink);

        spdlog::sinks::stdout_sink_st *_terminalSink;
        spdlog::sinks::basic_file_sink_st *_fileSink;
        
        spdlog::logger *_logHandle;

        std::string _logfilePath;
        Level _minLevel;
};

static Log logger = Log();

#endif  //  KIWIXLOG_H
