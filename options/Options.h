/*
 * Options.h
 *
 *  Created on: Feb 20, 2014
 *      Author: Jonas Kunze
 */

#pragma once
#ifndef Options_H_
#define Options_H_

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <string>
#include <vector>

namespace po = boost::program_options;

namespace na62 {
class Options {
public:
	static void PrintVM(boost::program_options::variables_map vm);
	static void Initialize(int argc, char* argv[],
			po::options_description desc);

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
