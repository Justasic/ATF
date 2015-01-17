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
class Network;

class User
{
	friend class Channel;
public:
	User(Network*, const Flux::string &nick, const Flux::string &ident, const Flux::string &host, const Flux::string &realname = "", const Flux::string &server = "");
	virtual ~User();

	Flux::string nickname;
	Flux::string hostname;
	Flux::string realname;
	Flux::string ident;
	Flux::string fullhost;
	Flux::string server;
	Network *n;

	// Functions
	void AddChan(Channel*);
	void DelChan(Channel*);
	Channel *FindChannel(const Flux::string&);
	virtual void SetNewNick(const Flux::string&);

	// NOTE: To send a message to a user you must send them via
	// the bot object. This is simply to keep track of users.
	// Bots will accept the User object as a message destination.
	// This will allow the proper bot to reply or message the
	// user instead of a random bot chosen.
};

extern std::map<User*, std::vector<Channel*>> CUserMap;
