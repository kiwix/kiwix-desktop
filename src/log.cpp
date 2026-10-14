#include "log.h"

Log::Log()
{
    terminal_sink = std::make_shared<spdlog::sinks::stdout_sink_st>();
    terminal_sink->set_level(spdlog::level::info);
    terminal_sink->set_pattern("[%H:%M:%S %z] [%l]: %v");
    // file_sink = std::make_unique<spdlog::sinks::basic_file_sink_st>();
    // file_sink->set_level(spdlog::level::info)
    spdlog::sinks_init_list sinks = {terminal_sink};
    terminal_handle = std::make_unique<spdlog::logger>("terminal", sinks);
}

void Log::info(std::string str)
{
    terminal_handle->info(str);
    return;
};

Log::~Log(){};