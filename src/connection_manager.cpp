#include <musync/connection_manager.hpp>
namespace musync {
namespace server {
	connection_manager::connection_manager(config_ptr vars) {
		std::size_t number_of_threads = vars->worker_threads;
		while(number_of_threads > 0) {
			io_context_ptr io_context = std::make_shared<asio::io_context>();
			io_contexts.push_back(io_context);
			work.push_back(asio::make_work_guard(*io_context));
			worker_thread_ptr worker =
				std::make_shared<worker_thread>(io_context, vars);
			workers.push_back(worker);

			worker->init();

			--number_of_threads;
		}
	}

	/// Searches for a worker_thread which is handling the least amount
	/// of connections currently
	connection_ptr connection_manager::get_available_connection() {
		std::size_t index_of_min_thread = 0;
		std::size_t worker_count = workers.size();
		for(std::size_t i = 1; i < worker_count; ++i) {
			if(workers[i]->active_connection_count() <
					workers[index_of_min_thread]->active_connection_count())
				index_of_min_thread = i;
		}
		return workers[index_of_min_thread]->get_available_connection();
	}

	void connection_manager::stop() {
		for(auto io_context : io_contexts) {
			io_context->stop();
		}
		for(auto worker : workers) {
			worker->join();
		}
	}
}
}
