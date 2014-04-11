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
//#include <boost/algorithm/string.hpp>
#include <boost/any.hpp>
#include <boost/filesystem/operations.hpp>
//#include <boost/filesystem.hpp>
//#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/value_semantic.hpp>
//#include <boost/program_options.hpp>
#include <boost/thread/detail/thread.hpp>
//#include <boost/thread.hpp>
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
void Options::Initialize(int argc, char* argv[],
		po::options_description desc) {
	Options::desc = desc;

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

int Options::GetInt(char* parameter) {
	if (GetOptionType(parameter) == typeid(int)) {
		return vm[parameter].as<int>();
	}
	return Utils::ToUInt(vm[parameter].as<std::string>());
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

std::vector<std::pair<std::string, std::string> > Options::GetPairList(
		char* parameter) {
	/*
	 * The parameter format must be A:a,B:b...
	 */

	std::vector<std::pair<std::string, std::string> > values;

	std::string comaSeparatedList = vm[parameter].as<std::string>();
	std::vector<std::string> pairStrings;
	boost::split(pairStrings, comaSeparatedList, boost::is_any_of(","));

	for (std::string pairString : pairStrings) {

		std::vector<std::string> tuple;
		boost::split(tuple, pairString, boost::is_any_of(":")); // Now we have A in tuple[0] and a in tuple[1]

		if (tuple.size() != 2) {
			throw BadOption(OPTION_DATA_SOURCE_IDS,
					"Bad format! Must be 'A:a,B:b'");
		}
		values.push_back(std::make_pair(tuple[0], tuple[1]));

	}
	return values;
}

std::vector<std::pair<int, int> > Options::GetIntPairList(char* parameter) {
	auto pairs = GetPairList(parameter);
	std::vector<std::pair<int, int> > values;
	for (auto pair : pairs) {
		try {
			/*
			 * convert 1:3-5 to 1:3,1:4,1:5
			 */
			std::vector<std::string> minMax;
			boost::split(minMax, pair.second, boost::is_any_of("-")); // Now we have A in tuple[0] and a in tuple[1]
			if (minMax.size() == 2) {
				int min = Utils::ToUInt(minMax[0]);
				int max = Utils::ToUInt(minMax[1]);

				for (int i = min; i <= max + 1; i++) {
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

float Options::GetFloat(char* parameter) {
	return vm[parameter].as<float>();
}

const std::type_info& Options::GetOptionType(std::string key) {
	return vm[key].value().type();
}
}
