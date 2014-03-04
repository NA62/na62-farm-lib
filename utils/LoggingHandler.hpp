/*
 * LoggingHandler.hpp
 *
 *  Created on: Feb 26, 2014
 *      Author: root
 */

#pragma once

#include <glog/logging.h>
#include <iostream>
#include <string>

#include "../options/Options.h"

#ifndef LOGGINGHANDLER_HPP_
#define LOGGINGHANDLER_HPP_

namespace na62 {
static void InitializeLogging(char* argv[]) {

	if (Options::GetInt(OPTION_LOGTOSTDERR)) {
		FLAGS_logtostderr = true;
	}
	FLAGS_minloglevel = 2 - Options::GetInt(OPTION_VERBOSITY);

	FLAGS_log_dir = "/var/log/";
	google::InitGoogleLogging(argv[0]);
	LOG(INFO)<<"Found " << 235 << " cookies!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!";

}
}

#endif /* LOGGINGHANDLER_HPP_ */
