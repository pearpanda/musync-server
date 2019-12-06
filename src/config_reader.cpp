#include <musync/config_reader.hpp>
namespace musync {
namespace server {
	#ifndef MUSYNC_CONFIG_STRICT
		std::string config_reader::trim_spaces(const std::string& src, const std::string& whitespace=" \t") {
			std::size_t str_begin = src.find_first_not_of(whitespace);
			if(str_begin == std::string::npos) {
				return "";
			}

			std::size_t str_end = src.find_last_not_of(whitespace);
			std::size_t str_range = str_end - str_begin + 1;

			return src.substr(str_begin, str_range);
		}

		std::string config_reader::reduce(const std::string& src, const std::string& fill = " ",
			const std::string& whitespace=" \t") {
			std::size_t fillLen = fill.length();
			auto result = trim_spaces(src);
			std::size_t begin_whitespace = result.find_first_of(whitespace);
			while(begin_whitespace != std::string::npos) {
				std::size_t end_whitespace = result.find_first_not_of(whitespace, begin_whitespace);
				std::size_t len = end_whitespace - begin_whitespace + 1;

				result.replace(begin_whitespace, len, fill);
				begin_whitespace = result.find_first_of(whitespace,
					begin_whitespace + fillLen);
			}
			return result;
		}
	#endif

	config_reader::config_reader(std::string config_file) {
		//TODO: Check whether try/catch is needed, exception safety
		std::ifstream config_fstream(config_file, std::ifstream::in | std::fstream::binary);
		std::string line;
		std::size_t pos, ind = 0;
		std::string key, val;
		while(std::getline(config_fstream,line)) {
			++ind;
			if(line.length() > 0) {
				if(line[0] == '#') continue;
				pos = line.find('=');
				if(pos == std::string::npos) { //TODO: replace with logging system
					std::cerr << "ERROR: Configuration file at line " << ind << ": no value to assign";
					continue;
				}
				key = line.substr(0,pos);
				val = line.substr(pos+1);
				#ifndef MUSYNC_CONFIG_STRICT
					key = reduce(key);
					val = reduce(val);
				#endif
				if(!key.empty() && !val.empty()) {
					key_value_pairs.emplace(key,val);
					std::cout << key << " -> " << val << std::endl;
				}
			}
		}
		config_fstream.close();
	}

	bool config_reader::assign_value_by_key(const std::string& key,
			std::string& output) const {
		auto it = key_value_pairs.find(key);
		if(it != key_value_pairs.end()) {
			output = it->second;
			return true;
		}
		return false;
	}
}
}
