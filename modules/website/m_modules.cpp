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




class ListModules : public Page
{
public:
    ListModules(Module *m) : Page(m, "^/modules/(.*)")
    {
    }

    bool Run(Request &r, const Flux::string &url)
    {
        if (r.GetType() == "GET")
        {
            r.Write("Content-Type: text/html\r\n\r\n");

            r.Write("<html>");

            r.Write("<head>");
            r.Write("<title>Loaded Modules - ATF</title>");
            r.Write("<link rel=\"stylesheet\" href=\"/static/css/skeleton.css\">");
            r.Write("<link rel=\"stylesheet\" href=\"/static/css/normalize.css\">");
            r.Write("</head>");

            r.Write("<h2>Module List</h2>");

            r.Write("<form action=\"#\">");
            r.Write("<table class=\"u-full-width\">");
            r.Write("<thread>");
            r.Write("<tr>");
            r.Write("<th>Module</th>");
            r.Write("<th>Author</th>");
            r.Write("<th>Version</th>");
            r.Write("<th>Delete</th>");
            r.Write("</tr>");
            r.Write("</thread>");
            r.Write("<tbody>");

            for (auto it : Modules)
            {
                r.Write("<tr>");
                r.Write("<td><a href=\"/modules/%s\">%s</a></td>", it->name, it->name);
                r.Write("<td>%s</td>", it->GetAuthor());
                r.Write("<td>%s</td>", it->GetVersion());
                r.Write("<td><input type=\"checkbox\" id=\"%s\">", it->name);
                r.Write("</tr>");
            }

            r.Write("</tbody>");
            r.Write("</table>");

            r.Write("</html>");
            return true;
        }

        return false;
    }
};

class m_modules : public Module
{
    ListModules mlist;
public:
    m_modules(const Flux::string &Name) : Module(Name, MOD_NORMAL), mlist(this)
    {
        this->SetAuthor("Justasic");
        this->SetVersion(VERSION);
    }

    ~m_modules()
    {
    }
};

MODULE_HOOK(m_modules)
