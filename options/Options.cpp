/*
 * Options.cpp
 *
 *  Created on: Feb 20, 2014
 *      Author: Jonas Kunze
 */

#include "Options.h"

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/detail/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/any.hpp>
#include <boost/filesystem/v3/operations.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/value_semantic.hpp>
#include <stddef.h>
#include <sys/types.h>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <map>
#include <typeinfo>
#include <utility>

po::variables_map Options::vm;
po::options_description Options::desc("Allowed options");

std::map<std::string, std::string> SettingStore;

/*
 * Configurable Variables
 */
bool Options::VERBOSE;

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
			}
		}
		std::cout << std::endl;
	}
}
/**
 * The constructor must be public but should not be called! Use Instance() as factory Method instead.
 */
void Options::Initialize(int argc, char* argv[]) {
	desc.add_options()

	(OPTION_HELP, "Produce help message")

	(OPTION_VERBOSE, "Verbose mode")

	(OPTION_CONFIG_FILE,
			po::value<std::string>()->default_value("/etc/na62-farm2_0.cfg"),
			"Config file for these options")

	(OPTION_L0_RECEIVER_PORT, po::value<int>()->default_value(58913),
			"UDP-Port for L1 data reception")

	(OPTION_CREAM_RECEIVER_PORT, po::value<int>()->default_value(58915),
			"UDP-Port for L2 CREAM data reception")

	(OPTION_EOB_BROADCAST_PORT, po::value<int>()->default_value(14162),
			"Port for Broadcast packets used to distribute the last event numbers ob a burst.")

			;

	po::store(po::parse_command_line(argc, argv, desc), vm);

	if (vm.count(OPTION_HELP)) {
		std::cout << desc << "\n";
		exit(EXIT_SUCCESS);
	}

	if (vm.count(OPTION_CONFIG_FILE)) {
		if (!boost::filesystem::exists(
				vm[OPTION_CONFIG_FILE ].as<std::string>())) {
			std::cout << "Config file does not exist: "
					<< vm[OPTION_CONFIG_FILE ].as<std::string>() << std::endl;
		} else {

			std::cout << "======= Reading config file "
					<< vm[OPTION_CONFIG_FILE ].as<std::string>() << std::endl;

			po::store(
					po::parse_config_file<char>(
							vm[OPTION_CONFIG_FILE ].as<std::string>().data(),
							desc), vm);

			// Override file settings with argv settings
			po::store(po::parse_command_line(argc, argv, desc), vm);
		}
	}

	po::notify(vm); // Check the configuration

	std::cout << "======= Running with following configuration:" << std::endl;
	PrintVM(vm);

	VERBOSE = vm.count(OPTION_VERBOSE) > 0;
}

bool Options::Isset(char* parameter) {
	return vm.count(parameter);
}

std::string Options::GetString(char* parameter) {
	std::string str;
	if (SettingStore.find(std::string(parameter)) != SettingStore.end()) {
		str = SettingStore[parameter].c_str();
	} else {
		str = vm[parameter].as<std::string>();
	}

	size_t pos = 0;
	while ((pos = str.find("\\n", pos)) != std::string::npos) {
		str.replace(pos, 2, "\n");
		pos += 1;
	}
	return str;
}

int Options::GetInt(char* parameter) {
	if (SettingStore.find(std::string(parameter)) != SettingStore.end()) {
		return atoi(SettingStore[parameter].c_str());
	}
	return vm[parameter].as<int>();
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
				values.push_back(boost::lexical_cast<int>(str));
			}
		} catch (boost::bad_lexical_cast &e) {
			std::cerr
					<< "Unable to cast '" + str
							+ "' to int! Try correct option " << parameter
					<< std::endl;
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
			std::cerr
					<< "Unable to cast '" + str
							+ "' to double! Try correct option " << parameter
					<< std::endl;
			exit(1);
		}
	}

	return values;
}

float Options::GetFloat(char* parameter) {
	if (SettingStore.find(std::string(parameter)) != SettingStore.end()) {
		return atof(SettingStore[parameter].c_str());
	}
	return vm[parameter].as<float>();
}

const std::type_info& Options::GetOptionType(std::string key) {
	return vm[key].value().type();
}
