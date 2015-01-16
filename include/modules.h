/* Arbitrary Navn Tool -- Main include for modules
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
#include "module.h"
#include "network.h" //We'll solve includes later
#include "flux.h"
#include "tinyformat.h"
#include "log.h"
#include "Request.h"
#include "timers.h"
#include "user.h"
#include "ThreadEngine.h"
#include "SocketException.h"
#include "dns.h"
#include "Config.h"
#include "misc.h"
#include "page.h"
//#include "bot.h"

extern Flux::insensitive_map<Network*> Networks;

#define MODULE_HOOK(x) \
extern "C" Module *ModInit(const Flux::string &name) { return new x(name); } \
extern "C" void ModunInit(x *m) { delete m; }


// extern void startup(int argc, char** argv, char *envp[]);
// extern void Rehash();
// extern Flux::string execute(const char *cmd);
// extern Flux::string urlify(const Flux::string &received);
// extern Flux::string getprogdir(const Flux::string &dir);
