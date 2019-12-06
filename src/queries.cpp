#include <string>
namespace musync {
namespace server {
namespace query {
std::string login(const std::string& user_or_email,
		const std::string& passwd) {
	return "SELECT user_id FROM users WHERE "
		"(username = '" + user_or_email + "' OR "
		"email = '" + user_or_email + "') "
		"AND passwd = '" + passwd + "'";
}

std::string user_register(
		const std::string& username,
		const std::string& email,
		const std::string& passwd,
		const std::string& first_name) {
	return "INSERT INTO users(username,email,passwd,first_name) "
		"VALUES("
		"'"+username+"',"
		"'"+email+"',"
		"'"+passwd+"',"
		"'"+first_name+"')";
}

std::string user_register(
		const std::string& username,
		const std::string& email,
		const std::string& passwd,
		const std::string& first_name,
		const std::string& last_name) {
	return "INSERT INTO users(username,email,passwd,first_name,last_name) "
		"VALUES("
		"'"+username+"',"
		"'"+email+"',"
		"'"+passwd+"',"
		"'"+first_name+"',"
		"'"+last_name+"')";
}

std::string next_file_id(const std::string& user_id) {
	return "SELECT COALESCE(MAX(song_id)+1,1) "
		"FROM files WHERE user_id = " + user_id;
}

std::string list_files(const std::string& user_id) {
	return "SELECT username,filename,song_title,artist,album "
		"FROM libraries l JOIN users u ON(l.song_user_id = u.user_id) "
		"JOIN files f ON (l.song_user_id = f.user_id AND l.song_id = f.song_id) "
		"WHERE l.user_id = " + user_id;
}

std::string is_file_in_library(
		const std::string& library_owner_id,
		const std::string& song_owner_name,
		const std::string& song_filename) {
	return "SELECT COUNT(*) FROM libraries l JOIN files f ON "
		"(l.song_user_id = f.user_id AND l.song_id = f.song_id) JOIN "
		"users u ON (l.song_user_id = u.user_id) "
		"WHERE l.user_id = " + library_owner_id + " AND "
		"u.username = '" + song_owner_name + "' AND "
		"f.filename = '" + song_filename + "'";
}

std::string user_id_to_username(const std::string& user_id) {
	return "SELECT username FROM users WHERE user_id = " + user_id;
}

std::string file_exists(const std::string& user_id,
		const std::string& filename) {
	return "SELECT COUNT(*) FROM files WHERE user_id="
		+ user_id + " AND filename='" + filename + "'";
}

std::string file_id_to_filename(
		const std::string& user_id,
		const std::string& song_id) {
	return "SELECT filename FROM files WHERE user_id="
		+ user_id + " AND song_id=" + song_id;
}

std::string add_file(
		const std::string& user_id,
		const std::string& filename,
		const std::string& song_id) {
	return "INSERT INTO files(user_id,song_id,filename) "
		"VALUES("+user_id+","+song_id+",'"+filename+"')";
}

std::string remove_file(
		const std::string& user_id,
		const std::string& filename) {
	return "DELETE FROM files "
		"WHERE user_id="+user_id+" AND filename='"+filename+"'";
}

std::string remove_file_from_library(
		const std::string& library_user_id,
		const std::string& song_username,
		const std::string& filename) {
	return "DELETE l FROM libraries l "
		"JOIN files f ON (l.song_user_id = f.user_id AND l.song_id = f.song_id) "
		"JOIN users u ON (u.user_id = f.user_id) "
		"WHERE l.user_id="+library_user_id+
		" AND u.username='"+song_username+"'"+
		" AND filename='"+filename+"'";
}

std::string share(
		const std::string& song_owner_id,
		const std::string& library_username,
		const std::string& filename) {
	return "INSERT INTO libraries(user_id,song_user_id,song_id) "
		"VALUES("
		"(SELECT user_id FROM users WHERE username='"+library_username+"')," +
		song_owner_id + "," +
		"(SELECT song_id FROM files WHERE user_id="+song_owner_id+" AND "
			"filename='"+filename+"'))";
}

std::string revoke_share(
		const std::string& song_owner_id,
		const std::string& library_username,
		const std::string& filename) {
	return "DELETE l FROM libraries l "
		"JOIN files f ON (l.song_user_id = f.user_id AND l.song_id = f.song_id) "
		"JOIN users u ON (u.user_id = l.user_id) "
		"WHERE l.song_user_id="+song_owner_id+
		" AND u.username='"+library_username+"'"+
		" AND filename='"+filename+"'";
}
// To be tested
/*
std::string remove_from_library(
		const std::string& user_id,
		const std::string& filename) {
	return "DELETE FROM libraries "
		"WHERE user_id="+user_id+
		" AND (song_user_id,song_id) IN ("
		"SELECT user_id,song_id FROM files "
		"WHERE filename = '"+filename+"')";
}
*/
} //namespace query
} //namespace server
} //namespace musync
