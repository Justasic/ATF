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
#include "dns.h"
#include "module.h"
#include "Config.h"

Flux::insensitive_map<Network*> Network::Networks;
bool quitting = 0;

Network::Network(const Flux::string &host, const Flux::string &p, const Flux::string &n): RTimer(nullptr),
hostname(host), port(p), CurHost(0)
{
	if (host.empty() || p.empty())
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

	for (auto it : this->Bots)
	{
		it.second->Disconnect("Network Object being destroyed");
		delete it.second;
	}

	if (RTimer)
		delete RTimer;
	Networks.erase(this->name);
}

#if 0
bool Network::Disconnect()
{
	// say this network is disconnecting
	this->disconnecting = true;
	// return that we did something
	return true;
}

bool Network::Disconnect(const Flux::string &buf)
{
	if (!buf.empty() && this->s)
		this->proto.quit(buf);
	this->Disconnect();
	return true;
}


bool Network::IsSynced() const
{
	return this->issynced && this->s && this->s->GetStatus(SF_CONNECTED);
}

void Network::Sync()
{
	// 	if (this->isupport.UserModes.search('B'))
	// 		this->proto->SetMode("+B"); //FIXME: get bot mode?
	// 	if (this->isupport.IRCdVersion.search_ci("ircd-seven") && this->isupport.UserModes.search('Q'))
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
	// 	FOREACH_MOD(I_OnNetworkSync, OnNetworkSync(this));
	Log(LOG_DEBUG) << "Network " << this->name << " is synced!";
}

#endif



// bool Network::Connect()
// {
// 	this->disconnecting = false;
// 	// ###: Does ANT load channels it was in?
// // 	EventResult e;
// // 	FOREACH_RESULT(I_OnPreConnect, OnPreConnect(this), e);
// // 	if (e != EVENT_CONTINUE)
// // 		return false;
//
// 	if (!this->s)
// 		this->s = new NetworkSocket(this);
// 	return true;
// }

User *Network::FindUser(const Flux::string &fnick)
{
	auto it = this->UserNickList.find(fnick);
	if (it != this->UserNickList.end())
		return it->second;
	return nullptr;
}

Channel *Network::FindChannel(const Flux::string &channel)
{
	auto it = this->ChanMap.find(channel);
	if (it != this->ChanMap.end())
		return it->second;
	return NULL;
}

Network *FindNetwork(const Flux::string &name)
{
	auto it = Network::Networks.find(name);
	if (it != Network::Networks.end())
		return it->second;
	return nullptr;
}

Network *FindNetworkByHost(const Flux::string &name)
{
	for (auto it : Network::Networks)
	{
		Network *n = it.second;
		if (n->hostname == name)
			return n;
	}
	return nullptr;
}



ReconnectTimer::ReconnectTimer(int wait, Network *net) : Timer(wait), n(net)
{
	if (!net)
		return; // Just ignore, we might be exiting from a CoreException

		n->RTimer = this;
}

void ReconnectTimer::Tick(time_t)
{
	Log(LOG_TERMINAL) << "JoinQueue.size() = " << n->JoinQueue.size();
	try
	{
		if (!quitting)
		{
			for(auto it : n->ChanMap)
				n->JoinQueue.push_back(it.second);
// 			n->Connect();
		}
	}
	catch (const SocketException &e)
	{
		Log() << "Connection to " << n->name << " [" << n->GetConHost() << ':'
		<< n->port << "] Failed! (" << e.GetReason() << ") Retrying in " << config->RetryWait << " seconds.";

		new ReconnectTimer(config->RetryWait, n);
		return;
	}
	n->RTimer = nullptr;
}
