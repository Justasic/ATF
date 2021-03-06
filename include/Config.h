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

	//////////////////////////////////////////////////////////
	// General server/daemon-related items
	bool daemonize;
	int SockWait;
	Flux::string pidfile;

	//////////////////////////////////////////////////////////
	// MySQL authentication and connection information
	Flux::string username;
	Flux::string password;
	Flux::string database;
	Flux::string hostname;
	short int mysqlport;

	//////////////////////////////////////////////////////////
	// Web front end related items -- part of server section in config.
	Flux::string bind;
	short int port;

	//////////////////////////////////////////////////////////
	// IRC-socket related variables
	int SendQLines;
	bool SendQEnabled;
	int BurstRate;
	int SendQRate;
	int RetryWait; // How long we wait until we retry connecting again.
	Flux::string BotPrefix;
	Flux::string BotIdent;
	Flux::string BotRealname;

	//////////////////////////////////////////////////////////
	// DNS resolver related variables
	int DNSTimeout;
	Flux::string NameServer;

	/////////////////////////////////////////////////////////
	// Log related variables
	time_t LogAge;
	Flux::string LogFile;
	Flux::string LogColor;

	/////////////////////////////////////////////////////////
	// Module related variables
	Flux::vector Modules;
	Flux::string ModuleDir;

	// Call an event.
	Event<Config*> OnRehash;
};

extern Config *config;
