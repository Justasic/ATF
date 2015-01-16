#pragma once
#include "user.h"
#include "flux.h"
#include "network.h"


class Bot : public User
{
		int nicknum;
public:
		Bot(Network *n, const Flux::string &modes);
		~Bot();

		void GenerateNextNick();

		// When a user messages a bot it is handled here
		void OnMessage(User *u, const Flux::string &msg);

		// Things used to control the flow of the bot
		void Join(Channel *c);
		void Part(Channel *c, const Flux::string &msg);

		// Message sending functions
		void Message(Channel *c, const Flux::string &msg);
		void Message(User *u, const Flux::string &msg);

		void Notice(Channel *c, const Flux::string &msg);
		void Notice(User *u, const Flux::stringf &msg);
};
