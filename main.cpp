/*---------------------------------------------------------*/
/* ----------- Proyecto Final: Panteón Oaxaca -------------*/
/*-----------------    Semestre 2027-1   ------------------*/
/*---------------------------------------------------------*/
#include <Windows.h>
#include <glad/glad.h>
#include <glfw3.h>                      // main
#include <stdlib.h>     
#include <glm/glm.hpp>                  // camera y model
#include <glm/gtc/matrix_transform.hpp> // camera y model
#include <glm/gtc/type_ptr.hpp>
#include <time.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>                  // Texture
#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>
#include <shader_m.h>
#include <camera.h>
#include <modelAnim.h>
#include <model.h>
#include <Skybox.h>
#include <iostream>
#include <mmsystem.h>

// Prototipos de funciones
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void my_input(GLFWwindow* window, int key, int scancode, int action, int mods);
void animate(void);
void getResolution(void);

// Configuración de ventana
unsigned int SCR_WIDTH = 800;
unsigned int SCR_HEIGHT = 600;
GLFWmonitor* monitors;

// Cámara sintética (Posición inicial sobre el piso)
Camera camera(glm::vec3(0.0f, 10.0f, 50.0f));
float MovementSpeed = 2.0f;
GLfloat lastX = SCR_WIDTH / 2.0f,
        lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// Control de tiempo (60 FPS)
const int FPS = 60;
const int LOOP_TIME = 1000 / FPS;
double deltaTime = 0.0f,
lastFrame = 0.0f;

// Iluminación
glm::vec3 lightPosition(0.0f, 5.0f, 0.0f);     // Posición para luz puntual (ej. veladora)
glm::vec3 lightDirection(0.0f, -1.0f, -0.5f);  // Dirección de la luz ambiental nocturna

// Variables para KeyFrames (Listas para las animaciones de esta semana)
float posX = 0.0f, posY = 0.0f, posZ = 0.0f;
float incX = 0.0f, incY = 0.0f, incZ = 0.0f;

#define MAX_FRAMES 9
int i_max_steps = 60;
int i_curr_steps = 0;

typedef struct _frame {
    float posX;
    float posY;
    float posZ;
} FRAME;

FRAME KeyFrame[MAX_FRAMES];
int FrameIndex = 0;
bool play = false;
int playIndex = 0;

void saveFrame(void) {
    std::cout << "Frame Index = " << FrameIndex << std::endl;
    KeyFrame[FrameIndex].posX = posX;
    KeyFrame[FrameIndex].posY = posY;
    KeyFrame[FrameIndex].posZ = posZ;
    FrameIndex++;
}

void resetElements(void) {
    posX = KeyFrame[0].posX;
    posY = KeyFrame[0].posY;
    posZ = KeyFrame[0].posZ;
}

void interpolation(void) {
    incX = (KeyFrame[playIndex + 1].posX - KeyFrame[playIndex].posX) / i_max_steps;
    incY = (KeyFrame[playIndex + 1].posY - KeyFrame[playIndex].posY) / i_max_steps;
    incZ = (KeyFrame[playIndex + 1].posZ - KeyFrame[playIndex].posZ) / i_max_steps;
}

void animate(void) {
    // Interpolación de KeyFrames
    if (play) {
        if (i_curr_steps >= i_max_steps) {
            playIndex++;
            if (playIndex > FrameIndex - 2) {
                std::cout << "Animation ended" << std::endl;
                playIndex = 0;
                play = false;
            }
            else {
                i_curr_steps = 0;
                interpolation();
            }
        }
        else {
            posX += incX;
            posY += incY;
            posZ += incZ;
            i_curr_steps++;
        }
    }
}

void getResolution() {
    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    SCR_WIDTH = mode->width;
    SCR_HEIGHT = (mode->height) - 80;
}

int main() {
    // Inicializar GLFW
    glfwInit();

    monitors = glfwGetPrimaryMonitor();
    getResolution();

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Proyecto Final CG - Panteon Oaxaca", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwSetWindowPos(window, 0, 30);
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, my_input);

    // Captura del cursor (Puedes cambiar GLFW_CURSOR_NORMAL a GLFW_CURSOR_DISABLED si quieres ocultar el puntero)
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    // Inicializar GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    // Compilación de Shaders
    Shader staticShader("Shaders/shader_Lights.vs", "Shaders/shader_Lights_mod.fs");
    Shader skyboxShader("Shaders/skybox.vs", "Shaders/skybox.fs");
    Shader animShader("Shaders/anim.vs", "Shaders/anim.fs");

    vector<std::string> faces{
        "resources/skybox/right.jpg",
        "resources/skybox/left.jpg",
        "resources/skybox/top.jpg",
        "resources/skybox/bottom.jpg",
        "resources/skybox/front.jpg",
        "resources/skybox/back.jpg"
    };

    Skybox skybox = Skybox(faces);

    skyboxShader.use();

    skyboxShader.setInt("skybox", 0);

    // ------------------------------------------------------------------
    // Carga de modelos del escenario
    // ------------------------------------------------------------------
    Model Piso("resources/objects/Piso/Piso.obj");
    Model Tumba("resources/objects/Tumba/Tumba.obj");
    Model Tumba2("resources/objects/Tumba2/Tumba2.obj");
    Model Tumba3("resources/objects/Tumba3/Tumba3.obj");
    Model Entrada("resources/objects/Entrada_arco/Entrada_arco.obj");
    Model Muro("resources/objects/Muro/Muro.obj");


    // Inicialización de KeyFrames en cero
    for (int i = 0; i < MAX_FRAMES; i++) {
        KeyFrame[i].posX = 0;
        KeyFrame[i].posY = 0;
        KeyFrame[i].posZ = 0;
    }

    glm::mat4 modelOp = glm::mat4(1.0f);
    glm::mat4 viewOp = glm::mat4(1.0f);
    glm::mat4 projectionOp = glm::mat4(1.0f);

    // ------------------------------------------------------------------
    // Ciclo de renderizado (Render Loop)
    // ------------------------------------------------------------------
    while (!glfwWindowShouldClose(window)) {
        skyboxShader.setInt("skybox", 0);

        lastFrame = SDL_GetTicks();

        animate();

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Activar shader principal para modelos e iluminación
        staticShader.use();
        staticShader.setVec3("viewPos", camera.Position);

        // Luz Direccional
        staticShader.setVec3("dirLight.direction", lightDirection);
        staticShader.setVec3("dirLight.ambient", glm::vec3(0.18f, 0.18f, 0.25f));
        staticShader.setVec3("dirLight.diffuse", glm::vec3(0.25f, 0.25f, 0.35f));
        staticShader.setVec3("dirLight.specular", glm::vec3(0.1f, 0.1f, 0.1f));

        // Vela 
        float vela_constant = 1.0f;
        float vela_linear = 0.009f;
        float vela_quadratic = 0.00032f;
        glm::vec3 vela_ambient = glm::vec3(0.05f, 0.02f, 0.0f);
        glm::vec3 vela_diffuse = glm::vec3(0.6f, 0.3f, 0.05f); // Tono cálido
        glm::vec3 vela_specular = glm::vec3(0.2f, 0.1f, 0.0f);

        // Luz Puntual 0 
        staticShader.setVec3("pointLight[0].position", glm::vec3(0.0f, 12.0f, -250.0f));
        staticShader.setVec3("pointLight[0].ambient", vela_ambient);
        staticShader.setVec3("pointLight[0].diffuse", vela_diffuse);
        staticShader.setVec3("pointLight[0].specular", vela_specular);
        staticShader.setFloat("pointLight[0].constant", vela_constant);
        staticShader.setFloat("pointLight[0].linear", vela_linear);
        staticShader.setFloat("pointLight[0].quadratic", vela_quadratic);

        // Luz Puntual 1 
        staticShader.setVec3("pointLight[1].position", glm::vec3(-250.0f, 12.0f, 200.0f));
        staticShader.setVec3("pointLight[1].ambient", vela_ambient);
        staticShader.setVec3("pointLight[1].diffuse", vela_diffuse);
        staticShader.setVec3("pointLight[1].specular", vela_specular);
        staticShader.setFloat("pointLight[1].constant", vela_constant);
        staticShader.setFloat("pointLight[1].linear", vela_linear);
        staticShader.setFloat("pointLight[1].quadratic", vela_quadratic);

        // Luz Puntual 2 
        staticShader.setVec3("pointLight[2].position", glm::vec3(250.0f, 12.0f, -250.0f));
        staticShader.setVec3("pointLight[2].ambient", vela_ambient);
        staticShader.setVec3("pointLight[2].diffuse", vela_diffuse);
        staticShader.setVec3("pointLight[2].specular", vela_specular);
        staticShader.setFloat("pointLight[2].constant", vela_constant);
        staticShader.setFloat("pointLight[2].linear", vela_linear);
        staticShader.setFloat("pointLight[2].quadratic", vela_quadratic);

        // SpotLight 
        staticShader.setVec3("spotLight[0].position", glm::vec3(camera.Position));
        staticShader.setVec3("spotLight[0].direction", glm::vec3(camera.Front));
        staticShader.setVec3("spotLight[0].ambient", glm::vec3(0.0f));
        staticShader.setVec3("spotLight[0].diffuse", glm::vec3(0.0f));
        staticShader.setVec3("spotLight[0].specular", glm::vec3(0.0f));
        staticShader.setFloat("spotLight[0].cutOff", glm::cos(glm::radians(10.0f)));
        staticShader.setFloat("spotLight[0].outerCutOff", glm::cos(glm::radians(25.0f)));
        staticShader.setFloat("spotLight[0].constant", 1.0f);
        staticShader.setFloat("spotLight[0].linear", 0.0009f);
        staticShader.setFloat("spotLight[0].quadratic", 0.0005f);

        staticShader.setFloat("material_shininess", 32.0f);

        // Transformaciones de Vista y Proyección
        projectionOp = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10000.0f);
        viewOp = camera.GetViewMatrix();
        staticShader.setMat4("projection", projectionOp);
        staticShader.setMat4("view", viewOp);


        // Transformaciones de Vista y Proyección
        projectionOp = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10000.0f);
        viewOp = camera.GetViewMatrix();
        staticShader.setMat4("projection", projectionOp);
        staticShader.setMat4("view", viewOp);


        // ------------------------------------------------------------------
        // Dibujo del Escenario
        // ------------------------------------------------------------------
        // Piso 
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(1.0f));
        staticShader.setMat4("model", modelOp);
        Piso.Draw(staticShader);

        // Muro trasero (20 muros)
        for (int i = 0; i < 20; i++) {
            modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(-760.0f + (i * 80.0f), 0.0f, -720.0f));
            modelOp = glm::rotate(modelOp, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            modelOp = glm::scale(modelOp, glm::vec3(2.4f));
            staticShader.setMat4("model", modelOp);
            Muro.Draw(staticShader);
        }

        // Muro izquierdo (18 muros)
        for (int i = 0; i < 18; i++) {
            modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(-800.0f, 0.0f, -680.0f + (i * 80.0f)));
            modelOp = glm::rotate(modelOp, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            modelOp = glm::scale(modelOp, glm::vec3(2.4f));
            staticShader.setMat4("model", modelOp);
            Muro.Draw(staticShader);
        }

        // Muro derecho (18 muros)
        for (int i = 0; i < 18; i++) {
            modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(800.0f, 0.0f, -680.0f + (i * 80.0f)));
            modelOp = glm::rotate(modelOp, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            modelOp = glm::scale(modelOp, glm::vec3(2.4f));
            staticShader.setMat4("model", modelOp);
            Muro.Draw(staticShader);
        }

        // Muro frontal (9 muros)
        for (int i = 0; i < 9; i++) {
            modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(-760.0f + (i * 80.0f), 0.0f, 720.0f));
            modelOp = glm::scale(modelOp, glm::vec3(2.4f));
            staticShader.setMat4("model", modelOp);
            Muro.Draw(staticShader);
        }

        // Lado derecho (9 módulos, centros de 120 a 760)
        for (int i = 0; i < 9; i++) {
            modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(120.0f + (i * 80.0f), 0.0f, 720.0f));
            modelOp = glm::scale(modelOp, glm::vec3(2.4f));
            staticShader.setMat4("model", modelOp);
            Muro.Draw(staticShader);
        }

        // Entrada en forma de arco
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 720.0f));
        modelOp = glm::scale(modelOp, glm::vec3(6.0f));
        staticShader.setMat4("model", modelOp);
        Entrada.Draw(staticShader);

        // Tumba 1 (21 tumbas)
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(-520.0f, 0.0f, -480.0f));
        modelOp = glm::rotate(modelOp, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(2.2f));
        staticShader.setMat4("model", modelOp);
        Tumba.Draw(staticShader);

        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(-300.0f, 0.0f, -480.0f));
        modelOp = glm::rotate(modelOp, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(2.2f));
        staticShader.setMat4("model", modelOp);
        Tumba.Draw(staticShader);

        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(-120.0f, 0.0f, -480.0f));
        modelOp = glm::rotate(modelOp, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(2.2f));
        staticShader.setMat4("model", modelOp);
        Tumba.Draw(staticShader);

        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(120.0f, 0.0f, -480.0f));
        modelOp = glm::rotate(modelOp, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(2.2f));
        staticShader.setMat4("model", modelOp);
        Tumba.Draw(staticShader);

        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(300.0f, 0.0f, -480.0f));
        modelOp = glm::rotate(modelOp, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(2.2f));
        staticShader.setMat4("model", modelOp);
        Tumba.Draw(staticShader);

        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(520.0f, 0.0f, -480.0f));
        modelOp = glm::rotate(modelOp, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(2.2f));
        staticShader.setMat4("model", modelOp);
        Tumba.Draw(staticShader);

        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(-500.0f, 0.0f, -350.0f));
        modelOp = glm::rotate(modelOp, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(2.2f));
        staticShader.setMat4("model", modelOp);
        Tumba.Draw(staticShader);

        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(-280.0f, 0.0f, -350.0f));
        modelOp = glm::rotate(modelOp, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(2.2f));
        staticShader.setMat4("model", modelOp);
        Tumba.Draw(staticShader);

        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(280.0f, 0.0f, -350.0f));
        modelOp = glm::rotate(modelOp, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(2.2f));
        staticShader.setMat4("model", modelOp);
        Tumba.Draw(staticShader);

        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(420.0f, 0.0f, -350.0f));
        modelOp = glm::rotate(modelOp, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(2.2f));
        staticShader.setMat4("model", modelOp);
        Tumba.Draw(staticShader);

        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(520.0f, 0.0f, -350.0f));
        modelOp = glm::rotate(modelOp, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(2.2f));
        staticShader.setMat4("model", modelOp);
        Tumba.Draw(staticShader);

        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(-520.0f, 0.0f, -200.0f));
        modelOp = glm::rotate(modelOp, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(2.2f));
        staticShader.setMat4("model", modelOp);
        Tumba.Draw(staticShader);

        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(-300.0f, 0.0f, -200.0f));
        modelOp = glm::rotate(modelOp, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(2.2f));
        staticShader.setMat4("model", modelOp);
        Tumba.Draw(staticShader);

        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(-700.0f, 0.0f, -600.0f));
        modelOp = glm::rotate(modelOp, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(2.2f));
        staticShader.setMat4("model", modelOp);
        Tumba.Draw(staticShader);

        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(700.0f, 0.0f, -600.0f));
        modelOp = glm::rotate(modelOp, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(2.2f));
        staticShader.setMat4("model", modelOp);
        Tumba.Draw(staticShader);

        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(-700.0f, 0.0f, -100.0f));
        modelOp = glm::rotate(modelOp, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(2.2f));
        staticShader.setMat4("model", modelOp);
        Tumba.Draw(staticShader);

        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(700.0f, 0.0f, -100.0f));
        modelOp = glm::rotate(modelOp, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(2.2f));
        staticShader.setMat4("model", modelOp);
        Tumba.Draw(staticShader);

        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(-350.0f, 0.0f, 600.0f));
        modelOp = glm::rotate(modelOp, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(2.2f));
        staticShader.setMat4("model", modelOp);
        Tumba.Draw(staticShader);

        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(350.0f, 0.0f, 600.0f));
        modelOp = glm::rotate(modelOp, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(2.2f));
        staticShader.setMat4("model", modelOp);
        Tumba.Draw(staticShader);

        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(-650.0f, 0.0f, 400.0f));
        modelOp = glm::rotate(modelOp, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(2.2f));
        staticShader.setMat4("model", modelOp);
        Tumba.Draw(staticShader);

        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(650.0f, 0.0f, 400.0f));
        modelOp = glm::rotate(modelOp, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(2.2f));
        staticShader.setMat4("model", modelOp);
        Tumba.Draw(staticShader);

        // Tumba 2 (21 tumbas)
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(-120.0f, 0.0f, -200.0f));
        modelOp = glm::rotate(modelOp, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(4.0f));
        staticShader.setMat4("model", modelOp);
        Tumba2.Draw(staticShader);

        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(120.0f, 0.0f, -200.0f));
        modelOp = glm::rotate(modelOp, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(4.0f));
        staticShader.setMat4("model", modelOp);
        Tumba2.Draw(staticShader);

        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(350.0f, 0.0f, -200.0f));
        modelOp = glm::rotate(modelOp, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(4.0f));
        staticShader.setMat4("model", modelOp);
        Tumba2.Draw(staticShader);

        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(520.0f, 0.0f, -200.0f));
        modelOp = glm::rotate(modelOp, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(4.0f));
        staticShader.setMat4("model", modelOp);
        Tumba2.Draw(staticShader);

        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(-400.0f, 0.0f, -30.0f));
        modelOp = glm::rotate(modelOp, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(4.0f));
        staticShader.setMat4("model", modelOp);
        Tumba2.Draw(staticShader);

        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(-200.0f, 0.0f, -30.0f));
        modelOp = glm::rotate(modelOp, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(4.0f));
        staticShader.setMat4("model", modelOp);
        Tumba2.Draw(staticShader);

        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(200.0f, 0.0f, -30.0f));
        modelOp = glm::rotate(modelOp, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(4.0f));
        staticShader.setMat4("model", modelOp);
        Tumba2.Draw(staticShader);

        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(450.0f, 0.0f, -30.0f));
        modelOp = glm::rotate(modelOp, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(4.0f));
        staticShader.setMat4("model", modelOp);
        Tumba2.Draw(staticShader);

        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(-520.0f, 0.0f, 120.0f));
        modelOp = glm::rotate(modelOp, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(4.0f));
        staticShader.setMat4("model", modelOp);
        Tumba2.Draw(staticShader);

        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(-300.0f, 0.0f, 120.0f));
        modelOp = glm::rotate(modelOp, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(4.0f));
        staticShader.setMat4("model", modelOp);
        Tumba2.Draw(staticShader);

        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(-120.0f, 0.0f, 120.0f));
        modelOp = glm::rotate(modelOp, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(4.0f));
        staticShader.setMat4("model", modelOp);
        Tumba2.Draw(staticShader);

        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(120.0f, 0.0f, 120.0f));
        modelOp = glm::rotate(modelOp, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(4.0f));
        staticShader.setMat4("model", modelOp);
        Tumba2.Draw(staticShader);

        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(300.0f, 0.0f, 120.0f));
        modelOp = glm::rotate(modelOp, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(4.0f));
        staticShader.setMat4("model", modelOp);
        Tumba2.Draw(staticShader);

        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(-500.0f, 0.0f, -620.0f));
        modelOp = glm::rotate(modelOp, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(4.0f));
        staticShader.setMat4("model", modelOp);
        Tumba2.Draw(staticShader);

        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(500.0f, 0.0f, -620.0f));
        modelOp = glm::rotate(modelOp, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(4.0f));
        staticShader.setMat4("model", modelOp);
        Tumba2.Draw(staticShader);

        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(-700.0f, 0.0f, 150.0f));
        modelOp = glm::rotate(modelOp, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(4.0f));
        staticShader.setMat4("model", modelOp);
        Tumba2.Draw(staticShader);

        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(700.0f, 0.0f, 150.0f));
        modelOp = glm::rotate(modelOp, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(4.0f));
        staticShader.setMat4("model", modelOp);
        Tumba2.Draw(staticShader);

        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(-500.0f, 0.0f, 580.0f));
        modelOp = glm::rotate(modelOp, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(4.0f));
        staticShader.setMat4("model", modelOp);
        Tumba2.Draw(staticShader);

        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(500.0f, 0.0f, 580.0f));
        modelOp = glm::rotate(modelOp, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(4.0f));
        staticShader.setMat4("model", modelOp);
        Tumba2.Draw(staticShader);

        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(-150.0f, 0.0f, -620.0f));
        modelOp = glm::rotate(modelOp, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(4.0f));
        staticShader.setMat4("model", modelOp);
        Tumba2.Draw(staticShader);

        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(150.0f, 0.0f, -620.0f));
        modelOp = glm::rotate(modelOp, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(4.0f));
        staticShader.setMat4("model", modelOp);
        Tumba2.Draw(staticShader);

        // TUMBA 3 (21 Tumbas)
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(520.0f, 0.0f, 120.0f));
        modelOp = glm::rotate(modelOp, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(2.8f));
        staticShader.setMat4("model", modelOp);
        Tumba3.Draw(staticShader);

        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(-500.0f, 0.0f, 280.0f));
        modelOp = glm::rotate(modelOp, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(2.8f));
        staticShader.setMat4("model", modelOp);
        Tumba3.Draw(staticShader);

        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(-280.0f, 0.0f, 280.0f));
        modelOp = glm::rotate(modelOp, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(2.8f));
        staticShader.setMat4("model", modelOp);
        Tumba3.Draw(staticShader);

        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(-120.0f, 0.0f, 280.0f));
        modelOp = glm::rotate(modelOp, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(2.8f));
        staticShader.setMat4("model", modelOp);
        Tumba3.Draw(staticShader);

        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(120.0f, 0.0f, 280.0f));
        modelOp = glm::rotate(modelOp, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(2.8f));
        staticShader.setMat4("model", modelOp);
        Tumba3.Draw(staticShader);

        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(350.0f, 0.0f, 280.0f));
        modelOp = glm::rotate(modelOp, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(2.8f));
        staticShader.setMat4("model", modelOp);
        Tumba3.Draw(staticShader);

        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(520.0f, 0.0f, 280.0f));
        modelOp = glm::rotate(modelOp, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(2.8f));
        staticShader.setMat4("model", modelOp);
        Tumba3.Draw(staticShader);

        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(-520.0f, 0.0f, 440.0f));
        modelOp = glm::rotate(modelOp, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(2.8f));
        staticShader.setMat4("model", modelOp);
        Tumba3.Draw(staticShader);

        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(-300.0f, 0.0f, 440.0f));
        modelOp = glm::rotate(modelOp, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(2.8f));
        staticShader.setMat4("model", modelOp);
        Tumba3.Draw(staticShader);

        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(-120.0f, 0.0f, 440.0f));
        modelOp = glm::rotate(modelOp, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(2.8f));
        staticShader.setMat4("model", modelOp);
        Tumba3.Draw(staticShader);

        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(200.0f, 0.0f, 440.0f));
        modelOp = glm::rotate(modelOp, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(2.8f));
        staticShader.setMat4("model", modelOp);
        Tumba3.Draw(staticShader);

        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(350.0f, 0.0f, 440.0f));
        modelOp = glm::rotate(modelOp, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(2.8f));
        staticShader.setMat4("model", modelOp);
        Tumba3.Draw(staticShader);

        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(520.0f, 0.0f, 440.0f));
        modelOp = glm::rotate(modelOp, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(2.8f));
        staticShader.setMat4("model", modelOp);
        Tumba3.Draw(staticShader);

        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(-250.0f, 0.0f, -620.0f));
        modelOp = glm::rotate(modelOp, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(2.8f));
        staticShader.setMat4("model", modelOp);
        Tumba3.Draw(staticShader);

        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(250.0f, 0.0f, -620.0f));
        modelOp = glm::rotate(modelOp, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(2.8f));
        staticShader.setMat4("model", modelOp);
        Tumba3.Draw(staticShader);

        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(-700.0f, 0.0f, 300.0f));
        modelOp = glm::rotate(modelOp, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(2.8f));
        staticShader.setMat4("model", modelOp);
        Tumba3.Draw(staticShader);

        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(700.0f, 0.0f, 300.0f));
        modelOp = glm::rotate(modelOp, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(2.8f));
        staticShader.setMat4("model", modelOp);
        Tumba3.Draw(staticShader);

        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(-200.0f, 0.0f, 600.0f));
        modelOp = glm::rotate(modelOp, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(2.8f));
        staticShader.setMat4("model", modelOp);
        Tumba3.Draw(staticShader);

        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(200.0f, 0.0f, 600.0f));
        modelOp = glm::rotate(modelOp, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(2.8f));
        staticShader.setMat4("model", modelOp);
        Tumba3.Draw(staticShader);

        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(-650.0f, 0.0f, -300.0f));
        modelOp = glm::rotate(modelOp, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(2.8f));
        staticShader.setMat4("model", modelOp);
        Tumba3.Draw(staticShader);

        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(650.0f, 0.0f, -300.0f));
        modelOp = glm::rotate(modelOp, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(2.8f));
        staticShader.setMat4("model", modelOp);
        Tumba3.Draw(staticShader);
        
        // Skybox
        skyboxShader.use();
        glm::mat4 skyboxView = glm::rotate(viewOp, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        skybox.Draw(skyboxShader, skyboxView, projectionOp, camera);

        
        // Limitador de Framerate a 60 FPS
        deltaTime = SDL_GetTicks() - lastFrame;
        if (deltaTime < LOOP_TIME) {
            SDL_Delay((int)(LOOP_TIME - deltaTime));
            deltaTime = LOOP_TIME;
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

// ------------------------------------------------------------------
// Control de Teclado y Cámara
// ------------------------------------------------------------------
void my_input(GLFWwindow* window, int key, int scancode, int action, int mode) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Desplazamiento libre de la cámara sintética
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, (float)deltaTime * MovementSpeed);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, (float)deltaTime * MovementSpeed);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, (float)deltaTime * MovementSpeed);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, (float)deltaTime * MovementSpeed);

    // Posiciones rápidas de cámara requeridas por el proyecto (Teclas 1, 2 y 3)
    if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
        // 1) Puerta principal / Fachada exterior
        camera.Position = glm::vec3(0.0f, 10.0f, 120.0f);
    }
    if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
        // 2) Cerca de un elemento con animación
        camera.Position = glm::vec3(-30.0f, 8.0f, 20.0f);
    }
    if (key == GLFW_KEY_3 && action == GLFW_PRESS) {
        // 3) Cerca del elemento de gran detalle (Ofrenda central)
        camera.Position = glm::vec3(0.0f, 8.0f, 0.0f);
    }

    // Reproducir animación por KeyFrames (Tecla P)
    if (key == GLFW_KEY_P && action == GLFW_PRESS) {
        if (play == false && (FrameIndex > 1)) {
            std::cout << "Play animation" << std::endl;
            resetElements();
            interpolation();
            play = true;
            playIndex = 0;
            i_curr_steps = 0;
        }
        else {
            play = false;
            std::cout << "Not enough Key Frames" << std::endl;
        }
    }

    // Guardar un KeyFrame (Tecla L)
    if (key == GLFW_KEY_L && action == GLFW_PRESS) {
        if (FrameIndex < MAX_FRAMES) {
            saveFrame();
        }
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    double xoffset = xpos - lastX;
    double yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll(yoffset);
}
