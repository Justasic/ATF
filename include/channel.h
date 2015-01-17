/* Arbitrary Navn Tool -- Prototype for Channel class
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
#include "command.h"
#include "tinyformat.h"
// Forward declare the Network class
class Network;
class User;

class Channel
{
	friend class User;
public:
	Channel(Network*, const Flux::string&);
	~Channel();

	// The network this channel is attached to
	Network *n;
	// Various informational bits
	time_t topic_time;
	time_t creation_time;
	Flux::string name;
	Flux::string topic;
	Flux::string topic_setter;
	Flux::string modes;

	// Find a user inside the channel
	User *FindUser(const Flux::string&);

	// Add or delete a user from the channel
	void AddUser(User*);
	void DelUser(User*);
};

extern std::map<Channel*, std::vector<User*>> UChanMap;
