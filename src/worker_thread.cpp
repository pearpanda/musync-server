#include <musync/worker_thread.hpp>
namespace musync {
namespace server {
worker_thread::worker_thread(io_context_ptr io,
		config_ptr vars_global)
	:io_context(io), active_conn_count(0), vars(vars_global),
	worker(
		[this]() {
			mysql_thread_init();

			if(io_context) {
				io_context->run();
			}

			mysql_thread_end();
		}
	) {
	if(vars_global->database == "mysql" ||
		vars_global->database == "mariadb") {
		data = std::make_shared<mysql_link>(vars_global);
	}
}

void worker_thread::init() {
	std::size_t starting_connection_count = vars->worker_connections;
	while(starting_connection_count > 0) {
		available_connections.push(std::make_shared<connection>(shared_from_this(), io_context, vars, data));
		--starting_connection_count;
	}
}

void worker_thread::join() {
	active_connections.clear();
	while(!available_connections.empty()) {
		available_connections.pop();
	}
	worker.join();
}

connection_ptr worker_thread::get_available_connection() {
	connection_ptr conn;
	if(!available_connections.empty()) {
		conn = available_connections.front();
		available_connections.pop();
		active_connections.insert(conn);
		return conn;
	} else {
		conn = std::make_shared<connection>(shared_from_this(), io_context, vars, data);
		active_connections.insert(conn);
		return conn;
	}
}

void worker_thread::remove_unused(std::size_t min_connections) {
	std::size_t qSize = available_connections.size();
	if(min_connections <= qSize)
		while(!available_connections.empty() && qSize >= min_connections) {
			available_connections.front()->finish();
			available_connections.pop();
			--qSize;
		}
}

const std::size_t worker_thread::active_connection_count() {
	return active_conn_count;
}

void worker_thread::handle_finish(connection_ptr connection) {
	active_connections.erase(connection);
}
}
}
