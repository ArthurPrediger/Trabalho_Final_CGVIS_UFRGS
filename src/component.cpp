#include "component.h"

Component::Component(const std::string& type, const std::string& name)
	:
	Element(name.empty() ? type + "_" : name)
{
	this->type = type;	
}

void Component::AttachComponent(std::shared_ptr<Component> component)
{
	if (component == nullptr)
		return;
	component->owner = this;
	components[component->GetType()].push_back(component);
}

TransformComp::TransformComp(const std::string& name)
	:
	Component(GetComponentType(), name)
{
}

SceneObjectComp::SceneObjectComp(const std::string& name)
	:
	Component(GetComponentType(), name)
{
}
