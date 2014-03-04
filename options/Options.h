/*
 * Options.h
 *
 *  Created on: Feb 20, 2014
 *      Author: Jonas Kunze
 */

#ifndef OPTIONS_H_
#define OPTIONS_H_

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <string>
#include <vector>

namespace po = boost::program_options;

/*
 * Compile time options
 */
#define MTU 1500
#define LKR_SOURCE_ID 0x24

/*
 * Dynamic Options
 */
#define OPTION_HELP (char*)"help"
#define OPTION_VERBOSITY (char*)"verbosity"
#define OPTION_CONFIG_FILE (char*)"configFile"
#define OPTION_LOGTOSTDERR (char*)"logtostderr"

/*
 * Listening Ports
 */
#define OPTION_L0_RECEIVER_PORT (char*)"L0Port"
#define OPTION_CREAM_RECEIVER_PORT (char*)"CREAMPort"
#define OPTION_EOB_BROADCAST_IP (char*)"EOBBroadcastIP"
#define OPTION_EOB_BROADCAST_PORT (char*)"EOBBroadcastPort"

/*
 * Event Building
 */
#define OPTION_NUMBER_OF_EBS (char*)"numberOfEB"
#define OPTION_DATA_SOURCE_IDS (char*)"L0DataSourceIDs"

#define OPTION_TS_SOURCEID (char*)"timestampSourceID"

#define OPTION_CREAM_CRATES (char*)"CREAMCrates"

#define OPTION_FIRST_BURST_ID (char*)"firstBurstID"

#define OPTION_CREAM_MULTICAST_GROUP (char*)"creamMulticastIP"
#define OPTION_CREAM_MULTICAST_PORT (char*)"creamMulticastPort"
#define OPTION_MAX_TRIGGERS_PER_L1MRP (char*)"maxTriggerPerL1MRP"

/*
 * Triggering
 */
#define OPTION_L1_DOWNSCALE_FACTOR  (char*)"L1DownscaleFactor"
#define OPTION_L2_DOWNSCALE_FACTOR  (char*)"L2DownscaleFactor"

#define OPTION_MIN_USEC_BETWEEN_L1_REQUESTS (char*)"minUsecsBetweenL1Requests"

/*
 * Merger
 */
#define OPTION_MERGER_HOST_NAME (char*)"mergerHostName"
#define OPTION_MERGER_PORT (char*)"mergerPort"

namespace na62 {
class Options {
public:
	static void PrintVM(boost::program_options::variables_map vm);
	static void Initialize(int argc, char* argv[]);

	static bool Isset(char* parameter);
	static std::string GetString(char* parameter);
	static int GetInt(char* parameter);
	static std::vector<int> GetIntList(char* parameter);
	static std::vector<double> GetDoubleList(char* parameter);

	/*
	 * If a parameter is formated like A:a,B:b this will return the following vector v:
	 * v[0]: std::make_pair(A, a)
	 * v[1]: std::make_pair(B, b)
	 */
	static std::vector<std::pair<std::string, std::string> > GetPairList(
			char* parameter);

	static std::vector<std::pair<int, int> > GetIntPairList(char* parameter);

	static float GetFloat(char* parameter);

	static void Save(void);
	static void Save(std::string fileName);

	static const std::type_info& GetOptionType(std::string key);

	static void UpdateValue(std::string key, float f, bool notify = true);
	static void UpdateValue(std::string key, std::string str,
			bool notify = true);

	/*
	 * Can be used to access all Descriptions
	 */
	static std::vector<boost::shared_ptr<po::option_description> > GetOptions() {
		return desc.options();
	}

	static po::options_description desc;

private:
	static po::variables_map vm;
};
}
#endif /* OPTIONS_H_ */
