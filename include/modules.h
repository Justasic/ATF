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
#include "bot.h"

extern Flux::insensitive_map<Network*> Networks;


// extern void startup(int argc, char** argv, char *envp[]);
// extern void Rehash();
// extern Flux::string execute(const char *cmd);
// extern Flux::string urlify(const Flux::string &received);
// extern Flux::string getprogdir(const Flux::string &dir);
