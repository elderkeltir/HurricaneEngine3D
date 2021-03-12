#pragma once
#include <string>
#include <fstream>
#include <ctime>
#include <chrono> 

class Logger {
public:
	enum loglevel
	{
		logCRITICAL, logERROR, logWARNING, logDEBUG, logINFO,
	};
	Logger(std::string file_name, loglevel level) : full_buffer_size (100)
	{
		log_level_needed = level;
		out.open(file_name, std::ofstream::out | std::ofstream::trunc);
	}
	~Logger()
	{
		out << full_buffer;
		out.close();
	}
	const char* timenow()
	{
		auto timenow = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		return  ctime(&timenow);
	}

	template<typename... Ts>
	void hlog(int log_level, std::string word, Ts ... args)
	{
		if (log_level <= log_level_needed)
		{
			const int buffer_size = 100;
			char buffer[buffer_size];
			snprintf(buffer, buffer_size, word.c_str(), args...);
			std::string wordM = buffer;
			wordM = timenow() + wordM + "\n";
			full_buffer = full_buffer + wordM;
			if (full_buffer.length() >= full_buffer_size)
			{
				out << full_buffer;
				full_buffer.clear();
			}
		}
		else return;
	}

private:
	std::ofstream out;
	std::string full_buffer;
	const int full_buffer_size;
	loglevel log_level_needed;
};

