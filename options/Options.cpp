/*
 * Options.cpp
 *
 *  Created on: Feb 20, 2014
 *      Author: Jonas Kunze
 */

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

	(OPTION_VERBOSITY, po::value<int>()->default_value(0),
			"Verbosity mode:\n0: Error\n1: Warning\n2: Info")

	(OPTION_CONFIG_FILE,
			po::value<std::string>()->default_value("/etc/na62-farm.cfg"),
			"Config file for these options")

	(OPTION_LOGTOSTDERR, po::value<int>()->default_value(0),
			"Show logs in stderr")

	(OPTION_L0_RECEIVER_PORT, po::value<int>()->default_value(58913),
			"UDP-Port for L1 data reception")

	(OPTION_EOB_BROADCAST_IP, po::value<std::string>()->required(),
			"Broadcast IP for the distribution of the last event number. Should be the broadcast IP of a network attached to NICs with standard kernel drivers (e.g. farm-out).")

	(OPTION_EOB_BROADCAST_PORT, po::value<int>()->default_value(14162),
			"Port for Broadcast packets used to distribute the last event numbers ob a burst.")

	(OPTION_NUMBER_OF_EBS,
			po::value<int>()->default_value(
					boost::thread::hardware_concurrency() - 4),
			"Number of threads to be used for eventbuilding and L1/L2 processing")

	(OPTION_DATA_SOURCE_IDS, po::value<std::string>()->required(),
			"Comma separated list of all available data source IDs sending Data to L1 (all but LKr) together with the expected numbers of packets per source. The format is like following (A,B,C are sourceIDs and a,b,c are the number of expected packets per source):\n \t A:a,B:b,C:c")

	(OPTION_CREAM_RECEIVER_PORT, po::value<int>()->default_value(58915),
			"UDP-Port for L2 CREAM data reception")

	(OPTION_CREAM_CRATES, po::value<std::string>()->required(),
			"Defines the expected sourceIDs within the data packets from the CREAMs. The format is $crateID1:$CREAMIDs,$crateID1:$CREAMIDs,$crateID2:$CREAMIDs... E.g. 1:2-4,1:11-13,2:2-5,2:7 for two crates (1 and 2) with following IDs (2,3,4,11,12,13 and 2,3,4,5,7).")

	(OPTION_TS_SOURCEID, po::value<std::string>()->required(),
			"Source ID of the detector which timestamp should be written into the final event and sent to the LKr for L1-triggers.")

	(OPTION_FIRST_BURST_ID, po::value<int>()->required(),
			"The current or first burst ID. This must be set if a PC starts during a run.")

	(OPTION_L1_DOWNSCALE_FACTOR, po::value<int>()->required(),
			"With this integer you can downscale the event rate going to L2 to a factor of 1/L1DownscaleFactor. The L1 Trigger will accept every even if  i++%downscaleFactor==0")

	(OPTION_L2_DOWNSCALE_FACTOR, po::value<int>()->required(),
			"With this integer you can downscale the event rate accepted by L2 to a factor of 1/L1DownscaleFactor. The L2 Trigger will accept every even if  i++%downscaleFactor==0")

	(OPTION_MIN_USEC_BETWEEN_L1_REQUESTS, po::value<int>()->default_value(1000),
			"Minimum time between two MRPs sent to the CREAMs")

	(OPTION_CREAM_MULTICAST_GROUP, po::value<std::string>()->required(),
			"The multicast group IP for L1 requests to the CREAMs (MRP)")

	(OPTION_CREAM_MULTICAST_PORT, po::value<int>()->default_value(58914),
			"The port all L1 multicast MRPs to the CREAMs should be sent to")

	(OPTION_MAX_TRIGGERS_PER_L1MRP, po::value<int>()->default_value(100),
			"Maximum number of Triggers per L1 MRP")

	(OPTION_NUMBER_OF_EVENTS_PER_BURST_EXPECTED,
			po::value<int>()->default_value(10000000),
			"Expected number of events per burst and PC")

	(OPTION_MERGER_HOST_NAME, po::value<std::string>()->required(),
			"IP or hostname of the merger PC.")

	(OPTION_MERGER_PORT, po::value<int>()->required(),
			"The TCP port the merger is listening to.")

	(OPTION_ZMQ_IO_THREADS, po::value<int>()->default_value(1),
			"Number of ZMQ IO threads")

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
