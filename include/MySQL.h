#pragma once
#include <cstdio>
#include <iostream>
#include <cstdlib>
#include <cstdint>
#include <map>
#include <vector>
#include <string>
#include <cstring>

#include "flux.h"
#include "EventDispatcher.h"

//                 field/col    value
typedef std::map<Flux::string, Flux::string> MySQL_Row;
// vector of rows
typedef std::vector<MySQL_Row> MySQL_Result;

class MySQLException : public std::exception
{
	const char *str;
	const char *query;
public:
	MySQLException(const Flux::string &mstr, const Flux::string &query = "")
	{
		// we're crashing, I honestly have bigger issues than memleaks right now.
		this->str = strdup(mstr.c_str());
		this->query = strdup(query.c_str());
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

	const char *Query() const noexcept
	{
		return this->query;
	}

};

class MySQL
{
	// Because including my_global.h defines a bunch of shit we don't fucking want
	// (and thanks MySQL for making it impossible to disable these generically-named global items)
	// we must rely upon the linker to resolve this type to the proper pointer type.
	// it is only used internally in MySQL.cpp for actual connection info and this just defines
	// the C++ type but this means that this class is the ONLY way you can interact with MySQL.
	// Boy do I love working around shittily programmed libraries.
	typedef struct st_mysql MYSQL;

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
