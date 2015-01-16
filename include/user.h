/* Arbitrary Navn Tool -- User Prototypes
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
#include "ircproto.h"
#include "tinyformat.h"
#include <vector>

class Channel;

class User
{
	friend class Channel;
public:
	User(Network*, const Flux::string&, const Flux::string&, const Flux::string&, const Flux::string &realname = "", const Flux::string &server ="");
	virtual ~User();

	Flux::string nick;
	Flux::string host;
	Flux::string realname;
	Flux::string ident;
	Flux::string fullhost;
	Flux::string server;
	Network *n;

	// Functions
	void AddChan(Channel*);
	void DelChan(Channel*);
	Channel *FindChannel(const Flux::string&);
	void SendWho();
	virtual void SetNewNick(const Flux::string&);
	virtual void SendMessage(const Flux::string&);
	virtual void SendPrivmsg(const Flux::string&);

	template<typename... Args> void SendMessage(const Flux::string &fmt, const Args&... args) { this->SendMessage(tfm::format(fmt, args...)); }
	template<typename... Args> void SendPrivmsg(const Flux::string &fmt, const Args&... args) { this->SendPrivmsg(tfm::format(fmt, args...)); }
};

extern std::map<User*, std::vector<Channel*>> CUserMap;