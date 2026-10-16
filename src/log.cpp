#include "log.h"

Log::Log(Log::Level level, std::string outfile)
{
    _terminalSink    = nullptr;
    _fileSink        = nullptr;
    _logHandle       = nullptr;
    
    _maxLevel = level;
    _logfilePath = outfile;

    _terminalSink = new spdlog::sinks::stdout_sink_st();
    configureDestination(*_terminalSink);

    if(!std::empty(_logfilePath))
    {
        _fileSink = new spdlog::sinks::basic_file_sink_st(outfile, true);
        configureDestination(*_fileSink);

        spdlog::sinks_init_list sinks = {static_cast<spdlog::sink_ptr>(_terminalSink), static_cast<spdlog::sink_ptr>(_fileSink)};
        _logHandle = new spdlog::logger("logger", sinks);
    }
    else 
    {
        _logHandle = new spdlog::logger("logger", static_cast<spdlog::sink_ptr>(_terminalSink));
    }
}

void Log::info(const char *fmt, ...)
{
    //  Capture variatic inputs and format them using printf style formatting
    //  Note that va_list and va_start may only appear inside a function that has an ellipsis (...)
    va_list variaticArgs;
    va_start(variaticArgs, fmt);
    std::string out = format(fmt, variaticArgs);

    _logHandle->info(out);
    
    return;
};

void Log::debug(const char *fmt, ...)
{
    va_list variaticArgs;
    va_start(variaticArgs, fmt);
    std::string out = format(fmt, variaticArgs);

    _logHandle->debug(out);
    
    return;
};

void Log::warn(const char *fmt, ...)
{
    va_list variaticArgs;
    va_start(variaticArgs, fmt);
    std::string out = format(fmt, variaticArgs);

    _logHandle->warn(out);
    
    return;
};

void Log::error(const char *fmt, ...)
{
    va_list variaticArgs;
    va_start(variaticArgs, fmt);
    std::string out = format(fmt, variaticArgs);

    _logHandle->error(out);
    
    return;
};

void Log::critical(const char *fmt, ...)
{
    va_list variaticArgs;
    va_start(variaticArgs, fmt);
    std::string out = format(fmt, variaticArgs);

    _logHandle->critical(out);
    
    return;
};

std::string Log::format(const char *fmt, va_list &args)
{
    char *buffer = 0;
    std::string out = "";

    //  Determine the buffer size
    //  Add an additional byte for storing the null terminator
    int size = std::vsnprintf(NULL, 0, fmt, args) + 1;
    if(size > 0)
    {
        //  format arguments into string-out
        buffer = new char[size];
        std::vsnprintf(buffer, size, fmt, args);
    
        out = std::string(buffer, size);
        delete[] buffer;
    }

    //  Finish the parsing of variatic arguments
    va_end(args);

    return out;
}

template<typename T> void Log::configureDestination(T &sink)
{
    //  Set log-level
    if(_maxLevel > Level::NONE && _maxLevel <= Level::CRITICAL)
    {
        for(size_t i = 1; i < _maxLevel; i++)
            sink.set_level(static_cast<spdlog::level::level_enum>(i));
    
        //  Format pattern output 
        //  I.e. [UTC datetime] [log-level]: input
        sink.set_pattern("[%H:%M:%S %z] [%l]: %v");
    }
    else
    {
        sink.set_level(spdlog::level::off);
    }
    
    return;
}

Log::~Log()
{   
    spdlog::drop_all();
    spdlog::shutdown();
    
    delete _terminalSink;
    if(!std::empty(_logfilePath)) delete _fileSink;
};
