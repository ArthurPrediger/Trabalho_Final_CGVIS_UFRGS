#include "obj_model.h"

#include <limits>
#include <stdexcept>

#include <stb_image.h>

constexpr int32_t max_int32 = (std::numeric_limits<int32_t>::max)();

// set makeprg=cd\ ..\ &&\ make\ run\ >/dev/null
// vim: set spell spelllang=pt_br :

// Função que carrega uma imagem para ser utilizada como textura
GLuint LoadTextureImage(const char* filename)
{
    printf("Carregando imagem \"%s\"... ", filename);

    // Primeiro fazemos a leitura da imagem do disco
    stbi_set_flip_vertically_on_load(true);
    int width;
    int height;
    int channels;
    unsigned char* data = stbi_load(filename, &width, &height, &channels, 3);

    if (data == NULL)
    {
        fprintf(stderr, "ERROR: Cannot open image file \"%s\".\n", filename);
        std::exit(EXIT_FAILURE);
    }

    printf("OK (%dx%d).\n", width, height);

    // Agora criamos objetos na GPU com OpenGL para armazenar a textura
    GLuint texture_id;
    glGenTextures(1, &texture_id);

    // Agora enviamos a imagem lida do disco para a GPU
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    // Texture parameters directly on the texture object (simpler for most cases)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);

    glBindTexture(GL_TEXTURE_2D, 0);   // unbind

    return texture_id;
}

ObjModel::ObjModel(const char* filepath, const char* basepath, bool triangulate)
{
    this->filepath = filepath;
    printf("Carregando objetos do arquivo \"%s\"...\n", filepath);

    // Se basepath == NULL, então setamos basepath como o dirname do
    // filename, para que os arquivos MTL sejam corretamente carregados caso
    // estejam no mesmo diretório dos arquivos OBJ.
    std::string fullpath(filepath);
    std::string dirname;
    if (basepath == NULL)
    {
        auto i = fullpath.find_last_of("/");
        if (i != std::string::npos)
        {
            dirname = fullpath.substr(0, i + 1);
            basepath = dirname.c_str();
        }
    }

    std::string warn;
    std::string err;
    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath, basepath, triangulate);

    if (!err.empty())
        fprintf(stderr, "\n%s\n", err.c_str());

    if (!ret)
        throw std::runtime_error("Erro ao carregar modelo.");

    for (size_t shape = 0; shape < shapes.size(); ++shape)
    {
        if (shapes[shape].name.empty())
        {
            fprintf(stderr,
                "*********************************************\n"
                "Erro: Objeto sem nome dentro do arquivo '%s'.\n"
                "Veja https://www.inf.ufrgs.br/~eslgastal/fcg-faq-etc.html#Modelos-3D-no-formato-OBJ .\n"
                "*********************************************\n",
                filepath);
            throw std::runtime_error("Objeto sem nome.");
        }
        printf("- Objeto '%s'\n", shapes[shape].name.c_str());
    }

    printf("OK.\n");

    int32_t material_index = 0;
    for (const auto& mat : materials)
    {
        bool has_any_texture = false;
        MaterialTexturesIds mat_textures_ids = { max_int32, max_int32, max_int32 };

        if (!mat.diffuse_texname.empty())
        {
            std::string text_path = dirname + mat.diffuse_texname;

            mat_textures_ids.diffuse_id = LoadTextureImage(text_path.c_str());
            has_any_texture = true;
        }

        if (!mat.emissive_texname.empty())
        {
            std::string text_path = dirname + mat.emissive_texname;

            mat_textures_ids.emissive_id = LoadTextureImage(text_path.c_str());
            has_any_texture = true;
        }

        if (!mat.alpha_texname.empty())
        {
            std::string text_path = dirname + mat.alpha_texname;

            mat_textures_ids.opacity_id = LoadTextureImage(text_path.c_str());
            has_any_texture = true;
        }

        if (has_any_texture) textures_ids.emplace(material_index, mat_textures_ids);

        material_index++;
    }
}

