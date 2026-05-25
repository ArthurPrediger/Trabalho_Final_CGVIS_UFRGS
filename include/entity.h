#pragma once

#include "element.h"
#include "component.h"

#include <unordered_map>
#include <vector>
#include <memory>

class Entity : public Element
{
public:
	Entity(const std::string& name = "");
	Entity(const Entity& other) = default;
	Entity(Entity&& other) noexcept = default;
	Entity& operator=(const Entity& other) = default;
	Entity& operator=(Entity&& other) noexcept = default;
	~Entity() = default;

	template<typename T>
	void AddComponent(std::shared_ptr<T> component)
	{
		if (component == nullptr)
			return;
		component->owner = this;
		components[component->GetType()].push_back(component);
	}

	template<typename T>
	void AddComponents(std::vector<std::shared_ptr<T>> components)
	{
		for (auto& component : components)
			AddComponent<T>(component);
	}

	template <typename T>
	std::shared_ptr<T> AddComponent()
	{
		std::shared_ptr<T> component = std::make_shared<T>();
		AddComponent(component);
		return component;
	}

	template<typename T>
	std::vector<std::shared_ptr<T>> GetComponentsByType() const
	{
		auto it = components.find(T::GetComponentType());

		if (it == components.end())
			return {};

		std::vector<std::shared_ptr<T>> result;

		for (const auto& ptr : it->second)
		{
			result.push_back(std::static_pointer_cast<T>(ptr));
		}

		return result;
	}

public:
	Element* owner = nullptr;
	std::unique_ptr<TransformComp> root;
	std::unordered_map<std::string, std::vector<std::shared_ptr<Component>>> components = {};
};