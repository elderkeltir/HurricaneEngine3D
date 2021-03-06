#pragma once

#include <chrono>

namespace cmn
{
	class timer
	{
	public:
		float elapsed_time(){
            std::chrono::duration<float> duration = std::chrono::milliseconds::zero();
            if (!m_paused)
            {
                duration = std::chrono::high_resolution_clock::now() - m_restartTime;
            }
            
            return duration.count();
        }

		void restart(){
            if (!m_paused)
		    m_restartTime = std::chrono::high_resolution_clock::now();
        }

		void pause(){
            m_paused = true;
        }

		void resume(){
            m_restartTime = std::chrono::high_resolution_clock::now();
	        m_paused = false;
        }
		timer() : m_paused(false) {
            restart();
        }
	private:
		std::chrono::high_resolution_clock::time_point m_restartTime;
		bool m_paused;
	};
}