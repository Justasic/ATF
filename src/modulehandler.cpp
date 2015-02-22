/* Arbitrary Navn Tool -- Internal Module Handler
 *
 * (C) 2011-2012 Azuru
 * Contact us at Development@Azuru.net
 *
 * Please read COPYING and README for further details.
 *
 * Based on the original code of CIA.vc by Micah Dowty
 * Based on the original code of Anope by The Anope Team.
 */
#include "module.h"
#include "file.h"
#include "flux.h"
#include "Config.h"
#include <list>
#include <cassert>
#include <dlfcn.h>
#include <regex>
extern Flux::string binary_dir;
extern std::list<Module*> Modules;

Flux::string DecodeModErr(ModErr err)
{
	switch(err)
	{
		case MOD_ERR_OK:
			return "No error (MOD_ERR_OK)";
		case MOD_ERR_MEMORY:
			return "Out of memory (MOD_ERR_MEMORY)";
		case MOD_ERR_PARAMS:
			return "Insufficient parameters (MOD_ERR_PARAMS)";
		case MOD_ERR_EXISTS:
			return "Module Exists (MOD_ERR_EXISTS)";
		case MOD_ERR_NOEXIST:
			return "Module does not exist (MOD_ERR_NOEXIST)";
		case MOD_ERR_NOLOAD:
			return "Module cannot be loaded (MOD_ERR_NOLOAD)";
		case MOD_ERR_UNKNOWN:
			return "Unknown error (MOD_ERR_UNKNOWN)";
		case MOD_ERR_FILE_IO:
			return "File I/O Error (MOD_ERR_FILE_IO)";
		case MOD_ERR_EXCEPTION:
			return "Module Exception caught (MOD_ERR_EXCEPTION)";
		default:
			return "Unknown error code";
	}
}

/*  This code was found online at http://www.linuxjournal.com/article/3687#comment-26593 */
template<class TYPE> TYPE class_cast(void *symbol)
{
	union
	{
		void *symbol;
		TYPE function;
	} cast;
	cast.symbol = symbol;
	return cast.function;
}

ModErr ModuleHandler::LoadModule(const Flux::string &modname)
{
	if(modname.empty())
		return MOD_ERR_PARAMS;

	if(FindModule(modname))
		return MOD_ERR_EXISTS;

	Log() << "\033[0m[\033[1;32m*\033[0m] Loading Module:\t\033[1;32m" << modname << "\033[0m" << config->LogColor;

	Flux::string modbasename = FileSystem::Basename(modname);

	Flux::string mdir = binary_dir + "/runtime/" + (modbasename.search(".so") ? modbasename : modbasename + ".so");
	Flux::string input = modname;

	File *dest = FileSystem::OpenTemporaryFile(mdir);
	File *src = FileSystem::OpenFile(input, FS_READ);

	assert(FileSystem::CopyFile(dest, src));

	dlerror();

	void *handle = dlopen(dest->GetPath().c_str(), RTLD_LAZY);
	const char *err = dlerror();

	if (!handle && err && *err)
	{
		Log() << '[' << modname << "] " << err;
		return MOD_ERR_NOLOAD;
	}

	dlerror();

	Module *(*f)(const Flux::string&) = class_cast<Module *(*)(const Flux::string&)>(dlsym(handle, "ModInit"));
	err = dlerror();

	if (!f && err && *err)
	{
		Log() << "No Module init function, moving on.";
		dlclose(handle);
		return MOD_ERR_NOLOAD;
	}

	if (!f)
		throw CoreException("Can't find Module constructor, yet no moderr?");

	Module *m;
	try
	{
		m = f(modbasename);
	}
	catch (const ModuleException &e)
	{
		Log() << "Error while loading " << modname << ": " << e.GetReason();
		return MOD_ERR_EXCEPTION;
	}

	m->filepath = dest->GetPath();
	m->filename = (modname.search(".so") ? modname : modname + ".so");
	m->handle = reinterpret_cast<void*>(handle); //we'll convert to auto later, for now reinterpret_cast.

	FileSystem::CloseFile(src);
	FileSystem::CloseFile(dest);

	//FOREACH_MOD(I_OnModuleLoad, OnModuleLoad(m));

	return MOD_ERR_OK;
}

bool ModuleHandler::DeleteModule(Module *m)
{
    if (!m || !m->handle)
	return false;

    auto *handle = m->handle;
    Flux::string filepath = m->filepath;

    dlerror();

    void (*df)(Module*) = class_cast<void (*)(Module*)>(dlsym(m->handle, "ModunInit"));
    const char *err = dlerror();

    if (!df && err && *err)
    {
		Log(LOG_DEBUG) << "No destroy function found for " << m->name << ", chancing delete...";
		delete m; /* we just have to chance they haven't overwrote the delete operator then... */
    }

    if(!df)
    {
		Log() << "[" << m->name << ".so] Module has no destroy function? (wtf?)";
		return false;
    }
    else
		df(m);

    if(handle)
		if(dlclose(handle))
			Log() << "[" << m->name << ".so] " << dlerror();

	return true;
}

bool ModuleHandler::Unload(Module *m)
{
    if(!m || m->GetPermanent())
	return false;

    //FOREACH_MOD(I_OnModuleUnload, OnModuleUnload(m));
    return DeleteModule(m);
}

void ModuleHandler::UnloadAll()
{
    for(std::list<Module*>::iterator it = Modules.begin(), it_end = Modules.end(); it != it_end;)
    {
	Module *m = *it;
	++it;
	// ignore Unload function because we're forcing a unload regardless of whether it's permanent or not.
	//FOREACH_MOD(I_OnModuleUnload, OnModuleUnload(m));
	DeleteModule(m);
    }
    Modules.clear();
}

void ModuleHandler::SanitizeRuntime()
{
    Log(LOG_DEBUG) << "Cleaning up runtime directory.";
    Flux::string dirbuf = binary_dir+"/runtime/";

    if(!FileSystem::IsDirectory(dirbuf))
    {
			FileSystem::MakeDirectory(dirbuf);
    }
    else
    {
	Flux::vector files = FileSystem::DirectoryList(dirbuf);
	for(auto it : files)
	    unlink(Flux::string(dirbuf+it).c_str());
    }
}

/**
 * \fn Module *FindModule(const Flux::string &name)
 * \brief Find a Module in the Module list
 * \param name A string containing the Module name you're looking for
 */
Module *ModuleHandler::FindModule(const Flux::string &name)
{
    for(auto it : Modules)
    {
	Module *m = it;
	if(m->name.equals_ci(name))
	    return m;
    }

    return nullptr;
}

/******************configuration variables***********************/
/**Rehash void
 * \fn void Readconfig()
 * This will re-read the config file values when told to do so
 */
void LoadModules()
{
	ModuleHandler::SanitizeRuntime();
	Log(LOG_DEBUG) << "Loading modules...";
	auto files = FileSystem::DirectoryList(config->ModuleDir);

    for(unsigned i = 0; i < files.size(); ++i)
    {
	    // Make sure we're a shared object and not something dumb.
	    std::regex IsSharedObject("(.*)\\.so$");
	    if (!std::regex_match(files[i].std_str(), IsSharedObject))
		    continue;

		ModErr e = ModuleHandler::LoadModule(files[i]);
		//SocketEngine::Process(true);
		if(e != MOD_ERR_OK)
		{
			Log() << "\n\033[0m[\033[1;31m*\033[0m] " << files[i] << ": " << DecodeModErr(e) << config->LogColor << "\n";
			throw CoreException("Module Load Error");
		}
    }
    TimerManager::TickTimers(time(NULL));
}
/******************End configuration variables********************/
