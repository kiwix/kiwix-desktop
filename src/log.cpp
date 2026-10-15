#include "log.h"

Log::Log(Log::Level max_level, std::string outfile)
{
    terminalSink    = nullptr;
    terminalHandle  = nullptr;
    fileSink        = nullptr;
    fileHandle      = nullptr;

    if(max_level > Level::NONE)
    {
        if(!std::empty(outfile))
        {
            fileSink = std::make_unique<spdlog::sinks::basic_file_sink_st>(outfile, true);
            fileSink->set_level(spdlog::level::info);
            fileSink->set_pattern("[%H:%M:%S %z] [%l]: %v");

            fileHandle = std::make_unique<spdlog::logger>("file_handle", fileSink);
        }
    
        terminalSink = std::make_shared<spdlog::sinks::stdout_sink_st>();
        terminalSink->set_level(spdlog::level::info);
        terminalSink->set_pattern("[%H:%M:%S %z] [%l]: %v");
        
        terminalHandle = std::make_unique<spdlog::logger>("terminal_handle", terminalSink);
    }
}

void Log::info(std::string str)
{
    terminalHandle->info(str);
    // fileHandle->info(str);
    return;
};

Log::~Log(){};