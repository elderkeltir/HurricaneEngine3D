#pragma once
#include <string>
#include <fstream>
#include <ctime>
#include <chrono> 

class logger {
public:
	enum loglevel
	{
		logCRITICAL, logERROR, logWARNING, logDEBUG, logINFO,
	};
	Logger(std::string file_name, loglevel level) : full_buffer_size (100)
	{
		log_level_needed = level;
		m_fileStream.open(file_name, std::ofstream::m_fileStream | std::ofstream::trunc);
	}
	~Logger()
	{
		m_fileStream << m_full_buffer;
		m_fileStream.close();
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
			m_full_buffer = m_full_buffer + wordM;
			if (m_full_buffer.length() >= m_full_buffer_size)
			{
				m_fileStream << m_full_buffer;
				m_full_buffer.clear();
			}
		}
		else
		{
			return;
		}
	}

private:
	std::ofstream m_fileStream;
	std::string m_full_buffer;
	const int m_full_buffer_size;
	loglevel log_level_needed;
};

