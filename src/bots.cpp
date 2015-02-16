#include "bots.h"
#include "network.h"
#include "log.h"
#include "Config.h"
#include <cassert>

static inline Flux::string GenerateBotNick(int botid)
{
	return tfm::format("%s%d", config->BotPrefix, botid);
}

Bot::Bot(Network *n, const Flux::string &modes) : User(n, GenerateBotNick(1), config->BotIdent, "", config->BotRealname),
Socket(-1), ConnectionSocket(), BufferedSocket(), pings(0), botid(1), IRCProto(this)
{
	n->Bots[this->nickname] = this;

	this->n->CurHost++;
	if (static_cast<unsigned int>(this->n->CurHost) >= this->n->hostnames.size())
		this->n->CurHost = 1;

	this->n->SetConnectedHostname(this->n->hostnames[this->n->CurHost]);

	Log(LOG_TERMINAL) << "New bot for " << n->name << " connecting to "
	<< n->hostname << ':' << n->port << '(' << n->GetConHost() << ')';

	ConnectionSocket::Connect(n->GetConHost(), n->port);
}

Bot::~Bot()
{
	auto it = this->n->Bots.find(this->nickname);
	if (it != this->n->Bots.end())
		this->n->Bots.erase(it);

	this->Write("QUIT :Socket Closed\n");
	this->ProcessWrite();

	Log() << "Closing Connection to " << this->n->name;

	if (!this->IsDisconnecting())
	{
		Log() << "Connection to " << this->n->name << " [" << this->n->GetConHost() << ':'
		<< this->n->port << "] Failed! Retrying in " << config->RetryWait << " seconds.";

		new ReconnectTimer(config->RetryWait, this->n);
	}
}

void Bot::CheckBots(Network *n)
{
	assert(n);
	Log(LOG_DEBUG) << "Checking bots...";
	for (auto it : n->Bots)
		it.second->CheckBot();
}

void Bot::Join(Channel *c)
{
	assert(c);
	c->AddUser(this);
	this->AddChan(c);
	this->join(c->name);
}

void Bot::Part(Channel *c, const Flux::string &msg)
{
	assert(c);
	c->DelUser(this);
	this->DelChan(c);
	this->part(c->name, msg);
}

void Bot::Message(Channel *c, const Flux::string &msg)
{
	assert(c);
	this->privmsg(c->name, msg);
}

void Bot::Message(User *u, const Flux::string &msg)
{
	assert(u);
	this->privmsg(u->nickname, msg);
}

void Bot::Notice(Channel *c, const Flux::string &msg)
{
	assert(c);
	this->notice(c->name, msg);
}

void Bot::Notice(User *u, const Flux::string &msg)
{
	assert(u);
	this->notice(u->nickname, msg);
}

void Bot::SetNewNick(const Flux::string &nick)
{
	if (nick.empty())
		return;

	// TODO: ensure valid nickname?

	// First, we erase the old nick
	auto it = this->n->Bots.find(this->nickname);
	if (it != this->n->Bots.end())
		this->n->Bots.erase(it);


	// We set the new nickname
	this->nickname = nick;
	this->nick(nick);

	// now we update our map.
	this->n->Bots[this->nickname] = this;
}

void Bot::CheckBot()
{

}

void Bot::OnMessage(User *u, const Flux::string &msg)
{

}

bool Bot::Read(const Flux::string &buf)
{
	// since BufferedSocket was modified to accommodate stupid HTTP, this code is required.
	if (buf.empty())
		return true;

	// 	Log(LOG_RAWIO) << '[' << this->n->name << "] " << FixBuffer(buf);
	Log(LOG_RAWIO) << '[' << this->n->name << "] " << buf;
	Flux::vector params = buf.explode(" ");

	if (!params.empty() && params[0].search_ci("ERROR"))
	{
		// 		FOREACH_MOD(I_OnSocketError, OnSocketError(buf));
		Log(LOG_TERMINAL) << "Socket Error, Closing socket!";
		return false; //Socket is dead so we'll let the socket engine handle it
	}

	extern void process(Bot *n, const Flux::string &buffer);
	process(this, buf);

	if (!params.empty() && params[0].equals_ci("PING"))
	{
		this->Write("PONG :" + params[1]);
		this->ProcessWrite();
	}
	return true;
}

void Bot::OnConnect()
{
	this->introduce_client(this->nickname, this->ident, this->realname);
	Log(LOG_TERMINAL) << "Successfully connected to " << this->n->name << " ["
	<< this->n->hostname << ':' << this->n->port << "] (" << this->n->GetConHost() << ")";
	//
	// 	new Bot(this->n, printfify("%stmp%03d", config->NicknamePrefix.strip('-').c_str(),
	// 								 randint(0, 999)), config->Ident, config->Realname);
	//
	// 	this->n->b->introduce();
	// 	FOREACH_MOD(I_OnPostConnect, OnPostConnect(this, this->n));
	this->ProcessWrite();
}

bool Bot::Disconnect()
{

}

// Same as above except with a message
bool Bot::Disconnect(const Flux::string&)
{

}

void Bot::OnError(const Flux::string &buf)
{
	Log(LOG_TERMINAL) << "Unable to connect to " << this->n->name << " ("
	<< this->n->hostname << ':' << this->n->port << ')' << (!buf.empty()?(": " + buf):"");
}

bool Bot::ProcessWrite()
{
	// 	Log(LOG_RAWIO) << '[' << this->n->name << ']' << ' ' << FixBuffer(this->WriteBuffer);
	Log(LOG_RAWIO) << '[' << this->n->name << ']' << ' ' << this->WriteBuffer;
	return ConnectionSocket::ProcessWrite() && BufferedSocket::ProcessWrite();
}
