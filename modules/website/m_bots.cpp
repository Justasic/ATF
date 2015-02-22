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


class BotList : public Page
{
public:
    BotList(Module *m) : Page(m, "^/bots/(.*)")
    {
    }

    bool Run(Request &r, const Flux::string &url)
    {
        if (r.GetType() == "GET")
        {
            r.Write("Content-Type: text/html\r\n\r\n");

            r.Write("<html>");

            r.Write("<head>");
            r.Write("<title>Overview - ATF</title>");
            r.Write("<link rel=\"stylesheet\" href=\"/static/css/skeleton.css\">");
            r.Write("<link rel=\"stylesheet\" href=\"/static/css/normalize.css\">");
            r.Write("<link rel=\"stylesheet\" href=\"/static/css/basic.css\">");
            r.Write("</head>");

            r.Write("<h1>Bot List</h1>");
            r.Write("<article><div class=\"box\">");
            r.Write("<h6>Active Bots</h6>");
            r.Write("<table class=\"u-full-width\">");
            r.Write("<thread>");
            r.Write("<tr>");
            r.Write("<th>Nickname</th>");
            r.Write("<th>Network</th>");
            r.Write("<th>More info</th>");
            r.Write("</tr>");
            r.Write("</thread>");
            r.Write("<tbody>");

            for (auto it : Network::Networks)
            {
                for (auto bot : it.second->Bots)
                {
                    r.Write("<tr>");
                    r.Write("<td>%s</a></td>", bot.first);
                    r.Write("<td>%s</td>", it.first);
                    r.Write("<td><a href=\"/bots/%s/%s\">%s</a></td>", it.first, bot.first);
                    r.Write("</tr>");
                }
            }

            r.Write("</tbody>");
            r.Write("</table>");
            r.Write("</div></article>");

            r.Write("</html>");
            return true;
        }

        return false;
    }
};

class m_bots : public Module
{
    BotList bots;
public:
    m_bots(const Flux::string &Name) : Module(Name, MOD_NORMAL), bots(this)
    {
        this->SetAuthor("Justasic");
        this->SetVersion(VERSION);
    }

    ~m_bots()
    {
    }
};

MODULE_HOOK(m_bots)
