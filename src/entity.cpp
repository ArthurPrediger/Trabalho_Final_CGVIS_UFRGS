#include "entity.h"

Entity::Entity(const std::string& name)
	:
	Element(name)
{
	root = std::make_unique<TransformComp>();
}

void Entity::AddComponent(std::shared_ptr<Component> component)
{
	if (component == nullptr)
		return;
	component->owner = this;
	components[component->GetType()].push_back(component);
}

void Entity::AddComponents(std::vector<std::shared_ptr<Component>> components)
{
	for (auto& component : components)
		AddComponent(component);
}
