/* Arbitrary Navn Tool -- Channel Handler and functions
 *
 * (C) 2011-2012 Azuru
 * Contact us at Development@Azuru.net
 *
 * Please read COPYING and README for further details.
 *
 * Based on the original code of CIA.vc by Micah Dowty
 * Based on the original code of Anope by The Anope Team.
 */
#include "flux.h"
#include "channel.h"
#include "network.h"
#include "user.h"

std::map<Channel*, std::vector<User*>> UChanMap;
// std::map<User*, std::vector<Channel*>> CUserMap;
Channel::Channel(Network *net, const Flux::string &nname): name(nname), n(net), topic_time(0), creation_time(0)
{
	if(nname.empty())
		throw CoreException("I don't like empty channel names in my channel constructor >:d");
	if(!net)
		throw CoreException("Channel \""+nname+"\" created with no network!");
	if(!net->IsValidChannel(nname))
		throw CoreException("An Invalid channel was passed into the Channel constructor :<");

	this->n->ChanMap[this->name] = this;
	UChanMap[this] = std::vector<User*>();
	Log(LOG_DEBUG) << "Created new channel '" << nname << "' on " << net->name;
}

Channel::~Channel()
{
	// Erase ourselves from the map.
	UChanMap.erase(this);
	// Iterate through the users
	for(auto it : CUserMap)
	{
		// iterate through the user's channels and see if we're in there, if we are then remove us.
		for(std::vector<Channel*>::iterator it2 = it.second.begin(), it2_end = it.second.end(); it2 != it2_end; ++it2)
			if((*it2) == this)
				it.second.erase(it2);
	}
	Log(LOG_DEBUG) << "Deleted channel " << this->name;
	this->n->ChanMap.erase(this->name);
}

User *Channel::FindUser(const Flux::string &usr)
{
	for(auto it : UChanMap)
	if(it.first == this)
	{
		for(auto it2 : it.second)
			if(it2->nick.equals_ci(usr))
				return it2;
	}
	else
		throw CoreException("Channel "+this->name+" is not in UChanMap? major WTF!");
	return nullptr;
}

void Channel::SendJoin()
{
	if(!this->n)
		return; // incase we have some weird shit

	this->n->proto.join(this->name);
	this->SendWho();
	this->n->s->Write("MODE %s", this->name.c_str());
}

void Channel::AddUser(User *u)
{
	if(u)
		UChanMap[this].push_back(u);
}

void Channel::DelUser(User *u)
{
	// I fucking hate this code! - Justasic
	if(u)
	{
		for(auto it : UChanMap)
		{
			if(it.first == this)
				for(std::vector<User*>::iterator it2 = it.second.begin(), it2_end = it.second.end(); it2 != it2_end; ++it2)
					if((*it2) == u)
						UChanMap[this].erase(it2);
		}
    }
}

void Channel::SendPart(const Flux::string &reason)
{
	this->n->proto.part(this->name, reason);
}

void Channel::SendMessage(const Flux::string &message)
{
	this->n->proto.privmsg(this->name, message);
}

void Channel::SendAction(const Flux::string &message)
{
	this->n->proto.action(this->name, message);
}

void Channel::SendNotice(const Flux::string &message)
{
	this->n->proto.notice(this->name, message);
}

void Channel::SendWho()
{
	if(!this->n)
		return; // incase we have some weird shit

	this->n->proto.who(this->name);
}
/****************************************************************/
// void QuitUser(Network *n, User *u)
// {
//     if(!u)
// 	return;
//     for(auto var : n->ChanMap)
// 	for(auto var1 : var.second->UserList)
// 	    if(var1.first == u)
// 		var1.second->DelUser(u);
//     delete u;
// }

