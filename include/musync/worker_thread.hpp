#ifndef MUSYNC_WORKER_THREAD
#define MUSYNC_WORKER_THREAD
#include <iostream>
#include <thread>
#include <memory>
#include <unordered_set>
#include <queue>
#include <asio.hpp>
#include <musync/config_vars.hpp>
#include <musync/db_link.hpp>
	#include <musync/mysql/mysql_link.hpp>
#include <musync/connection.hpp>
namespace musync {
namespace server {
typedef std::shared_ptr<connection> connection_ptr;
typedef std::shared_ptr<asio::io_context> io_context_ptr;
class worker_thread : public std::enable_shared_from_this<worker_thread> {
public:
	worker_thread(worker_thread&) = delete;
	worker_thread& operator=(worker_thread&) = delete;

	/// Create a worker thread.
	explicit worker_thread(io_context_ptr io, config_ptr vars_global);

	void init();
	void join();

	connection_ptr get_available_connection();
	void remove_unused(std::size_t min_connections = 0);
	const std::size_t active_connection_count();

	void handle_finish(connection_ptr connection);
private:
	io_context_ptr io_context;
	std::size_t active_conn_count;
	config_ptr vars;
	std::thread worker;
	std::unordered_set<connection_ptr> active_connections;
	std::queue<connection_ptr> available_connections;
	db_link_ptr data;
};
typedef std::shared_ptr<worker_thread> worker_thread_ptr;
}
}
#endif
