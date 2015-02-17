#include "MySQL.h"
#include "tinyformat.h"
#include "misc.h"


MySQL::MySQL(const Flux::string &host, const Flux::string &user, const Flux::string &pass, const Flux::string &db, short p = 0) :
	user(user), pass(pass), host(host), db(db), port(p)
{
	printf("MySQL client version: %s\n", mysql_get_client_info());

	// Create the MySQL object
	this->con = mysql_init(NULL);

	if (!con)
		throw MySQLException("Failed to create a mysql object!");

	// Authenticate to the MySQL database
	if (!this->DoConnection())
		throw MySQLException(tfm::format("%s (%d)", mysql_error(this->con), mysql_errno(this->con)));

	printf("Connected to %s: %lu using database \"%s\"\n", host.c_str(), mysql_thread_id(this->con), db.c_str());
}

MySQL::~MySQL()
{
	mysql_close(con);
}

MySQL_Result MySQL::Query(const Flux::string &query)
{
	// Form our object
	MySQL_Result res;

	// If we fail to connect, just return an empty query.
	if (!this->con)
		if (!this->CheckConnection())
			return res;

	// Run the query
	if (mysql_query(this->con, query.c_str()))
		throw MySQLException(tfm::format("%s (%d)\n", mysql_error(this->con), mysql_errno(this->con)), query);

	// Store the query result
	MYSQL_RES *result = mysql_store_result(this->con);
	if (result == NULL)
		throw MySQLException(tfm::format("%s (%d)\n", mysql_error(this->con), mysql_errno(this->con)), query);

	// Get total columns/fields w/e
	unsigned fieldslen = mysql_num_fields(result);
	// Get the field structures from MySQL so we can convert them to a key-value map which is much more useful.
	MYSQL_FIELD *fields = mysql_fetch_fields(result);

	// Loop through the MySQL objects and create the array for the query result
	for (MYSQL_ROW row = mysql_fetch_row(result); row; row = mysql_fetch_row(result))
	{
		// Iterate all fields in this row specifically
		MySQL_Row cpprow;
		for (unsigned i = 0; i < fieldslen; ++i)
		{
			// Import the row to the map.
			cpprow[fields[i].name] = row[i];
		}
		res.push_back(cpprow);
	}

	mysql_free_result(result);

	return res;
}

bool MySQL::DoConnection()
{
	// If mysql_real_connect returns NULL then we have failed to connect, return true or false or whatever
	return mysql_real_connect(this->con, this->host.c_str(), this->user.c_str(),
				  this->pass.c_str(), this->db.c_str(), this->port, NULL, 0) != NULL;
}

bool MySQL::CheckConnection()
{
	dprintf("CheckConnection()...\n");
	if (!this->con)
		goto tryconnect;

	// Ping the server, if it doesn't reply then start reconnecting.
	if (mysql_ping(this->con) != 0)
	{
tryconnect:
		// Retry 5 times to connect to the server, return true if we do
		for (int i = 0; i < 5; ++i)
			if (this->DoConnection())
				return true;
		// otherwise we fall through and failed to connect 5 times so return false.
		return false;
	}

	return true;
}

Flux::string MySQL::Escape(const Flux::string &str)
{
	char *tmp = new char[str.length() * 2 + 1];
	mysql_real_escape_string(this->con, tmp, str.c_str(), str.length());
	Flux::string retStr(tmp);
	delete [] tmp;
	return retStr;
}
