#include "entity.h"

Entity::Entity(const std::string& name)
	:
	Element(name)
{
	root = std::make_unique<TransformComp>();
}
