#pragma once
#include "user.h"
#include "flux.h"
#include "network.h"
#include "EventDispatcher.h"

class Bot : public User, public ConnectionSocket, public BufferedSocket, public IRCProto
{
	bool disconnecting, issynced;
	int botid;
	std::vector<Channel*> subscribed;
public:
	Bot(Network *n, const Flux::string &modes);
	~Bot();

	// Sync this network, make sure all channels are joined, etc.
	void Sync();
	// Has the channel been synced yet?
	bool IsSynced() const;
	// Check all bots in a network
	static void CheckBots();
	// When a user messages a bot it is handled here
	void OnMessage(User *u, const Flux::string &msg);
	// Ensure the bot is still connected and has the correct nickname.
	void CheckBot();
	// Check if this network is disconnecting
	inline bool IsDisconnecting() { return this->disconnecting; }
	// Handle nick changes or set a new nickname.
	void SetNewNick(const Flux::string&) override;
	// Find a bot that is in the channel specified
	static Bot *FindBotInChannel(Channel *c);


	// Subscribe to a channel to announce commits in.
	void Subscribe(Channel *c);
	// Check if a bot is subscribed to a channel
	bool IsSubscribed(Channel *c);

	// Things used to control the flow of the bot
	void Join(Channel *c);
	void Part(Channel *c, const Flux::string &msg);
	// Message sending functions
	template<typename... Args> void Message(Channel *c, const Flux::string &fmt, const Args&... args) { this->Message(c, tfm::format(fmt, args...)); }
	template<typename... Args> void Message(User *u, const Flux::string &fmt, const Args&... args) { this->Message(u, tfm::format(fmt, args...)); }
	void Message(Channel *c, const Flux::string &msg);
	void Message(User *u, const Flux::string &msg);

	template<typename... Args> void Notice(Channel *c, const Flux::string &fmt, const Args&... args) { this->Notice(c, tfm::format(fmt, args...)); }
	template<typename... Args> void Notice(User *u, const Flux::string &fmt, const Args&... args) { this->Notice(u, tfm::format(fmt, args...)); }
	void Notice(Channel *c, const Flux::string &msg);
	void Notice(User *u, const Flux::string &msg);

	// Connect to the network
	bool Connect();
	// Disconnect from this network
	bool Disconnect();
	// Same as above except with a message
	bool Disconnect(const Flux::string&);
	template<typename... Args>
	bool Disconnect(const Flux::string &fmt, const Args&... args)
	{
		this->Disconnect(tfm::format(fmt, args...));
	}

	// Socket code
	int pings;
	bool Read(const Flux::string&);
	bool ProcessWrite();
	void OnConnect();
	void OnError(const Flux::string&);


	/////////////////////////////////////////////////////////////////////////////
	// Network Events                                                          //
	/////////////////////////////////////////////////////////////////////////////
	Event<const Flux::string&, const Flux::vector&>                  OnCommand;
	Event<User*, const Flux::vector&>                                OnPrivmsg;
	Event<User*, Channel*, const Flux::vector&>                      OnChanmsg;
	Event<const Flux::string&>                                       OnPreReceiveMessage;
	Event<const Flux::string&, const Flux::vector&, Network*>        OnCTCP;
	Event<User*, Channel*, const Flux::vector&>                      OnChannelAction;
	Event<User*, const Flux::vector&>                                OnAction;
	Event<User*, const Flux::string&>                                OnPreNickChange;
	Event<User*>                                                     OnNickChange;
	Event<User*, const Flux::string&>                                OnQuit;
	Event<User*, Channel*, const Flux::string&>                      OnPart;
	Event<const Flux::vector&, Network*>                             OnPing;
	Event<const Flux::vector&, Network*>                             OnPong;
	Event<User*, User*, Channel*, const Flux::string&>               OnKick;
	Event<const Flux::string&>                                       OnConnectionError;
	Event<User*, const Flux::string&>                                OnInvite;
	Event<User*, const Flux::vector&>                                OnNotice;
	Event<User*, Channel*, const Flux::vector&>                      OnChannelNotice;
	Event<User*, Channel*, const Flux::string&>                      OnChannelMode;
	Event<User*, Channel*, const Flux::string&, const Flux::string&> OnChannelOp;
	Event<User*, const Flux::string&, const Flux::string&>           OnUserMode;
	Event<User*, Channel*>                                           OnJoin;
	Event<Network*>                                                  OnUserRegister;
	Event<int, Network*, const Flux::vector&>                        OnNumeric;
};
