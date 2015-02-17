#pragma once

#include <functional>
#include <vector>
#include <typeinfo>

template<class... arguments__>
class Event
{
protected:
	std::vector<std::function<bool(arguments__...)>> callables;
public:
	template<class callable>
	void add(callable c)
	{
		std::function<bool(arguments__...)> func = c;
		callables.push_back(func);
	}

	bool call(arguments__... args)
	{
		for (auto it : callables)
		{
			if (!it) // Not callable.
				continue;

			// Call the event, if it returns false then stop the loop and
			// return false (event stop)
			// otherwise continue until everything has been called.
			if (it(args...) == false)
				return false;
		}
		return true;
	}

	// I would have done a templated call and compare std::function to std::function
	// but std::function::operator == was deleted so I can't make that compare.
	// lil shits.
	bool remove(const size_t pos)
	{
		if (pos < this->callables.size())
		{
			this->callables.erase(this->callables.begin()+pos);
			return true;
		}
		else
			return false;
	}

	template<typename callable>
	bool remove(callable c)
	{
		for (auto begin = this->callables.begin(), end = this->callables.end(); begin != end; ++begin)
		{
			if (typeid(c) == (*begin).target_type())
			{
				this->callables.erase(begin);
				return true;
			}
		}
		return false;
	}

	void reset()
	{
		this->callables.clear();
		this->callables.shrink_to_fit();
	}

	size_t eventcount() const
	{
		return this->callables.size();
	}

	bool hasevents() const
	{
		return !this->callables.empty();
	}

	std::function<bool(arguments__...)> geteventcallable(const size_t pos)
	{
		if (pos < this->callables.size())
			return this->callables[pos];
		else
			return std::function<bool(arguments__...)>();
	}

	// Same as geteventcallable
	std::function<bool(arguments__...)> operator [] (const size_t pos)
	{
		return this->geteventcallable(pos);
	}

	// Same as hasevents
	bool operator ! () const
	{
		return this->hasevents();
	}

	// Same as call
	bool operator () (arguments__... args)
	{
		return this->call(args...);
	}

	// Same as add
	template<class callable>
	void operator += (callable c)
	{
		std::function<bool(arguments__...)> func = c;
		callables.push_back(func);
	}
};
