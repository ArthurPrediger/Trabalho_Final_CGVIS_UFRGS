#pragma once

#include "element.h"

#include <unordered_map>
#include <vector>
#include <memory>
#include <glm/vec3.hpp>
#include <glad/glad.h> 
#include <tiny_obj_loader.h>

class Component : public Element
{
public:
	Component(const std::string& type, const std::string& name = "");
	virtual ~Component() = default;

	std::string GetType() { return type; }

	void AttachComponent(std::shared_ptr<Component> component);

	template <typename T>
	std::shared_ptr<T> AttachComponent()
	{
		std::shared_ptr<T> component = std::make_shared<T>();
		AttachComponent(component);
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
	std::unordered_map<std::string, std::vector<std::shared_ptr<Component>>> components = {};

protected:
	std::string type;
};

class TransformComp : public Component
{
public:
	TransformComp(const std::string& name = "");

	static std::string GetComponentType() { return "TransformComp"; };

public:
	glm::vec3 position = { 0, 0, 0 };
	glm::vec3 scale = { 1, 1, 1 };
	glm::vec3 rotation = { 0, 0, 0 };
};

// Definimos uma estrutura que armazenará dados necessários para renderizar
// cada objeto da cena virtual.
class SceneObjectComp : public Component
{
public:
	SceneObjectComp(const std::string& name = "");

	static std::string GetComponentType() { return "SceneObjectComp"; };

public:
	std::string  object_name;        // Nome do objeto
	size_t       first_index; // Índice do primeiro vértice dentro do vetor indices[] definido em BuildTrianglesAndAddToVirtualScene()
	size_t       num_indices; // Número de índices do objeto dentro do vetor indices[] definido em BuildTrianglesAndAddToVirtualScene()
	GLenum       rendering_mode; // Modo de rasterização (GL_TRIANGLES, GL_TRIANGLE_STRIP, etc.)
	GLuint       vertex_array_object_id; // ID do VAO onde estão armazenados os atributos do modelo
	glm::vec3    bbox_min; // Axis-Aligned Bounding Box do objeto
	glm::vec3    bbox_max;

	class ObjModel*  model;
	uint32_t submesh_index;     // Índice do submesh dentro do modelo 3D (objeto do tipo ObjModel, definido em objloader.h)
};