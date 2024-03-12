#pragma once

#include <string>

struct Action {
	enum Actions {
		Mouse,
		Keyboard
	};
};

class Command
{
private: 
	std::string m_name{ "NONE" };
	std::string m_type{ "NONE" };

	Action::Actions action;

public:
	Command();
	Command(const std::string& name, const std::string& type, Action::Actions action);

	const std::string& name() const;
	const std::string& type() const;

	std::string toString() const;
};

