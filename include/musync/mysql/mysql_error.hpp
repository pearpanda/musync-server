#ifndef MUSYNC_MYSQL_ERROR
#define	MUSYNC_MYSQL_ERROR
#include <exception>
#include <string>
#include <mysql.h>
namespace musync {
namespace server {
class musync_mariadb_error : public std::exception {
public:
	musync_mariadb_error(MYSQL *conn);
	virtual const char* what() const throw() {
		return error_string.c_str();
	}
private:
	std::string error_string;
};
}
}
#endif
