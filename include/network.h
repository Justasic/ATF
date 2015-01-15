/* Arbitrary Navn Tool -- Network Class Prototype
 *
 * (C) 2011-2012 Azuru
 * Contact us at Development@Azuru.net
 *
 * Please read COPYING and README for further details.
 *
 * Based on the original code of CIA.vc by Micah Dowty
 * Based on the original code of Anope by The Anope Team.
 */
#pragma once
#include "Socket.h"
#include "timers.h"
#include "flux.h"
#include "channel.h"

// class Bot;
class NetworkSocket;
class ReconnectTimer;

struct CommitMessage
{
    Flux::map<Flux::string> info;
    Flux::vector Files;
    std::vector<Channel*> Channels;
    Network *network;
};

struct iSupport
{
    // Other options the server might send.
    Flux::map<Flux::string> other;
    // Supported chan modes
    Flux::string ChanModes;
    // Supported chan types
    Flux::string ChanTypes;
    // The IRCd version
    Flux::string IRCdVersion;
    // User modes
    Flux::string UserModes;
    // Network name
    Flux::string Network;
    // Servers hostname
    Flux::string ServerHost;
    // Max away length
    int AwayLen;
    // Max kick length
    int KickLen;
    // Max Channel length
    int ChannelLen;
    // Max channels you can join
    int MaxChannels;
    // Max number of bans settable
    int MaxBans;
    // Max nickname length
    int NickLen;
    // Max Topic Length
    int TopicLen;
};

class Network
{
protected:
	bool disconnecting, issynced;
	Flux::string usedhostname;
public:
	Network(const Flux::string&, const Flux::string&, const Flux::string &n = "");
	~Network();
	// ReconnectTimer if we have one
	ReconnectTimer *RTimer;
	// The socket
	NetworkSocket *s;
	// What that network supports
	iSupport isupport;
	// When we join a network but aren't synced yet
	std::deque<Channel*> JoinQueue;
	// bot pointer for the network
// 	Bot *b;
	IRCProto proto;
	// Map of all users in the network
	Flux::insensitive_map<User*> UserNickList;
	// Map of all channels in the network
	Flux::insensitive_map<Channel*> ChanMap;
	// Network name
	Flux::string name;
	// Network hostname currently in use
	Flux::string hostname;
	// Usable hostnames
	std::map<int, Flux::string> hostnames;
	// Port to connect to
	Flux::string port;
	// name of the server we're connected to (ie. pulsar.azuru.net)
	Flux::string servername;
	int CurHost;

	// All the networks within the application
	static Flux::insensitive_map<Network*> Networks;

	// Check if that network accepts that chan type.
	inline bool IsValidChannel(const Flux::string &chan)
	{
		for(unsigned i = 0; i < isupport.ChanTypes.size(); ++i)
		{
			char ch = isupport.ChanTypes[i];
			if (chan[0] != ch)
				return false;
		}
		return true;
	}

	// Sync this network, make sure all channels are joined, etc.
	void Sync();
	// Has the channel been synced yet?
	bool IsSynced() const;
	// Set the current connected hostname
	inline void SetConnectedHostname(const Flux::string &str) { this->usedhostname = str; }
	// Get the current connected hostname
	inline Flux::string GetConHost() const { return this->usedhostname; }
	// Check if this network is disconnecting
	inline bool IsDisconnecting() { return this->disconnecting; }
	// Find a channel inside the network
	Channel *FindChannel(const Flux::string&);
	// Find a user inside the network
	User *FindUser(const Flux::string &fnick);
	// Join a channel, if there is no connection, queue the channel for join
	bool JoinChannel(const Flux::string &chan);
	// Disconnect from this network
	bool Disconnect();
	// Same as above except with a message
	bool Disconnect(const Flux::string&);
	template<typename... Args>
	bool Disconnect(const Flux::string &fmt, const Args&... args)
	{
		this->Disconnect(tfm::format(fmt, args...));
	}
	// Connect to the network
	bool Connect();
};

class ReconnectTimer : public Timer
{
	Network *n;
public:
	ReconnectTimer(int, Network*);
	void Tick(time_t);
};

class NetworkSocket : public ConnectionSocket, public BufferedSocket
{
public:
	NetworkSocket(Network*);
	~NetworkSocket();
	Network *net;
	int pings;
	bool Read(const Flux::string&);
	bool ProcessWrite();
	void OnConnect();
	void OnError(const Flux::string&);
};
