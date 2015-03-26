#ifndef SLAM_CLOCK_HPP
#define SLAM_CLOCK_HPP

#include <sys/time.h>

namespace slam
{
	/**
	 * @class Clock
	 * @author Sebastian Kasperski
	 * @date 03/17/15
	 * @file Clock.hpp
	 * @brief Base class for different clock types, to get timestamps
	 * for messages and samples. It might rely on system time, published clock
	 * from log files or simulated ticks from a simulation environment.
	 */
	class Clock
	{
	public:
	
		/**
		 * @brief Returns current time depending on the clock.
		 * @return Current timestamp
		 */
		virtual timeval now()
		{
			timeval tv;
			gettimeofday(&tv, NULL);
			return tv;
		}
	};
}

#endif