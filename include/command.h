/* Arbitrary Navn Tool -- Prototype for Command Routines
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
#include "user.h"
#include "bots.h"
#include "flux.h"
#include "tinyformat.h"
#include <map>

class Module;

enum CommandType
{
	C_NULL,
	C_CHANNEL,
	C_PRIVATE
};

/**
 * \struct CommandSource "user.h" USER_H
 * \brief A wrapper class for the source of the Command
 * Contains information about where the command came from, as well as a Reply() function to quickly send a PrivMessage to the Command sender.
 */
struct CommandSource
{
	User *u;
	Channel *c; /* Channel name, this will be replaced with channel class */
	Bot *b;
	Flux::string command;
	Flux::string raw;
	Flux::vector params;

	template<typename... Args>
	void Reply(const Flux::string &fmt, const Args&... args)
	{
		this->Reply(tfm::format(fmt.c_str(), args...));
	}

	void Reply(const Flux::string&);
};

/**
 * \class Command
 * \brief A wrapper class for Module commands
 * Contains methods and properties for handling/getting information from module commands.
 * \note Not to be confused with the Commands class.
 */
class Command
{
		Flux::string desc;
		Flux::vector syntax;
		CommandType type;
public:
		size_t MaxParams;
		size_t MinParams;
		Flux::string name;
		Module *mod;
		Command(Module *m, const Flux::string &sname, CommandType t = C_NULL, size_t min_params=0, size_t max_params=0);
		virtual ~Command();
protected:
		void SetDesc(const Flux::string&);
		void SetSyntax(const Flux::string&);
		void SendSyntax(CommandSource&);
		void SendSyntax(CommandSource&, const Flux::string&);
public:
		const Flux::string &GetDesc() const;
		CommandType GetType() const;
		virtual void Run(CommandSource&, const Flux::vector &params);
		virtual bool OnHelp(CommandSource&, const Flux::string&);
		virtual void OnList(User *u);
		virtual void OnSyntaxError(CommandSource&, const Flux::string&);
		static void ProcessCommand(CommandSource &Source, Flux::vector &params2, const Flux::string &receiver, const Flux::string &command);
};

typedef std::map<Flux::string, Command*, ci::less> CommandMap;

extern Command *FindCommand(const Flux::string &name, CommandType type);
