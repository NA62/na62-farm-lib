/*
 * Options.cpp
 *
 *  Created on: Feb 20, 2014
 *      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#include <time.h>

#include "Options.h"

#include <boost/foreach.hpp>
#include <boost/program_options.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <stddef.h>
#include <sys/types.h>
#include <cstdlib>
#include <iostream>
#include <map>
#include <typeinfo>
#include <utility>

#include "../exceptions/BadOption.h"
#include "../utils/Utils.h"

namespace na62 {
po::variables_map Options::vm;
po::options_description Options::desc("Allowed options");

std::vector<char*> Options::fileNameOptions;

void Options::PrintVM(po::variables_map vm) {
	using namespace po;
	for (variables_map::iterator it = vm.begin(); it != vm.end(); ++it) {
		std::cout << it->first << "=";

		const variable_value& v = it->second;
		if (!v.empty()) {
			const std::type_info& type = v.value().type();
			if (type == typeid(::std::string)) {
				std::cout << v.as<std::string>();
			} else if (type == typeid(int)) {
				std::cout << v.as<int>();
			} else if (type == typeid(double)) {
				std::cout << v.as<double>();
			}
		}
		std::cout << std::endl;
	}
}

void Options::Initialize(int argc, char* argv[], po::options_description desc) {
	fileNameOptions.push_back(OPTION_CONFIG_FILE);

	desc.add_options()

	(OPTION_HELP, "Produce help message")

	(OPTION_VERBOSITY, po::value<int>()->default_value(0),
			"Verbosity mode:\n0: Silent\n1: Error\n2: Warning\n3: Info")

	(OPTION_LOGTOSTDERR, po::value<int>()->default_value(0),
			"Show logs in stderr")

	(OPTION_LOG_FILE,
			po::value<std::string>()->default_value("/var/log/na62-farm"),
			"Directory where the log files should be written to")

	(OPTION_DELAY_EOB_PROCESSING, po::value<int>()->default_value(2000),
			"Delay in milliseconds before the EOB cleanup.")

			;

	po::store(po::parse_command_line(argc, argv, desc), vm);

	if (vm.count(OPTION_HELP)) {
		std::cout << desc << "\n";
		exit(EXIT_SUCCESS);
	}

	for (char* fileNameOption : fileNameOptions) {
		if (vm.count(fileNameOption)) {
			if (!boost::filesystem::exists(
					vm[fileNameOption].as<std::string>())) {
				std::cout << "Config file does not exist: "
						<< vm[fileNameOption].as<std::string>() << std::endl;
			} else {

				std::cout << "======= Reading config file "
						<< vm[fileNameOption].as<std::string>() << std::endl;

				po::store(
						po::parse_config_file<char>(
								vm[fileNameOption].as<std::string>().data(),
								desc), vm);

				// Override file settings with argv settings
				po::store(po::parse_command_line(argc, argv, desc), vm);
			}
		}
	}

	po::notify(vm); // Check the configuration

	std::cout << "======= Running with following configuration:" << std::endl;
	PrintVM(vm);

#ifdef USE_GLOG
	if (Options::GetInt(OPTION_LOGTOSTDERR)) {
		FLAGS_logtostderr = true;
	}
	FLAGS_minloglevel = 3 - Options::GetInt(OPTION_VERBOSITY);

	boost::filesystem::path dir(Options::GetString(OPTION_LOG_FILE));
	if (!boost::filesystem::exists(dir)
			&& !boost::filesystem::create_directory(dir)) {
		std::cerr<< "Unable to create directory " << dir.string() << std::endl;
	}

	FLAGS_log_dir = GetString(OPTION_LOG_FILE);
	google::InitGoogleLogging(argv[0]);
	std::cout << "Writing logs to " << FLAGS_log_dir << " With min log level "
			<< FLAGS_minloglevel << std::endl;

	google::SetLogDestination(google::INFO, std::string(GetString(OPTION_LOG_FILE)+"/na62-farm.info").c_str());
	google::SetLogDestination(google::WARNING, std::string(GetString(OPTION_LOG_FILE)+"/na62-farm.warn").c_str());
	google::SetLogDestination(google::ERROR, std::string(GetString(OPTION_LOG_FILE)+"/na62-farm.err").c_str());
#elif USE_ERS
	if (Options::GetInt(OPTION_LOGTOSTDERR) == false) {
		time_t now = time(0);
		// Convert now to tm struct for local timezone
		std::time_t result = std::time(nullptr);

		std::string ts = std::to_string(result);
		freopen (std::string(GetString(OPTION_LOG_FILE) +"/" + ts + "na62-farm.info").c_str(),"w",stdout);
		freopen (std::string(GetString(OPTION_LOG_FILE) +"/"  + ts + "na62-farm.err").c_str(),"w",stderr);
	}
#endif
}

bool Options::Isset(char* parameter) {
	return vm.count(parameter);
}

std::string Options::GetString(char* parameter) {
	std::string str = vm[parameter].as<std::string>();

	size_t pos = 0;
	while ((pos = str.find("\\n", pos)) != std::string::npos) {
		str.replace(pos, 2, "\n");
		pos += 1;
	}
	return str;
}

std::vector<std::string> Options::GetStringList(char* parameter) {
	std::vector<std::string> list;
	std::string optionString = vm[parameter].as<std::string>();

	if (optionString == "") {
		return list;
	}

	boost::split(list, optionString, boost::is_any_of(";,"));
	return list;
}

int Options::GetInt(char* parameter) {
	if (GetOptionType(parameter) == typeid(int)) {
		return vm[parameter].as<int>();
	}
	if (GetOptionType(parameter) == typeid(uint)) {
		return vm[parameter].as<uint>();
	}
	return Utils::ToUInt(vm[parameter].as<std::string>());
}

bool Options::GetBool(char* parameter) {
	if (GetOptionType(parameter) == typeid(int)) {
		return vm[parameter].as<int>();
	}
	return vm[parameter].as<bool>();
}

std::vector<int> Options::GetIntList(char* parameter) {
	std::vector<int> values;
	std::string comaSeparatedString = GetString(parameter);

	std::vector<std::string> stringList;
	boost::split(stringList, comaSeparatedString, boost::is_any_of(","));

	for (uint i = 0; i < stringList.size(); i++) {
		std::string str = stringList[i];
		boost::trim(str);
		try {
			if (str.size() > 0) {
				values.push_back(Utils::ToUInt(str));
			}
		} catch (boost::bad_lexical_cast &e) {
			LOG_ERROR("Unable to cast '" + str
			+ "' to int! Try correct option " << parameter);
			exit(1);
		}
	}

	return values;
}

std::vector<double> Options::GetDoubleList(char* parameter) {
	std::vector<double> values;
	std::string comaSeparatedString = GetString(parameter);

	std::vector<std::string> stringList;
	boost::split(stringList, comaSeparatedString, boost::is_any_of(","));

	for (uint i = 0; i < stringList.size(); i++) {
		std::string str = stringList[i];
		boost::trim(str);
		try {
			if (str.size() > 0) {
				values.push_back(boost::lexical_cast<double>(str));
			}
		} catch (boost::bad_lexical_cast &e) {
			LOG_ERROR("Unable to cast '" + str
			+ "' to double! Try correct option " << parameter);
			exit(1);
		}
	}

	return values;
}

std::vector<std::pair<std::string, std::string> > Options::GetPairList(
		char* parameter) {
	/*
	 * The parameter format must be A:a,B:b...
	 */

	std::vector<std::pair<std::string, std::string> > values;

	std::string comaSeparatedList = vm[parameter].as<std::string>();

	/*
	 * Check if the parameter is empty
	 */
	if (comaSeparatedList.size() == 0) {
		return values;
	}

	std::vector<std::string> pairStrings;
	boost::split(pairStrings, comaSeparatedList, boost::is_any_of(","));

	for (std::string pairString : pairStrings) {

		std::vector<std::string> tuple;
		boost::split(tuple, pairString, boost::is_any_of(":")); // Now we have A in tuple[0] and a in tuple[1]

		if (tuple.size() != 2) {
			throw BadOption(parameter, "Bad format! Must be 'A:a,B:b'");
		}
		values.push_back(std::make_pair(tuple[0], tuple[1]));

	}
	return values;
}

std::vector<std::pair<int, int> > Options::GetIntPairList(char* parameter) {
	auto pairs = GetPairList(parameter);
	std::vector<std::pair<int, int> > values;
	for (auto& pair : pairs) {
		try {
			/*
			 * convert 1:3-5 to 1:3,1:4,1:5
			 */
			std::vector<std::string> minMax;
			boost::split(minMax, pair.second, boost::is_any_of("-")); // Now we have A in tuple[0] and a in tuple[1]
			if (minMax.size() == 2) {
				int min = Utils::ToUInt(minMax[0]);
				int max = Utils::ToUInt(minMax[1]);

				for (int i = min; i <= max; i++) {
					values.push_back(
							std::make_pair(Utils::ToUInt(pair.first), i));
				}
			} else {
				values.push_back(
						std::make_pair(Utils::ToUInt(pair.first),
								na62::Utils::ToUInt(pair.second)));
			}
		} catch (boost::bad_lexical_cast const&) {
			throw BadOption(parameter, "Not an integer: '" + pair.first + "'");
		}
	}
	return values;
}

double Options::GetDouble(char* parameter) {
	return vm[parameter].as<double>();
}

const std::type_info& Options::GetOptionType(std::string key) {
	return vm[key].value().type();
}


void Options::addOption(std::string optionName,
		int defaultValue,
		std::string description) {
	desc.add_options()(optionName.c_str(), po::value<int>()->default_value(defaultValue), description.c_str());
}
}
