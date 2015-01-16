/* Arbitrary Navn Tool -- Module Class Prototypes
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
#ifndef Module_H
#define Module_H

// #include "Socket.h"
#include "user.h"
#include "command.h"
#include "network.h"

#ifdef HAVE_FCNTL_H
# include <dlfcn.h>
#else
# error dlfcn.h is required by ATF to compile Modules!
#endif

class Socket;

enum ModType
{
	MOD_UNDEFINED,
	MOD_ENCRYPTION,
	MOD_PROTOCOL,
	MOD_SOCKETENGINE,
	MOD_DATABASE,
	MOD_NORMAL
};

enum ModErr
{
	MOD_ERR_OK,
	MOD_ERR_MEMORY,
	MOD_ERR_PARAMS,
	MOD_ERR_EXISTS,
	MOD_ERR_NOEXIST,
	MOD_ERR_NOLOAD,
	MOD_ERR_UNKNOWN,
	MOD_ERR_FILE_IO,
	MOD_ERR_EXCEPTION
};



class Module
{
	Flux::string author, version;
	time_t loadtime;
	bool permanent;
	ModType type;
protected:
	void SetAuthor(const Flux::string&);
	void SetVersion(const Flux::string&);
	void SetPermanent(bool);
public:
	void *handle;
	Flux::string name, filename, filepath;
	inline Flux::string GetAuthor() const { return this->author; }
	inline Flux::string GetVersion() const { return this->version; }
	inline bool GetPermanent() const { return this->permanent; }
	inline time_t GetLoadTime() const { return this->loadtime; }
	inline ModType GetModuleType() const { return this->type; }

	Module(const Flux::string&, ModType = MOD_UNDEFINED);
	virtual ~Module();

// 	virtual bool OnPreReceiveMessage(const Flux::string&) { return true; }
// 	virtual void OnPrivmsg(User*, const Flux::vector&) {}
// 	virtual void OnChannelAction(User*, Channel*, const Flux::vector&) {}
// 	virtual void OnAction(User*, const Flux::vector&) {}
// 	virtual void OnChanmsg(User*, Channel*, const Flux::vector&) {}
// 	virtual void OnCommit(CommitMessage&) {}
// 	virtual void OnGarbageCleanup() {}
// 	virtual void OnSignal(int) {}
// 	virtual bool OnLog(Log*) { return true; }
// 	virtual void OnNetworkSync(Network*) {}
// 	virtual bool OnPreCommit(CommitMessage&) { return true; }
// 	virtual void OnUserRegister(Network*) {}
// 	virtual void OnDatabasesWrite(void (*)(const char*, ...)) {}
// 	virtual void OnDatabasesRead(const Flux::vector&) {}
// 	virtual void OnSaveDatabases() {}
// 	virtual void OnForceDatabasesRead() {}
// 	virtual void OnNotice(User*, const Flux::vector&) {}
// 	virtual void OnChannelNotice(User*, Channel*, const Flux::vector&) {}
// 	virtual void OnCTCP(const Flux::string&, const Flux::vector&, Network*) {}
// 	virtual void OnPing(const Flux::vector&, Network*) {}
// 	virtual void OnPong(const Flux::vector&, Network*) {}
// 	virtual void OnArgument(int, const Flux::string&) {}
// 	virtual void OnModuleLoad(Module*) {}
// 	virtual void OnFork(int) {}
// 	virtual void OnSocketError(const Flux::string&) {}
// 	virtual void OnModuleUnload(Module*){}
// 	virtual void OnRestart(const Flux::string&) {}
// 	virtual void OnShutdown() {}
// 	virtual void OnNickChange(User*) {}
// 	virtual void OnPreNickChange(User*, const Flux::string&) {}
// 	virtual void OnQuit(User*, const Flux::string&) {}
// 	virtual void OnJoin(User*, Channel*) {}
// 	virtual void OnKick(User*, User*, Channel*, const Flux::string&) {}
// 	virtual void OnNumeric(int, Network*, const Flux::vector&) {}
// 	virtual void OnReload() {}
// 	virtual void OnCommand(const Flux::string&, const Flux::vector&) {}
// 	virtual void OnStart(int, char**) {}
// 	virtual void OnChannelMode(User*, Channel*, const Flux::string&) {}
// 	virtual void OnChannelOp(User*, Channel*, const Flux::string&, const Flux::string&) {}
// 	virtual void OnPart(User*, Channel*, const Flux::string&) {}
// 	virtual void OnUserMode(User*, const Flux::string&, const Flux::string&) {}
// 	virtual bool OnPreConnect(const Flux::string&, const Flux::string&) { return true; }
// 	virtual bool OnPreConnect(Network*) { return true; }
// 	virtual void OnPostConnect(Socket*, Network*) {}
// 	virtual void OnPostConnect(Socket*) {}
// 	virtual void OnConnectionError(const Flux::string&) {}
// 	virtual void OnInvite(User *u, const Flux::string&) {}
};

class ModuleHandler
{
public:
// 	static std::vector<Module*> EventHandlers[I_END];
	static ModErr LoadModule(const Flux::string&);
	static void SanitizeRuntime();
	static void UnloadAll();
	static bool Unload(Module*);

	static Module *FindModule(const Flux::string &name);

	//////////////////////////////////////////////////////////////////////
	// Module-specific events which modules may register for

private:
	static bool DeleteModule(Module*);
};
#endif
