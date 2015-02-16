#pragma once
#include <cstdio>
#include <iostream>
#include <cstdlib>
#include <cstdint>
#include <map>
#include <vector>
#include <string>
#include <cstring>

// Prevents mysql from typedefing something not used and will throw errros
// this is stupid but it works
#define HAVE_INT32

// Mysql
#include <mysql/my_global.h>
#include <mysql/mysql.h>

// Fixes for mysql-specific defines which we don't use
#undef VERSION
#undef int32

#include "flux.h"
#include "EventDispatcher.h"

//                 field/col    value
typedef std::map<Flux::string, Flux::string> MySQL_Row;
// vector of rows
typedef std::vector<MySQL_Row> MySQL_Result;

class MySQLException : public std::exception
{
	const char *str;
public:
	MySQLException(const Flux::string &mstr)
	{
		// we're crashing, I honestly have bigger issues than memleaks right now.
		this->str = strdup(mstr.c_str());
	}

	~MySQLException()
	{
		// albeit this is very bad, I won't concern myself with it right now.
		delete this->str;
	}

	const char *what() const noexcept
	{
		return this->str;
	}
};

class MySQL
{
	// MySQL connection object
	MYSQL *con;

	// As much as I hate storing these, we must to reconnect on timeouts.
	Flux::string user, pass, host, db;

	// MySQL server's port.
	short int port;

	// Used to connect to the database
	bool DoConnection();
public:

	MySQL(const Flux::string &host, const Flux::string &user, const Flux::string &pass, const Flux::string &db, short);
	~MySQL();

	// Run a query
	MySQL_Result Query(const Flux::string &query);

	// Check the connection to the database.
	bool CheckConnection();

	// Escape a string
	Flux::string Escape(const Flux::string &str);

	Event<Flux::string, MySQL_Result> OnQuery;
};
