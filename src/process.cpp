/* Arbitrary Navn Tool -- Main IRC Parsing Routines
 *
 * (C) 2011-2012 Azuru
 * Contact us at Development@Azuru.net
 *
 * Please read COPYING and README for further details.
 *
 * Based on the original code of CIA.vc by Micah Dowty
 * Based on the original code of Anope by The Anope Team.
 */

// Abandon all hope, ye who enter here!
// below here be scary parsing related things!

#include "user.h"
#include "module.h"
#include "command.h"
#include "misc.h"
#include "bots.h"
/**
 * \fn void ProcessJoin(CommandSource &source, const Flux::string &chan)
 * \brief Processes the /who numeric (352), this should only be used in Process() unless its for something special
 * \param source CommandSource struct used to find all the information needed to make new users
 * \param chan The channel we're processing
 */

void ProcessJoin(CommandSource &source, const Flux::string &chan)
{
	Flux::vector &params = source.params;
	if (params.size() < 8)
		return;

	Flux::string channel = params[1];
	Flux::string Ident = params[2];
	Flux::string Host = params[3];
	Flux::string Server = params[4];
	Flux::string Nickname = params[5];
	Flux::string flags = params[6];
	Flux::string hops = params[7].substr(0,1);
	Flux::string realname = params[7].erase(0,2);
	/*******************************************************/
	User *u = source.b->n->FindUser(Nickname);
	if (!u)
	{
		if ((!Host.empty() || !Nickname.empty() || !Ident.empty()) && source.b)
			u = new User(source.b->n, Nickname, Ident, Host, realname, Server);
	}

	Channel *c = source.b->n->FindChannel(channel);
	if (!c)
	{
		if (!channel.empty() && source.b->n)
			c = new Channel(source.b->n, channel);
	}

	if (u)
		u->AddChan(c);
	if (c)
		c->AddUser(u);
}
/*********************************************************************************/
/**
 * \fn void ProcessCommand(CommandSource &Source, std::vector<Flux::string> &params2, const Flux::string &receiver, const Flux::string &command)
 * \brief Processes the Command class commands, this should only be used in Process() unless its for something special
 * \param CommandSource CommandSource struct used to find all the information needed to make new users
 * \param std::vector vector of the command string sent to the bot
 * \param Flux::string receiver
 * \param Flux::string Command sent to the bot in raw form
 */
void ProcessCommand(CommandSource &Source, Flux::vector &params2, const Flux::string &receiver, const Flux::string &command)
{
	User *u = Source.u;
	Channel *c = Source.c;

	if (!command.is_pos_number_only())
	{
		Source.b->OnCommand(command, params2);
// 		FOREACH_MOD(I_OnCommand, OnCommand(command, params2));
	}

	if (!FindCommand(params2[0], C_PRIVATE) && command == "PRIVMSG")
	{
// 		if (!protocoldebug)
			Log(LOG_TERMINAL) << '<' << u->nickname << '-' << receiver << "> " << Source.params[1];

		if (!Source.b->n->IsValidChannel(receiver))
		{
			Source.Reply("Unknown command \2%s\2", Flux::Sanitize(params2[0]).c_str());
			Source.b->OnPrivmsg(u, params2);
// 			FOREACH_MOD(I_OnPrivmsg, OnPrivmsg(u, params2));
		}
		else
		{
			Command *ccom = FindCommand(params2[0], C_CHANNEL);
			if (ccom)
			{
				Source.command = ccom->name;
				params2.erase(params2.begin());

				while(ccom->MaxParams > 0 && params2.size() > ccom->MaxParams)
				{
					params2[ccom->MaxParams - 1] += " " + params2[ccom->MaxParams];
					params2.erase(params2.begin() + ccom->MaxParams);
				}

				if (params2.size() < ccom->MinParams)
				{
					ccom->OnSyntaxError(Source, !params2.empty() ? params2[params2.size() - 1] : "");
					return;
				}

				ccom->Run(Source, params2);
			}
			else
			{
				Source.b->OnChanmsg(u, c, params2);
// 				FOREACH_MOD(I_OnChanmsg, OnChanmsg(u, c, params2)); //This will one day be a actual function for channel only messages..
			}
		}
	}
	else
	{
		Command *com = FindCommand(params2[0], C_PRIVATE);
		if (com && !Source.b->n->IsValidChannel(receiver) && command == "PRIVMSG")
		{
			Source.command = com->name;
			params2.erase(params2.begin());

			while(com->MaxParams > 0 && params2.size() > com->MaxParams)
			{
				params2[com->MaxParams - 1] += " " + params2[com->MaxParams];
				params2.erase(params2.begin() + com->MaxParams);
			}

			if (params2.size() < com->MinParams)
			{
				com->OnSyntaxError(Source, !params2.empty() ? params2[params2.size() - 1] : "");
				return;
			}

			com->Run(Source, params2);
		}
		else
		{
// 			if (!protocoldebug)
				Log(LOG_DEBUG) << Flux::Sanitize(Source.raw); //This receives ALL server commands sent to the bot..
		}
	}
}

/*********************************************************************************/

/**
 * \fn void process(const Flux::string &buffer)
 * \brief Main Processing function
 * \param buffer The raw socket buffer
 */
void process(Bot *b, const Flux::string &buffer)
{

//   EventResult e;
//   FOREACH_RESULT(I_OnPreReceiveMessage, OnPreReceiveMessage(buffer), e);
//   if (e != EVENT_CONTINUE)
//     return;

	if (!b->OnPreReceiveMessage(buffer))
		return;

	Flux::string buf = buffer;
	buf = buf.replace_all_cs("  ", " ");

	if (buf.empty())
		return;

	Flux::string source;
	if (buf[0] == ':')
	{
		size_t space = buf.find_first_of(" ");

		if (space == Flux::string::npos)
			return;

		source = buf.substr(1, space - 1);
		buf = buf.substr(space + 1);

		if (source.empty() || buf.empty())
			return;
	}

	Flux::string command = buf;
	Flux::vector tokens = buf.explode(" "), params;

	for (auto it : tokens)
	{
		if (it[0] == ':')
		{
			params.push_back(it.substr(1));
		}
		else
			params.push_back(it);
	}

// 	sepstream bufferseparator(buf, ' ');
// 	Flux::string bufferseparator_token;
// 	Flux::string command = buf;
//
// 	if (bufferseparator.GetToken(bufferseparator_token))
// 		command = bufferseparator_token;
//
// 	std::vector<Flux::string> params;
// 	while(bufferseparator.GetToken(bufferseparator_token))
// 	{
// 		if (bufferseparator_token[0] == ':')
// 		{
// 			if (!bufferseparator.StreamEnd())
// 				params.push_back(bufferseparator_token.substr(1)+" "+bufferseparator.GetRemaining());
// 			else
// 				params.push_back(bufferseparator_token.substr(1));
// 			break;
// 		}
// 		else
// 			params.push_back(bufferseparator_token);
// 	}

	if (protocoldebug)
	{
		Log(LOG_TERMINAL) << "Source: " << (source.empty()?"No Source":source);
		Log(LOG_TERMINAL) << (command.is_number_only()?"Numeric":"Command") << ": " << command;

		if (params.empty())
			Log(LOG_TERMINAL) << "No Params";
		else
			for(unsigned i =0; i < params.size(); ++i)
			Log(LOG_TERMINAL) << "Params " << i << ": " << Flux::Sanitize(params[i]);
	}

	if (!b->n)
	{
		Log(LOG_TERMINAL) << "Process() called with no source Network??";
		return;
	}
	/***********************************************/
	/* make local variables instead of global ones */
	const Flux::string &receiver = params.size() > 0 ? params[0] : "";
	Flux::string message = params.size() > 1? params[1] : "";
	Flux::string nickname = source.isolate(':', '!');
	Flux::string uident = source.isolate('!', '@');
	Flux::string uhost = source.isolate('@', ' ');
	User *u = b->n->FindUser(nickname);
	Channel *c = b->n->FindChannel(receiver);
	Flux::vector params2 = message.explode(" "); //ParamitizeString(message, ' ');
	//   for(unsigned i = 0; i < params2.size(); ++i)
	//     Log(LOG_TERMINAL) << "Params2[" << i << "]: " << Flux::Sanitize(params2[i]);
	/***********************************************/

	if (command == "004" && source.search('.'))
	{
		b->OnUserRegister(b->n);
// 		FOREACH_MOD(I_OnUserRegister, OnUserRegister(n));
	}

	if (message[0] == '\1' && message[message.length() -1] == '\1' && !params2[0].equals_cs("\001ACTION"))
	{
		//Dont allow the rest of the system to process ctcp's as it will be caught by the command handler.
		b->OnCTCP(nickname, params2, b->n);
// 		FOREACH_MOD(I_OnCTCP, OnCTCP(nickname, params2, n));
		return;
	}
	// Handle Actions (ie. /me's )
	else if (message[0] == '\1' && message[message.length() - 1] == '\1' && params2[0].equals_ci("\001ACTION"))
	{
		if (c)
		{
			b->OnChannelAction(u, c, params2);
// 			FOREACH_MOD(I_OnChannelAction, OnChannelAction(u, c, params2));
		}
		else
		{
			b->OnAction(u, params2);
// 			FOREACH_MOD(I_OnAction, OnAction(u, params2));
		}
	}

	if (command.equals_cs("NICK") && u)
	{
		b->OnPreNickChange(u, params[0]);
// 		FOREACH_MOD(I_OnPreNickChange, OnPreNickChange(u, params[0]));
		u->SetNewNick(params[0]);
		b->OnNickChange(u);
// 		FOREACH_MOD(I_OnNickChange, OnNickChange(u));
	}

	if (!u && !b->n->FindUser(nickname) && (!nickname.empty() || !uident.empty() || !uhost.empty()))
	{
		if (!nickname.search('.') && b->n)
			u = new User(b->n, nickname, uident, uhost);
	}

	if (command.equals_ci("QUIT"))
	{
		b->OnQuit(u, params[0]);
// 		FOREACH_MOD(I_OnQuit, OnQuit(u, params[0]));
		delete u;
		//     QuitUser(n, u);
	}

	if (command.equals_ci("PART"))
	{
		b->OnPart(u, c, params[0]);
// 		FOREACH_MOD(I_OnPart, OnPart(u, c, params[0]));
		if (b->n->IsValidChannel(receiver) && c && u && u == b)
			delete c; //This should remove the channel from all users if the bot is parting..
		else
		{
			if (u)
				u->DelChan(c);

			if (c)
				c->DelUser(u);

			if (u && c && !u->FindChannel(c->name))
			{
				Log(LOG_TERMINAL) << "Deleted " << u->nickname << '|' << c->name << '|' << u->FindChannel(c->name);
				delete u;
			}
		}
	}

	if (command.is_pos_number_only())
	{
		b->OnNumeric((int)command, b->n, params);
// 		FOREACH_MOD(I_OnNumeric, OnNumeric((int)command, n, params));
	}

	if (command.equals_ci("PING"))
		b->OnPing(params, b->n);
	else if (command.equals_ci("PONG"))
		b->OnPong(params, b->n);
	else if (command.equals_ci("KICK"))
		b->OnKick(u, b->n->FindUser(params[1]), b->n->FindChannel(params[0]), params[2]);
	else if (command.equals_ci("ERROR"))
		b->OnConnectionError(buffer);
	else if (command.equals_ci("INVITE"))
		b->OnInvite(u, params[1]);
// 	EVENT_HOOK(command, "PING", I_OnPing, OnPing(params, n));
// 	EVENT_HOOK(command, "PONG", I_OnPong, OnPong(params, n));
// 	EVENT_HOOK(command, "KICK", I_OnKick, OnKick(u, b->n->FindUser(params[1]), b->n->FindChannel(params[0]), params[2]));
// 	EVENT_HOOK(command, "ERROR", I_OnConnectionError, OnConnectionError(buffer));
// 	EVENT_HOOK(command, "INVITE", I_OnInvite, OnInvite(u, params[1]));

	if (command.equals_ci("NOTICE") && !source.find('.'))
	{
		if (!b->n->IsValidChannel(receiver))
		{
			b->OnNotice(u, params2);
// 			FOREACH_MOD(I_OnNotice, OnNotice(u, params2));
		}
		else
		{
			b->OnChannelNotice(u, c, params2);
// 			FOREACH_MOD(I_OnNotice, OnChannelNotice(u, c, params2));
		}
	}

	if (command.equals_ci("MODE"))
	{
		if (b->n->IsValidChannel(params[0]) && params.size() == 2)
		{
			b->OnChannelMode(u, c, params2);
// 			FOREACH_MOD(I_OnChannelMode, OnChannelMode(u, c, params[1]));
		}

		else if (b->n->IsValidChannel(params[0]) && params.size() == 3)
		{
			b->OnChannelOp(u, c, params[1], params[2]);
// 			FOREACH_MOD(I_OnChannelOp, OnChannelOp(u, c, params[1], params[2]));
		}

// 		else if (params[0] == b->n->b->nick)
// 		{
// // 			FOREACH_MOD(I_OnUserMode, OnUserMode(u, params[0], params[1]));
// 		}
	}

	if (command.equals_ci("JOIN"))
	{
		if (!u && b->n && (!nickname.empty() || !uident.empty() || !uhost.empty()))
			u = new User(b->n, nickname, uident, uhost);
		else if (!c && b->n && b->n->IsValidChannel(receiver))
			c = new Channel(b->n, receiver);
		else if (!u->FindChannel(c->name))
			u->AddChan(c);
		else if (!c->FindUser(u->nickname))
			c->AddUser(u);
// 		else if (u != b->n->b)
// 		{
// 			FOREACH_MOD(I_OnJoin, OnJoin(u, c));
// 		}
	}

	// Get channel timestamp and modes with the below numerics
	if (command == "329")
	{
		Channel *chan = b->n->FindChannel(params[1]);
		if (chan)
			chan->creation_time = static_cast<long>(params[2]);
	}

	if (command == "324")
	{
		// NOTE: This *only* does the modes with + in front of them
		Channel *chan = b->n->FindChannel(params[1]);
		if (chan)
			chan->modes = params[2];
	}

	// Get the actual topic
	if (command == "332")
	{
		Channel *chan = b->n->FindChannel(params[1]);
		if (chan)
			chan->topic = params[2];
	}

	// Get the topic setter and timestamp
	if (command == "333")
	{
		Channel *chan = b->n->FindChannel(params[1]);
		if (chan)
		{
			chan->topic_time = static_cast<long>(params[3]);
			chan->topic_setter = params[2];
		}
	}

	/**************************************/
	CommandSource Source;
	Source.u = u; //User class
	Source.b = b; //Bot class
	Source.c = c; //Channel class
	Source.params = params;
	Source.raw = buffer;
	/**************************************/
	if (command == "352")
		ProcessJoin(Source, c->name);

	if (source.empty() || message.empty() || params2.empty())
		return;

	ProcessCommand(Source, params2, receiver, command);
	command.clear();
}
