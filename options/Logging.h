#ifndef LOGGING_H_
#define LOGGING_H_

#ifdef USE_GLOG
	#include <glog/logging.h>
	//#define LOG_INFO LOG(INFO)
	//#define LOG_ERROR LOG(ERROR)
	//#define LOG_WARNING LOG(WARNING)
	//#define ENDL ""

	#define LOG_INFO(message)	LOG(INFO) << message
	#define LOG_ERROR(message)	LOG(ERROR) << message 
	#define LOG_WARNING(message)	LOG(WARNING) << message

#elif USE_ERS
	//#include "FunctionBuf.h"
	//#define LOG_INFO na62::logger::log_info
	//#define LOG_ERROR na62::logger::log_err
	//#define LOG_WARNING na62::logger::log_warn
	//#define ENDL std::flush


	#include <ers/ers.h>
	#define LOG_INFO(message) 	ERS_DEBUG(3,message)
	#define LOG_ERROR(message)	ERS_ERROR(message)
	#define LOG_WARNING(message)	ERS_WARNING(message)

#else
	#include <iostream>
	//#define LOG_INFO std::cout
	//#define LOG_ERROR std::cerr
	//#define LOG_WARNING std::cerr
	//#define ENDL std::endl

	#define LOG_INFO(message)	std::cout << message << std::endl
	#define LOG_ERROR(message)	std::cerr << message << std::endl
	#define LOG_WARNING(message)	std::cerr << message << std::endl


#endif

#endif /* LOGGING_H_ */
