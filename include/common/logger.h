#pragma once

#include <string>
#include <fstream>
#include <ctime>
#include <chrono> 
#include <assert.h>

class logger {
public:
	enum log_level
	{
		ll_CRITICAL, 
		ll_ERROR, 
		ll_WARNING, 
		ll_DEBUG, 
		ll_INFO
	};
	logger(const std::string &file_name, log_level level) : m_full_buffer_size (100), m_log_level_needed (level)
	{
		m_fileStream.open(file_name, std::ofstream::out | std::ofstream::trunc);
		assert(m_fileStream.is_open());
	
	}
	~logger()
	{
		if (m_buffer.length() >= 1)
		{
			m_fileStream << m_buffer;
		}
		m_fileStream.close();
	}
	const std::string timenow()
	{
		auto timenow = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		std::string time_now = ctime(&timenow);
		return  time_now;
	}

	template<typename... Ts>
	void hlog(int log_level, std::string line, Ts ... args)
	{
		if (log_level <= m_log_level_needed)
		{
			const int buffer_size = 100;
			char buffer[buffer_size];
			snprintf(buffer, buffer_size, line.c_str(), args...);
			std::string text_line = buffer;
			text_line = timenow() + text_line + "\n";
			m_buffer = m_buffer + text_line;
			if (m_buffer.length() >= m_full_buffer_size)
			{
				m_fileStream << m_buffer;
				m_buffer.clear();
			}
		}
		else
		{
			return;
		}
	}

private:
	std::ofstream m_fileStream;
	std::string m_buffer;
	const size_t m_full_buffer_size;
	log_level m_log_level_needed;
};

