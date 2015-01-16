/* Arbitrary Navn Tool -- Logging Prototype
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
#ifndef LOG_H
#define LOG_H
#include <fstream>
#include <sstream>
#include "flux.h"
#include "tinyformat.h"

// Forward declare because we dont need recursive includes.
class Command;
class User;

enum LogType
{
	LOG_DEBUG,
	LOG_MEMORY,
	LOG_NORMAL,
	LOG_RAWIO,
	LOG_TERMINAL,
	LOG_WARN,
	LOG_DNS,
	LOG_CRITICAL,
	LOG_THREAD,
	LOG_SILENT
};

class Log
{
protected:
	std::fstream log;
public:
	Log(LogType type = LOG_NORMAL);
	Log(LogType, User*);
	Log(User*);
	Log(User*, Command*);
	Log(LogType, User*, Command*);
	~Log();

	LogType type;
	Flux::string filename;
	User *u;
	Command *c;
	std::stringstream buffer;
	std::stringstream logstream;
	static Flux::string TimeStamp();


	template<typename T> Log &operator<<(T val)
	{
		this->buffer << val;
		return *this;
	}

	template<typename... Args> Log &format(const Flux::string &fmt, const Args&... args)
	{
		this->buffer << tfm::format(fmt, args...);
		return *this;
	}
};

#endif
