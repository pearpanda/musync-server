#include <musync/mysql/mysql_error.hpp>
namespace musync {
namespace server {
	musync_mariadb_error::musync_mariadb_error(MYSQL *conn) {
		error_string = "ERROR ";
		error_string += mysql_errno(conn);
		error_string += " [";
		error_string += mysql_sqlstate(conn);
		error_string += "]: \"";
		error_string += mysql_error(conn);
		error_string +=  "\"";
	}
}
}
