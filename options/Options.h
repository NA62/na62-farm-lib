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
#define OPTION_VERBOSE (char*)"verbose"
#define OPTION_CONFIG_FILE (char*)"configFile"

/*
 * Listening Ports
 */
#define OPTION_L0_RECEIVER_PORT (char*)"L0Port"
#define OPTION_CREAM_RECEIVER_PORT (char*)"CREAMPort"
#define OPTION_EOB_BROADCAST_PORT (char*)"EOBBroadcastPort"

class INotifiable;
class Options {
public:
	static void PrintVM(boost::program_options::variables_map vm);
	static void Initialize(int argc, char* argv[]);

	static bool Isset(char* parameter);
	static std::string GetString(char* parameter);
	static int GetInt(char* parameter);
	static std::vector<int> GetIntList(char* parameter);
	static std::vector<double> GetDoubleList(char* parameter);

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

	/*
	 * Configurable Variables
	 */
	static bool VERBOSE;

	static po::options_description desc;

	/*
	 * sourceID must be a valid SourceID! So use checkSourceID if you are not sure!
	 * 0 <= sourceID < L1_LARGEST_DATA_SOURCE_ID
	 */
	static inline uint8_t SourceIDToNum(const uint8_t sourceID) throw () {
//		return Options::L0_DATA_SOURCE_ID_TO_NUM[sourceID];
		return 0;
	}

	/*
	 * @return bool <true> if the sourceID is correct, <false> else
	 */
	static inline bool CheckL0SourceID(const uint8_t sourceID) throw () {
//		if (sourceID > Options::Instance()->LARGEST_L0_DATA_SOURCE_ID) {
//			return false;
//		}
//		uint8_t num = Options::Instance()->L0_DATA_SOURCE_ID_TO_NUM[sourceID];
//		if (sourceID != Options::Instance()->L0_DATA_SOURCE_IDS[num]) {
//			return false;
//		}
		return true;
	}

	static inline bool CheckCREAMID(const uint8_t crateID, const uint8_t CREAM_ID) throw () {
//		std::map<uint16_t, uint16_t>* cratesMap = &Options::Instance()->CRATE_AND_CREAM_IDS_TO_LOCAL_ID;
//		if (cratesMap->find((crateID << 8) | CREAM_ID) == cratesMap->end()) {
//			return false;
//		}
		return true;
	}

private:
	static po::variables_map vm;
};
#endif /* OPTIONS_H_ */
