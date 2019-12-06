#include <musync/db_link.hpp>
namespace musync {
namespace server {
bool db_link::is_locked(const std::string& key) {
	std::shared_lock<std::shared_mutex> lock(locks_access_mutex);
	return locks.count(key) /* > 0*/;
}
void db_link::insert_lock(const std::string& key) {
	std::unique_lock<std::shared_mutex> lock(locks_access_mutex);
	locks.insert(key);
}
void db_link::remove_lock(const std::string& key) {
	std::unique_lock<std::shared_mutex> lock(locks_access_mutex);
	locks.erase(key);
}
} //namespace server
} //namespace musync
