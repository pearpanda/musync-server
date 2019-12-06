#ifndef MUSYNC_CONFIG_VARS_HPP
#define MUSYNC_CONFIG_VARS_HPP
#include <memory>
#include <string>
#include <sstream>
#include <vector>
#include <exception>
#include <filesystem>
#include <musync/config_reader.hpp>
namespace musync {
namespace server {
class config_vars {
public:
	config_vars(const std::string& source);
	config_vars(const config_reader& config);
	// The following constructor is automatically created
	//config_vars(const config_vars& vars);
	config_vars(const config_vars& vars, const std::string& config_file);
#ifndef MUSYNC_CONFIG_REQUIRED
	config_vars();
#else
	config_vars() = delete;
#endif
	void read_config(const config_reader& config);
	void read_config(const std::string& source);

	std::string ip_host;
	std::string port;
	std::filesystem::path files_root;

	std::size_t worker_threads;
	std::size_t worker_connections;

	std::size_t max_request_size;
	std::size_t buffer_size;

	std::string database; // To be implemented: mariadb/mysql or filesystem

	// MariaDB settings
	std::string mariadb_host;
	std::string mariadb_user;
	bool mariadb_passwd_exists;
	std::string mariadb_passwd;
	std::size_t mariadb_port;
	bool mariadb_use_unix_socket;
	std::string mariadb_path_to_socket;
private:
	void set_defaults();
};
typedef std::shared_ptr<config_vars> config_ptr;
}
}
#endif
