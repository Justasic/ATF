#include <cstdio>
#include <string>
#include <cstring>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <cstdlib>
#include <future>
#include <cerrno>
#include <atomic>
#include <regex>

// Our thread engine.
#include "ThreadEngine.h"
#include "EventDispatcher.h"
#include "MySQL.h"
#include "Config.h"
#include "Request.h"
#include "flux.h"
#include "timers.h"
#include "Socket.h"
#include "page.h"

std::atomic_bool quit;

ThreadHandler *threads;
MySQL *ms;
Config *config;
Event<Request, Flux::string> OnRequest;
bool protocoldebug = 1;


void OpenListener(int sock_fd)
{
	printf("Listener spawned for socket %d in thread %d\n", sock_fd, ThreadHandler::GetThreadID());
	FCGX_Request request;
	memset(&request, 0, sizeof(FCGX_Request));

	FCGX_InitRequest(&request, sock_fd, 0);

	// Idle infinitely and accept requests.
	while(FCGX_Accept_r(&request) == 0)
	{
		printf("Thread %d handling request\n", ThreadHandler::GetThreadID());
		Request r(&request);

		// Handle the pages
		extern std::map<std::string, Page*> PageMap;
		// Get the request URI from the server
		std::string url = r.GetParam("SCRIPT_NAME").c_str();
		bool foundmatch = false;
		// Iterate over all our pages
		for (auto it : PageMap)
		{
			// Attempt to completely match a URL and run it
			if (std::regex_match(url, std::regex(it.first)))
			{
				if (it.second->Run(r, url))
					foundmatch = true;
			}
		}

		// Nope. best say 404 and let the web server handle it.
		if (!foundmatch)
			r.SetStatus(404);

		FCGX_Finish_r(&request);
	}

	printf("Exiting listener thread %d\n", ThreadHandler::GetThreadID());

	FCGX_Free(&request, sock_fd);
}

int main(int argc, char **argv)
{
		std::vector<Flux::string> args(argv, argv+argc);
		quit = false;

		// Parse our config before anything
		Config conf("atf.conf");
		config = &conf;

		printf("Config:\n");
		config->Parse();

		ThreadHandler th;
		th.Initialize();
		threads = &th;

		FCGX_Init();
		// Formulate the string from the config.
		std::stringstream val;
		val << config->bind << ":" << config->port;
		// Initialize a new FastCGI socket.
		std::cout << "Opening FastCGI socket: " << val.str() << std::endl;
		int sock_fd = FCGX_OpenSocket(val.str().c_str(), 1024);
		printf("Opened socket fd: %d\n", sock_fd);

		// Initialize MySQL
		MySQL m(config->hostname, config->username, config->password, config->database, config->mysqlport);
		ms = &m;

		for (unsigned int i = 0; i < (th.totalConcurrentThreads * 2) / 2; ++i)
			th.AddQueue(OpenListener, sock_fd);

		printf("Submitting jobs...\n");
		th.Submit();
		// Spawn more threads to compensate for the ones which were consumed.
		th.SpawnMore((th.totalConcurrentThreads * 2) / 2);


		printf("Idling main thread.\n");
		while(!quit)
		{
				// Check database connection first
				ms->CheckConnection();
				// Process reads/writes/errors from the sockets
				SocketEngine::Process();
				// Tick all our timer events, call any which need calling.
				TimerManager::TickTimers(time(NULL));
		}

		printf("Shutting down.\n");
		th.Shutdown();

		return EXIT_SUCCESS;
}
