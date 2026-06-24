#include "element.h"

Element::Element(const std::string& name)
{
	static int32_t id_counter = 0;
	id = id_counter++;

	this->name = name.empty() ? std::string{ "Element_" + std::to_string(id) } : name + std::to_string(id);
}
