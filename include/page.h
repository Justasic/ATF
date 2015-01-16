#pragma once
// #include <regex>
#include "module.h"
#include "flux.h"
#include "Request.h"


class Module;

class Page
{
	std::string urlre; // The URL regular expression to match to
public:
	Page(Module *m, std::string url);
	~Page();

	virtual bool Run(Request &r, const Flux::string &url) = 0;

	virtual std::string GetExpression() { return this->urlre; }

	Module *owner;
};
