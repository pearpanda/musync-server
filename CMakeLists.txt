cmake_minimum_required (VERSION 3.10)

project(musync-server LANGUAGES CXX)

include_directories("${PROJECT_SOURCE_DIR}/include")

option(USE_SYSTEM_ASIO "If this option is enabled, the system ASIO library will be used instead of the one supplied with this project." OFF)

if(NOT USE_SYSTEM_ASIO)
	include_directories("${PROJECT_SOURCE_DIR}/external/asio")
endif(NOT USE_SYSTEM_ASIO)

set(LIBS_MARIADB_CONNECTOR mariadb dl m ssl crypto)

add_executable(server src/main.cpp
	src/connection.cpp
	src/connection_user_management.cpp
	src/connection_file_management.cpp
	src/connection_list_files.cpp
	src/connection_manager.cpp
	src/main.cpp
	src/request.cpp
	src/server.cpp
	src/worker_thread.cpp
	src/config_vars.cpp
	src/config_reader.cpp
	src/mysql_link.cpp
	src/mysql_error.cpp
	src/queries.cpp
)

set_target_properties(server PROPERTIES
	CXX_STANDARD 17
	CXX_STANDARD_REQUIRED ON
)

add_definitions(-DASIO_STANDALONE)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(server Threads::Threads)
target_link_libraries(server ${LIBS_MARIADB_CONNECTOR})
target_link_libraries(server stdc++fs)
