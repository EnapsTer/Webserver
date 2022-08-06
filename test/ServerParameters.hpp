#ifndef SERVERPARAMETERS_HPP
#define SERVERPARAMETERS_HPP

#include <algorithm>
#include <iostream>
#include <vector>
#include <map>

namespace ft {
	struct ValidConfigKeys {
		std::vector<std::string>			servParams;
		std::vector<std::string>			locParams;
		std::map<std::string, int>			servKeys;
		std::map<std::string, std::string>	errorsMap;
		std::vector<std::string>			errorPage;
		std::vector<std::string>			autoindexPage;

		ValidConfigKeys();
		ValidConfigKeys(int responder);
		~ValidConfigKeys();
	};
}

#endif
