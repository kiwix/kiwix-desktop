#include "log.h"

Log::Log(Log::Level level, std::string outfile)
{
    terminalSink    = nullptr;
    fileSink        = nullptr;
    terminalHandle  = nullptr;
    fileHandle      = nullptr;
    
    maxLevel = level;
    logfilePath = outfile;

    if(level > Level::NONE)
    {
        if(!std::empty(logfilePath))
        {
            fileSink = new spdlog::sinks::basic_file_sink_st(outfile, true);
            fileSink->set_level(spdlog::level::info);
            fileSink->set_pattern("[%H:%M:%S %z] [%l]: %v");

            fileHandle = new spdlog::logger("file_handle", static_cast<spdlog::sink_ptr>(fileSink));
        }
    
        terminalSink = new spdlog::sinks::stdout_sink_st();
        terminalSink->set_level(spdlog::level::info);
        terminalSink->set_pattern("[%H:%M:%S %z] [%l]: %v");
        
        terminalHandle = new spdlog::logger("terminal_handle", static_cast<spdlog::sink_ptr>(terminalSink));
    }
}

void Log::info(const char *fmt, ...)
{
    //  Note: list start and end may only appear in functions which contain an ellipsis
    va_list variaticArgs;
    va_start(variaticArgs, fmt);
    std::string out = format(fmt, variaticArgs);
    va_end(variaticArgs);

    terminalHandle->info(out);
    // fileHandle->info(str);
    return;
};

std::string Log::format(const char *fmt, va_list &args)
{
    char *buffer = 0;
    std::string out = "";
    
    //  Make a copy of the variatic arguments so that they may be
    //  used twice: Once to determine the size and again to format
    va_list copy;
    va_copy(copy, args);

    //  Determine the buffersize, make sure it was successful
    int size = std::vsnprintf(NULL, 0, fmt, copy) + 1;
    va_end(copy);
    
    if(size > 0)
    {
        //  format arguments into string-out
        buffer = new char[size];
        std::vsnprintf(buffer, size, fmt, args);
    
        out = std::string(buffer, size);
        delete[] buffer;
    }


    return out;
}

Log::~Log()
{   
    spdlog::drop_all();
    spdlog::shutdown();
    
    delete terminalSink;
    if(!std::empty(logfilePath)) delete fileSink;
};
