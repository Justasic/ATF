/* Arbitrary Navn Tool -- XML-RPC Parser Module
 *
 * (C) 2011-2012 Azuru
 * Contact us at Development@Azuru.net
 *
 * Please read COPYING and README for further details.
 *
 * Based on the original code of CIA.vc by Micah Dowty
 * Based on the original code of Anope by The Anope Team.
 */

/*
 * This is the module file for the XML_RPC commits, this will handle 98% of the commits the bot will process!
 */
#include "modules.h"
#include "rapidxml/rapidxml.hpp"
#include "rapidxml/rapidxml_utils.hpp"
#include "rapidxml/rapidxml_iterators.hpp"
#include "rapidxml/rapidxml_print.hpp"

// Convert the XML data to something parsable.
Flux::string SanitizeXML(const Flux::string &str)
{
	static struct chars
	{
		Flux::string character;
		Flux::string replace;
		chars(const Flux::string &c, const Flux::string &r) : character(c), replace(r) { }
    }
	special[] = {
		chars("&amp;", "&"),
		chars("&quot;", "\""),
		chars("&lt;", "<"),
		chars("&gt;", ">"),
		chars("&#39", "'"),
		chars("&#xA", "\n"),
		chars("\r", ""),
		chars("", "")
    };

	Flux::string ret = str;
	for(int i = 0; special[i].character.empty() == false; ++i)
		ret = ret.replace_all_cs(special[i].character, special[i].replace);
	return ret;
}

// Simple web page in case a web browser decides to go to the link
static const Flux::string systemver = VERSION_FULL;
static const Flux::string HTTPREQUEST =
"<center><h1>ANT Commit system version "+systemver+"</h1></center>\n"
"<center><h4>This is the address for XML-RPC commits</h4>\n"
"<p>This will not provide XML-RPC requests and ONLY uses POST to commit the data (same as most <a href=\"http://cia.vc/doc/clients/\">CIA.vc scripts</a>), if you are looking for the site, please see <a href=\"http://www.Azuru.net/\">Azuru.net</a> for the sites location or optionally connect to Azuru IRC for support:</p>\n"
"<a href=\"irc://irc.Azuru.net/Computers\"><FONT COLOR=\"red\">irc.Azuru.net</FONT>:<FONT COlOR=\"Blue\">6667</FONT></a></br>\n"
"Channel: <FONT COLOR=\"Green\">#Commits</FONT></br>\n"
"Channel: <FONT COLOR=\"Green\">#Computers</FONT></br></center>\n";

class XMLRPC : public Page
{
public:
	XMLRPC(Module *m) : Page(m, "^/RPC[0-9]+")
	{
	}

	bool Run(Request &r, const Flux::string &url)
	{
		if (r.GetType() == "POST")
		{
			std::vector<char> buffer;

			uint8_t *buf = new uint8_t[1024];
			size_t len = r.ReadData(buf, 1024);

			while (len == 1024)
			{
				len = r.ReadData(buf, 1024);
				buffer.insert(buffer.end(), buf, buf + len);
			}

			delete [] buf;

			buffer.push_back(0);

			return HandleMessage(Flux::string(buffer.begin(), buffer.end()));
		}
		else if (r.GetType() == "GET")
		{
			r.Write("Content-Type: text/html\r\n");
			r.Write("Content-Length: %zu\r\n\r\n", HTTPREQUEST.size());
			r.Write(HTTPREQUEST);
			return true;
		}
		else
			return false;
	}

	bool HandleMessage(const Flux::string&);
};

/* This is down here so we don't make a huge mess in the middle of the file */
// Parse our message then announce it as a commit using the OnCommit event.
bool XMLRPC::HandleMessage(const Flux::string &content)
{
	if (content.empty())
		return false;

	Log(LOG_TERMINAL) << "[XML-RPC] Message Handler Called!";

	Flux::string blah = SanitizeXML(content);

	// Strip out all the XML garbage we don't need since RapidXML will crash if we don't
	size_t pos1 = blah.find("<?");
	size_t pos2 = blah.find("<message>");

	blah = blah.erase(pos1, pos2).replace_all_cs("  ", " ");

	try
	{
		// Our message
		CommitMessage message;
		// Parse the XML
		rapidxml::xml_document<> doc;
		doc.parse<0>(blah.cc_str());

		// Attempt to get the first node of the commit.
		rapidxml::xml_node<> *main_node = doc.first_node("message", 0, true);
		if(!main_node)
		{
			Log(LOG_TERMINAL) << "Invalid XML data!";
			return false;
		}

		/* message.generator section */
		rapidxml::xml_node<> *node = main_node->first_node("generator", 0, true);
		if(node)
		{
			if(node->first_node("name", 0, true))
				message.info["ScriptName"] = node->first_node("name", 0, true)->value();
			if(node->first_node("version", 0, true))
				message.info["ScriptVersion"] = node->first_node("version", 0, true)->value();
			if(node->first_node("url", 0, true))
				message.info["ScriptURL"] = node->first_node("url", 0, true)->value();
			if(node->first_node("author", 0, true))
				message.info["ScriptAuthor"] = node->first_node("author", 0, true)->value();
		}

		/* message.source section */
		node = main_node->first_node("source", 0, true);
		if(node)
		{
			if(node->first_node("project", 0, true))
				message.info["project"] = node->first_node("project", 0, true)->value();
			if(node->first_node("branch", 0, true))
				message.info["branch"] = node->first_node("branch", 0, true)->value();
			if(node->first_node("module", 0, true))
				message.info["module"] = node->first_node("module", 0, true)->value();
		}
		else
			return false;

		/* message.timestamp section */
		if(main_node->first_node("timestamp", 0, true))
			message.info["timestamp"] = main_node->first_node("timestamp", 0, true)->value();

		/* message.body.commit section */
		rapidxml::xml_node<> *mnode = main_node->first_node("body", 0, true);
		if(mnode)
		{
			node = mnode->first_node("commit", 0, true);
			if(node)
			{
				if(node->first_node("author", 0, true))
					message.info["author"] = node->first_node("author", 0, true)->value();
				if(node->first_node("revision", 0, true))
					message.info["revision"] = node->first_node("revision", 0, true)->value();
				if(node->first_node("log", 0, true))
					message.info["log"] = node->first_node("log", 0, true)->value();

				/* message.body.commit.files section */
				if(node->first_node("files", 0, true) && node->first_node("files", 0, true)->first_node("file", 0, true))
				{
					// Put our children in a vector, damn kids.. packing into vectors nowadays, i remember when
					// it was cool to pack into hashes!
					for (rapidxml::xml_node<> *child = node->first_node("files", 0, true)->first_node("file", 0, true); child; child = child->next_sibling())
						message.Files.push_back(child->value());

					// Sort our messages and remove any duplicates.
					std::sort(message.Files.begin(), message.Files.end());
					message.Files.erase(std::unique(message.Files.begin(), message.Files.end()), message.Files.end());
				}
			}

			node = mnode->first_node("stats", 0, true);
			if(node)
			{
				if(node->first_node("insertions", 0, true))
					message.info["insertions"] = node->first_node("insertions", 0, true)->value();
				if(node->first_node("deletions", 0, true))
					message.info["deletions"] = node->first_node("deletions", 0, true)->value();
			}
		}

		// remove everything from memory
		doc.clear();

		for(auto IT : Networks)
		{
			for(auto it : IT.second->ChanMap)
				message.Channels.push_back(it.second);
		}

		Log(LOG_TERMINAL) << "\n*** COMMIT INFO! ***";
		for(auto it : message.info)
			Log(LOG_TERMINAL) << it.first << ": " << it.second;

		int i = 0;
		for(auto it : message.Files)
			Log(LOG_TERMINAL) << "File[" << ++i << "]: " << it;

		i = 0;
		for(auto it : message.Channels)
			Log(LOG_TERMINAL) << Flux::Sanitize("Channel["+ value_cast<Flux::string>(++i) + "]: " + it->name + " - " + it->n->name + " | @") << it;

		Log(LOG_TERMINAL) << "*** END COMMIT INFO! ***\n";

		/* Announce to other modules for commit announcement */
		// Call the global commit event.
		OnCommit(message);
	}
	catch (std::exception &ex)
	{
		Log(LOG_TERMINAL) << "XML Exception Caught: " << ex.what();
		return false;
	}
	return true;
}

class xmlrpcmod : public Module
{
	XMLRPC xmlrpcpage;
public:
	xmlrpcmod(const Flux::string &Name) : Module(Name, MOD_NORMAL), xmlrpcpage(this)
    {
		this->SetAuthor("Justasic");
		this->SetVersion(VERSION);
	}

	~xmlrpcmod()
	{
	}
};

MODULE_HOOK(xmlrpcmod)
