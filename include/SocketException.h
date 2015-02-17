/* Arbitrary Navn Tool -- Exceptions
 *
 * (C) 2011-2012 Azuru
 * Contact us at Development@Azuru.net
 *
 * Please read COPYING and README for further details.
 *
 * Based on the original code of CIA.vc by Micah Dowty
 * Based on the original code of Anope by The Anope Team.
 */
/**
 *\file  SocketException.h
 *\brief Contains the Exception throw classes.
 */
#pragma once
#include "flux.h"
#include "tinyformat.h"

/**CoreExceptions are to be thrown with everything else
 * Throws Core Exception
 * @param throw CoreException(message)
 */

class CoreException : public std::exception
{
 public:
	/** Holds the error message to be displayed
	 */
	const Flux::string err;
	/** Source of the exception
	 */
	const Flux::string source;

	/** Default constructor, just uses the error mesage 'Core threw an exception'.
	 */
	CoreException() : err("Core threw an exception"), source("The core") { }

	/** This constructor can be used to specify an error message before throwing.
	 */
	CoreException(const Flux::string &message) : err(message), source("The core") { }

	/** This constructor can be used to specify an error message before throwing,
	 * and to specify the source of the exception.
	 */
	CoreException(const Flux::string &message, const Flux::string &src) : err(message), source(src) { }

	template<typename... Args>
	CoreException(const Flux::string &message, const Flux::string &src, const Args&... args) :
	err(tfm::format(message, args...)), source(src) { }


	/** This destructor solves world hunger, cancels the world debt, and causes the world to end.
	 * Actually no, it does nothing. Never mind.
	 * @throws Nothing!
	 */
	virtual ~CoreException() throw() { };

	/** Returns the reason for the exception.
	 * The module should probably put something informative here as the user will see this upon failure.
	 */
	virtual const char* GetReason() const
	{
		return err.c_str();
	}

	virtual const char* GetSource()
	{
		return source.c_str();
	}

	const char *what() const noexcept
	{
		return this->GetReason();
	}
};

class LogException : public CoreException
{
 public:
		LogException(const Flux::string &message) : CoreException(message, "A Log") { }
		virtual ~LogException() throw() { }
};

class ModuleException : public CoreException
{
 public:
		ModuleException(const Flux::string &message) : CoreException(message, "A Module") { }
		virtual ~ModuleException() throw() { }
};

class ConfigException : public CoreException
{
 public:
 		ConfigException() : CoreException("Config threw an exception", "Config Parser") { }
		ConfigException(const Flux::string &msg) : CoreException(msg, "A Config") { }
		virtual ~ConfigException() throw() { }
};

class SocketException : public CoreException
{
public:
	SocketException(const Flux::string &message) : CoreException(message) { }

	template<typename... Args>
	SocketException(const Flux::string &message, const Flux::string &src, const Args&... args) :
	CoreException(message, src, args...) { }

	virtual ~SocketException() throw() { }
};
