#ifndef MUSYNC_CONNECTION_MANAGER
#define MUSYNC_CONNECTION_MANAGER
#include <iostream>
#include <memory>
#include <asio.hpp>
#include <musync/config_vars.hpp>
#include <musync/worker_thread.hpp>
namespace musync {
namespace server {
class connection_manager {
public:
	connection_manager(connection_manager&) = delete;
	connection_manager& operator=(connection_manager&) = delete;

	/// Create a worker thread. The parameter represents the number
	/// of connections the thread will preallocate
	explicit connection_manager(config_ptr vars);
	connection_ptr get_available_connection();

	//Check whether run is needed
	void stop();
private:
	typedef asio::executor_work_guard<asio::io_context::executor_type>
		io_context_work;

	std::vector<worker_thread_ptr> workers;
	std::vector<io_context_ptr> io_contexts;
	std::vector<io_context_work> work;
};
}
}
#endif

