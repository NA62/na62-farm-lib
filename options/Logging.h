/*
 * Logging.h
 *
 *  Created on: Nov 24, 2014
 *      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#ifndef LOGGING_H_
#define LOGGING_H_

#ifdef USE_GLOG
	#include <glog/logging.h>
#else
	#include <iostream>
#endif

#ifdef USE_GLOG
	#define LOG_INFO LOG(INFO)
	#define LOG_ERROR LOG(ERROR)
	#define LOG_WARNING LOG(WARNING)
	#define ENDL ""
#else
	#define LOG_INFO std::cout
	#define LOG_ERROR std::cerr
	#define LOG_WARNING std::cerr
	#define ENDL std::endl
//	#define LOG_INFO std::cout
//	#define LOG_ERROR std::cerr << "\033[1;31m"
//	#define LOG_WARNING std::cerr << "\033[1;33m"
//	#define ENDL "\033[0m" << std::endl
#endif

#endif /* LOGGING_H_ */
