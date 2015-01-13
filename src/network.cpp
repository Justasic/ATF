/* Arbitrary Navn Tool -- Network class and NetworkSocket
 *
 * (C) 2011-2012 Azuru
 * Contact us at Development@Azuru.net
 *
 * Please read COPYING and README for further details.
 *
 * Based on the original code of CIA.vc by Micah Dowty
 * Based on the original code of Anope by The Anope Team.
 */

#include "network.h"
#include "bot.h"
#include "dns.h"
#include "module.h"

Flux::insensitive_map<Network*> Network::Networks;

Network::Network(const Flux::string &host, const Flux::string &p, const Flux::string &n): disconnecting(false),
issynced(false), RTimer(nullptr), s(nullptr), proto(this), hostname(host), port(p), CurHost(0)
{
	if(host.empty() || p.empty())
		throw CoreException("Network class created with incorrect parameters given");

	//If we didn't specify the network name, use the hostname.
	this->name = n.empty() ? host : n;

	// TODO: Non-Blocking queries are a must
	DNSQuery rep = DNSManager::BlockingQuery(host, host.search(':') ? DNS_QUERY_AAAA : DNS_QUERY_A);
	this->hostnames[1] = !rep.answers.empty() ? rep.answers.front().rdata : host;
	Networks[this->name] = this;

	Log(LOG_DEBUG) << "New network created: " << n << " " << host << ':' << p;
}

Network::~Network()
{
	Log(LOG_DEBUG) << "Deleting network " << this->name << " (" << this->hostname << ':' << this->port << ')';
	this->Disconnect("Network Removed");
	if(RTimer)
		delete RTimer;
	Networks.erase(this->name);
}

bool Network::JoinChannel(const Flux::string &chan)
{
	Log(LOG_DEBUG) << "Scheduling Channel " << chan << " for join.";
	if(IsValidChannel(chan))
	{
		Channel *c = this->FindChannel(chan);
		if(!c)
			c = new Channel(this, chan);

		if(!this->s || !this->s->GetStatus(SF_CONNECTED))
		{
			for(auto it : this->JoinQueue)
			{
				if(it == c)
				{
					Log(LOG_WARN) << "Channel " << c->name << " already in queue to join network " << c->n->name << "! ignoring..";
					return true;
				}
				else
					this->JoinQueue.push_back(c);
			}
		}
		else
			c->SendJoin();

		return true;
	}
	return false;
}

bool Network::Disconnect()
{
	// Check if we have a socket to send on
	if(!this->s)
		return false;
	// Delete the bot object
	if(this->proto)
	{
		delete this->proto;
		this->proto = nullptr;
	}
	// We'll let the socket engine delete the socket
	this->s->SetDead(true);
	// say this network is disconnecting
	this->disconnecting = true;
	// return that we did something
	return true;
}

bool Network::Disconnect(const char *fmt, ...)
{
	va_list args;
	char buffer[BUFSIZE] = "";
	if(fmt)
	{
		va_start(args, fmt);
		vsnprintf(buffer, sizeof(buffer), fmt, args);
		this->Disconnect(Flux::string(buffer));
		va_end(args);
	}
	return true;
}

bool Network::Disconnect(const Flux::string &buf)
{
	if(!buf.empty() && this->s && this->proto)
		this->proto->quit(buf);
	this->Disconnect();
	return true;
}

bool Network::IsSynced() const
{
	return this->issynced && this->s && this->s->GetStatus(SF_CONNECTED);
}

void Network::Sync()
{
// 	if(this->isupport.UserModes.search('B'))
// 		this->proto->SetMode("+B"); //FIXME: get bot mode?
// 	if(this->isupport.IRCdVersion.search_ci("ircd-seven") && this->isupport.UserModes.search('Q'))
// 		this->proto->SetMode("+Q"); //for freenode to stop those redirects

	// Join pending channels
	while(!this->JoinQueue.empty())
	{
		Channel *c = this->JoinQueue.front();
		this->JoinQueue.pop_front();
		Log(LOG_DEBUG) << "Joining " << c->name << " (" << this->name << ')';
		c->SendJoin();
	}

	this->servername = this->isupport.ServerHost;
	this->issynced = true;
	FOREACH_MOD(I_OnNetworkSync, OnNetworkSync(this));
	Log(LOG_DEBUG) << "Network " << this->name << " is synced!";
}


NetworkSocket::NetworkSocket(Network *tnet) : Socket(-1), ConnectionSocket(), BufferedSocket(), net(tnet), pings(0)
{
	if(!tnet)
		throw CoreException("Network socket created with no network? lolwut?");

	this->net->CurHost++;
	if(static_cast<unsigned int>(this->net->CurHost) >= this->net->hostnames.size())
		this->net->CurHost = 1;

	this->net->SetConnectedHostname(this->net->hostnames[this->net->CurHost]);

	Log(LOG_TERMINAL) << "New Network Socket for " << tnet->name << " connecting to "
	<< tnet->hostname << ':' << tnet->port << '(' << tnet->GetConHost() << ')';

	this->Connect(tnet->GetConHost(), tnet->port);
}

NetworkSocket::~NetworkSocket()
{
	this->Write("QUIT :Socket Closed\n");
	this->ProcessWrite();
	this->net->s = nullptr;

	Log() << "Closing Connection to " << net->name;

	if(!this->net->IsDisconnecting())
	{
		Log() << "Connection to " << net->name << " [" << net->GetConHost() << ':'
		<< net->port << "] Failed! Retrying in " << Config->RetryWait << " seconds.";

		new ReconnectTimer(Config->RetryWait, this->net);
	}
}

bool NetworkSocket::Read(const Flux::string &buf)
{
	// since BufferedSocket was modified to accommodate stupid HTTP, this code is required.
	if(buf.empty())
		return true;

	Log(LOG_RAWIO) << '[' << this->net->name << "] " << FixBuffer(buf);
	Flux::vector params = ParamitizeString(buf, ' ');

	if(!params.empty() && params[0].search_ci("ERROR"))
	{
		FOREACH_MOD(I_OnSocketError, OnSocketError(buf));
		Log(LOG_TERMINAL) << "Socket Error, Closing socket!";
		return false; //Socket is dead so we'll let the socket engine handle it
	}

	process(this->net, buf);

	if(!params.empty() && params[0].equals_ci("PING"))
	{
		this->Write("PONG :"+params[1]);
		this->ProcessWrite();
	}
	return true;
}

void NetworkSocket::OnConnect()
{
// 	Log(LOG_TERMINAL) << "Successfully connected to " << this->net->name << " ["
// 	<< this->net->hostname << ':' << this->net->port << "] (" << this->net->GetConHost() << ")";
//
// 	new Bot(this->net, printfify("%stmp%03d", Config->NicknamePrefix.strip('-').c_str(),
// 								 randint(0, 999)), Config->Ident, Config->Realname);
//
// 	this->net->b->introduce();
// 	FOREACH_MOD(I_OnPostConnect, OnPostConnect(this, this->net));
// 	this->ProcessWrite();
}

void NetworkSocket::OnError(const Flux::string &buf)
{
	Log(LOG_TERMINAL) << "Unable to connect to " << this->net->name << " ("
	<< this->net->hostname << ':' << this->net->port << ')' << (!buf.empty()?(": " + buf):"");
}

bool NetworkSocket::ProcessWrite()
{
	Log(LOG_RAWIO) << '[' << this->net->name << ']' << ' ' << FixBuffer(this->WriteBuffer);
	return ConnectionSocket::ProcessWrite() && BufferedSocket::ProcessWrite();
}


bool Network::Connect()
{
	this->disconnecting = false;
	// ###: Does ANT load channels it was in?
	EventResult e;
	FOREACH_RESULT(I_OnPreConnect, OnPreConnect(this), e);
	if(e != EVENT_CONTINUE)
		return false;

	if(!this->s)
		this->s = new NetworkSocket(this);
	return true;
}

User *Network::FindUser(const Flux::string &fnick)
{
	auto it = this->UserNickList.find(fnick);
	if(it != this->UserNickList.end())
		return it->second;
	return nullptr;
}

Channel *Network::FindChannel(const Flux::string &channel)
{
	auto it = this->ChanMap.find(channel);
	if(it != this->ChanMap.end())
		return it->second;
	return NULL;
}

Network *FindNetwork(const Flux::string &name)
{
	auto it = Networks.find(name);
	if(it != Networks.end())
		return it->second;
	return nullptr;
}

Network *FindNetworkByHost(const Flux::string &name)
{
	for (auto it : Networks)
	{
		Network *n = it.second;
		if (n->hostname == name)
			return n;
	}
	return nullptr;
}



ReconnectTimer::ReconnectTimer(int wait, Network *net) : Timer(wait), n(net)
{
	if(!net)
		return; // Just ignore, we might be exiting from a CoreException

		n->RTimer = this;
}

void ReconnectTimer::Tick(time_t)
{
	Log(LOG_TERMINAL) << "JoinQueue.size() = " << n->JoinQueue.size();
	try
	{
		if(!quitting)
		{
			for(auto it : n->ChanMap)
				n->JoinQueue.push_back(it.second);
			n->Connect();
		}
	}
	catch (const SocketException &e)
	{
		n->s = nullptr; // ###: Does this memleak?
		Log() << "Connection to " << n->name << " [" << n->GetConHost() << ':'
		<< n->port << "] Failed! (" << e.GetReason() << ") Retrying in " << Config->RetryWait << " seconds.";

		new ReconnectTimer(Config->RetryWait, n);
		return;
	}
	n->RTimer = nullptr;
}
