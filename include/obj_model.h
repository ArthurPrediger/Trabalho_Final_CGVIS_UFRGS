#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include <glad/glad.h>
#include <glm/vec3.hpp>
#include <tiny_obj_loader.h>

// Estrutura que representa um modelo geométrico carregado a partir de um
// arquivo ".obj". Veja https://en.wikipedia.org/wiki/Wavefront_.obj_file .
class ObjModel
{
public:
    // Este construtor lê o modelo de um arquivo utilizando a biblioteca tinyobjloader.
    // Veja: https://github.com/syoyo/tinyobjloader
    ObjModel(const char* filepath, const char* basepath = NULL, bool triangulate = true);

    struct MaterialTexturesIds
    {
        int32_t diffuse_id;
        int32_t emissive_id;
        int32_t opacity_id;
    };

    std::string                       filepath;
    tinyobj::attrib_t                 attrib;
    std::vector<tinyobj::shape_t>     shapes;
    std::vector<tinyobj::material_t>  materials;
    std::unordered_map<uint32_t, MaterialTexturesIds> textures_ids;

    glm::vec3 min_bounds;
    glm::vec3 max_bounds;

    // Data per submesh/shape
    std::vector<size_t>       first_indices; // Índice do primeiro vértice dentro do vetor indices[] definido em BuildTrianglesAndAddToVirtualScene()
    std::vector<size_t>       num_indices; // Número de índices do objeto dentro do vetor indices[] definido em BuildTrianglesAndAddToVirtualScene()
    std::vector<GLuint>       vertex_array_object_ids; // ID do VAO onde estão armazenados os atributos do modelo
    std::vector<glm::vec3>    bboxes_min; // Axis-Aligned Bounding Box do objeto
    std::vector<glm::vec3>    bboxes_max;
};

GLuint LoadTextureImage(const char* filename); // Função que carrega imagens de textura