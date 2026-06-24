#pragma once

#include <string>

class Element
{
public:
	Element(const std::string& name = "");
	virtual ~Element() = default;

	uint32_t GetId() const { return id; };

public:
	std::string name;

private:
	uint32_t id;
};