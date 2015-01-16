#include "page.h"
#include <map>

std::map<std::string, Page*> PageMap;

Page::Page(Module *m, std::string url) : urlre(url), owner(m)
{
	PageMap[url] = this;
}

Page::~Page()
{
	auto it = PageMap.find(this->urlre);
	if (it != PageMap.end())
		PageMap.erase(it);
}
