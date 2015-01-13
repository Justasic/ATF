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
	// Join a channel
	void SendJoin();
	// Send /who to the whole channel
	void SendWho();
	// Part a channel
	void SendPart(const Flux::string &msg = "");
	template<typename... Args>
	void SendPart(const Flux::string &fmt, const Args&... args)
	{
		this->SendPart(tfm::format(fmt, args...));
	}
	// Send a message
	void SendMessage(const Flux::string&);
	template<typename... Args>
	void SendMessage(const Flux::string &fmt, const Args&... args)
	{
		this->SendMessage(tfm::format(fmt, args...));
	}
	// Send a /me to the channel
	void SendAction(const Flux::string&);
	template<typename... Args>
	void SendAction(const Flux::string &fmt, const Args&... args)
	{
		this->SendAction(tfm::format(fmt, args...));
	}
	// Send a /notice to the channel
	void SendNotice(const Flux::string&);
	template<typename... Args>
	void SendNotice(const Flux::string &fmt, const Args&... args)
	{
		this->SendNotice(tfm::format(fmt, args...));
	}
};
