#include <cstring>
#include <iostream>
#include <vector>
#include <musync/config_vars.hpp>
#include <musync/server.hpp>
using server = musync::server::server;
using config_vars = musync::server::config_vars;
int main(int argc, char* argv[]) {
	server *s;
	s = new server();
	s->run();
	return 0;
}

/* This code is temporarily removed, due to the need for faster testing
	if(argc == 1) {
		s = new server();
	} else {
		bool param = false;
		config_vars v;
		std::string config_file;
		for(int i = 0; i < argc; i+=2) {
			if(strcmp(argv[i-1],"--ip_host")==0) v.ip_host = argv[i];
			else if(strcmp(argv[i-1],"--port")==0) v.port = argv[i];
			else if(strcmp(argv[i-1],"--config")==0) config_file.assign(argv[i]);
			// ...
		}
		std::cout << "Creating server" << config_file;
		if(config_file.empty()) s = new server(v);
		else s = new server(v,config_file);
	}
*/
