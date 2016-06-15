/*
 * Options.h
 *
 *  Created on: Feb 20, 2014
 *      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#pragma once
#ifndef Options_H_
#define Options_H_

/*
 * Bugfix: see https://svn.boost.org/trac/boost/ticket/8546
 */
#if defined(__GNUC__) && ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 7)))
#  define BOOST_ATTRIBUTE_UNUSED __attribute__((unused))
#else
#  define BOOST_ATTRIBUTE_UNUSED
#endif

#include <boost/program_options.hpp>
#include <string>
#include <vector>

#include "Logging.h"


namespace po = boost::program_options;

/*
 * Compile time options
 */

#define MTU 65000

/*
 * Dynamic options
 */
#define OPTION_HELP (char*)"help"
#define OPTION_CONFIG_FILE (char*)"configFile"
#define OPTION_LOGTOSTDERR (char*)"logtostderr"
#define OPTION_VERBOSITY (char*)"verbosity"
#define OPTION_LOG_FILE (char*)"logDir"
#define OPTION_APP_NAME (char*)"appName"
#define OPTION_DELAY_EOB_PROCESSING (char*)"delayEOBProcessing"

namespace na62 {
class Options {
public:
	static void PrintVM(boost::program_options::variables_map vm);

	/**
	 * Loads all options from the config file stored in desc via OPTION_CONFIG_FILE. Then all options defined in argv will be taken to overwrite these otpions.
	 *
	 * You should call something like following once at the beginning of your code:
	 *
	 desc.add_options()
	 (OPTION_CONFIG_FILE,
	 po::value<std::string>()->default_value("/etc/na62-farm.cfg"),
	 "Config file for the options shown here")
	 // some more options here
	 ;
	 Options::Initialize(argc, argv, desc);
	 */
	static void Initialize(int argc, char* argv[],
			po::options_description desc);

	static bool Isset(char* parameter);
	static std::string GetString(char* parameter);

	/**
	 * Takes the String GetString(parameter) and splits it at every ','
	 */
	static std::vector<std::string> GetStringList(char* parameter);
	static int GetInt(char* parameter);
	static bool GetBool(char* parameter);

	/**
	 * Returns a list of integers split by ',' within the value of the given parameter
	 */
	static std::vector<int> GetIntList(char* parameter);
	static std::vector<double> GetDoubleList(char* parameter);

	/*
	 * If a parameter is formated like A:a,B:b this will return the following vector v:
	 * v[0]: std::make_pair(A, a)
	 * v[1]: std::make_pair(B, b)GetString
	 */
	static std::vector<std::pair<std::string, std::string> > GetPairList(
			char* parameter);

	static std::vector<std::pair<int, int> > GetIntPairList(char* parameter);

	static double GetDouble(char* parameter);

	static void Save(void);
	static void Save(std::string fileName);

	static const std::type_info& GetOptionType(std::string key);

	static void UpdateValue(std::string key, float f, bool notify = true);
	static void UpdateValue(std::string key, std::string str,
			bool notify = true);

	static void addOption(std::string optionName, int defaultValue, std::string description);

protected:
	static po::options_description desc;
	static std::vector<char*> fileNameOptions;

private:
	static po::variables_map vm;

};
}
#endif /* OPTIONS_H_ */
