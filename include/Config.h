#pragma once
#include <map>
#include <vector>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <cstdint>
#include "flux.h"

#include "EventDispatcher.h"

// Forward declare for the private parser variable
namespace yy {
	class Parser;
};

class Config
{
	const Flux::string filepath;
	// The Bison++ parser class
	yy::Parser *p;
public:
	Config(const Flux::string &fp);
	~Config();

	// Parse the config (again)
	int Parse();

	// Are we daemonizing?
	bool daemonize;

	// PID file path
	Flux::string pidfile;

	// MySQL authentication and connection information
	Flux::string username;
	Flux::string password;
	Flux::string database;
	Flux::string hostname;
	short int mysqlport;

	// Port and interface to bind to
	Flux::string bind;
	short int port;

	//////////////////////////////////////////////////////////
	// IRC-socket related variables
	int SockWait;
	int SendQLines;
	bool SendQEnabled;
	int BurstRate;
	int SendQRate;

	//////////////////////////////////////////////////////////
	// DNS resolver related variables
	int DNSTimeout;
	Flux::string NameServer;

	/////////////////////////////////////////////////////////
	// Log related variables
	time_t LogAge;
	Flux::string LogFile;
	Flux::string LogColor;

	// Call an event.
	Event<Config*> OnRehash;
};

extern Config *config;
