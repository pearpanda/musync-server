#include <musync/config_vars.hpp>
namespace musync {
namespace server {
#ifndef MUSYNC_CONFIG_REQUIRED
	config_vars::config_vars() {
		set_defaults();
	}
#endif
	config_vars::config_vars(const std::string& config_file) {
		set_defaults();
		config_reader c(config_file);
		read_config(c);
	}

	config_vars::config_vars(const config_reader& config) {
		set_defaults();
		read_config(config);
	}

	config_vars::config_vars(const config_vars& vars,
			const std::string& config_file) :
		config_vars::config_vars(vars) {
		read_config(config_file);
	}

	void config_vars::read_config(const config_reader& config) {
		std::string temp;
		if(config.assign_value_by_key("host_ip",temp)) {
			ip_host = temp;
		}
		if(config.assign_value_by_key("port",temp)) {
			port = temp;
		}
		if(config.assign_value_by_key("files_root",temp)) {
			files_root = temp;
		}
		if(config.assign_value_by_key("worker_threads",temp)) {
			std::stringstream str(temp);
			str >> worker_threads;
		}
		if(config.assign_value_by_key("database",temp)) {
			database = temp;
		}
		if(config.assign_value_by_key("max_request_size",temp)) {
			std::stringstream str(temp);
			str >> max_request_size;
		}
		if(config.assign_value_by_key("connection_buffer_size",temp)) {
			std::stringstream str(temp);
			str >> buffer_size;
		}
		if(config.assign_value_by_key("worker_connections",temp)) {
			std::stringstream str(temp);
			str >> worker_connections;
		}
		if(config.assign_value_by_key("mariadb_host",temp)) {
			mariadb_host = temp;
		}
		if(config.assign_value_by_key("mariadb_user",temp)) {
			mariadb_user = temp;
		}
		if(config.assign_value_by_key("mariadb_port",temp)) {
			std::stringstream str(temp);
			str >> mariadb_port;
		}
		if(config.assign_value_by_key("mariadb_password",temp)) {
			mariadb_passwd_exists = true;
			mariadb_passwd = temp;
		} else mariadb_passwd_exists = false;
		if(config.assign_value_by_key("mariadb_unix_socket",temp)) {
			mariadb_use_unix_socket = true;
			mariadb_path_to_socket = temp;
		} else mariadb_use_unix_socket = false;
	}

	void config_vars::read_config(const std::string& source) {
		config_reader config(source);
		read_config(config);
	}

#ifndef MUSYNC_CONFIG_REQUIRED
	void config_vars::set_defaults() {
		ip_host = "0.0.0.0";
		port = "2200";
		files_root = "/home/test/musync";
		worker_threads = 2;
		database = "mariadb";
		max_request_size = 0;
		buffer_size = 4096;

		mariadb_host = "localhost";
		mariadb_user = "test";
		mariadb_passwd_exists = true;
		mariadb_passwd = "test_password";
		mariadb_port = 3306;
		mariadb_use_unix_socket = false;
		mariadb_path_to_socket = "/var/run/mysqld/mysqld.sock";
	}
#endif
}
}
