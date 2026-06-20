
//     Universidade Federal do Rio Grande do Sul
//             Instituto de Informática
//       Departamento de Informática Aplicada
//
//    INF01047 Computação Gráfica e Visualização I
//               Prof. Eduardo Gastal
//
//     CÓDIGO BASE PARA O TRABALHO FINAL
//

// Arquivos "headers" padrões de C podem ser incluídos em um
// programa C++, sendo necessário somente adicionar o caractere
// "c" antes de seu nome, e remover o sufixo ".h". Exemplo:
//    #include <stdio.h> // Em C
//  vira
//    #include <cstdio> // Em C++
//
#include <cmath>
#include <cstdio>
#include <cstdlib>

// Headers abaixo são específicos de C++
#include <set>
#include <map>
#include <stack>
#include <string>
#include <vector>
#include <array>
#include <limits>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <unordered_map>
#include <iostream>
#include <format>

// Headers das bibliotecas OpenGL
#include <glad/glad.h>   // Criação de contexto OpenGL 3.3
#include <GLFW/glfw3.h>  // Criação de janelas do sistema operacional

// Headers da biblioteca GLM: criação de matrizes e vetores.
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/type_ptr.hpp>

// Headers da biblioteca para carregar modelos obj
#include <tiny_obj_loader.h>

#include <stb_image.h>

// Headers locais, definidos na pasta "include/"
#include "utils.h"

#include "entity.h"
#include "curve_path_loader.h"
#include "matrix_operations.h"

constexpr int32_t max_int32 = std::numeric_limits<GLuint>::max();

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

// Declaração de funções utilizadas para pilha de matrizes de modelagem.
//void PushMatrix(glm::mat4 M);
//void PopMatrix(glm::mat4& M);

// Declaração de várias funções utilizadas em main().  Essas estão definidas
// logo após a definição de main() neste arquivo.
void BuildTrianglesAndBuffers(std::shared_ptr<ObjModel> model); // Constrói representação de um ObjModel como malha de triângulos para renderização
void ComputeNormals(std::shared_ptr<ObjModel> model); // Computa normais de um ObjModel, caso não existam.
void DivideModelMeshesByMaterial(std::shared_ptr<ObjModel> model);
void LoadShadersFromFiles(); // Carrega os shaders de vértice e fragmento, criando um programa de GPU
GLuint LoadTextureImage(const char* filename); // Função que carrega imagens de textura
void DrawVirtualMesh(std::shared_ptr<MeshComp> virtual_mesh_component); // Desenha uma um componente de malha 
GLuint LoadShader_Vertex(const char* filename);   // Carrega um vertex shader
GLuint LoadShader_Fragment(const char* filename); // Carrega um fragment shader
void LoadShader(const char* filename, GLuint shader_id); // Função utilizada pelas duas acima
GLuint CreateGpuProgram(GLuint vertex_shader_id, GLuint fragment_shader_id); // Cria um programa de GPU
void PrintObjModelInfo(std::shared_ptr<ObjModel> model); // Função para debugging

// Declaração de funções auxiliares para renderizar texto dentro da janela
// OpenGL. Estas funções estão definidas no arquivo "textrendering.cpp".
void TextRendering_Init();
float TextRendering_LineHeight(GLFWwindow* window);
float TextRendering_CharWidth(GLFWwindow* window);
void TextRendering_PrintString(GLFWwindow* window, const std::string& str, float x, float y, float scale = 1.0f, glm::vec3 text_color = { 0, 0, 0 });
void TextRendering_PrintMatrix(GLFWwindow* window, glm::mat4 M, float x, float y, float scale = 1.0f);
void TextRendering_PrintVector(GLFWwindow* window, glm::vec4 v, float x, float y, float scale = 1.0f);
void TextRendering_PrintMatrixVectorProduct(GLFWwindow* window, glm::mat4 M, glm::vec4 v, float x, float y, float scale = 1.0f);
void TextRendering_PrintMatrixVectorProductMoreDigits(GLFWwindow* window, glm::mat4 M, glm::vec4 v, float x, float y, float scale = 1.0f);
void TextRendering_PrintMatrixVectorProductDivW(GLFWwindow* window, glm::mat4 M, glm::vec4 v, float x, float y, float scale = 1.0f);

// Funções abaixo renderizam como texto na janela OpenGL algumas matrizes e
// outras informações do programa. Definidas após main().
void TextRendering_ShowModelViewProjection(GLFWwindow* window, glm::mat4 projection, glm::mat4 view, glm::mat4 model, glm::vec4 p_model);
void TextRendering_ShowEulerAngles(GLFWwindow* window);
void TextRendering_ShowProjection(GLFWwindow* window);
void TextRendering_ShowFramesPerSecond(GLFWwindow* window);

// Funções callback para comunicação com o sistema operacional e interação do
// usuário. Veja mais comentários nas definições das mesmas, abaixo.
void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
void ErrorCallback(int error, const char* description);
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void CursorPosCallback(GLFWwindow* window, double xpos, double ypos);
void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

// New user classes definitions
class Car : public Entity
{
public:
	Car(const std::string& name = "") : Entity(name) {};

public:
    glm::vec3 forward = glm::vec3(0.0f);
	float speed;
	std::array<int32_t, 4> input_keys;
    std::vector<std::shared_ptr<MeshComp>> wheels_mesh_comps;
    std::vector<std::shared_ptr<TransformComp>> wheels_transform_comps;
    int32_t cur_curve_point;
    glm::vec3 cur_curve_pos;
    int32_t cur_lane;
    glm::vec3 transition_curve_pos;
    int32_t target_lane;
	bool is_accelerating = false;
	bool is_out_of_control = false;
    float yaw_tremble_timer = 0.0f;
	float out_of_control_timer = 0.0f;
	float lane_transitioning_length = 0.0f;
	int32_t laps_completed = 0;
};

class Track : public Entity
{
public:
    Track(const std::string& name = "") : Entity(name) {};
public:
	std::vector<std::vector<glm::vec3>> lanes;
	std::vector<float> lane_lengths;
	std::vector<float> normalized_lane_lengths;
};

// New user functions declarations
void UpdateGameStartScreen(GLFWwindow* window, double delta_time);
void UpdateGameOverScreen(GLFWwindow* window, double delta_time, const std::vector<std::shared_ptr<Car>>& cars, std::shared_ptr<Track> track);
void RestartGame(const std::vector<std::shared_ptr<Car>>& cars, std::shared_ptr<Track> track);
void UpdateFreeCamera(double delta_time);
void UpdateRaceCamera(double delta_time, const std::vector<std::shared_ptr<Car>>& cars);
void UpdateCountdownCamera(float normalized_countdown_time);
void DrawEntity(const std::shared_ptr<Entity> entity);
void UpdateRaceUserInterface(GLFWwindow* window, const std::vector<std::shared_ptr<Car>>& cars);
std::vector<std::shared_ptr<MeshComp>> CreateMeshComponentsForModelByName(const std::string& model_name);
std::shared_ptr<Car> CreateCar(const std::string& name, const std::shared_ptr<ObjModel>& model, const std::array<int32_t, 4>& input_keys, glm::vec3 color = { 0, 0, 0 });
void UpdateCarInputAndAnimation(double delta_time, std::shared_ptr<Car> car, std::shared_ptr<Track> track);
void UpdateCarsPhysics(double delta_time, std::vector<std::shared_ptr<Car>> cars, std::shared_ptr<Track> track);
std::vector<std::vector<glm::vec3>> SplitCurvePathInLanes(const std::vector<glm::vec3>& points, int32_t num_lanes);
std::vector<float> ComputeLaneLengths(const std::vector<std::vector<glm::vec3>>& lanes);
std::vector<float> ComputeNormalizedLaneLengths(const std::vector<std::vector<glm::vec3>>& lanes);
float ComputeCurveRadius(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2);
float ComputeCarSpeedRelativeToTrackCurvature(std::shared_ptr<Car> car, std::shared_ptr<Track> track);

// Abaixo definimos variáveis globais utilizadas em várias funções do código.

// A cena virtual é uma lista de objetos nomeados, guardados em um dicionário
// (map).  Veja dentro da função BuildTrianglesAndAddToVirtualScene() como que são incluídos
// objetos dentro da variável g_VirtualScene, e veja na função main() como
// estes são acessados.
std::unordered_map<std::string, std::shared_ptr<ObjModel>> g_loaded_models;
std::unordered_map<uint32_t, std::vector<std::shared_ptr<MeshComp>>> g_entities_virtual_meshes;

// Pilha que guardará as matrizes de modelagem.
//std::stack<glm::mat4>  g_MatrixStack;

// Razão de proporção da janela (largura/altura). Veja função FramebufferSizeCallback().
float g_ScreenRatio = 1.0f;

// Ângulos de Euler que controlam a rotação de um dos cubos da cena virtual
float g_AngleX = 0.0f;
float g_AngleY = 0.0f;
float g_AngleZ = 0.0f;

// "g_LeftMouseButtonPressed = true" se o usuário está com o botão esquerdo do mouse
// pressionado no momento atual. Veja função MouseButtonCallback().
bool g_left_mouse_button_pressed = false;
bool g_right_mouse_button_pressed = false; // Análogo para botão direito do mouse
bool g_middle_mouse_button_pressed = false; // Análogo para botão do meio do mouse
double g_last_mouse_cursor_x = 0.0;
double g_last_mouse_cursor_y = 0.0;
double g_mouse_cursor_delta_x = 0.0;
double g_mouse_cursor_delta_y = 0.0;

bool g_first_mouse = true;
bool keys[GLFW_KEY_LAST];

bool g_is_on_game_start = true;
bool g_is_playing_countdown = false;
bool g_is_game_running = false;
bool g_is_game_over = false;
int32_t g_num_laps = 4;
constexpr int32_t g_init_curve_point = 775;

static constexpr float g_countdown_duration = 4.0f;
static float g_countdown_time = g_countdown_duration;

static constexpr float g_lane_transitioning_length = 10.0f;

glm::vec4 camera_position = { 6.99f, 7.26f, -5.00f, 1.0f };
glm::vec4 camera_forward = { 0.0f, 0.0f, 1.0f, 0.0f };
glm::vec4 camera_up = { 0.0f, 1.0f, 0.0f, 0.0f };
glm::vec4 camera_right = glm::vec4(glm::cross(glm::vec3(camera_forward), glm::vec3(camera_up)), 0.0f);
float camera_pitch = glm::radians<float>(-36.38);
float camera_yaw = glm::radians<float>(-212.90);
constexpr float camera_move_speed = 16.0f;
constexpr float camera_rotation_speed = 0.005f;
//double delta_time = 0.0f;

// Variável que controla o tipo de projeção utilizada: perspectiva ou ortográfica.
bool g_UsePerspectiveProjection = true;

// Variável que controla se o texto informativo será mostrado na tela.
bool g_show_info_text = true;

// Variáveis que definem um programa de GPU (shaders). Veja função LoadShadersFromFiles().
GLuint g_GpuProgramID = 0;
GLint g_model_uniform;
GLint g_view_uniform;
GLint g_camera_position_uniform;
GLint g_projection_uniform;
GLint g_object_id_uniform;
GLint g_bbox_min_uniform;
GLint g_bbox_max_uniform;
GLint g_texture_uniform_0;
GLint g_texture_uniform_1;
GLint g_texture_uniform_2;
GLint g_has_kd_texture_uniform;
GLint g_has_ke_texture_uniform;
GLint g_has_opacity_texture_uniform;
GLint g_kd_uniform;
GLint g_ks_uniform;
GLint g_ke_uniform;
GLint g_ns_uniform;
GLint g_opacity_uniform;

// Número de texturas carregadas pela função LoadTextureImage()
//GLuint g_NumLoadedTextures = 0;

int main(int argc, char* argv[])
{
    // Inicializamos a biblioteca GLFW, utilizada para criar uma janela do
    // sistema operacional, onde poderemos renderizar com OpenGL.
    int success = glfwInit();
    if (!success)
    {
        fprintf(stderr, "ERROR: glfwInit() failed.\n");
        std::exit(EXIT_FAILURE);
    }

    // Definimos o callback para impressão de erros da GLFW no terminal
    glfwSetErrorCallback(ErrorCallback);

    // Pedimos para utilizar OpenGL versão 3.3 (ou superior)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    #ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

    // Pedimos para utilizar o perfil "core", isto é, utilizaremos somente as
    // funções modernas de OpenGL.
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Criamos uma janela do sistema operacional, com 800 colunas e 600 linhas
    // de pixels, e com título "INF01047 ...".
    GLFWwindow* window;
    window = glfwCreateWindow(800, 600, "INF01047 - Seu Cartao - Seu Nome", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        fprintf(stderr, "ERROR: glfwCreateWindow() failed.\n");
        std::exit(EXIT_FAILURE);
    }

    // Definimos a função de callback que será chamada sempre que o usuário
    // pressionar alguma tecla do teclado ...
    glfwSetKeyCallback(window, KeyCallback);
    // ... ou clicar os botões do mouse ...
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
    // ... ou movimentar o cursor do mouse em cima da janela ...
    glfwSetCursorPosCallback(window, CursorPosCallback);
    // ... ou rolar a "rodinha" do mouse.
    glfwSetScrollCallback(window, ScrollCallback);

    // Indicamos que as chamadas OpenGL deverão renderizar nesta janela
    glfwMakeContextCurrent(window);

    // Carregamento de todas funções definidas por OpenGL 3.3, utilizando a
    // biblioteca GLAD.
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);

    // Definimos a função de callback que será chamada sempre que a janela for
    // redimensionada, por consequência alterando o tamanho do "framebuffer"
    // (região de memória onde são armazenados os pixels da imagem).
    glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
    FramebufferSizeCallback(window, 800, 600); // Forçamos a chamada do callback acima, para definir g_ScreenRatio.

    // Imprimimos no terminal informações sobre a GPU do sistema
    const GLubyte *vendor      = glGetString(GL_VENDOR);
    const GLubyte *renderer    = glGetString(GL_RENDERER);
    const GLubyte *glversion   = glGetString(GL_VERSION);
    const GLubyte *glslversion = glGetString(GL_SHADING_LANGUAGE_VERSION);

    printf("GPU: %s, %s, OpenGL %s, GLSL %s\n", vendor, renderer, glversion, glslversion);

    // Carregamos os shaders de vértices e de fragmentos que serão utilizados
    // para renderização. Veja slides 180-200 do documento Aula_03_Rendering_Pipeline_Grafico.pdf.
    //
    LoadShadersFromFiles();

    // Track model and entity
    std::shared_ptr<ObjModel> curve_model = std::make_shared<ObjModel>("../../data/curve/curve.obj");
    ComputeNormals(curve_model);
    BuildTrianglesAndBuffers(curve_model);
	g_loaded_models.emplace(curve_model->filepath, curve_model);

    std::shared_ptr<Track> track = std::make_shared<Track>("track");
    track->AddComponents(CreateMeshComponentsForModelByName(curve_model->filepath));
    g_entities_virtual_meshes.emplace(track->GetId(), track->GetComponentsByType<MeshComp>());
    track->root->scale = { 0.25f, 0.25f, 0.25f };

    std::vector<glm::vec3> curve_points = LoadCurvePath("../../data/curve/trail.txt");

    track->lanes = SplitCurvePathInLanes(curve_points, 2),
    track->lane_lengths = ComputeLaneLengths(track->lanes);
    track->normalized_lane_lengths = ComputeNormalizedLaneLengths(track->lanes);

	// Finish line model and entity
	std::shared_ptr<ObjModel> finish_line_model = std::make_shared<ObjModel>("../../data/finish_line/finish_line.obj");
    ComputeNormals(finish_line_model);
    BuildTrianglesAndBuffers(finish_line_model);
	g_loaded_models.emplace(finish_line_model->filepath, finish_line_model);

	std::shared_ptr<Entity> finish_line = std::make_shared<Entity>("finish_line");
	finish_line->AddComponents(CreateMeshComponentsForModelByName(finish_line_model->filepath));
	g_entities_virtual_meshes.emplace(finish_line->GetId(), finish_line->GetComponentsByType<MeshComp>());
    glm::mat4 model = matops::MatrixIdentity(); // Transformação identidade de modelagem
    model *= matops::MatrixScale(track->root->scale.x, track->root->scale.y, track->root->scale.z);
	finish_line->root->position = model * glm::vec4(curve_points[(g_init_curve_point + 10) % curve_points.size()], 1.0f);
	finish_line->root->position.y += 0.01f;
	finish_line->root->rotation = { 0.0, 82.0f, 0.0f };
	finish_line->root->scale = { 1.0f, 1.0f, 0.2f };

    // Finish line model and entity
    std::shared_ptr<ObjModel> ground_model = std::make_shared<ObjModel>("../../data/ground/ground.obj");
    ComputeNormals(ground_model);
    BuildTrianglesAndBuffers(ground_model);
    g_loaded_models.emplace(ground_model->filepath, ground_model);

    std::shared_ptr<Entity> ground = std::make_shared<Entity>("ground");
    ground->AddComponents(CreateMeshComponentsForModelByName(ground_model->filepath));
    g_entities_virtual_meshes.emplace(ground->GetId(), ground->GetComponentsByType<MeshComp>());
    ground->root->position = track->root->position + glm::vec3(0.0f, curve_points[0].y * track->root->scale.y - 0.01, 0.0f);
	ground->root->scale = { 90.0f, 90.0f, 90.0f };

    // Car model and entities
    std::shared_ptr<ObjModel> car_zr1_model = std::make_shared<ObjModel>("../../data/zr1_model/ZR1.obj");
    ComputeNormals(car_zr1_model);
    DivideModelMeshesByMaterial(car_zr1_model);
    BuildTrianglesAndBuffers(car_zr1_model);
    g_loaded_models.emplace(car_zr1_model->filepath, car_zr1_model);

	std::vector<std::shared_ptr<Car>> cars = { 
        CreateCar("ZR1_car_0", car_zr1_model,{ GLFW_KEY_W, GLFW_KEY_S, GLFW_KEY_D, GLFW_KEY_A }, { 0.53, 0.13, 0.11 }),
        CreateCar("ZR1_car_1", car_zr1_model,{ GLFW_KEY_UP, GLFW_KEY_DOWN, GLFW_KEY_RIGHT, GLFW_KEY_LEFT }, { 0.09, 0.34, 0.063 }),
    };

    // Inicializamos o código para renderização de texto.
    TextRendering_Init();

    // Habilitamos o Z-buffer. Veja slides 104-116 do documento Aula_09_Projecoes.pdf.
    glEnable(GL_DEPTH_TEST);

    // Habilitamos o Backface Culling. Veja slides 8-13 do documento Aula_02_Fundamentos_Matematicos.pdf, slides 23-34 do documento Aula_13_Clipping_and_Culling.pdf e slides 112-123 do documento Aula_14_Laboratorio_3_Revisao.pdf.
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glfwGetCursorPos(window, &g_last_mouse_cursor_x, &g_last_mouse_cursor_y);

    RestartGame(cars, track);

    // Ficamos em um loop infinito, renderizando, até que o usuário feche a janela
    while (!glfwWindowShouldClose(window))
    {
        static double last_time = 0.0f;
        const double cur_time = glfwGetTime();
        double delta_time = cur_time - last_time;
        last_time = cur_time;

        if(g_is_playing_countdown)
        {
            g_countdown_time -= (float)(delta_time);
            UpdateCountdownCamera(std::clamp(g_countdown_time / g_countdown_duration, 0.0f, 1.0f));

            if(g_countdown_time <= 0)
            {
                g_is_playing_countdown = false;
                g_is_game_running = true;
			}
		}
        // INPUTS UPDATE
        if (g_is_game_running)
        {
            //UpdateFreeCamera(delta_time);
            UpdateRaceCamera(delta_time, cars);

            // Cars movement and animation updates based on user input
			for (std::shared_ptr<Car> car : cars)
            {
                UpdateCarInputAndAnimation(delta_time, car, track);
            }

            UpdateCarsPhysics(delta_time, cars, track);
        }

        // Aqui executamos as operações de renderização

        // Definimos a cor do "fundo" do framebuffer como branco.  Tal cor é
        // definida como coeficientes RGBA: Red, Green, Blue, Alpha; isto é:
        // Vermelho, Verde, Azul, Alpha (valor de transparência).
        // Conversaremos sobre sistemas de cores nas aulas de Modelos de Iluminação.
        //
        //           R     G     B     A
		float gray_value = powf(0.65f, 1.0f / 2.2f);
        glClearColor(gray_value, gray_value, gray_value, 1.0f);

        // "Pintamos" todos os pixels do framebuffer com a cor definida acima,
        // e também resetamos todos os pixels do Z-buffer (depth buffer).
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Pedimos para a GPU utilizar o programa de GPU criado acima (contendo
        // os shaders de vértice e fragmentos).
        glUseProgram(g_GpuProgramID);

        glUniform1i(g_texture_uniform_0, 0);
        glUniform1i(g_texture_uniform_1, 1);
        glUniform1i(g_texture_uniform_2, 2);

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Computamos a matriz "View" utilizando os parâmetros da câmera para
        // definir o sistema de coordenadas da câmera.  Veja slides 2-14, 184-190 e 236-242 do documento Aula_08_Sistemas_de_Coordenadas.pdf.
        glm::mat4 view = matops::MatrixCameraView(camera_position, camera_forward, camera_up);

        // Agora computamos a matriz de Projeção.
        glm::mat4 projection;

        // Note que, no sistema de coordenadas da câmera, os planos near e far
        // estão no sentido negativo! Veja slides 176-204 do documento Aula_09_Projecoes.pdf.
        float nearplane = -0.1f;  // Posição do "near plane"
        float farplane  = -1000.0f; // Posição do "far plane"

        // Projeção Perspectiva.
        // Para definição do field of view (FOV), veja slides 205-215 do documento Aula_09_Projecoes.pdf.
        float field_of_view = 3.141592 / 3.0f;
        projection = matops::MatrixPerspective(field_of_view, g_ScreenRatio, nearplane, farplane);

        // Enviamos as matrizes "view" e "projection" para a placa de vídeo
        // (GPU). Veja o arquivo "shader_vertex.glsl", onde estas são
        // efetivamente aplicadas em todos os pontos.
        glUniformMatrix4fv(g_view_uniform       , 1 , GL_FALSE , glm::value_ptr(view));
        glUniformMatrix4fv(g_projection_uniform , 1 , GL_FALSE , glm::value_ptr(projection));
        glUniform3f(g_camera_position_uniform, camera_position.x, camera_position.y, camera_position.z);

        // Desenhamos o chão
        DrawEntity(ground);
        // Desenhamos a pista
		DrawEntity(track);
        // Desenhamos a linha de chegada
		DrawEntity(finish_line);

        // Desenhamos os carros
        for (std::shared_ptr<Car> car : cars)
        {
            DrawEntity(car);
        }

        if (g_is_on_game_start)
        {
            UpdateGameStartScreen(window, delta_time);
        }
        else if (g_is_playing_countdown || g_countdown_time <= 0)
        {
            constexpr float countdown_text_scale = 4.0f;
			constexpr glm::vec3 countdown_text_color = { 0.75f, 0.75f, 0.1f };
            if(g_countdown_time > 0)
            {
                TextRendering_PrintString(window, std::to_string(int(std::ceil(g_countdown_time))), 0.0f - (TextRendering_CharWidth(window) / 2) * countdown_text_scale, 0.0f, countdown_text_scale, countdown_text_color);
            }
            else
            {
				TextRendering_PrintString(window, "GO!", 0.0f - (TextRendering_CharWidth(window) * 3 / 2) * countdown_text_scale, 0.0f, countdown_text_scale, countdown_text_color);
                
                g_countdown_time -= delta_time;
                g_countdown_time = g_countdown_time < -1.5f ? g_countdown_duration : g_countdown_time;
            }
		}
        else if(g_is_game_over)
        {
            UpdateGameOverScreen(window, delta_time, cars, track);
		}

        if (g_is_playing_countdown || g_is_game_running)
        {
            UpdateRaceUserInterface(window, cars);
        }

		//TextRendering_PrintVector(window, camera_position, -0.9, 0.9f);
		//TextRendering_PrintString(window, std::to_string(glm::degrees(camera_pitch)), -0.7, 0.9f);
		//TextRendering_PrintString(window, std::to_string(glm::degrees(camera_yaw)), -0.7, 0.85f);

        // Imprimimos na tela os ângulos de Euler que controlam a rotação do
        // terceiro cubo.
        TextRendering_ShowEulerAngles(window);

        // Imprimimos na informação sobre a matriz de projeção sendo utilizada.
        TextRendering_ShowProjection(window);

        // Imprimimos na tela informação sobre o número de quadros renderizados
        // por segundo (frames per second).
        TextRendering_ShowFramesPerSecond(window);

        // O framebuffer onde OpenGL executa as operações de renderização não
        // é o mesmo que está sendo mostrado para o usuário, caso contrário
        // seria possível ver artefatos conhecidos como "screen tearing". A
        // chamada abaixo faz a troca dos buffers, mostrando para o usuário
        // tudo que foi renderizado pelas funções acima.
        // Veja o link: https://en.wikipedia.org/w/index.php?title=Multiple_buffering&oldid=793452829#Double_buffering_in_computer_graphics
        glfwSwapBuffers(window);

        // Verificamos com o sistema operacional se houve alguma interação do
        // usuário (teclado, mouse, ...). Caso positivo, as funções de callback
        // definidas anteriormente usando glfwSet*Callback() serão chamadas
        // pela biblioteca GLFW.
        glfwPollEvents();
    }

    // Finalizamos o uso dos recursos do sistema operacional
    glfwTerminate();

    // Fim do programa
    return 0;
}

// Função que carrega uma imagem para ser utilizada como textura
GLuint LoadTextureImage(const char* filename)
{
    printf("Carregando imagem \"%s\"... ", filename);

    // Primeiro fazemos a leitura da imagem do disco
    stbi_set_flip_vertically_on_load(true);
    int width;
    int height;
    int channels;
    unsigned char *data = stbi_load(filename, &width, &height, &channels, 3);

    if ( data == NULL )
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

// Função que desenha um objeto armazenado em g_VirtualScene. Veja definição
// dos objetos na função BuildTrianglesAndAddToVirtualScene().
void DrawVirtualMesh(std::shared_ptr<MeshComp> virtual_scene_obj_component)
{
    // "Ligamos" o VAO. Informamos que queremos utilizar os atributos de
    // vértices apontados pelo VAO criado pela função BuildTrianglesAndAddToVirtualScene(). Veja
    // comentários detalhados dentro da definição de BuildTrianglesAndAddToVirtualScene().
    glBindVertexArray(virtual_scene_obj_component->vertex_array_object_id);

    // Setamos as variáveis "bbox_min" e "bbox_max" do fragment shader
    // com os parâmetros da axis-aligned bounding box (AABB) do modelo.
    glm::vec3 bbox_min = virtual_scene_obj_component->bbox_min;
    glm::vec3 bbox_max = virtual_scene_obj_component->bbox_max;
    glUniform4f(g_bbox_min_uniform, bbox_min.x, bbox_min.y, bbox_min.z, 1.0f);
    glUniform4f(g_bbox_max_uniform, bbox_max.x, bbox_max.y, bbox_max.z, 1.0f);

    // Pedimos para a GPU rasterizar os vértices dos eixos XYZ
    // apontados pelo VAO como linhas. Veja a definição de
    // g_VirtualScene[""] dentro da função BuildTrianglesAndAddToVirtualScene(), e veja
    // a documentação da função glDrawElements() em
    // http://docs.gl/gl3/glDrawElements.
    glDrawElements(
        virtual_scene_obj_component->rendering_mode,
        virtual_scene_obj_component->num_indices,
        GL_UNSIGNED_INT,
        (void*)(virtual_scene_obj_component->first_index * sizeof(GLuint))
    );

    // "Desligamos" o VAO, evitando assim que operações posteriores venham a
    // alterar o mesmo. Isso evita bugs.
    glBindVertexArray(0);
}

// Função que carrega os shaders de vértices e de fragmentos que serão
// utilizados para renderização. Veja slides 180-200 do documento Aula_03_Rendering_Pipeline_Grafico.pdf.
//
void LoadShadersFromFiles()
{
    // Note que o caminho para os arquivos "shader_vertex.glsl" e
    // "shader_fragment.glsl" estão fixados, sendo que assumimos a existência
    // da seguinte estrutura no sistema de arquivos:
    //
    //    + FCG_Lab_01/
    //    |
    //    +--+ bin/
    //    |  |
    //    |  +--+ Release/  (ou Debug/ ou Linux/)
    //    |     |
    //    |     o-- main.exe
    //    |
    //    +--+ src/
    //       |
    //       o-- shader_vertex.glsl
    //       |
    //       o-- shader_fragment.glsl
    //
    GLuint vertex_shader_id = LoadShader_Vertex("../../shaders/shader_vertex.glsl");
    //GLuint fragment_shader_id = LoadShader_Fragment("../../shaders/shader_fragment.glsl");
    GLuint fragment_shader_id = LoadShader_Fragment("../../shaders/shader_fragment_blinn_phong.glsl");

    // Deletamos o programa de GPU anterior, caso ele exista.
    if ( g_GpuProgramID != 0 )
        glDeleteProgram(g_GpuProgramID);

    // Criamos um programa de GPU utilizando os shaders carregados acima.
    g_GpuProgramID = CreateGpuProgram(vertex_shader_id, fragment_shader_id);

    // Buscamos o endereço das variáveis definidas dentro do Vertex Shader.
    // Utilizaremos estas variáveis para enviar dados para a placa de vídeo
    // (GPU)! Veja arquivo "shader_vertex.glsl" e "shader_fragment.glsl".
    g_model_uniform       = glGetUniformLocation(g_GpuProgramID, "model"); // Variável da matriz "model"
    g_view_uniform        = glGetUniformLocation(g_GpuProgramID, "view"); // Variável da matriz "view" em shader_vertex.glsl
    g_projection_uniform  = glGetUniformLocation(g_GpuProgramID, "projection"); // Variável da matriz "projection" em shader_vertex.glsl
	g_camera_position_uniform = glGetUniformLocation(g_GpuProgramID, "camera_position"); // Variável da posição da câmera em shader_fragment.glsl
    g_object_id_uniform   = glGetUniformLocation(g_GpuProgramID, "object_id"); // Variável "object_id" em shader_fragment.glsl
    g_bbox_min_uniform    = glGetUniformLocation(g_GpuProgramID, "bbox_min");
    g_bbox_max_uniform    = glGetUniformLocation(g_GpuProgramID, "bbox_max");
	g_has_kd_texture_uniform = glGetUniformLocation(g_GpuProgramID, "has_kd_texture");
	g_has_ke_texture_uniform = glGetUniformLocation(g_GpuProgramID, "has_ke_texture");
	g_has_opacity_texture_uniform = glGetUniformLocation(g_GpuProgramID, "has_opacity_texture");
	g_kd_uniform          = glGetUniformLocation(g_GpuProgramID, "kd");
	g_ks_uniform          = glGetUniformLocation(g_GpuProgramID, "ks");
	g_ke_uniform          = glGetUniformLocation(g_GpuProgramID, "ke");
	g_ns_uniform          = glGetUniformLocation(g_GpuProgramID, "ns");
	g_opacity_uniform          = glGetUniformLocation(g_GpuProgramID, "opacity");

    // Variáveis em "shader_fragment.glsl" para acesso das imagens de textura
    glUseProgram(g_GpuProgramID);
    g_texture_uniform_0 = glGetUniformLocation(g_GpuProgramID, "texture_sampler_kd");
    g_texture_uniform_1 = glGetUniformLocation(g_GpuProgramID, "texture_sampler_ke");
    g_texture_uniform_2 = glGetUniformLocation(g_GpuProgramID, "texture_sampler_opacity");
    //glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage0"), 0);
    //glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage1"), 1);
    //glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage2"), 2);
    glUseProgram(0);
}

// Função que computa as normais de um ObjModel, caso elas não tenham sido
// especificadas dentro do arquivo ".obj"
void ComputeNormals(std::shared_ptr<ObjModel> model)
{
    if ( !model->attrib.normals.empty() )
        return;

    // Primeiro computamos as normais para todos os TRIÂNGULOS.
    // Segundo, computamos as normais dos VÉRTICES através do método proposto
    // por Gouraud, onde a normal de cada vértice vai ser a média das normais de
    // todas as faces que compartilham este vértice e que pertencem ao mesmo "smoothing group".

    // Obtemos a lista dos smoothing groups que existem no objeto
    std::set<unsigned int> sgroup_ids;
    for (size_t shape = 0; shape < model->shapes.size(); ++shape)
    {
        size_t num_triangles = model->shapes[shape].mesh.num_face_vertices.size();

        assert(model->shapes[shape].mesh.smoothing_group_ids.size() == num_triangles);

        for (size_t triangle = 0; triangle < num_triangles; ++triangle)
        {
            assert(model->shapes[shape].mesh.num_face_vertices[triangle] == 3);
            unsigned int sgroup = model->shapes[shape].mesh.smoothing_group_ids[triangle];
            assert(sgroup >= 0);
            sgroup_ids.insert(sgroup);
        }
    }

    size_t num_vertices = model->attrib.vertices.size() / 3;
    model->attrib.normals.reserve( 3*num_vertices );

    // Processamos um smoothing group por vez
    for (const unsigned int & sgroup : sgroup_ids)
    {
        std::vector<int> num_triangles_per_vertex(num_vertices, 0);
        std::vector<glm::vec4> vertex_normals(num_vertices, glm::vec4(0.0f,0.0f,0.0f,0.0f));

        // Acumulamos as normais dos vértices de todos triângulos deste smoothing group
        for (size_t shape = 0; shape < model->shapes.size(); ++shape)
        {
            size_t num_triangles = model->shapes[shape].mesh.num_face_vertices.size();

            for (size_t triangle = 0; triangle < num_triangles; ++triangle)
            {
                unsigned int sgroup_tri = model->shapes[shape].mesh.smoothing_group_ids[triangle];

                if (sgroup_tri != sgroup)
                    continue;

                glm::vec4  vertices[3];
                for (size_t vertex = 0; vertex < 3; ++vertex)
                {
                    tinyobj::index_t idx = model->shapes[shape].mesh.indices[3*triangle + vertex];
                    const float vx = model->attrib.vertices[3*idx.vertex_index + 0];
                    const float vy = model->attrib.vertices[3*idx.vertex_index + 1];
                    const float vz = model->attrib.vertices[3*idx.vertex_index + 2];
                    vertices[vertex] = glm::vec4(vx,vy,vz,1.0);
                }

                const glm::vec4  a = vertices[0];
                const glm::vec4  b = vertices[1];
                const glm::vec4  c = vertices[2];

                const glm::vec4  n = glm::vec4(glm::cross(glm::vec3(b-a),glm::vec3(c-a)), 0.0f);

                for (size_t vertex = 0; vertex < 3; ++vertex)
                {
                    tinyobj::index_t idx = model->shapes[shape].mesh.indices[3*triangle + vertex];
                    num_triangles_per_vertex[idx.vertex_index] += 1;
                    vertex_normals[idx.vertex_index] += n;
                }
            }
        }

        // Computamos a média das normais acumuladas
        std::vector<size_t> normal_indices(num_vertices, 0);

        for (size_t vertex_index = 0; vertex_index < vertex_normals.size(); ++vertex_index)
        {
            if (num_triangles_per_vertex[vertex_index] == 0)
                continue;

            glm::vec4 n = vertex_normals[vertex_index] / (float)num_triangles_per_vertex[vertex_index];
            n = glm::normalize(n);

            model->attrib.normals.push_back( n.x );
            model->attrib.normals.push_back( n.y );
            model->attrib.normals.push_back( n.z );

            size_t normal_index = (model->attrib.normals.size() / 3) - 1;
            normal_indices[vertex_index] = normal_index;
        }

        // Escrevemos os índices das normais para os vértices dos triângulos deste smoothing group
        for (size_t shape = 0; shape < model->shapes.size(); ++shape)
        {
            size_t num_triangles = model->shapes[shape].mesh.num_face_vertices.size();

            for (size_t triangle = 0; triangle < num_triangles; ++triangle)
            {
                unsigned int sgroup_tri = model->shapes[shape].mesh.smoothing_group_ids[triangle];

                if (sgroup_tri != sgroup)
                    continue;

                for (size_t vertex = 0; vertex < 3; ++vertex)
                {
                    tinyobj::index_t idx = model->shapes[shape].mesh.indices[3*triangle + vertex];
                    model->shapes[shape].mesh.indices[3*triangle + vertex].normal_index =
                        normal_indices[ idx.vertex_index ];
                }
            }
        }

    }
}

void DivideModelMeshesByMaterial(std::shared_ptr<ObjModel> model)
{
    std::vector<tinyobj::shape_t> new_shapes;

    for (const auto& shape : model->shapes)
    {
        // Check whether all faces use the same material
        bool multiple_materials = false;

        if (!shape.mesh.material_ids.empty())
        {
            int first_mat = shape.mesh.material_ids[0];

            for (size_t i = 1; i < shape.mesh.material_ids.size(); i++)
            {
                if (shape.mesh.material_ids[i] != first_mat)
                {
                    multiple_materials = true;
                    break;
                }
            }
        }

        // If already single-material, keep as-is
        if (!multiple_materials)
        {
            new_shapes.push_back(shape);
            continue;
        }

        // Split by material
        std::unordered_map<int, tinyobj::shape_t> split_shapes;

        size_t index_offset = 0;

        for (size_t face = 0; face < shape.mesh.num_face_vertices.size(); face++)
        {
            int material_id = shape.mesh.material_ids[face];

            // Create split shape if necessary
            if (split_shapes.find(material_id) == split_shapes.end())
            {
                tinyobj::shape_t split_shape;

                split_shape.name =
                    shape.name + "_mat_" + std::to_string(material_id);

                split_shapes[material_id] = split_shape;
            }

            auto& dst_shape = split_shapes[material_id];

            uint8_t fv = shape.mesh.num_face_vertices[face];

            // Copy face vertex count
            dst_shape.mesh.num_face_vertices.push_back(fv);

            // Copy material id
            dst_shape.mesh.material_ids.push_back(material_id);

            // Copy indices
            for (size_t v = 0; v < fv; v++)
            {
                dst_shape.mesh.indices.push_back(
                    shape.mesh.indices[index_offset + v]
                );
            }

            index_offset += fv;
        }

        // Append split shapes
        for (auto& kv : split_shapes)
        {
            new_shapes.push_back(std::move(kv.second));
        }
    }

    // Replace original shapes vector
    model->shapes = std::move(new_shapes);
}

// Constrói triângulos para futura renderização a partir de um ObjModel.
void BuildTrianglesAndBuffers(std::shared_ptr<ObjModel> model)
{
    GLuint vertex_array_object_id;
    glGenVertexArrays(1, &vertex_array_object_id);
    glBindVertexArray(vertex_array_object_id);

    std::vector<GLuint> indices;
    std::vector<float>  model_coefficients;
    std::vector<float>  normal_coefficients;
    std::vector<float>  texture_coefficients;

    constexpr float minval = std::numeric_limits<float>::lowest();
    constexpr float maxval = std::numeric_limits<float>::max();

    model->min_bounds = glm::vec3(maxval, maxval, maxval);
    model->max_bounds = glm::vec3(minval, minval, minval);

    for (size_t shape = 0; shape < model->shapes.size(); ++shape)
    {
        size_t first_index = indices.size();
        size_t num_triangles = model->shapes[shape].mesh.num_face_vertices.size();

        glm::vec3 bbox_min = glm::vec3(maxval,maxval,maxval);
        glm::vec3 bbox_max = glm::vec3(minval,minval,minval);

        for (size_t triangle = 0; triangle < num_triangles; ++triangle)
        {
            assert(model->shapes[shape].mesh.num_face_vertices[triangle] == 3);

            for (size_t vertex = 0; vertex < 3; ++vertex)
            {
                tinyobj::index_t idx = model->shapes[shape].mesh.indices[3*triangle + vertex];

                indices.push_back(first_index + 3*triangle + vertex);

                const float vx = model->attrib.vertices[3*idx.vertex_index + 0];
                const float vy = model->attrib.vertices[3*idx.vertex_index + 1];
                const float vz = model->attrib.vertices[3*idx.vertex_index + 2];
                //printf("tri %d vert %d = (%.2f, %.2f, %.2f)\n", (int)triangle, (int)vertex, vx, vy, vz);
                model_coefficients.push_back( vx ); // X
                model_coefficients.push_back( vy ); // Y
                model_coefficients.push_back( vz ); // Z
                model_coefficients.push_back( 1.0f ); // W

                bbox_min.x = std::min(bbox_min.x, vx);
                bbox_min.y = std::min(bbox_min.y, vy);
                bbox_min.z = std::min(bbox_min.z, vz);
                bbox_max.x = std::max(bbox_max.x, vx);
                bbox_max.y = std::max(bbox_max.y, vy);
                bbox_max.z = std::max(bbox_max.z, vz);

                // Inspecionando o código da tinyobjloader, o aluno Bernardo
                // Sulzbach (2017/1) apontou que a maneira correta de testar se
                // existem normais e coordenadas de textura no ObjModel é
                // comparando se o índice retornado é -1. Fazemos isso abaixo.

                if ( idx.normal_index != -1 )
                {
                    const float nx = model->attrib.normals[3*idx.normal_index + 0];
                    const float ny = model->attrib.normals[3*idx.normal_index + 1];
                    const float nz = model->attrib.normals[3*idx.normal_index + 2];
                    normal_coefficients.push_back( nx ); // X
                    normal_coefficients.push_back( ny ); // Y
                    normal_coefficients.push_back( nz ); // Z
                    normal_coefficients.push_back( 0.0f ); // W
                }

                if ( idx.texcoord_index != -1 )
                {
                    const float u = model->attrib.texcoords[2*idx.texcoord_index + 0];
                    const float v = model->attrib.texcoords[2*idx.texcoord_index + 1];
                    texture_coefficients.push_back( u );
                    texture_coefficients.push_back( v );
                }
            }
        }

        size_t last_index = indices.size() - 1;

        model->first_indices.push_back(first_index); // Primeiro índice
        model->num_indices.push_back(last_index - first_index + 1); // Número de indices
        model->vertex_array_object_ids.push_back(vertex_array_object_id);
        model->bboxes_min.push_back(bbox_min);
        model->bboxes_max.push_back(bbox_max);

        model->min_bounds = glm::min(model->min_bounds, bbox_min);
        model->max_bounds = glm::max(model->max_bounds, bbox_max);
    }

    GLuint VBO_model_coefficients_id;
    glGenBuffers(1, &VBO_model_coefficients_id);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_model_coefficients_id);
    glBufferData(GL_ARRAY_BUFFER, model_coefficients.size() * sizeof(float), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, model_coefficients.size() * sizeof(float), model_coefficients.data());
    GLuint location = 0; // "(location = 0)" em "shader_vertex.glsl"
    GLint  number_of_dimensions = 4; // vec4 em "shader_vertex.glsl"
    glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(location);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    if ( !normal_coefficients.empty() )
    {
        GLuint VBO_normal_coefficients_id;
        glGenBuffers(1, &VBO_normal_coefficients_id);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_normal_coefficients_id);
        glBufferData(GL_ARRAY_BUFFER, normal_coefficients.size() * sizeof(float), NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, normal_coefficients.size() * sizeof(float), normal_coefficients.data());
        location = 1; // "(location = 1)" em "shader_vertex.glsl"
        number_of_dimensions = 4; // vec4 em "shader_vertex.glsl"
        glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(location);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    if ( !texture_coefficients.empty() )
    {
        GLuint VBO_texture_coefficients_id;
        glGenBuffers(1, &VBO_texture_coefficients_id);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_texture_coefficients_id);
        glBufferData(GL_ARRAY_BUFFER, texture_coefficients.size() * sizeof(float), NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, texture_coefficients.size() * sizeof(float), texture_coefficients.data());
        location = 2; // "(location = 1)" em "shader_vertex.glsl"
        number_of_dimensions = 2; // vec2 em "shader_vertex.glsl"
        glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(location);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    GLuint indices_id;
    glGenBuffers(1, &indices_id);

    // "Ligamos" o buffer. Note que o tipo agora é GL_ELEMENT_ARRAY_BUFFER.
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indices.size() * sizeof(GLuint), indices.data());
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // XXX Errado!
    //

    // "Desligamos" o VAO, evitando assim que operações posteriores venham a
    // alterar o mesmo. Isso evita bugs.
    glBindVertexArray(0);
}

// Carrega um Vertex Shader de um arquivo GLSL. Veja definição de LoadShader() abaixo.
GLuint LoadShader_Vertex(const char* filename)
{
    // Criamos um identificador (ID) para este shader, informando que o mesmo
    // será aplicado nos vértices.
    GLuint vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);

    // Carregamos e compilamos o shader
    LoadShader(filename, vertex_shader_id);

    // Retorna o ID gerado acima
    return vertex_shader_id;
}

// Carrega um Fragment Shader de um arquivo GLSL . Veja definição de LoadShader() abaixo.
GLuint LoadShader_Fragment(const char* filename)
{
    // Criamos um identificador (ID) para este shader, informando que o mesmo
    // será aplicado nos fragmentos.
    GLuint fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);

    // Carregamos e compilamos o shader
    LoadShader(filename, fragment_shader_id);

    // Retorna o ID gerado acima
    return fragment_shader_id;
}

// Função auxilar, utilizada pelas duas funções acima. Carrega código de GPU de
// um arquivo GLSL e faz sua compilação.
void LoadShader(const char* filename, GLuint shader_id)
{
    // Lemos o arquivo de texto indicado pela variável "filename"
    // e colocamos seu conteúdo em memória, apontado pela variável
    // "shader_string".
    std::ifstream file;
    try {
        file.exceptions(std::ifstream::failbit);
        file.open(filename);
    } catch ( std::exception& e ) {
        fprintf(stderr, "ERROR: Cannot open file \"%s\".\n", filename);
        std::exit(EXIT_FAILURE);
    }
    std::stringstream shader;
    shader << file.rdbuf();
    std::string str = shader.str();
    const GLchar* shader_string = str.c_str();
    const GLint   shader_string_length = static_cast<GLint>( str.length() );

    // Define o código do shader GLSL, contido na string "shader_string"
    glShaderSource(shader_id, 1, &shader_string, &shader_string_length);

    // Compila o código do shader GLSL (em tempo de execução)
    glCompileShader(shader_id);

    // Verificamos se ocorreu algum erro ou "warning" durante a compilação
    GLint compiled_ok;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &compiled_ok);

    GLint log_length = 0;
    glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &log_length);

    // Alocamos memória para guardar o log de compilação.
    // A chamada "new" em C++ é equivalente ao "malloc()" do C.
    GLchar* log = new GLchar[log_length];
    glGetShaderInfoLog(shader_id, log_length, &log_length, log);

    // Imprime no terminal qualquer erro ou "warning" de compilação
    if ( log_length != 0 )
    {
        std::string  output;

        if ( !compiled_ok )
        {
            output += "ERROR: OpenGL compilation of \"";
            output += filename;
            output += "\" failed.\n";
            output += "== Start of compilation log\n";
            output += log;
            output += "== End of compilation log\n";
        }
        else
        {
            output += "WARNING: OpenGL compilation of \"";
            output += filename;
            output += "\".\n";
            output += "== Start of compilation log\n";
            output += log;
            output += "== End of compilation log\n";
        }

        fprintf(stderr, "%s", output.c_str());
    }

    // A chamada "delete" em C++ é equivalente ao "free()" do C
    delete [] log;
}

// Esta função cria um programa de GPU, o qual contém obrigatoriamente um
// Vertex Shader e um Fragment Shader.
GLuint CreateGpuProgram(GLuint vertex_shader_id, GLuint fragment_shader_id)
{
    // Criamos um identificador (ID) para este programa de GPU
    GLuint program_id = glCreateProgram();

    // Definição dos dois shaders GLSL que devem ser executados pelo programa
    glAttachShader(program_id, vertex_shader_id);
    glAttachShader(program_id, fragment_shader_id);

    // Linkagem dos shaders acima ao programa
    glLinkProgram(program_id);

    // Verificamos se ocorreu algum erro durante a linkagem
    GLint linked_ok = GL_FALSE;
    glGetProgramiv(program_id, GL_LINK_STATUS, &linked_ok);

    // Imprime no terminal qualquer erro de linkagem
    if ( linked_ok == GL_FALSE )
    {
        GLint log_length = 0;
        glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &log_length);

        // Alocamos memória para guardar o log de compilação.
        // A chamada "new" em C++ é equivalente ao "malloc()" do C.
        GLchar* log = new GLchar[log_length];

        glGetProgramInfoLog(program_id, log_length, &log_length, log);

        std::string output;

        output += "ERROR: OpenGL linking of program failed.\n";
        output += "== Start of link log\n";
        output += log;
        output += "\n== End of link log\n";

        // A chamada "delete" em C++ é equivalente ao "free()" do C
        delete [] log;

        fprintf(stderr, "%s", output.c_str());
    }

    // Os "Shader Objects" podem ser marcados para deleção após serem linkados 
    glDeleteShader(vertex_shader_id);
    glDeleteShader(fragment_shader_id);

    // Retornamos o ID gerado acima
    return program_id;
}

// Definição da função que será chamada sempre que a janela do sistema
// operacional for redimensionada, por consequência alterando o tamanho do
// "framebuffer" (região de memória onde são armazenados os pixels da imagem).
void FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    // Indicamos que queremos renderizar em toda região do framebuffer. A
    // função "glViewport" define o mapeamento das "normalized device
    // coordinates" (NDC) para "pixel coordinates".  Essa é a operação de
    // "Screen Mapping" ou "Viewport Mapping" vista em aula ({+ViewportMapping2+}).
    glViewport(0, 0, width, height);

    // Atualizamos também a razão que define a proporção da janela (largura /
    // altura), a qual será utilizada na definição das matrizes de projeção,
    // tal que não ocorra distorções durante o processo de "Screen Mapping"
    // acima, quando NDC é mapeado para coordenadas de pixels. Veja slides 205-215 do documento Aula_09_Projecoes.pdf.
    //
    // O cast para float é necessário pois números inteiros são arredondados ao
    // serem divididos!
    g_ScreenRatio = (float)width / height;
}

// Variáveis globais que armazenam a última posição do cursor do mouse, para
// que possamos calcular quanto que o mouse se movimentou entre dois instantes
// de tempo. Utilizadas no callback CursorPosCallback() abaixo.

// Função callback chamada sempre que o usuário aperta algum dos botões do mouse
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        // Se o usuário pressionou o botão esquerdo do mouse, guardamos a
        // posição atual do cursor nas variáveis g_LastCursorPosX e
        // g_LastCursorPosY.  Também, setamos a variável
        // g_LeftMouseButtonPressed como true, para saber que o usuário está
        // com o botão esquerdo pressionado.
        //glfwGetCursorPos(window, &g_LastCursorPosX, &g_LastCursorPosY);
        g_left_mouse_button_pressed = true;
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    {
        // Quando o usuário soltar o botão esquerdo do mouse, atualizamos a
        // variável abaixo para false.
        g_left_mouse_button_pressed = false;
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
    {
        // Se o usuário pressionou o botão esquerdo do mouse, guardamos a
        // posição atual do cursor nas variáveis g_LastCursorPosX e
        // g_LastCursorPosY.  Também, setamos a variável
        // g_RightMouseButtonPressed como true, para saber que o usuário está
        // com o botão esquerdo pressionado.
        //glfwGetCursorPos(window, &g_LastCursorPosX, &g_LastCursorPosY);
        g_right_mouse_button_pressed = true;
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
    {
        // Quando o usuário soltar o botão esquerdo do mouse, atualizamos a
        // variável abaixo para false.
        g_right_mouse_button_pressed = false;
    }
    if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS)
    {
        // Se o usuário pressionou o botão esquerdo do mouse, guardamos a
        // posição atual do cursor nas variáveis g_LastCursorPosX e
        // g_LastCursorPosY.  Também, setamos a variável
        // g_MiddleMouseButtonPressed como true, para saber que o usuário está
        // com o botão esquerdo pressionado.
        //glfwGetCursorPos(window, &g_LastCursorPosX, &g_LastCursorPosY);
        g_middle_mouse_button_pressed = true;
    }
    if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_RELEASE)
    {
        // Quando o usuário soltar o botão esquerdo do mouse, atualizamos a
        // variável abaixo para false.
        g_middle_mouse_button_pressed = false;
    }
}

// Função callback chamada sempre que o usuário movimentar o cursor do mouse em
// cima da janela OpenGL.
void CursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
    // Abaixo executamos o seguinte: caso o botão esquerdo do mouse esteja
    // pressionado, computamos quanto que o mouse se movimento desde o último
    // instante de tempo, e usamos esta movimentação para atualizar os
    // parâmetros que definem a posição da câmera dentro da cena virtual.
    // Assim, temos que o usuário consegue controlar a câmera.

    if (g_left_mouse_button_pressed)
    {
        g_mouse_cursor_delta_x += g_last_mouse_cursor_x - xpos;
        g_mouse_cursor_delta_y += g_last_mouse_cursor_y - ypos;
    }

    if (g_right_mouse_button_pressed)
    {
    }

    if (g_middle_mouse_button_pressed)
    {
    }

    g_last_mouse_cursor_x = xpos;
    g_last_mouse_cursor_y = ypos;
}

// Função callback chamada sempre que o usuário movimenta a "rodinha" do mouse.
void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    // Atualizamos a distância da câmera para a origem utilizando a
    // movimentação da "rodinha", simulando um ZOOM.
    //g_CameraDistance -= 0.1f*yoffset;

    // Uma câmera look-at nunca pode estar exatamente "em cima" do ponto para
    // onde ela está olhando, pois isto gera problemas de divisão por zero na
    // definição do sistema de coordenadas da câmera. Isto é, a variável abaixo
    // nunca pode ser zero. Versões anteriores deste código possuíam este bug,
    // o qual foi detectado pelo aluno Vinicius Fraga (2017/2).
    //const float verysmallnumber = std::numeric_limits<float>::epsilon();
    //if (g_CameraDistance < verysmallnumber)
    //    g_CameraDistance = verysmallnumber;
}

void Correcao_KeyCallback(int key, int action, int mod);

// Definição da função que será chamada sempre que o usuário pressionar alguma
// tecla do teclado. Veja http://www.glfw.org/docs/latest/input_guide.html#input_key
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mod)
{
    // =======================
    // Não modifique esta chamada! Ela é utilizada para correção automatizada dos
    // laboratórios. Deve ser sempre o primeiro comando desta função KeyCallback().
    Correcao_KeyCallback(key, action, mod);
    // =======================

    // Se o usuário pressionar a tecla ESC, fechamos a janela.
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    // O código abaixo implementa a seguinte lógica:
    //   Se apertar tecla X       então g_AngleX += delta;
    //   Se apertar tecla shift+X então g_AngleX -= delta;
    //   Se apertar tecla Y       então g_AngleY += delta;
    //   Se apertar tecla shift+Y então g_AngleY -= delta;
    //   Se apertar tecla Z       então g_AngleZ += delta;
    //   Se apertar tecla shift+Z então g_AngleZ -= delta;

    float delta = 3.141592 / 16; // 22.5 graus, em radianos.

    if (key == GLFW_KEY_X && action == GLFW_PRESS)
    {
        g_AngleX += (mod & GLFW_MOD_SHIFT) ? -delta : delta;
    }

    if (key == GLFW_KEY_Y && action == GLFW_PRESS)
    {
        g_AngleY += (mod & GLFW_MOD_SHIFT) ? -delta : delta;
    }
    if (key == GLFW_KEY_Z && action == GLFW_PRESS)
    {
        g_AngleZ += (mod & GLFW_MOD_SHIFT) ? -delta : delta;
    }

    // Se o usuário apertar a tecla espaço, resetamos os ângulos de Euler para zero.
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
    {
        g_AngleX = 0.0f;
        g_AngleY = 0.0f;
        g_AngleZ = 0.0f;
        //g_ForearmAngleX = 0.0f;
        //g_ForearmAngleZ = 0.0f;
        //g_TorsoPositionX = 0.0f;
        //g_TorsoPositionY = 0.0f;
    }

    // Se o usuário apertar a tecla P, utilizamos projeção perspectiva.
    if (key == GLFW_KEY_P && action == GLFW_PRESS)
    {
        g_UsePerspectiveProjection = true;
    }

    // Se o usuário apertar a tecla O, utilizamos projeção ortográfica.
    if (key == GLFW_KEY_O && action == GLFW_PRESS)
    {
        g_UsePerspectiveProjection = false;
    }

    // Se o usuário apertar a tecla H, fazemos um "toggle" do texto informativo mostrado na tela.
    if (key == GLFW_KEY_H && action == GLFW_PRESS)
    {
        g_show_info_text = !g_show_info_text;
    }

    // Se o usuário apertar a tecla R, recarregamos os shaders dos arquivos "shader_fragment.glsl" e "shader_vertex.glsl".
    if (key == GLFW_KEY_R && action == GLFW_PRESS)
    {
        LoadShadersFromFiles();
        fprintf(stdout,"Shaders recarregados!\n");
        fflush(stdout);
    }

    // Held down keys update
    if (action == GLFW_PRESS)
        keys[key] = true;

    if (action == GLFW_RELEASE)
        keys[key] = false;
}

// Definimos o callback para impressão de erros da GLFW no terminal
void ErrorCallback(int error, const char* description)
{
    fprintf(stderr, "ERROR: GLFW: %s\n", description);
}

// Esta função recebe um vértice com coordenadas de modelo p_model e passa o
// mesmo por todos os sistemas de coordenadas armazenados nas matrizes model,
// view, e projection; e escreve na tela as matrizes e pontos resultantes
// dessas transformações.
void TextRendering_ShowModelViewProjection(
    GLFWwindow* window,
    glm::mat4 projection,
    glm::mat4 view,
    glm::mat4 model,
    glm::vec4 p_model
)
{
    if ( !g_show_info_text )
        return;

    glm::vec4 p_world = model*p_model;
    glm::vec4 p_camera = view*p_world;
    glm::vec4 p_clip = projection*p_camera;
    glm::vec4 p_ndc = p_clip / p_clip.w;

    float pad = TextRendering_LineHeight(window);

    TextRendering_PrintString(window, " Model matrix             Model     In World Coords.", -1.0f, 1.0f-pad, 1.0f);
    TextRendering_PrintMatrixVectorProduct(window, model, p_model, -1.0f, 1.0f-2*pad, 1.0f);

    TextRendering_PrintString(window, "                                        |  ", -1.0f, 1.0f-6*pad, 1.0f);
    TextRendering_PrintString(window, "                            .-----------'  ", -1.0f, 1.0f-7*pad, 1.0f);
    TextRendering_PrintString(window, "                            V              ", -1.0f, 1.0f-8*pad, 1.0f);

    TextRendering_PrintString(window, " View matrix              World     In Camera Coords.", -1.0f, 1.0f-9*pad, 1.0f);
    TextRendering_PrintMatrixVectorProduct(window, view, p_world, -1.0f, 1.0f-10*pad, 1.0f);

    TextRendering_PrintString(window, "                                        |  ", -1.0f, 1.0f-14*pad, 1.0f);
    TextRendering_PrintString(window, "                            .-----------'  ", -1.0f, 1.0f-15*pad, 1.0f);
    TextRendering_PrintString(window, "                            V              ", -1.0f, 1.0f-16*pad, 1.0f);

    TextRendering_PrintString(window, " Projection matrix        Camera                    In NDC", -1.0f, 1.0f-17*pad, 1.0f);
    TextRendering_PrintMatrixVectorProductDivW(window, projection, p_camera, -1.0f, 1.0f-18*pad, 1.0f);

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    glm::vec2 a = glm::vec2(-1, -1);
    glm::vec2 b = glm::vec2(+1, +1);
    glm::vec2 p = glm::vec2( 0,  0);
    glm::vec2 q = glm::vec2(width, height);

    glm::mat4 viewport_mapping = glm::mat4(
        (q.x - p.x)/(b.x-a.x), 0.0f, 0.0f, (b.x*p.x - a.x*q.x)/(b.x-a.x),
        0.0f, (q.y - p.y)/(b.y-a.y), 0.0f, (b.y*p.y - a.y*q.y)/(b.y-a.y),
        0.0f , 0.0f , 1.0f , 0.0f ,
        0.0f , 0.0f , 0.0f , 1.0f
    );

    TextRendering_PrintString(window, "                                                       |  ", -1.0f, 1.0f-22*pad, 1.0f);
    TextRendering_PrintString(window, "                            .--------------------------'  ", -1.0f, 1.0f-23*pad, 1.0f);
    TextRendering_PrintString(window, "                            V                           ", -1.0f, 1.0f-24*pad, 1.0f);

    TextRendering_PrintString(window, " Viewport matrix           NDC      In Pixel Coords.", -1.0f, 1.0f-25*pad, 1.0f);
    TextRendering_PrintMatrixVectorProductMoreDigits(window, viewport_mapping, p_ndc, -1.0f, 1.0f-26*pad, 1.0f);
}

// Escrevemos na tela os ângulos de Euler definidos nas variáveis globais
// g_AngleX, g_AngleY, e g_AngleZ.
void TextRendering_ShowEulerAngles(GLFWwindow* window)
{
    if ( !g_show_info_text )
        return;

    float pad = TextRendering_LineHeight(window);

    char buffer[80];
    snprintf(buffer, 80, "Euler Angles rotation matrix = Z(%.2f)*Y(%.2f)*X(%.2f)\n", g_AngleZ, g_AngleY, g_AngleX);

    TextRendering_PrintString(window, buffer, -1.0f+pad/10, -1.0f+2*pad/10, 1.0f);
}

// Escrevemos na tela qual matriz de projeção está sendo utilizada.
void TextRendering_ShowProjection(GLFWwindow* window)
{
    if ( !g_show_info_text )
        return;

    float lineheight = TextRendering_LineHeight(window);
    float charwidth = TextRendering_CharWidth(window);

    if ( g_UsePerspectiveProjection )
        TextRendering_PrintString(window, "Perspective", 1.0f-13*charwidth, -1.0f+2*lineheight/10, 1.0f);
    else
        TextRendering_PrintString(window, "Orthographic", 1.0f-13*charwidth, -1.0f+2*lineheight/10, 1.0f);
}

// Escrevemos na tela o número de quadros renderizados por segundo (frames per
// second).
void TextRendering_ShowFramesPerSecond(GLFWwindow* window)
{
    if ( !g_show_info_text )
        return;

    // Variáveis estáticas (static) mantém seus valores entre chamadas
    // subsequentes da função!
    static float old_seconds = (float)glfwGetTime();
    static int   ellapsed_frames = 0;
    static char  buffer[20] = "?? fps";
    static int   numchars = 7;

    ellapsed_frames += 1;

    // Recuperamos o número de segundos que passou desde a execução do programa
    float seconds = (float)glfwGetTime();

    // Número de segundos desde o último cálculo do fps
    float ellapsed_seconds = seconds - old_seconds;

    if ( ellapsed_seconds > 1.0f )
    {
        numchars = snprintf(buffer, 20, "%.2f fps", ellapsed_frames / ellapsed_seconds);
    
        old_seconds = seconds;
        ellapsed_frames = 0;
    }

    float lineheight = TextRendering_LineHeight(window);
    float charwidth = TextRendering_CharWidth(window);

    TextRendering_PrintString(window, buffer, 1.0f-(numchars + 1)*charwidth, 1.0f-lineheight, 1.0f);
}

// Função para debugging: imprime no terminal todas informações de um modelo
// geométrico carregado de um arquivo ".obj".
// Veja: https://github.com/syoyo/tinyobjloader/blob/22883def8db9ef1f3ffb9b404318e7dd25fdbb51/loader_example.cc#L98
void PrintObjModelInfo(std::shared_ptr<ObjModel> model)
{
  const tinyobj::attrib_t                & attrib    = model->attrib;
  const std::vector<tinyobj::shape_t>    & shapes    = model->shapes;
  const std::vector<tinyobj::material_t> & materials = model->materials;

  printf("# of vertices  : %d\n", (int)(attrib.vertices.size() / 3));
  printf("# of normals   : %d\n", (int)(attrib.normals.size() / 3));
  printf("# of texcoords : %d\n", (int)(attrib.texcoords.size() / 2));
  printf("# of shapes    : %d\n", (int)shapes.size());
  printf("# of materials : %d\n", (int)materials.size());

  for (size_t v = 0; v < attrib.vertices.size() / 3; v++) {
    printf("  v[%ld] = (%f, %f, %f)\n", static_cast<long>(v),
           static_cast<const double>(attrib.vertices[3 * v + 0]),
           static_cast<const double>(attrib.vertices[3 * v + 1]),
           static_cast<const double>(attrib.vertices[3 * v + 2]));
  }

  for (size_t v = 0; v < attrib.normals.size() / 3; v++) {
    printf("  n[%ld] = (%f, %f, %f)\n", static_cast<long>(v),
           static_cast<const double>(attrib.normals[3 * v + 0]),
           static_cast<const double>(attrib.normals[3 * v + 1]),
           static_cast<const double>(attrib.normals[3 * v + 2]));
  }

  for (size_t v = 0; v < attrib.texcoords.size() / 2; v++) {
    printf("  uv[%ld] = (%f, %f)\n", static_cast<long>(v),
           static_cast<const double>(attrib.texcoords[2 * v + 0]),
           static_cast<const double>(attrib.texcoords[2 * v + 1]));
  }

  // For each shape
  for (size_t i = 0; i < shapes.size(); i++) {
    printf("shape[%ld].name = %s\n", static_cast<long>(i),
           shapes[i].name.c_str());
    printf("Size of shape[%ld].indices: %lu\n", static_cast<long>(i),
           static_cast<unsigned long>(shapes[i].mesh.indices.size()));

    size_t index_offset = 0;

    assert(shapes[i].mesh.num_face_vertices.size() ==
           shapes[i].mesh.material_ids.size());

    printf("shape[%ld].num_faces: %lu\n", static_cast<long>(i),
           static_cast<unsigned long>(shapes[i].mesh.num_face_vertices.size()));

    // For each face
    for (size_t f = 0; f < shapes[i].mesh.num_face_vertices.size(); f++) {
      size_t fnum = shapes[i].mesh.num_face_vertices[f];

      printf("  face[%ld].fnum = %ld\n", static_cast<long>(f),
             static_cast<unsigned long>(fnum));

      // For each vertex in the face
      for (size_t v = 0; v < fnum; v++) {
        tinyobj::index_t idx = shapes[i].mesh.indices[index_offset + v];
        printf("    face[%ld].v[%ld].idx = %d/%d/%d\n", static_cast<long>(f),
               static_cast<long>(v), idx.vertex_index, idx.normal_index,
               idx.texcoord_index);
      }

      printf("  face[%ld].material_id = %d\n", static_cast<long>(f),
             shapes[i].mesh.material_ids[f]);

      index_offset += fnum;
    }

    printf("shape[%ld].num_tags: %lu\n", static_cast<long>(i),
           static_cast<unsigned long>(shapes[i].mesh.tags.size()));
    for (size_t t = 0; t < shapes[i].mesh.tags.size(); t++) {
      printf("  tag[%ld] = %s ", static_cast<long>(t),
             shapes[i].mesh.tags[t].name.c_str());
      printf(" ints: [");
      for (size_t j = 0; j < shapes[i].mesh.tags[t].intValues.size(); ++j) {
        printf("%ld", static_cast<long>(shapes[i].mesh.tags[t].intValues[j]));
        if (j < (shapes[i].mesh.tags[t].intValues.size() - 1)) {
          printf(", ");
        }
      }
      printf("]");

      printf(" floats: [");
      for (size_t j = 0; j < shapes[i].mesh.tags[t].floatValues.size(); ++j) {
        printf("%f", static_cast<const double>(
                         shapes[i].mesh.tags[t].floatValues[j]));
        if (j < (shapes[i].mesh.tags[t].floatValues.size() - 1)) {
          printf(", ");
        }
      }
      printf("]");

      printf(" strings: [");
      for (size_t j = 0; j < shapes[i].mesh.tags[t].stringValues.size(); ++j) {
        printf("%s", shapes[i].mesh.tags[t].stringValues[j].c_str());
        if (j < (shapes[i].mesh.tags[t].stringValues.size() - 1)) {
          printf(", ");
        }
      }
      printf("]");
      printf("\n");
    }
  }

  for (size_t i = 0; i < materials.size(); i++) {
    printf("material[%ld].name = %s\n", static_cast<long>(i),
           materials[i].name.c_str());
    printf("  material.Ka = (%f, %f ,%f)\n",
           static_cast<const double>(materials[i].ambient[0]),
           static_cast<const double>(materials[i].ambient[1]),
           static_cast<const double>(materials[i].ambient[2]));
    printf("  material.Kd = (%f, %f ,%f)\n",
           static_cast<const double>(materials[i].diffuse[0]),
           static_cast<const double>(materials[i].diffuse[1]),
           static_cast<const double>(materials[i].diffuse[2]));
    printf("  material.Ks = (%f, %f ,%f)\n",
           static_cast<const double>(materials[i].specular[0]),
           static_cast<const double>(materials[i].specular[1]),
           static_cast<const double>(materials[i].specular[2]));
    printf("  material.Tr = (%f, %f ,%f)\n",
           static_cast<const double>(materials[i].transmittance[0]),
           static_cast<const double>(materials[i].transmittance[1]),
           static_cast<const double>(materials[i].transmittance[2]));
    printf("  material.Ke = (%f, %f ,%f)\n",
           static_cast<const double>(materials[i].emission[0]),
           static_cast<const double>(materials[i].emission[1]),
           static_cast<const double>(materials[i].emission[2]));
    printf("  material.Ns = %f\n",
           static_cast<const double>(materials[i].shininess));
    printf("  material.Ni = %f\n", static_cast<const double>(materials[i].ior));
    printf("  material.dissolve = %f\n",
           static_cast<const double>(materials[i].dissolve));
    printf("  material.illum = %d\n", materials[i].illum);
    printf("  material.map_Ka = %s\n", materials[i].ambient_texname.c_str());
    printf("  material.map_Kd = %s\n", materials[i].diffuse_texname.c_str());
    printf("  material.map_Ks = %s\n", materials[i].specular_texname.c_str());
    printf("  material.map_Ns = %s\n",
           materials[i].specular_highlight_texname.c_str());
    printf("  material.map_bump = %s\n", materials[i].bump_texname.c_str());
    printf("  material.map_d = %s\n", materials[i].alpha_texname.c_str());
    printf("  material.disp = %s\n", materials[i].displacement_texname.c_str());
    printf("  <<PBR>>\n");
    printf("  material.Pr     = %f\n", materials[i].roughness);
    printf("  material.Pm     = %f\n", materials[i].metallic);
    printf("  material.Ps     = %f\n", materials[i].sheen);
    printf("  material.Pc     = %f\n", materials[i].clearcoat_thickness);
    printf("  material.Pcr    = %f\n", materials[i].clearcoat_thickness);
    printf("  material.aniso  = %f\n", materials[i].anisotropy);
    printf("  material.anisor = %f\n", materials[i].anisotropy_rotation);
    printf("  material.map_Ke = %s\n", materials[i].emissive_texname.c_str());
    printf("  material.map_Pr = %s\n", materials[i].roughness_texname.c_str());
    printf("  material.map_Pm = %s\n", materials[i].metallic_texname.c_str());
    printf("  material.map_Ps = %s\n", materials[i].sheen_texname.c_str());
    printf("  material.norm   = %s\n", materials[i].normal_texname.c_str());
    std::map<std::string, std::string>::const_iterator it(
        materials[i].unknown_parameter.begin());
    std::map<std::string, std::string>::const_iterator itEnd(
        materials[i].unknown_parameter.end());

    for (; it != itEnd; it++) {
      printf("  material.%s = %s\n", it->first.c_str(), it->second.c_str());
    }
    printf("\n");
  }
}

// set makeprg=cd\ ..\ &&\ make\ run\ >/dev/null
// vim: set spell spelllang=pt_br :

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

void UpdateGameStartScreen(GLFWwindow* window, double delta_time)
{
    static constexpr float text_scale = 4.0f;
    static std::vector<std::string> options = { "Play", "Exit" };
	static int32_t selected_option = 0;
	static bool is_key_just_pressed = false;
    static float time_since_last_press = 0.0f;

    if (is_key_just_pressed)
    {
        time_since_last_press += delta_time;
        if (time_since_last_press >= 0.25f)
        {
            time_since_last_press = 0.0f;
            is_key_just_pressed = false;
        }
    }

    if(keys[GLFW_KEY_UP] && !is_key_just_pressed)
    {
        selected_option = (selected_option - 1 + options.size()) % options.size();
        is_key_just_pressed = true;
    }
    else if (keys[GLFW_KEY_DOWN] && !is_key_just_pressed)
    {
        selected_option = (selected_option + 1) % options.size();
        is_key_just_pressed = true;
    }
    if (keys[GLFW_KEY_ENTER] && !is_key_just_pressed)
    {
        if (selected_option == 0)
        {
			g_is_on_game_start = false;
            g_is_playing_countdown = true;
        }
        else if(selected_option == 1)
        {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
    }

    for(int32_t i = 0; i < options.size(); i++)
    {
        if(i == selected_option)
        {
            TextRendering_PrintString(window, options[i], 0.0f - (TextRendering_CharWidth(window) * (options[i].size() / 2) * text_scale), 0.1f - (i * 0.2f), text_scale, { 0.65f, 0.2f, 0.1f });
        }
        else
        {
            TextRendering_PrintString(window, options[i], 0.0f - (TextRendering_CharWidth(window) * (options[i].size() / 2) * text_scale), 0.1f - (i * 0.2f), text_scale, { 0.0f, 0.0f, 0.0f });
        }
	}
}

void UpdateGameOverScreen(GLFWwindow* window, double delta_time, const std::vector<std::shared_ptr<Car>>& cars, std::shared_ptr<Track> track)
{
    static constexpr float text_scale = 4.0f;
    static std::vector<std::string> options = { "Play Again", "Exit" };
    static int32_t selected_option = 0;
    static bool is_key_just_pressed = false;
    static float time_since_last_press = 0.0f;

    if (is_key_just_pressed)
    {
        time_since_last_press += delta_time;
        if (time_since_last_press >= 0.25f)
        {
            time_since_last_press = 0.0f;
            is_key_just_pressed = false;
        }
    }
    if(keys[GLFW_KEY_UP] && !is_key_just_pressed)
    {
        selected_option = (selected_option - 1 + options.size()) % options.size();
        is_key_just_pressed = true;
    }
    else if (keys[GLFW_KEY_DOWN] && !is_key_just_pressed)
    {
        selected_option = (selected_option + 1) % options.size();
        is_key_just_pressed = true;
    }
    if (keys[GLFW_KEY_ENTER] && !is_key_just_pressed)
    {
        if (selected_option == 0)
        {
            RestartGame(cars, track);
			g_is_game_over = false;
            g_is_playing_countdown = true;
        }
        else if(selected_option == 1)
        {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
    }

    int32_t num_winners = 0;
    int32_t player_index = 0;
    for (std::shared_ptr<Car> car : cars)
    {
        if (car->laps_completed == g_num_laps)
        {
            static constexpr float winner_msg_scale = 6.0f;
            std::string winner_msg = std::format("Player {} WON!", player_index);
            TextRendering_PrintString(
                window, winner_msg, 
                0.0f - (TextRendering_CharWidth(window) * (winner_msg.size() / 2) * winner_msg_scale), 
                0.5f - (num_winners * (TextRendering_LineHeight(window)) * winner_msg_scale), 
                winner_msg_scale);

            ++num_winners;
        }
        ++player_index;
    }
    
    for (int32_t i = 0; i < options.size(); i++)
    {
        if (i == selected_option)
        {
            TextRendering_PrintString(window, options[i], 0.0f - (TextRendering_CharWidth(window) * (options[i].size() / 2) * text_scale), -0.35f - (i * 0.2f), text_scale, { 0.65f, 0.2f, 0.1f });
        }
        else
        {
            TextRendering_PrintString(window, options[i], 0.0f - (TextRendering_CharWidth(window) * (options[i].size() / 2) * text_scale), -0.35f - (i * 0.2f), text_scale, { 0.0f, 0.0f, 0.0f });
        }
    }
}

void RestartGame(const std::vector<std::shared_ptr<Car>>& cars, std::shared_ptr<Track> track)
{
    for (int32_t i = 0; i < cars.size(); ++i)
    {
        std::shared_ptr<Car> car = cars[i];
		car->speed = 0.0f;
		car->yaw_tremble_timer = 0.0f;
        car->is_accelerating = false;
		car->is_out_of_control = false;
        car->cur_curve_point = g_init_curve_point;
        car->cur_lane = i;
        car->target_lane = i;
        car->cur_curve_pos = track->lanes[car->cur_lane].at(car->cur_curve_point);
        car->transition_curve_pos = car->cur_curve_pos;
        car->laps_completed = 0;

        UpdateCarInputAndAnimation(0, car, track);
    }

    UpdateCountdownCamera(g_countdown_time);
}

void UpdateFreeCamera(double delta_time)
{
    // Camera rotation update
    camera_yaw -= float(g_mouse_cursor_delta_x) * camera_rotation_speed;
    camera_pitch += float(g_mouse_cursor_delta_y) * camera_rotation_speed;

    camera_yaw = fmod(camera_yaw, glm::two_pi<float>());
    camera_pitch = std::clamp(camera_pitch, -glm::half_pi<float>() + 0.01f, glm::half_pi<float>() - 0.01f);

    camera_forward.x = cosf(camera_pitch) * cosf(camera_yaw);
    camera_forward.y = sinf(camera_pitch);
    camera_forward.z = cosf(camera_pitch) * sinf(camera_yaw);
    camera_forward = glm::normalize(camera_forward);

    camera_right = glm::vec4(glm::normalize(glm::cross(glm::vec3(camera_forward), { 0.0f, 1.0f, 0.0f })), 0.0f);
    camera_up = glm::vec4(glm::normalize(glm::cross(glm::vec3(camera_right), glm::vec3(camera_forward))), 0.0f);

    g_mouse_cursor_delta_x = 0.0f;
    g_mouse_cursor_delta_y = 0.0f;

    // Camera movement update
    if (keys[GLFW_KEY_W])
    {
        camera_position += camera_forward * float(camera_move_speed * delta_time);
    }
    if (keys[GLFW_KEY_S])
    {
        camera_position -= camera_forward * float(camera_move_speed * delta_time);
    }
    if (keys[GLFW_KEY_D])
    {
        camera_position += camera_right * float(camera_move_speed * delta_time);
    }
    if (keys[GLFW_KEY_A])
    {
        camera_position -= camera_right * float(camera_move_speed * delta_time);
    }
}

void UpdateRaceCamera(double delta_time, const std::vector<std::shared_ptr<Car>>& cars)
{
    if (cars.empty())
        return;

    //------------------------------------------------------
    // Tunable parameters
    //------------------------------------------------------

    constexpr float fixed_yaw = glm::radians(-75.0f);
    constexpr float fixed_pitch = glm::radians(-35.0f);

    constexpr float min_distance = 6.0f;
    constexpr float max_distance = 14.0f;
    constexpr float zoom_factor = 2.4f;

    constexpr float follow_speed = 6.0f;
    constexpr float look_speed = 8.0f;

    //------------------------------------------------------
    // Compute center of all cars
    //------------------------------------------------------

    glm::vec3 target_center(0.0f);

    for (const auto& car : cars)
    {
        target_center += glm::vec3(car->root->position);
    }

    target_center /= float(cars.size());

    //------------------------------------------------------
    // Measure cars spread
    //------------------------------------------------------

    float max_distance_from_center = 0.0f;

    for (const auto& car : cars)
    {
        float d = glm::distance(glm::vec3(car->root->position), target_center);

        max_distance_from_center = std::max(max_distance_from_center, d);
    }

    //------------------------------------------------------
    // Camera forward from fixed rotation
    //------------------------------------------------------

    glm::vec3 forward;

    forward.x = cosf(fixed_pitch) * cosf(fixed_yaw);
    forward.y = sinf(fixed_pitch);
    forward.z = cosf(fixed_pitch) * sinf(fixed_yaw);

    forward = glm::normalize(forward);

    //------------------------------------------------------
    // Zoom based on spread
    //------------------------------------------------------

    float target_distance =
        min_distance +
        max_distance_from_center * zoom_factor;

    target_distance =  std::clamp(target_distance, min_distance, max_distance);

    //------------------------------------------------------
    // Compute target camera position
    //------------------------------------------------------

    glm::vec3 target_position = target_center - forward * target_distance;

    //------------------------------------------------------
    // Smooth camera motion
    //------------------------------------------------------

    float position_t = 1.0f - expf(-follow_speed * float(delta_time));

    camera_position =
        glm::mix(
            glm::vec4(camera_position),
            glm::vec4(target_position, 1.0f),
            position_t);

    //------------------------------------------------------
    // Camera orientation always looks at pack center
    //------------------------------------------------------

    glm::vec3 look_dir = glm::normalize(target_center - glm::vec3(camera_position));

    camera_forward = glm::vec4(look_dir, 0.0f);

    camera_right =
        glm::vec4(
            glm::normalize(
                glm::cross(
                    glm::vec3(camera_forward),
                    glm::vec3(0, 1, 0))),
            0.0f);

    camera_up =
        glm::vec4(
            glm::normalize(
                glm::cross(
                    glm::vec3(camera_right),
                    glm::vec3(camera_forward))),
            0.0f);
}

void UpdateCountdownCamera(float normalized_countdown_time)
{
    constexpr glm::vec4 init_camera_position = { 1.62f, 0.52f, 8.88f, 1.0f };
    constexpr float init_camera_pitch = glm::radians<float>(-20.91);
    constexpr float init_camera_yaw = glm::radians<float>(-24.11);

    constexpr glm::vec4 end_camera_position = { -1.41f, 3.94f, 14.43f, 1.0f };
    constexpr float end_camera_pitch = glm::radians<float>(-12.6);
    constexpr float end_camera_yaw = glm::radians<float>(-79.4);

    normalized_countdown_time = glm::clamp(normalized_countdown_time, 0.0f, 1.0f);

    //--------------------------------------------------
    // Smooth cinematic easing
    //--------------------------------------------------

    float t = normalized_countdown_time;

    // cubic ease-in-out
    t = t * t * (3.0f - 2.0f * t);

    //--------------------------------------------------
    // Position interpolation
    //--------------------------------------------------

    camera_position =
        glm::mix(
            init_camera_position,
            end_camera_position,
            t);

    //--------------------------------------------------
    // Angle interpolation
    //--------------------------------------------------

    float pitch = glm::mix(init_camera_pitch, end_camera_pitch, t);

    float yaw = glm::mix(init_camera_yaw, end_camera_yaw, t);

    //--------------------------------------------------
    // Build forward vector
    //--------------------------------------------------

    camera_forward.x = cosf(pitch) * cosf(yaw);

    camera_forward.y = sinf(pitch);

    camera_forward.z = cosf(pitch) * sinf(yaw);

    camera_forward.w = 0.0f;

    camera_forward = glm::normalize(camera_forward);

    //--------------------------------------------------
    // Rebuild camera basis
    //--------------------------------------------------

    camera_right =
        glm::vec4(
            glm::normalize(
                glm::cross(
                    glm::vec3(camera_forward),
                    glm::vec3(0, 1, 0))),
            0.0f);

    camera_up =
        glm::vec4(
            glm::normalize(
                glm::cross(
                    glm::vec3(camera_right),
                    glm::vec3(camera_forward))),
            0.0f);
}

void DrawEntity(const std::shared_ptr<Entity> entity)
{
    const std::vector<std::shared_ptr<MeshComp>>& mesh_components = g_entities_virtual_meshes[entity->GetId()];

    if (mesh_components.empty()) return;

    auto draw_shape = [&](std::shared_ptr<MeshComp> mesh_comp)
        {
            std::shared_ptr<ObjModel> model_to_draw = mesh_comp->model;
            const tinyobj::shape_t& shape = model_to_draw->shapes[mesh_comp->submesh_index];

            glm::mat4 local_comp_transform_mat = matops::MatrixIdentity();

			auto transform_components = mesh_comp->GetComponentsByType<TransformComp>();

            if (!transform_components.empty())
            {
                glm::vec3 center = (mesh_comp->bbox_min + mesh_comp->bbox_max) / 2.0f;
                std::shared_ptr<TransformComp> transform_component = transform_components[0];
                local_comp_transform_mat = matops::MatrixTranslate(center.x, center.y, center.z)
                    * matops::MatrixTranslate(transform_component->position.x, transform_component->position.y, transform_component->position.z)
                    * matops::MatrixRotateZ(glm::radians(transform_component->rotation.z))
                    * matops::MatrixRotateY(glm::radians(transform_component->rotation.y))
                    * matops::MatrixRotateX(glm::radians(transform_component->rotation.x))
					* matops::MatrixScale(transform_component->scale.x, transform_component->scale.y, transform_component->scale.z)
                    * matops::MatrixTranslate(-center.x, -center.y, -center.z);
            }

            glm::mat4 model = matops::MatrixTranslate(entity->root->position.x, entity->root->position.y, entity->root->position.z)
                * matops::MatrixRotateX(glm::radians(entity->root->rotation.x))
                * matops::MatrixRotateY(glm::radians(entity->root->rotation.y))
                * matops::MatrixRotateZ(glm::radians(entity->root->rotation.z))
                * matops::MatrixScale(entity->root->scale.x, entity->root->scale.y, entity->root->scale.z)
                * local_comp_transform_mat;

            glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(model));

            glActiveTexture(GL_TEXTURE0);
            int material_idx = shape.mesh.material_ids[0];
            const auto& textures_ids_it = model_to_draw->textures_ids.find(material_idx);
            bool has_any_texture = (textures_ids_it != model_to_draw->textures_ids.end());
            if (has_any_texture && textures_ids_it->second.diffuse_id != max_int32)
            {
                glUniform1i(g_has_kd_texture_uniform, true);

                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, textures_ids_it->second.diffuse_id);
            }
            else
            {
                glUniform1i(g_has_kd_texture_uniform, false);

                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, 0);
            }
            if (has_any_texture && textures_ids_it->second.emissive_id != max_int32)
            {
                glUniform1i(g_has_ke_texture_uniform, true);

                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, textures_ids_it->second.emissive_id);
            }
            else
            {
                glUniform1i(g_has_ke_texture_uniform, false);

                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, 0);
            }
            if (has_any_texture && textures_ids_it->second.opacity_id != max_int32)
            {
                glUniform1i(g_has_opacity_texture_uniform, true);

                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, textures_ids_it->second.opacity_id);
            }
            else
            {
                glUniform1i(g_has_opacity_texture_uniform, false);

                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, 0);
            }

            const auto& mat = mesh_comp->material;
            glUniform3f(g_kd_uniform, mat.albedo[0], mat.albedo[1], mat.albedo[2]);
            glUniform3f(g_ks_uniform, mat.specular[0], mat.specular[1], mat.specular[2]);
            glUniform3f(g_ke_uniform, mat.emissive[0], mat.emissive[1], mat.emissive[2]);
            glUniform1f(g_ns_uniform, mat.shininess);
            glUniform1f(g_opacity_uniform, mat.opacity);

            DrawVirtualMesh(mesh_comp);
        };

    //std::map<float, const tinyobj::shape_t*> transparent_shapes;
    std::vector<std::shared_ptr<MeshComp>> transparent_objects;

    // OPAQUE PASS
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);

    for (const auto& mesh_comp : mesh_components)
    {
        std::shared_ptr<ObjModel> model_to_draw = mesh_comp->model;
		const auto& shape = model_to_draw->shapes[mesh_comp->submesh_index];

        int material_idx = shape.mesh.material_ids[0];
        const auto& mat = model_to_draw->materials[material_idx];
        const auto& textures_ids_it = model_to_draw->textures_ids.find(material_idx);
        bool is_transparent = (mat.dissolve < 0.999f) ||
            (textures_ids_it != model_to_draw->textures_ids.end() && textures_ids_it->second.opacity_id != max_int32);
        if (is_transparent)
        {
            transparent_objects.push_back(mesh_comp);
        }
        else
        {
            draw_shape(mesh_comp);
        }
    }

    // TRANSPARENT PASS
    glEnable(GL_BLEND);
    glDepthMask(GL_FALSE);

    for (const auto& scene_obj_comp : mesh_components)
    {
        std::shared_ptr<ObjModel> model_to_draw = scene_obj_comp->model;
        const auto& shape = model_to_draw->shapes[scene_obj_comp->submesh_index];

        draw_shape(scene_obj_comp);
    }

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0); // unbind
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0); // unbind
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, 0); // unbind
}

void UpdateRaceUserInterface(GLFWwindow* window, const std::vector<std::shared_ptr<Car>>& cars)
{
    static const std::vector<glm::vec2> screen_pos_players_info{
        { -0.9, 0.88 }, { 0.9, 0.88 }
    };

    static constexpr float text_scale = 1.5f;
    for (int32_t i = 0; i < cars.size(); ++i)
    {
        glm::vec2 info_pos = screen_pos_players_info[i];
        std::string player_text = std::format("Player {}: {}/{}", i, cars[i]->laps_completed, g_num_laps);
        info_pos.x -= info_pos.x > 0 ? TextRendering_CharWidth(window) * player_text.size() * text_scale : 0;
        TextRendering_PrintString(window, player_text, info_pos.x, info_pos.y, text_scale);
    }
}

std::vector<std::shared_ptr<MeshComp>> CreateMeshComponentsForModelByName(const std::string& model_name)
{
	std::vector<std::shared_ptr<MeshComp>> result;

    auto it = g_loaded_models.find(model_name);

	if (it == g_loaded_models.end()) return result;

	std::shared_ptr<ObjModel> model = it->second;

    for(int32_t shape_index = 0; shape_index < model->shapes.size(); ++shape_index)
    {
        std::shared_ptr<MeshComp> mesh_comp = std::make_shared<MeshComp>();
        mesh_comp->mesh_name = model->shapes[shape_index].name;
        mesh_comp->first_index    = model->first_indices[shape_index]; // Primeiro índice
        mesh_comp->num_indices    = model->num_indices[shape_index]; // Número de indices
        mesh_comp->rendering_mode = GL_TRIANGLES;       // Índices correspondem ao tipo de rasterização GL_TRIANGLES.
        mesh_comp->vertex_array_object_id = model->vertex_array_object_ids[shape_index];

        mesh_comp->bbox_min = model->bboxes_min[shape_index];;
        mesh_comp->bbox_max = model->bboxes_max[shape_index];;

        mesh_comp->model = model;
        mesh_comp->submesh_index = shape_index;

        tinyobj::material_t shape_mat = model->materials[model->shapes[shape_index].mesh.material_ids[0]];

        mesh_comp->name = shape_mat.name;
		mesh_comp->material.albedo = { shape_mat.diffuse[0], shape_mat.diffuse[1], shape_mat.diffuse[2] };
		mesh_comp->material.specular = { shape_mat.specular[0], shape_mat.specular[1], shape_mat.specular[2] };
		mesh_comp->material.emissive = { shape_mat.emission[0], shape_mat.emission[1], shape_mat.emission[2] };
		mesh_comp->material.shininess = shape_mat.shininess;
		mesh_comp->material.opacity = shape_mat.dissolve;

		result.push_back(mesh_comp);
	}

    return result;
}

std::shared_ptr<Car> CreateCar(const std::string& name, const std::shared_ptr<ObjModel>& model, const std::array<int32_t, 4>& input_keys, glm::vec3 color)
{
    std::shared_ptr<Car> car = std::make_shared<Car>(name);
    car->AddComponents(CreateMeshComponentsForModelByName(model->filepath));
    g_entities_virtual_meshes.emplace(car->GetId(), car->GetComponentsByType<MeshComp>());
    car->root->scale = { 0.25f, 0.25f, 0.25f };

    car->input_keys = input_keys;
    car->wheels_mesh_comps = {};
    car->wheels_transform_comps = {};

    for (std::shared_ptr<MeshComp> mesh_comp : car->GetComponentsByType<MeshComp>())
    {
        if (mesh_comp->mesh_name.find("Wheel.") != std::string::npos)
        {
            car->wheels_transform_comps.push_back(mesh_comp->AttachComponent<TransformComp>());
            car->wheels_mesh_comps.push_back(mesh_comp);
        }
        if (mesh_comp->mesh_name.find("_23") != std::string::npos)
        {
            mesh_comp->material.albedo = color;
        }
    }

    return car;
}

void UpdateCarTransformOnTrack(double delta_time, std::shared_ptr<Car> car, std::shared_ptr<Track> track)
{
    if (car->cur_lane != car->target_lane)
    {
        car->lane_transitioning_length += car->speed * (float)delta_time;

        if (car->lane_transitioning_length >= g_lane_transitioning_length)
        {
            car->cur_lane = car->target_lane;
            car->lane_transitioning_length = 0.0f;
            car->cur_curve_pos = car->transition_curve_pos;
        }
    }

    const auto& track_points = track->lanes[car->cur_lane];
    // Car animation path update
    int32_t next_point = (car->cur_curve_point + 1) % track_points.size();
    glm::vec3 car_forward;
    while (true)
    {
        car_forward = glm::normalize(track_points[next_point] - track_points[car->cur_curve_point]);
        glm::vec3 new_curve_pos = car->cur_curve_pos + car->speed * (float)delta_time * car_forward;

        if (glm::dot(car_forward, track_points[next_point] - new_curve_pos) < 0)
        {
            if (next_point == (g_init_curve_point - 1))
            {
                g_is_game_over = (++car->laps_completed == g_num_laps);
                g_is_game_running = !g_is_game_over;
            }
            next_point = (next_point + 1) % track_points.size();
            new_curve_pos = car->cur_curve_pos;
        }
        else
        {
            car->cur_curve_pos = new_curve_pos;
            car->transition_curve_pos = new_curve_pos;
            car->cur_curve_point = next_point - 1;
            if (car->cur_curve_point < 0 || car->cur_curve_point >= track_points.size())
                car->cur_curve_point = 0;
            break;
        }
    }

    const float transition_progress = car->lane_transitioning_length / g_lane_transitioning_length;
    const float left_right_rotation_factor = (car->target_lane - car->cur_lane) * (1.0f - std::powf(std::abs(transition_progress - 0.5f) * 2.0f, 2.0f));

    if (car->cur_lane != car->target_lane)
    {
        const auto& transitioning_lane_points = track->lanes[car->target_lane];
        glm::vec3 lanes_points_diff = transitioning_lane_points[car->cur_curve_point] - track_points[car->cur_curve_point];
        car->transition_curve_pos += lanes_points_diff * std::clamp((car->lane_transitioning_length / g_lane_transitioning_length), 0.0f, 1.0f);

        glm::vec3 steering_forward = glm::normalize(transitioning_lane_points[(car->cur_curve_point + size_t(track->lane_lengths[car->target_lane] / g_lane_transitioning_length) * 2) % track_points.size()] - track_points[car->cur_curve_point]);
        car_forward = glm::normalize(
            steering_forward * std::abs(left_right_rotation_factor) +
            car_forward * (1.0f - std::abs(left_right_rotation_factor)));
    }

    float theta = glm::degrees(acos(glm::dot({ 0.0f, 0.0f, 1.0f }, car_forward)));
    if (car_forward.x < 0)
        theta = glm::degrees(glm::two_pi<float>()) - theta;

    float phi = glm::degrees(asin(glm::dot({ 0.0f, 1.0f, 0.0f }, car_forward)));
    if (car_forward.z > 0)
        phi = glm::degrees(glm::two_pi<float>()) - phi;

    glm::vec3 world_up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 car_right = glm::normalize(glm::cross(world_up, car_forward));
    glm::vec3 car_up = glm::normalize(glm::cross(car_forward, car_right));
    float roll = glm::degrees(atan2(glm::dot(car_right, world_up), glm::dot(car_up, world_up)));

    glm::vec3 car_world_rotation = { phi, theta, roll };

    glm::mat4 model = matops::MatrixIdentity(); // Transformação identidade de modelagem
    model *= matops::MatrixScale(track->root->scale.x, track->root->scale.y, track->root->scale.z);
    glm::vec3 car_world_pos = model * glm::vec4(car->transition_curve_pos, 1.0f);

    car->root->position = car_world_pos;
    car->root->rotation = car_world_rotation;
    car->forward = car_forward;
}

void UpdateCarInputAndAnimation(double delta_time, std::shared_ptr<Car> car, std::shared_ptr<Track> track)
{
    static constexpr float max_car_speed = 50.0f;
    static constexpr float car_acceleration = 16.0f;
    static constexpr float asphalt_friction = 0.7f;

	const float transition_progress = car->lane_transitioning_length / g_lane_transitioning_length;
	const float transitioning_lane_length = track->normalized_lane_lengths[car->cur_lane] * (1 - transition_progress) +
        track->normalized_lane_lengths[car->target_lane] * transition_progress;

    car->is_accelerating = false;
    if (keys[car->input_keys.at(0)] && !car->is_out_of_control)
    {
        car->speed += car_acceleration * transitioning_lane_length * float(delta_time);
        car->is_accelerating = true;
    }
    else if (keys[car->input_keys.at(1)] && !car->is_out_of_control)
    {
        car->speed -= car_acceleration * transitioning_lane_length * float(delta_time);
    }

	float destabilization_factor = car->is_out_of_control ? 0.35f : 0.0f;
    car->speed *= powf(asphalt_friction - destabilization_factor, float(delta_time));

    car->speed = std::clamp(car->speed, 0.0f, max_car_speed * transitioning_lane_length);

    if (keys[car->input_keys.at(2)] && !car->is_out_of_control && car->cur_lane == car->target_lane)
    {
		car->target_lane = std::max(0, car->target_lane - 1);
    }
    else if (keys[car->input_keys.at(3)] && !car->is_out_of_control && car->cur_lane == car->target_lane)
    {
		car->target_lane = std::min(int32_t(track->lanes.size() - 1), car->target_lane + 1);
    }

	UpdateCarTransformOnTrack(delta_time, car, track);

    const float left_right_rotation_factor = (car->target_lane - car->cur_lane) * (1.0f - std::powf(std::abs(transition_progress - 0.5f) * 2.0f, 2.0f));

    // Car wheels animation update
    for (int32_t i = 0; i < car->wheels_mesh_comps.size(); ++i)
    {
        std::shared_ptr<MeshComp> mesh_comp = car->wheels_mesh_comps[i];
        std::shared_ptr<TransformComp> wheel_transform_comp = car->wheels_transform_comps[i];
        float radius = (mesh_comp->bbox_max.y - mesh_comp->bbox_min.y) * 0.5f * car->root->scale.y;
        wheel_transform_comp->rotation.x += glm::degrees((car->speed / radius) * float(delta_time));
        wheel_transform_comp->rotation.y = 35.0f * left_right_rotation_factor;
    }

	// Car destabilization update
    float car0_ratio = ComputeCarSpeedRelativeToTrackCurvature(car, track);

    static constexpr float warning_threshold = 0.6f;
    static constexpr float skid_threshold = 1.10f;
    car->yaw_tremble_timer += delta_time;

    float yaw_offset = 0.0f;

    if (!car->is_out_of_control)
    {
        if (car0_ratio > warning_threshold && car0_ratio <= skid_threshold)
        {
            float t = ((car0_ratio - warning_threshold) / (skid_threshold - warning_threshold)) + 0.25f;

            t = glm::clamp(t, 0.0f, 1.0f);

            // stronger near the limit
            //t = t * t;

            static constexpr float max_warning_tremble = 12.0f;
            static constexpr float warning_frequency = 16.0f;

            yaw_offset = max_warning_tremble * t * sinf(car->yaw_tremble_timer * warning_frequency);
        }
        else if (car0_ratio > skid_threshold && car->is_accelerating)
        {
            car->is_accelerating = false;
            car->is_out_of_control = true;
            car->out_of_control_timer = 0.0f;
        }
    }
    else
    {
        static constexpr float skid_tremble = 16.0f;
        static constexpr float skid_frequency = 25.0f;

        yaw_offset = skid_tremble * sinf(car->yaw_tremble_timer * skid_frequency);

        car->out_of_control_timer += delta_time;

        static constexpr float destabilization_duration = 2.0f;
        if (car->out_of_control_timer >= destabilization_duration)
        {
            car->is_out_of_control = false;
            car->out_of_control_timer = 0.0f;
        }
    }

    car->root->rotation.y += yaw_offset;
}

struct Obb
{
    glm::vec3 center;
    glm::vec3 half_extents;

    std::array<glm::vec3, 3> axis;
};

Obb CreateObb(const ObjModel& model, const glm::mat4& transform)
{
    Obb result;

    glm::vec3 local_center = (model.min_bounds + model.max_bounds) * 0.5f;

    glm::vec3 local_extents = (model.max_bounds - model.min_bounds) * 0.5f;


    glm::vec4 world_center = transform * glm::vec4(local_center, 1.0f);

    result.center = glm::vec3(world_center);

    result.half_extents = local_extents *
        glm::vec3(
            glm::length(glm::vec3(transform[0])),
            glm::length(glm::vec3(transform[1])),
            glm::length(glm::vec3(transform[2]))
        );

    result.axis[0] = glm::normalize(glm::vec3(transform[0]));
    result.axis[1] = glm::normalize(glm::vec3(transform[1]));
    result.axis[2] = glm::normalize(glm::vec3(transform[2]));

    return result;
}

bool Intersects(const Obb& a, const Obb& b)
{
    constexpr float EPSILON = 1e-6f;

    glm::mat3 R;
    glm::mat3 AbsR;

    // Rotation matrix from B's coordinate frame into A's frame
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            R[i][j] = glm::dot(a.axis[i], b.axis[j]);
            AbsR[i][j] = fabs(R[i][j]) + EPSILON;
        }
    }

    // Translation vector from A to B expressed in A's coordinate frame
    glm::vec3 t = b.center - a.center;

    t = glm::vec3(
        glm::dot(t, a.axis[0]),
        glm::dot(t, a.axis[1]),
        glm::dot(t, a.axis[2])
    );


    float ra;
    float rb;


    // Test axes of A
    for (int i = 0; i < 3; ++i)
    {
        ra = a.half_extents[i];

        rb =
            b.half_extents[0] * AbsR[i][0] +
            b.half_extents[1] * AbsR[i][1] +
            b.half_extents[2] * AbsR[i][2];

        if (fabs(t[i]) > ra + rb)
            return false;
    }


    // Test axes of B
    for (int i = 0; i < 3; ++i)
    {
        ra =
            a.half_extents[0] * AbsR[0][i] +
            a.half_extents[1] * AbsR[1][i] +
            a.half_extents[2] * AbsR[2][i];

        rb = b.half_extents[i];

        float distance =
            fabs(
                t[0] * R[0][i] +
                t[1] * R[1][i] +
                t[2] * R[2][i]
            );

        if (distance > ra + rb)
            return false;
    }


    // Test cross products of A's axes and B's axes
    // L = A0 x B0
    ra =
        a.half_extents[1] * AbsR[2][0] +
        a.half_extents[2] * AbsR[1][0];

    rb =
        b.half_extents[1] * AbsR[0][2] +
        b.half_extents[2] * AbsR[0][1];

    if (fabs(t[2] * R[1][0] - t[1] * R[2][0]) > ra + rb)
        return false;


    // L = A0 x B1
    ra =
        a.half_extents[1] * AbsR[2][1] +
        a.half_extents[2] * AbsR[1][1];

    rb =
        b.half_extents[0] * AbsR[0][2] +
        b.half_extents[2] * AbsR[0][0];

    if (fabs(t[2] * R[1][1] - t[1] * R[2][1]) > ra + rb)
        return false;


    // L = A0 x B2
    ra =
        a.half_extents[1] * AbsR[2][2] +
        a.half_extents[2] * AbsR[1][2];

    rb =
        b.half_extents[0] * AbsR[0][1] +
        b.half_extents[1] * AbsR[0][0];

    if (fabs(t[2] * R[1][2] - t[1] * R[2][2]) > ra + rb)
        return false;


    // L = A1 x B0
    ra =
        a.half_extents[0] * AbsR[2][0] +
        a.half_extents[2] * AbsR[0][0];

    rb =
        b.half_extents[1] * AbsR[1][2] +
        b.half_extents[2] * AbsR[1][1];

    if (fabs(t[0] * R[2][0] - t[2] * R[0][0]) > ra + rb)
        return false;


    // L = A1 x B1
    ra =
        a.half_extents[0] * AbsR[2][1] +
        a.half_extents[2] * AbsR[0][1];

    rb =
        b.half_extents[0] * AbsR[1][2] +
        b.half_extents[2] * AbsR[1][0];

    if (fabs(t[0] * R[2][1] - t[2] * R[0][1]) > ra + rb)
        return false;


    // L = A1 x B2
    ra =
        a.half_extents[0] * AbsR[2][2] +
        a.half_extents[2] * AbsR[0][2];

    rb =
        b.half_extents[0] * AbsR[1][1] +
        b.half_extents[1] * AbsR[1][0];

    if (fabs(t[0] * R[2][2] - t[2] * R[0][2]) > ra + rb)
        return false;


    // L = A2 x B0
    ra =
        a.half_extents[0] * AbsR[1][0] +
        a.half_extents[1] * AbsR[0][0];

    rb =
        b.half_extents[1] * AbsR[2][2] +
        b.half_extents[2] * AbsR[2][1];

    if (fabs(t[1] * R[0][0] - t[0] * R[1][0]) > ra + rb)
        return false;


    // L = A2 x B1
    ra =
        a.half_extents[0] * AbsR[1][1] +
        a.half_extents[1] * AbsR[0][1];

    rb =
        b.half_extents[0] * AbsR[2][2] +
        b.half_extents[2] * AbsR[2][0];

    if (fabs(t[1] * R[0][1] - t[0] * R[1][1]) > ra + rb)
        return false;


    // L = A2 x B2
    ra =
        a.half_extents[0] * AbsR[1][2] +
        a.half_extents[1] * AbsR[0][2];

    rb =
        b.half_extents[0] * AbsR[2][1] +
        b.half_extents[1] * AbsR[2][0];

    if (fabs(t[1] * R[0][2] - t[0] * R[1][2]) > ra + rb)
        return false;


    // No separating axis found
    return true;
}

void UpdateCarsPhysics(double delta_time, std::vector<std::shared_ptr<Car>> cars, std::shared_ptr<Track> track)
{
	std::vector<Obb> cars_obbs;

    auto create_car_obb = [&](std::shared_ptr<Car> car) {
        glm::mat4 model_transform = matops::MatrixTranslate(car->root->position.x, car->root->position.y, car->root->position.z)
            * matops::MatrixRotateX(glm::radians(car->root->rotation.x))
            * matops::MatrixRotateY(glm::radians(car->root->rotation.y))
            * matops::MatrixRotateZ(glm::radians(car->root->rotation.z))
            * matops::MatrixScale(car->root->scale.x, car->root->scale.y, car->root->scale.z);
        std::shared_ptr<ObjModel> car_model = g_entities_virtual_meshes[car->GetId()].front()->model;
        return CreateObb(*car_model, model_transform);
		};

    for (std::shared_ptr<Car> car : cars)
    {   
		cars_obbs.push_back(create_car_obb(car));
    }

    auto handle_lane_switch = [&](std::shared_ptr<Car> car_in_front, std::shared_ptr<Car> car_behind) {
        if(car_in_front->target_lane == car_behind->target_lane)
        {
            if(car_behind->target_lane != car_behind->cur_lane)
            {
                car_behind->lane_transitioning_length = std::max(g_lane_transitioning_length - car_behind->lane_transitioning_length, 0.0f);
				std::swap(car_behind->target_lane, car_behind->cur_lane);
                car_behind->cur_curve_pos = track->lanes[car_behind->cur_lane].at(car_behind->cur_curve_point);
            }
            else
            {
                car_behind->target_lane = car_behind->cur_lane >= track->lanes.size() - 1 ? car_behind->cur_lane - 1 : car_behind->cur_lane + 1;
                car_behind->lane_transitioning_length = 0.0f;
            }
        }

        if (!car_in_front->is_out_of_control)
        {
            car_in_front->is_accelerating = false;
            car_in_front->is_out_of_control = true;
            car_in_front->out_of_control_timer = 0.0f;
        }
        };

    auto handle_collision_overlap = [&](std::shared_ptr<Car> car_in_front, std::shared_ptr<Car> car_behind)
        {
			Obb car_in_front_obb = create_car_obb(car_in_front);
			Obb car_behind_obb = create_car_obb(car_behind);

			float temp_speed = car_in_front->speed;
            while(Intersects(car_in_front_obb, car_behind_obb))
            {
                car_in_front->speed = car_behind->speed;
                UpdateCarTransformOnTrack(0.01f, car_in_front, track);
                car_in_front_obb = create_car_obb(car_in_front);
            }

			car_in_front->speed = temp_speed;
        };


    for (int32_t i = 0; i < cars.size() - 1; ++i)
    {
        std::shared_ptr<Car> this_car = cars[i];
        const Obb& this_car_obb = cars_obbs[i];

        for (int32_t j = i + 1; j < cars.size(); ++j)
        {
            std::shared_ptr<Car> other_car = cars[j];
            const Obb& other_car_obb = cars_obbs[j];

            // if aabbs not overlap continue
            if (!Intersects(this_car_obb, other_car_obb)) continue;

			const glm::vec3& this_car_pos = this_car->root->position;
			const glm::vec3& other_car_pos = other_car->root->position;

			const glm::vec3 diff_other_to_this = other_car_pos - this_car_pos;

            // if this car is behind
            if (glm::dot(diff_other_to_this, this_car->forward) > 0.0f)
            {
                if (!this_car->is_out_of_control && !other_car->is_out_of_control)
                    handle_lane_switch(other_car, this_car);
                handle_collision_overlap(other_car, this_car);
            }
            // if the other car is behind
            else if (glm::dot(diff_other_to_this, other_car->forward) < 0.0f)
            {
                if (!this_car->is_out_of_control && !other_car->is_out_of_control)
                    handle_lane_switch(this_car, other_car);
                handle_collision_overlap(this_car, other_car);
            }
        }
    }
}

std::vector<std::vector<glm::vec3>> SplitCurvePathInLanes(const std::vector<glm::vec3>& points, int32_t num_lanes)
{
    std::vector<std::vector<glm::vec3>> lanes;

    lanes.resize(num_lanes);

    constexpr float lane_width = 3.5f;
    const float track_width = (num_lanes - 1) * lane_width;

    for (int32_t l = 0; l < num_lanes; ++l)
    {
        lanes[l].reserve(points.size());

        float lane_width_coord = l * (track_width / (num_lanes - 1)) - (track_width / 2);

        for (int32_t p = 0; p < points.size() - 1; ++p)
        {
            const glm::vec3& point = points[p];
            const glm::vec3& next_point = points[p + 1];

            glm::vec3 forward = glm::normalize(next_point - point);
            glm::vec3 right = glm::cross(glm::vec3(0, 1, 0), forward);
            glm::vec3 offset = right * lane_width_coord;
            lanes[l].push_back(point + offset);
        }
        const glm::vec3& point = points[points.size() - 1];
        const glm::vec3& next_point = points[0];

        glm::vec3 forward = glm::normalize(next_point - point);
        glm::vec3 right = glm::cross(glm::vec3(0, 1, 0), forward);
        glm::vec3 offset = right * lane_width_coord;
        lanes[l].push_back(point + offset);
    }

    return lanes;
}

std::vector<float> ComputeLaneLengths(const std::vector<std::vector<glm::vec3>>& lanes)
{
    std::vector<float> lane_lengths;
    lane_lengths.reserve(lanes.size());

    for (const auto& lane : lanes)
    {
        float lane_length = 0.0f;
        for (int32_t p = 0; p < lane.size() - 1; ++p)
        {
            lane_length += glm::distance(lane[p], lane[p + 1]);
        }
        lane_length += glm::distance(lane[lane.size() - 1], lane[0]);
        lane_lengths.push_back(lane_length);
    }

    return lane_lengths;
}

std::vector<float> ComputeNormalizedLaneLengths(const std::vector<std::vector<glm::vec3>>& lanes)
{
	std::vector<float> normalized_lane_lengths;
	normalized_lane_lengths.reserve(lanes.size());

    float min_lane_length = std::numeric_limits<float>::max();
    for(const auto& lane : lanes)
    {
        float lane_length = 0.0f;
        for (int32_t p = 0; p < lane.size() - 1; ++p)
        {
            lane_length += glm::distance(lane[p], lane[p + 1]);
        }
        lane_length += glm::distance(lane[lane.size() - 1], lane[0]);
        normalized_lane_lengths.push_back(lane_length);
        min_lane_length = std::min(min_lane_length, lane_length);
	}

    for(float& lane_length : normalized_lane_lengths)
    {
        lane_length /= min_lane_length;
	}

    return normalized_lane_lengths;
}

float ComputeCurveRadius(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2)
{
    float a = glm::length(p1 - p0);
    float b = glm::length(p2 - p1);
    float c = glm::length(p2 - p0);

    float area2 = glm::length(glm::cross(p1 - p0, p2 - p0));

    if (area2 < 0.0001f)
        return FLT_MAX; // almost straight line

    return (a * b * c) / area2;
}

float ComputeCarSpeedRelativeToTrackCurvature(std::shared_ptr<Car> car, std::shared_ptr<Track> track)
{
    const auto& points = track->lanes[car->cur_lane];

    int count = (int)points.size();

    int i0 = (car->cur_curve_point - 1 + count) % count;
    int i1 = car->cur_curve_point;
    int i2 = (car->cur_curve_point + 1) % count;

    float radius = ComputeCurveRadius(
        points[i0],
        points[i1],
        points[i2]);

    if (radius == FLT_MAX)
        return 0.0f;

    float lateral_accel = (car->speed * car->speed) / radius;

    static constexpr float grip_limit = 32.0f;

    return lateral_accel / grip_limit;
}
