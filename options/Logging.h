#ifndef LOGGING_H_
#define LOGGING_H_

#ifdef USE_GLOG
	#include <glog/logging.h>
	#define LOG_INFO(message)	LOG(INFO) << message
	#define LOG_ERROR(message)	LOG(ERROR) << message 
	#define LOG_WARNING(message)	LOG(WARNING) << message
#elif USE_ERS
	#include <ers/ers.h>
	#define ERS_INFO( message ) do { \
	{ \
		ERS_REPORT_IMPL( ers::info, ers::Message, message, ERS_EMPTY ); \
	} } while(0)

	#define ERS_WARNING( message ) do { \
	{ \
		ERS_REPORT_IMPL( ers::warning, ers::Message, message, ERS_EMPTY ); \
	} } while(0)

	#define ERS_ERROR( message ) do { \
	{ \
		ERS_REPORT_IMPL( ers::error, ers::Message, message, ERS_EMPTY ); \
	} } while(0)

	#define LOG_INFO(message) 	ERS_DEBUG(3, message)
	#define LOG_ERROR(message)	ERS_ERROR(message)
	#define LOG_WARNING(message)	ERS_WARNING(message)
#else
	#include <iostream>
	#define LOG_INFO(message)	std::cout << message << std::endl
	#define LOG_ERROR(message)	std::cerr << message << std::endl
	#define LOG_WARNING(message)	std::cerr << message << std::endl
#endif

#endif /* LOGGING_H_ */
