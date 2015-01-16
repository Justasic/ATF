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

// 		// Set the status to 200 OK
// 		r.SetStatus(200);
//
// 		// Call an event, this will later be used by plugins
// 		// once they register with the handler.
// 		//OnRequest.CallVoidEvent("REQUEST", r, r.GetParam("SCRIPT_NAME"));
//
// 		// Form the HTTP header enough, nginx will fill in the rest.
// 		r.Write("Content-Type: text/html\r\n\r\n");
//
// 		// Send our message
//
// 		r.Write("<html><head>");
// 		r.Write("<title>Example Test thingy</title>");
// 		r.Write("</head>");
//
// 		r.Write("<h2>%s thread %d</h2>", r.GetParam("QUERY_STRING"), ThreadHandler::GetThreadID());
//
// 		r.Write("<p>%s<br/>", r.GetParam("REMOTE_ADDR"));
// 		r.Write("%s<br/>", r.GetParam("REQUEST_URI"));
// 		r.Write("%s<br/>", r.GetParam("SERVER_PROTOCOL"));
// 		r.Write("%s<br/>", r.GetParam("REQUEST_METHOD"));
// 		r.Write("%s<br/>", r.GetParam("REMOTE_PORT"));
// 		r.Write("%s<br/>", r.GetParam("SCRIPT_NAME"));
// 		r.Write("</p>");
// 		r.Write("<br /><br /><br /><h2>MySQL Query:</h2><br />");
//
// 		printf("Running MySQL Query...\n");
// 		// Test Query
// 		try {
// 			MySQL_Result mr = ms->Query("SELECT * from " + ms->Escape("testtbl"));
// 			for (auto it : mr.rows)
// 			{
// 				for (int i = 0; i < mr.fields; ++i)
// 					r.Write("%s ", it.second[i] ? it.second[i] : "(NULL)");
// 				r.Write("<br/>");
// 			}
// 			printf("%lu rows with %d columns\n", mr.rows.size(), mr.fields);
// 		}
// 		catch(const MySQLException &e)
// 		{
// 			printf("MySQL error: %s\n", e.what());
// 			r.Write("<p>MySQL error: %s</p><br/>", e.what());
// 		}

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
#if 0
			// Attempt a regex match on our url
			std::smatch m;
			std::regex_search(url, m, std::regex(it.first));

			// we found one, run the page and let the page respond
			if (!m.empty())
				it.second.Run(r, url);
#endif
		}

		// Nope. best say 404 and let the web server handle it.
		if (!foundmatch)
			r.SetStatus(404);

// 		r.Write("</html>");

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
