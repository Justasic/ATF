/* Arbitrary Navn Tool -- CIA style ruleset parser
 *
 * (C) 2011-2012 Azuru
 * Contact us at Development@Azuru.net
 *
 * Please read COPYING and README for further details.
 *
 * Based on the original code of CIA.vc by Micah Dowty
 * Based on the original code of Anope by The Anope Team.
 */

#include "modules.h"
#include "colors.h"

// TODO: This needs to parse CIA style rulesets and then do a FOREACH_MOD(I_OnCommit, OnCommit) call
// to announce the messages in all of IRC and other systems. This should also be one of the main calls
// for all announcements in this system.


// Calculate files to announce.
// FIXME: This needs to be handled by the Rulesets system later on.
// link to CIA file formatter: http://code.google.com/p/cia-vc/source/browse/trunk/cia/LibCIA/Formatters/Commit.py

// This function is to do the same thing that pythons posix.PosixCommonPrefix does
// inside CIA.vc or whatever, It will build an array of directories with the files
// inside them as an array
Flux::map<Flux::vector> PosixCommonPrefix(const Flux::vector &files)
{
	Flux::map<Flux::vector> CommonDirectories;
	for(Flux::vector::const_iterator it = files.begin(); it != files.end(); ++it)
	{
		Flux::string path = *it;
		Flux::string directory = path.substr(0, path.rfind('/'));
		Flux::string file = path.substr(path.rfind('/') + 1);

		// Initialize the vector
		if(CommonDirectories.find(directory) == CommonDirectories.end())
			CommonDirectories[directory] = Flux::vector();

		CommonDirectories[directory].push_back(file);
	}

	return CommonDirectories;
}

// I swear, this function has some of the worst function calls ever.
Flux::string BuildFileString(const Flux::vector &files)
{
	Flux::map<Flux::vector> CommonDirectories = PosixCommonPrefix(files);

	Flux::string ret;
	Flux::string prefix;

	// Only one directory
	if(CommonDirectories.size() == 1)
	{
		// there's only one file
		if(CommonDirectories.begin()->second.size() == 1)
			prefix = CommonDirectories.begin()->first + "/" + (*CommonDirectories.begin()->second.begin());
			// Oh no! two files! wat do?!
		else if(CommonDirectories.begin()->second.size() == 2)
		{
			prefix = CommonDirectories.begin()->first;
			ret = CommonDirectories.begin()->second[0] + " " + CommonDirectories.begin()->second[1];
		}
		else
		{ // TOO MANY FILEZ!!!
			prefix = CommonDirectories.begin()->first;
			ret = tfm::format("%d files", CommonDirectories.begin()->second.size());
		}
	}
	else // Y U CHANGE SO MUCH?!
		ret = tfm::format("%u files in %u dirs", files.size(), CommonDirectories.size());

	// my god it makes my eyes hurt!
	return tfm::format("%s%s", (prefix.empty() ? "" : Flux::string(prefix+" ").c_str()),
			(ret.empty() ? "" : tfm::format("(%s)", ret.c_str()).c_str()));
}

// Small api for getting commit data easily
inline Flux::string GetCommitData(const CommitMessage &msg, const Flux::string &n)
{
	for(auto it : msg.info)
	{
		if(it.first.equals_ci(n))
			return it.second;
	}

	return "";
}

// Used to make string formatting less bulky
Flux::string FormatString(const CommitMessage &msg, const Flux::string &string)
{
	struct special_chars
	{
		Flux::string character;
		Flux::string replace;
		special_chars(const Flux::string &c, const Flux::string &r) : character(c), replace(r) { }
	} special[] = {
			special_chars("{project}",       GetCommitData(msg, "project")),
			special_chars("{author}",        GetCommitData(msg, "author")),
			special_chars("{branch}",        GetCommitData(msg, "branch")),
			special_chars("{timestamp}",     GetCommitData(msg, "timestamp")),
			special_chars("{module}",        GetCommitData(msg, "module")),
			special_chars("{scriptname}",    GetCommitData(msg, "scriptname")),
			special_chars("{scriptversion}", GetCommitData(msg, "scriptversion")),
			special_chars("{scripturl}",     GetCommitData(msg, "scripturl")),
			special_chars("{scriptauthor}",  GetCommitData(msg, "scriptauthor")),
			special_chars("{revision}",      GetCommitData(msg, "revision")),
			special_chars("{insertions}",    GetCommitData(msg, "insertions")),
			special_chars("{deletions}",     GetCommitData(msg, "deletions")),
			special_chars("{log}",           GetCommitData(msg, "log")),
			special_chars("{files}",         BuildFileString(msg.Files)),
			special_chars("","")
	};
	Flux::string ret = string;
	for(int i = 0; !special[i].character.empty(); ++i)
		ret = ret.replace_all_ci(special[i].character, special[i].replace);
	return ret.c_str();
}

bool HandleOnCommit(CommitMessage &msg)
{
	// FIXME: if they're no connections, buffer the message
	Log(LOG_DEBUG) << "AnnounceCommit Called.";

	// Build the commit message
	Flux::string message = "\0034\002{project}: \017\0037{author} * \017\002[{branch}]\017\0038 r{revision}"
			"\017 ~\0036 {insertions}(+) {deletions}(-)\017\002 | \017\00310{files}\017: {log}";

	Flux::string formattedmessgae = FormatString(msg, message);

	for (auto it : msg.Channels)
	{
		Channel *c = it;
		Log(LOG_DEBUG) << "Announcing in " << c->name << " (" << c->n->name << ')';
		Bot *b = Bot::FindBotInChannel(c);
		b->Message(c, formattedmessgae);
	}
}

/**************** Module Class *******************/

class CIA_RULESET_MOD : public Module
{
public:
	CIA_RULESET_MOD(const Flux::string &Name) : Module(Name, MOD_NORMAL)
	{
		this->SetAuthor("Justasic");
		this->SetVersion(VERSION);
		// Add a function to handle the event.
		OnCommit += &HandleOnCommit;
	}

	~CIA_RULESET_MOD()
	{
		OnCommit.remove(&HandleOnCommit);
	}
};

MODULE_HOOK(CIA_RULESET_MOD)
