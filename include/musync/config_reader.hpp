#ifndef MUSYNC_CONFIG_READER_HPP
#define MUSYNC_CONFIG_READER_HPP
#include <string>
#include <iostream>
#include <fstream>
#include <unordered_map>
namespace musync {
namespace server {
class config_reader {
public:
	config_reader(std::string config_file);
	// Returns true if the key is present in the hashmap
	bool assign_value_by_key(const std::string& key,
			std::string& output) const;
private:
	std::unordered_map<std::string,std::string> key_value_pairs;
	#ifndef MUSYNC_CONFIG_STRICT
		// A strict configuration means that no extra string processing is required, but there must not be any
		// more whitespaces than necessary
		std::string trim_spaces(const std::string& src,
				const std::string& whitespace);
		std::string reduce(const std::string& src,
				const std::string& fill,
				const std::string& whitespace);
	#endif
};
}
}
#endif
