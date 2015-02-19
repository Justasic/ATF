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


class Index : public Page
{
public:
	Index(Module *m) : Page(m, "^/$")
	{
	}

	bool Run(Request &r, const Flux::string &url)
	{
		if (r.GetType() == "GET")
		{
			r.Write("Content-Type: text/html\r\n\r\n");

			r.Write("<html>");
			r.Write("<head>");
			r.Write("<title>Hello World!</title>");
			r.Write("</head>");
			r.Write("<center><h2>Hello World!</h2></center>");
			r.Write("</html>");
			return true;
		}

		return false;
	}
};

class Overview : public Module
{
	Index idx;
public:
	Overview(const Flux::string &Name) : Module(Name, MOD_NORMAL), idx(this)
	{
		this->SetAuthor("Justasic");
		this->SetVersion(VERSION);
	}

	~Overview()
	{
	}
};

MODULE_HOOK(Overview)
