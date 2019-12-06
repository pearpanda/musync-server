#include <musync/mysql/mysql_link.hpp>
namespace musync {
namespace server {
mysql_link::mysql_link(config_ptr vars_global) : vars(vars_global){
	conn = mysql_init(NULL);
	if(vars_global->mariadb_use_unix_socket) {
		auto protocol_type = MYSQL_PROTOCOL_TCP;
		mysql_optionsv(conn,MYSQL_OPT_PROTOCOL,
				(void *)&protocol_type);
	}
	if(!mysql_real_connect(conn,
		vars_global->mariadb_host.c_str(),
		vars_global->mariadb_user.c_str(),
		(vars_global->mariadb_passwd_exists)?
			vars_global->mariadb_passwd.c_str():NULL,
		"musync",
		vars_global->mariadb_port,
		(vars_global->mariadb_use_unix_socket)?
			vars_global->mariadb_path_to_socket.c_str():NULL,
		0
		)) {
		throw musync_mariadb_error(conn);
	}
}

mysql_link::~mysql_link() {
	mysql_close(conn);
}

std::string mysql_link::get_next_file_id(
		const std::string& user_id) {
	std::string query =
		query::next_file_id(user_id);
	if(mysql_real_query(conn,
			query.c_str(), query.length())) {
		return "";
	}
	MYSQL_RES *res = mysql_store_result(conn);

	MYSQL_ROW row = mysql_fetch_row(res);
	std::size_t *len = mysql_fetch_lengths(res);
	mysql_free_result(res);
	return std::string(row[0],len[0]);
}

bool mysql_link::try_login(
		const std::string& username_or_email,
		const std::string& passwd,
		std::string& user_id) {
	std::string query =
		query::login(username_or_email, passwd);
	if(mysql_real_query(conn,
			query.c_str(), query.length())) {
		// Posto je ovo SELECT recenica, ukoliko ona ne uspe,
		// postoji neka greska u vezi sa bazom
		throw musync_mariadb_error(conn);
	}
	MYSQL_RES *res = mysql_store_result(conn);
	// The number of returned rows is either 0 or 1
	// because of database constraints (username and email are UNIQUE columns)
	if(mysql_num_rows(res) == 1) {
		MYSQL_ROW row = mysql_fetch_row(res);
		std::size_t *len = mysql_fetch_lengths(res);
		user_id = std::string(row[0],len[0]);
		mysql_free_result(res);
		return true;
	} else {
		mysql_free_result(res);
		return false;
	}
}

// SQLSTATE is 23000 for database integrity violations
// (Inserting a row which violates a unique key)

bool mysql_link::try_register(
		const std::string& username,
		const std::string& email,
		const std::string& passwd,
		const std::string& first_name) {
	std::string query =
		query::user_register(username,email,passwd,first_name);
	if(mysql_real_query(conn,
			query.c_str(), query.length())) {
		return false;
	}
	return true;
}

bool mysql_link::try_register(
		const std::string& username,
		const std::string& email,
		const std::string& passwd,
		const std::string& first_name,
		const std::string& last_name) {
	std::string query =
		query::user_register(username,email,passwd,first_name,last_name);
	if(mysql_real_query(conn,
			query.c_str(), query.length())) {
		return false;
	}
	return true;
}

bool mysql_link::list_files(
		const std::string& user_id,
		std::vector< std::pair<std::string,std::string> >& files) {
	std::string query =
		query::list_files(user_id);
	if(mysql_real_query(conn,
				query.c_str(), query.length())) {
		return false;
	}
	MYSQL_RES *res = mysql_store_result(conn);
	MYSQL_ROW row;
	while(row = mysql_fetch_row(res)) {
		std::size_t *len = mysql_fetch_lengths(res);
		files.emplace_back(std::string(row[0],len[0]),
				std::string(row[1],len[1]));
	}
	return true;
}

bool mysql_link::username_from_id(
		const std::string& user_id,
		std::string& username) {
	std::string query =
		query::user_id_to_username(user_id);
	if(mysql_real_query(conn,
			query.c_str(), query.length())) {
		return false;
	}
	MYSQL_RES *res = mysql_store_result(conn);

	MYSQL_ROW row = mysql_fetch_row(res);
	std::size_t *len = mysql_fetch_lengths(res);
	username = std::string(row[0],len[0]);
	mysql_free_result(res);
	return true;
}

bool mysql_link::is_file_in_library(
		const std::string& user_id,
		const std::string& song_owner_name,
		const std::string& filename) {
	// Since owner_id and filename are a UNIQUE key, the query returns either 0 or 1
	std::string query =
		query::is_file_in_library(user_id,song_owner_name,filename);
	if(mysql_real_query(conn,
			query.c_str(), query.length())) {
		return false;
	}
	MYSQL_RES *res = mysql_store_result(conn);

	MYSQL_ROW row = mysql_fetch_row(res);
	if(row[0][0] == '0') {
		mysql_free_result(res);
		return false;
	} else {
		mysql_free_result(res);
		return true;
	}
}

bool mysql_link::does_file_exist(
		const std::string& user_id,
		const std::string& filename) {
	std::string query =
		query::file_exists(user_id,filename);
	if(mysql_real_query(conn,
			query.c_str(), query.length())) {
		return false;
	}
	MYSQL_RES *res = mysql_store_result(conn);

	MYSQL_ROW row = mysql_fetch_row(res);
	if(row[0][0] == '0') {
		mysql_free_result(res);
		return false;
	} else {
		mysql_free_result(res);
		return true;
	}
}

/*
bool mysql_link::filename_from_id(
		const std::string& user_id,
		const std::string& song_id,
		std::string& filename) {
	std::string query =
		query::file_id_to_filename(user_id,song_id);
	if(mysql_real_query(conn,
			query.c_str(), query.length())) {
		return false;
	}
	MYSQL_RES *res = mysql_store_result(conn);

	MYSQL_ROW row = mysql_fetch_row(res);
	std::size_t *len = mysql_fetch_lengths(res);
	filename = std::string(row[0],len[0]);
	mysql_free_result(res);
	return true;
}
*/

bool mysql_link::add_file(
		const std::string& user_id,
		const std::string& filename) {
	std::string song_id = get_next_file_id(user_id);
	if(song_id.empty()) return false;
	std::string query =
		query::add_file(user_id,filename,song_id);
	if(mysql_real_query(conn,
			query.c_str(), query.length())) {
		return false;
	}
	return true;
}

bool mysql_link::remove_file(
		const std::string& user_id,
		const std::string& filename) {
	std::string query =
		query::remove_file(user_id,filename);
	if(mysql_real_query(conn,
			query.c_str(), query.length())) {
		return false;
	}
	std::size_t affected_rows = mysql_affected_rows(conn);
	return (affected_rows != -1 && affected_rows != 0);
}

bool mysql_link::remove_file_from_library(
		const std::string& user_id,
		const std::string& song_username,
		const std::string& filename) {
	std::string query =
		query::remove_file_from_library(user_id,song_username,filename);
	if(mysql_real_query(conn,
			query.c_str(), query.length())) {
		return false;
	}
	return true;
}

bool mysql_link::share(
		const std::string& song_owner_id,
		const std::string& library_username,
		const std::string& filename) {
	std::string query =
		query::share(song_owner_id,library_username,filename);
	if(mysql_real_query(conn,
			query.c_str(), query.length())) {
		return false;
	}
	return true;
}

bool mysql_link::revoke_share(
		const std::string& song_owner_id,
		const std::string& library_username,
		const std::string& filename) {
	std::string query =
		query::revoke_share(song_owner_id,library_username,filename);
	if(mysql_real_query(conn,
			query.c_str(), query.length())) {
		return false;
	}
	return true;
}
} //namespace server
} //namespace musync
