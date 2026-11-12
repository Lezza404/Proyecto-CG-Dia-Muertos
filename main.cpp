/*---------------------------------------------------------*/
/* ----------- Proyecto Final: Panteón Oaxaca -------------*/
/*-----------------    Semestre 2027-1   ------------------*/
/*---------------------------------------------------------*/
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include <Windows.h>
#include <glad/glad.h>
#include <glfw3.h>                      
#include <stdlib.h>     
#include <glm/glm.hpp>                  
#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtc/type_ptr.hpp>
#include <time.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>                  
#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>
#include <shader_m.h>
#include <camera.h>
#include <modelAnim.h>
#include <model.h>
#include <Skybox.h>
#include <iostream>
#include <mmsystem.h>


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

// Cámara sintética (inicia sobre el piso)
Camera camera(glm::vec3(0.0f, 40.0f, 50.0f));
float MovementSpeed = 2.0f;
GLfloat lastX = SCR_WIDTH / 2.0f,
        lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// Control de tiempo (60 FPS)
const int FPS = 60;
const int LOOP_TIME = 1000 / FPS;
double deltaTime = 0.0f,
lastFrame = 0.0f;



// Variables para KeyFrames
float posX = 0.0f, 
      posY = 0.0f, 
      posZ = 0.0f;
float incX = 0.0f, 
      incY = 0.0f, 
      incZ = 0.0f;


// Para la animación del viento
float tiempo = 0.0f, 
      anguloViento = 0.0f;
      

// Para la animación del fantasma
float alturaFantasma = 0.0f,
      movFantasma_x = 0.0f,
      movFantasma_z = 900.0f,
      orientaFantasma = 90.0f,
      pausa = 0.0f;
int estadoFantasma = 1;       


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
    // Variable de tiempo para las animaciones
    tiempo += 0.03f;


    // Movimiento del viento con una oscilación de 20 grados
    anguloViento = sin(tiempo * 2.0f) * glm::radians(20.0f);


    // Cambio de altura del fantasma de 20 unidades
    alturaFantasma = sin(tiempo * 1.5f) * 20.0f;


    // Máquina de estados para el movimiento del fastama en el escenario
    // Estado 1 - Se mueve hacia la derecha 
    if (estadoFantasma == 1) {
        if (movFantasma_x < 900.0f) {
            movFantasma_x += 2.0f;
            orientaFantasma = 90.0f;
        }
        else {
            pausa += 0.01f;
            if (pausa > 1.5f) {
                pausa = 0.0f;
                estadoFantasma = 2;
            }
        }
    }


    // Estado 2 - Se mueve hacia atrás
    else if (estadoFantasma == 2) {
        orientaFantasma = 180.0f;

        if (movFantasma_z > -1000.0f) {
            movFantasma_z -= 2.0f; 
        }

        else {
            pausa += 0.01f;
            
            if (pausa > 1.5f) {
                pausa = 0.0f;
                estadoFantasma = 3;
            }
        }
    }


    // Estado 3 - Se mueve a la izquierda
    else if (estadoFantasma == 3) {
        orientaFantasma = 270.0f; 
        
        if (movFantasma_x > -1000.0f) {
            movFantasma_x -= 2.0f;
        }
        
        else {
            pausa += 0.01f;

            if (pausa > 1.5f) {
                pausa = 0.0f;
                estadoFantasma = 4;
            }
        }
    }


    // Estado 4 - Se mueve hacia enfrente 
    else if (estadoFantasma == 4) {
        orientaFantasma = 0.0f;

        if (movFantasma_z < 900.0f) {
            movFantasma_z += 2.0f;
        }

        else {
            pausa += 0.01f;
            
            if (pausa > 1.5f) {
                pausa = 0.0f;
                estadoFantasma = 5;
            }
        }
    }


    // Estado 5 - Regresa al inicio
    else if (estadoFantasma == 5) {
        orientaFantasma = 90.0f;

        if (movFantasma_x < 0.0f) {
            movFantasma_x += 2.0f;
        }

        else {
            estadoFantasma = 1;
        }
    }


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
    // Se inicializa GLFW
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

    // Captura del cursor 
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    // Se inicializa GLAD
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
    // Carga de los modelos del escenario
    // ------------------------------------------------------------------
    Model Piso("resources/objects/Piso/Piso.obj");
    Model Pasto("resources/objects/Pasto/Pasto.obj");
    Model Tumba("resources/objects/Tumba/Tumba.obj");
    Model Tumba2("resources/objects/Tumba2/Tumba2.obj");
    Model Tumba3("resources/objects/Tumba3/Tumba3.obj");
    Model Entrada("resources/objects/Entrada_arco/Entrada_arco.obj");
    Model Muro("resources/objects/Muro/Muro.obj");
    Model Arbol("resources/objects/tree/autumn tree001-obj.obj");
    Model Postes("resources/objects/Postes_Papel_Picado/Postes.obj");
    Model Papel_picado("resources/objects/Postes_Papel_Picado/Papel_picado.obj");
    Model ArbolCentral("resources/objects/tree_center/maple-tree.obj");
    Model Banco("resources/objects/Banco/park-bench.obj");
    Model Lampara("resources/objects/Lampara/spooky-lamp-post.obj");
    Model Calavera_azucar("resources/objects/Calavera_dulce/pastel-sugar-skull.obj");
    Model Pila_Huesos("resources/objects/Pila_Huesos/bone-pile.obj");
    Model Fantasma("resources/objects/Fantasma/Floating+Cape+1.obj");
    Model Ofrenda("resources/objects/Ofrenda/Ofrenda.obj");
    

    // Inicialización de KeyFrames en 0
    for (int i = 0; i < MAX_FRAMES; i++) {
        KeyFrame[i].posX = 0;
        KeyFrame[i].posY = 0;
        KeyFrame[i].posZ = 0;
    }

    glm::mat4 modelOp = glm::mat4(1.0f);
    glm::mat4 viewOp = glm::mat4(1.0f);
    glm::mat4 projectionOp = glm::mat4(1.0f);


    // ==================================================================
    // Configuración de audio (miniaudio)
    // ==================================================================
    ma_engine engine;
    ma_result result = ma_engine_init(NULL, &engine);
    if (result != MA_SUCCESS) {
        std::cout << "Error al iniciar el motor de audio miniaudio" << std::endl;
        return -1;
    }

    // Se carga y configura la música de fondo (mp3)
    ma_sound musicaFondo;

    // Se añade la canción para la ambientación del escenario
    ma_sound_init_from_file(&engine, "resources/music/Spanish Rose.mp3", 0, NULL, NULL, &musicaFondo);

    // La canción se mete dentro de un bucle para que nunca deje de sonar
    ma_sound_set_looping(&musicaFondo, MA_TRUE);

    // Para ajustar el volumen de la canción
    ma_sound_set_volume(&musicaFondo, 0.1f);

    // Se inicia la reproducción
    ma_sound_start(&musicaFondo);


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


        // Luz Direccional (efecto de anochecer azul)
        staticShader.setVec3("dirLight.direction", glm::vec3(0.0f, -1.0f, -0.5f));
        staticShader.setVec3("dirLight.ambient", glm::vec3(0.02f, 0.05f, 0.12f));
        staticShader.setVec3("dirLight.diffuse", glm::vec3(0.08f, 0.12f, 0.20f));
        staticShader.setVec3("dirLight.specular", glm::vec3(0.05f, 0.05f, 0.1f));


        // Veladora 1 (Lado izquierdo de la ofrenda)
        staticShader.setVec3("pointLight[0].position", glm::vec3(-105.0f, 40.0f, 5.0f));
        staticShader.setVec3("pointLight[0].ambient", glm::vec3(0.4f, 0.15f, 0.0f));
        staticShader.setVec3("pointLight[0].diffuse", glm::vec3(1.0f, 0.6f, 0.1f));
        staticShader.setVec3("pointLight[0].specular", glm::vec3(0.8f, 0.4f, 0.1f));
        staticShader.setFloat("pointLight[0].constant", 1.0f);
        staticShader.setFloat("pointLight[0].linear", 0.0014f);
        staticShader.setFloat("pointLight[0].quadratic", 0.000007f);


        // Veladora 2 (Lado derecho de la ofrenda)
        staticShader.setVec3("pointLight[1].position", glm::vec3(115.0f, 15.0f, 35.0f));
        staticShader.setVec3("pointLight[1].ambient", glm::vec3(0.4f, 0.15f, 0.0f));
        staticShader.setVec3("pointLight[1].diffuse", glm::vec3(1.0f, 0.6f, 0.1f));
        staticShader.setVec3("pointLight[1].specular", glm::vec3(0.8f, 0.4f, 0.1f));
        staticShader.setFloat("pointLight[1].constant", 1.0f);
        staticShader.setFloat("pointLight[1].linear", 0.0014f);
        staticShader.setFloat("pointLight[1].quadratic", 0.000007f);


        // Veladora 3 (Centro 3)
        staticShader.setVec3("pointLight[2].position", glm::vec3(250.0f, 25.0f, -250.0f));
        staticShader.setVec3("pointLight[2].ambient", glm::vec3(0.4f, 0.15f, 0.0f));
        staticShader.setVec3("pointLight[2].diffuse", glm::vec3(1.0f, 0.6f, 0.1f));
        staticShader.setVec3("pointLight[2].specular", glm::vec3(0.8f, 0.4f, 0.1f));
        staticShader.setFloat("pointLight[2].constant", 1.0f);
        staticShader.setFloat("pointLight[2].linear", 0.0014f);
        staticShader.setFloat("pointLight[2].quadratic", 0.000007f);


        // Veladora 4 (Arriba Izquierda)
        staticShader.setVec3("pointLight[3].position", glm::vec3(-1000.0f, 25.0f, -1000.0f));
        staticShader.setVec3("pointLight[3].ambient", glm::vec3(0.4f, 0.15f, 0.0f));
        staticShader.setVec3("pointLight[3].diffuse", glm::vec3(1.0f, 0.6f, 0.1f));
        staticShader.setVec3("pointLight[3].specular", glm::vec3(0.8f, 0.4f, 0.1f));
        staticShader.setFloat("pointLight[3].constant", 1.0f);
        staticShader.setFloat("pointLight[3].linear", 0.0014f);
        staticShader.setFloat("pointLight[3].quadratic", 0.000007f);


        // Veladora 5 (Arriba Centro)
        staticShader.setVec3("pointLight[4].position", glm::vec3(0.0f, 25.0f, -1200.0f));
        staticShader.setVec3("pointLight[4].ambient", glm::vec3(0.4f, 0.15f, 0.0f));
        staticShader.setVec3("pointLight[4].diffuse", glm::vec3(1.0f, 0.6f, 0.1f));
        staticShader.setVec3("pointLight[4].specular", glm::vec3(0.8f, 0.4f, 0.1f));
        staticShader.setFloat("pointLight[4].constant", 1.0f);
        staticShader.setFloat("pointLight[4].linear", 0.0014f);
        staticShader.setFloat("pointLight[4].quadratic", 0.000007f);


        // Veladora 6 (Arriba Derecha)
        staticShader.setVec3("pointLight[5].position", glm::vec3(1000.0f, 25.0f, -1000.0f));
        staticShader.setVec3("pointLight[5].ambient", glm::vec3(0.4f, 0.15f, 0.0f));
        staticShader.setVec3("pointLight[5].diffuse", glm::vec3(1.0f, 0.6f, 0.1f));
        staticShader.setVec3("pointLight[5].specular", glm::vec3(0.8f, 0.4f, 0.1f));
        staticShader.setFloat("pointLight[5].constant", 1.0f);
        staticShader.setFloat("pointLight[5].linear", 0.0014f);
        staticShader.setFloat("pointLight[5].quadratic", 0.000007f);


        // Veladora 7 (Medio Izquierda)
        staticShader.setVec3("pointLight[6].position", glm::vec3(-1100.0f, 25.0f, 0.0f));
        staticShader.setVec3("pointLight[6].ambient", glm::vec3(0.4f, 0.15f, 0.0f));
        staticShader.setVec3("pointLight[6].diffuse", glm::vec3(1.0f, 0.6f, 0.1f));
        staticShader.setVec3("pointLight[6].specular", glm::vec3(0.8f, 0.4f, 0.1f));
        staticShader.setFloat("pointLight[6].constant", 1.0f);
        staticShader.setFloat("pointLight[6].linear", 0.0014f);
        staticShader.setFloat("pointLight[6].quadratic", 0.000007f);


        // Veladora 8 (Medio Derecha)
        staticShader.setVec3("pointLight[7].position", glm::vec3(1100.0f, 25.0f, 0.0f));
        staticShader.setVec3("pointLight[7].ambient", glm::vec3(0.4f, 0.15f, 0.0f));
        staticShader.setVec3("pointLight[7].diffuse", glm::vec3(1.0f, 0.6f, 0.1f));
        staticShader.setVec3("pointLight[7].specular", glm::vec3(0.8f, 0.4f, 0.1f));
        staticShader.setFloat("pointLight[7].constant", 1.0f);
        staticShader.setFloat("pointLight[7].linear", 0.0014f);
        staticShader.setFloat("pointLight[7].quadratic", 0.000007f);


        // Veladora 9 (Abajo Izquierda)
        staticShader.setVec3("pointLight[8].position", glm::vec3(-1000.0f, 25.0f, 1000.0f));
        staticShader.setVec3("pointLight[8].ambient", glm::vec3(0.4f, 0.15f, 0.0f));
        staticShader.setVec3("pointLight[8].diffuse", glm::vec3(1.0f, 0.6f, 0.1f));
        staticShader.setVec3("pointLight[8].specular", glm::vec3(0.8f, 0.4f, 0.1f));
        staticShader.setFloat("pointLight[8].constant", 1.0f);
        staticShader.setFloat("pointLight[8].linear", 0.0014f);
        staticShader.setFloat("pointLight[8].quadratic", 0.000007f);


        // Veladora 10 (Abajo Centro)
        staticShader.setVec3("pointLight[9].position", glm::vec3(0.0f, 25.0f, 1200.0f));
        staticShader.setVec3("pointLight[9].ambient", glm::vec3(0.4f, 0.15f, 0.0f));
        staticShader.setVec3("pointLight[9].diffuse", glm::vec3(1.0f, 0.6f, 0.1f));
        staticShader.setVec3("pointLight[9].specular", glm::vec3(0.8f, 0.4f, 0.1f));
        staticShader.setFloat("pointLight[9].constant", 1.0f);
        staticShader.setFloat("pointLight[9].linear", 0.0014f);
        staticShader.setFloat("pointLight[9].quadratic", 0.000007f);


        // Veladora 11 (Abajo Derecha)
        staticShader.setVec3("pointLight[10].position", glm::vec3(1000.0f, 25.0f, 1000.0f));
        staticShader.setVec3("pointLight[10].ambient", glm::vec3(0.4f, 0.15f, 0.0f));
        staticShader.setVec3("pointLight[10].diffuse", glm::vec3(1.0f, 0.6f, 0.1f));
        staticShader.setVec3("pointLight[10].specular", glm::vec3(0.8f, 0.4f, 0.1f));
        staticShader.setFloat("pointLight[10].constant", 1.0f);
        staticShader.setFloat("pointLight[10].linear", 0.0014f);
        staticShader.setFloat("pointLight[10].quadratic", 0.000007f);


        // SpotLight 
        staticShader.setVec3("spotLight[0].position", glm::vec3(camera.Position));
        staticShader.setVec3("spotLight[0].direction", glm::vec3(camera.Front));
        staticShader.setVec3("spotLight[0].ambient", glm::vec3(0.0f));
        staticShader.setVec3("spotLight[0].diffuse", glm::vec3(0.2f, 0.2f, 0.2f));
        staticShader.setVec3("spotLight[0].specular", glm::vec3(0.2f, 0.2f, 0.2f));
        staticShader.setFloat("spotLight[0].cutOff", glm::cos(glm::radians(12.0f)));
        staticShader.setFloat("spotLight[0].outerCutOff", glm::cos(glm::radians(20.0f)));
        staticShader.setFloat("spotLight[0].constant", 1.0f);
        staticShader.setFloat("spotLight[0].linear", 0.009f);
        staticShader.setFloat("spotLight[0].quadratic", 0.003f);


        staticShader.setFloat("material_shininess", 32.0f);


        // Transformaciones de Vista y Proyección
        projectionOp = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10000.0f);
        viewOp = camera.GetViewMatrix();
        staticShader.setMat4("projection", projectionOp);
        staticShader.setMat4("view", viewOp);


        // ------------------------------------------------------------------
        // Dibujo del Escenario
        // ------------------------------------------------------------------
        // Piso de tierra 
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(1.6f));
        staticShader.setMat4("model", modelOp);
        Piso.Draw(staticShader);


        // Piso de pasto 
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -5.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(2.1f));
        staticShader.setMat4("model", modelOp);
        Pasto.Draw(staticShader);


        // Árbol central del panteón
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f)); 
        // modelOp = glm::rotate(modelOp, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f)); 
        modelOp = glm::scale(modelOp, glm::vec3(200.0f));
        staticShader.setMat4("model", modelOp);
        ArbolCentral.Draw(staticShader);


        // Ofrenda
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(120.0f, 50.0f, 200.0f));
        modelOp = glm::scale(modelOp, glm::vec3(15.0f));
        staticShader.setMat4("model", modelOp);
        Ofrenda.Draw(staticShader);


        // Fantasma
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(movFantasma_x, 15.0f + alturaFantasma, movFantasma_z));
        modelOp = glm::rotate(modelOp, glm::radians(orientaFantasma), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(120.0f));
        staticShader.setMat4("model", modelOp);
        Fantasma.Draw(staticShader);


        // Bancas interiores (Pared frontal / Entrada)
        // 3 bancas del lado izquierdo y 3 del lado derecho
        glm::vec3 posBancas[6] = {
            // Lado izquierdo (de adentro viendo hacia afuera, X negativo)
            glm::vec3(-400.0f, 0.0f, 1500.0f),
            glm::vec3(-700.0f, 0.0f, 1500.0f),
            glm::vec3(-1000.0f, 0.0f, 1500.0f),

            // Lado derecho (X positivo)
            glm::vec3(400.0f, 0.0f, 1500.0f),
            glm::vec3(700.0f, 0.0f, 1500.0f),
            glm::vec3(1000.0f, 0.0f, 1500.0f)
        };


        for (int i = 0; i < 6; i++) {
            modelOp = glm::translate(glm::mat4(1.0f), posBancas[i]);
            modelOp = glm::rotate(modelOp, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            modelOp = glm::scale(modelOp, glm::vec3(85.0f));
            staticShader.setMat4("model", modelOp);
            Banco.Draw(staticShader);
        }


        // Muro trasero 
        for (int i = 0; i < 33; i++) {
            modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(-1600.0f + (i * 100.0f), 0.0f, -1600.0f));
            modelOp = glm::rotate(modelOp, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            modelOp = glm::scale(modelOp, glm::vec3(3.0f));
            staticShader.setMat4("model", modelOp);
            Muro.Draw(staticShader);
        }


        // Muro lateral izquierdo 
        for (int i = 0; i < 31; i++) {
            modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(-1600.0f, 0.0f, -1500.0f + (i * 100.0f)));
            modelOp = glm::rotate(modelOp, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            modelOp = glm::scale(modelOp, glm::vec3(3.0f));
            staticShader.setMat4("model", modelOp);
            Muro.Draw(staticShader);
        }


        // Muro lateral derecho 
        for (int i = 0; i < 31; i++) {
            modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(1600.0f, 0.0f, -1500.0f + (i * 100.0f)));
            modelOp = glm::rotate(modelOp, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            modelOp = glm::scale(modelOp, glm::vec3(3.0f));
            staticShader.setMat4("model", modelOp);
            Muro.Draw(staticShader);
        }


        // Lado izquierdo 
        for (int i = 0; i < 15; i++) {
            modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(-1600.0f + (i * 100.0f), 0.0f, 1600.0f));
            modelOp = glm::scale(modelOp, glm::vec3(3.0f));
            staticShader.setMat4("model", modelOp);
            Muro.Draw(staticShader);
        }


        // Lado derecho 
        for (int i = 0; i < 15; i++) {
            modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(200.0f + (i * 100.0f), 0.0f, 1600.0f));
            modelOp = glm::scale(modelOp, glm::vec3(3.0f));
            staticShader.setMat4("model", modelOp);
            Muro.Draw(staticShader);
        }


        // Entrada en forma de arco
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 1600.0f));
        modelOp = glm::scale(modelOp, glm::vec3(12.0f));
        staticShader.setMat4("model", modelOp);
        Entrada.Draw(staticShader);


        // Lampara 1
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(-150.0f, 0.0f, 1500.0f));
        modelOp = glm::scale(modelOp, glm::vec3(50.0f)); 
        staticShader.setMat4("model", modelOp);
        Lampara.Draw(staticShader);


        // Lampara 2
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(150.0f, 0.0f, 1500.0f));
        modelOp = glm::rotate(modelOp, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(50.0f)); 
        staticShader.setMat4("model", modelOp);
        Lampara.Draw(staticShader);


        // Calavera de azucar 1
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(-25.0f, 0.0f, 300.0f));
        modelOp = glm::scale(modelOp, glm::vec3(50.0f)); 
        staticShader.setMat4("model", modelOp);
        Calavera_azucar.Draw(staticShader);


        // Calavera de azucar 2
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(25.0f, 0.0f, 300.0f));
        modelOp = glm::scale(modelOp, glm::vec3(50.0f));
        staticShader.setMat4("model", modelOp);
        Calavera_azucar.Draw(staticShader);


        //Pila de Huesos
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(-250.0f, 0.0f, 200.0f));
        modelOp = glm::scale(modelOp, glm::vec3(50.0f));
        staticShader.setMat4("model", modelOp);
        Pila_Huesos.Draw(staticShader);

        
        // Árboles traseros
        for (int i = 0; i < 9; i++) {
            modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(-1600.0f + (i * 400.0f), 0.0f, -1900.0f));
            modelOp = glm::rotate(modelOp, glm::radians(i * 37.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            modelOp = glm::scale(modelOp, glm::vec3(0.6f));
            staticShader.setMat4("model", modelOp);
            Arbol.Draw(staticShader);
        }


        // Árboles frontales
        for (int i = 0; i < 9; i++) {
            if (i == 4) continue; 
            modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(-1600.0f + (i * 400.0f), 0.0f, 1900.0f));
            modelOp = glm::rotate(modelOp, glm::radians(i * 61.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            modelOp = glm::scale(modelOp, glm::vec3(0.6f));
            staticShader.setMat4("model", modelOp);
            Arbol.Draw(staticShader);
        }


        // Árboles izquierdos
        for (int i = 0; i < 9; i++) {
            modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(-1900.0f, 0.0f, -1600.0f + (i * 400.0f)));
            modelOp = glm::rotate(modelOp, glm::radians(i * 43.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            modelOp = glm::scale(modelOp, glm::vec3(0.6f));
            staticShader.setMat4("model", modelOp);
            Arbol.Draw(staticShader);
        }


        // Árboles derechos
        for (int i = 0; i < 9; i++) {
            modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(1900.0f, 0.0f, -1600.0f + (i * 400.0f)));
            modelOp = glm::rotate(modelOp, glm::radians(i * 19.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            modelOp = glm::scale(modelOp, glm::vec3(0.6f));
            staticShader.setMat4("model", modelOp);
            Arbol.Draw(staticShader);
        }


        // Postes
        for (int i = -4; i <= 4; i++) {
            if (i == 0) continue; 
            float posZ = i * 350.0f;
            modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 5.0f, posZ));
            modelOp = glm::scale(modelOp, glm::vec3(15.0f));
            staticShader.setMat4("model", modelOp);
            Postes.Draw(staticShader);
        }


        // Papeles picados
        for (int i = -4; i <= 4; i++) {
            if (i == 0) continue;
            float posZ = i * 350.0f;
            modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 700.0f, posZ));
            modelOp = glm::rotate(modelOp, anguloViento, glm::vec3(1.0f, 0.0f, 0.0f));
            modelOp = glm::scale(modelOp, glm::vec3(15.0f));
            staticShader.setMat4("model", modelOp);
            Papel_picado.Draw(staticShader);
        }

        
        // Tumba 1 con diseño Tumba
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(-1200.0f, 0.0f, -1350.0f));
        modelOp = glm::rotate(modelOp, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(3.5f));
        staticShader.setMat4("model", modelOp);
        Tumba.Draw(staticShader);


        // Tumba 2 con diseño Tumba
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(-700.0f, 0.0f, -1300.0f));
        modelOp = glm::rotate(modelOp, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(3.5f));
        staticShader.setMat4("model", modelOp);
        Tumba.Draw(staticShader);


        // Tumba 3 con diseño Tumba
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(-200.0f, 0.0f, -1400.0f));
        modelOp = glm::rotate(modelOp, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(3.5f));
        staticShader.setMat4("model", modelOp);
        Tumba.Draw(staticShader);


        // Tumba 4 con diseño Tumba
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(400.0f, 0.0f, -1300.0f));
        modelOp = glm::rotate(modelOp, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(3.5f));
        staticShader.setMat4("model", modelOp);
        Tumba.Draw(staticShader);


        // Tumba 5 con diseño Tumba
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(900.0f, 0.0f, -1350.0f));
        modelOp = glm::rotate(modelOp, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(3.5f));
        staticShader.setMat4("model", modelOp);
        Tumba.Draw(staticShader);


        // Tumba 6 con diseño Tumba
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(-1400.0f, 0.0f, -850.0f));
        modelOp = glm::rotate(modelOp, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(3.5f));
        staticShader.setMat4("model", modelOp);
        Tumba.Draw(staticShader);


        // Tumba 7 con diseño Tumba
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(-850.0f, 0.0f, -850.0f));
        modelOp = glm::rotate(modelOp, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(3.5f));
        staticShader.setMat4("model", modelOp);
        Tumba.Draw(staticShader);


        // Tumba 8 con diseño Tumba
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(600.0f, 0.0f, -850.0f));
        modelOp = glm::rotate(modelOp, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(3.5f));
        staticShader.setMat4("model", modelOp);
        Tumba.Draw(staticShader);


        // Tumba 9 con diseño Tumba
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(1150.0f, 0.0f, -900.0f));
        modelOp = glm::rotate(modelOp, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(3.5f));
        staticShader.setMat4("model", modelOp);
        Tumba.Draw(staticShader);


        // Tumba 10 con diseño Tumba
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(-1100.0f, 0.0f, -400.0f));
        modelOp = glm::rotate(modelOp, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(3.5f));
        staticShader.setMat4("model", modelOp);
        Tumba.Draw(staticShader);


        // Tumba 11 con diseño Tumba
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(-550.0f, 0.0f, -450.0f));
        modelOp = glm::rotate(modelOp, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(3.5f));
        staticShader.setMat4("model", modelOp);
        Tumba.Draw(staticShader);


        // Tumba 12 con diseño Tumba
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(250.0f, 0.0f, -500.0f));
        modelOp = glm::rotate(modelOp, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(3.5f));
        staticShader.setMat4("model", modelOp);
        Tumba.Draw(staticShader);


        // Tumba 13 con diseño Tumba
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(800.0f, 0.0f, -400.0f));
        modelOp = glm::rotate(modelOp, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(3.5f));
        staticShader.setMat4("model", modelOp);
        Tumba.Draw(staticShader);


        // Tumba 14 con diseño Tumba
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(-1350.0f, 0.0f, 100.0f));
        modelOp = glm::rotate(modelOp, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(3.5f));
        staticShader.setMat4("model", modelOp);
        Tumba.Draw(staticShader);


        // Tumba 15 con diseño Tumba
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(-750.0f, 0.0f, 50.0f));
        modelOp = glm::rotate(modelOp, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(3.5f));
        staticShader.setMat4("model", modelOp);
        Tumba.Draw(staticShader);


        // Tumba 16 con diseño Tumba
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(450.0f, 0.0f, 100.0f));
        modelOp = glm::rotate(modelOp, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(3.5f));
        staticShader.setMat4("model", modelOp);
        Tumba.Draw(staticShader);


        // Tumba 17 con diseño Tumba
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(1300.0f, 0.0f, 50.0f));
        modelOp = glm::rotate(modelOp, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(3.5f));
        staticShader.setMat4("model", modelOp);
        Tumba.Draw(staticShader);


        // Tumba 18 con diseño Tumba
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(-1000.0f, 0.0f, 600.0f));
        modelOp = glm::rotate(modelOp, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(3.5f));
        staticShader.setMat4("model", modelOp);
        Tumba.Draw(staticShader);


        // Tumba 19 con diseño Tumba
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(-450.0f, 0.0f, 550.0f));
        modelOp = glm::rotate(modelOp, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(3.5f));
        staticShader.setMat4("model", modelOp);
        Tumba.Draw(staticShader);


        // Tumba 20 con diseño Tumba
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(550.0f, 0.0f, 600.0f));
        modelOp = glm::rotate(modelOp, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(3.5f));
        staticShader.setMat4("model", modelOp);
        Tumba.Draw(staticShader);


        // Tumba 21 con diseño Tumba
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(1050.0f, 0.0f, 550.0f));
        modelOp = glm::rotate(modelOp, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(3.5f));
        staticShader.setMat4("model", modelOp);
        Tumba.Draw(staticShader);


        // Tumba 22 con diseño Tumba
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(-1300.0f, 0.0f, 1100.0f));
        modelOp = glm::rotate(modelOp, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(3.5f));
        staticShader.setMat4("model", modelOp);
        Tumba.Draw(staticShader);


        // Tumba 23 con diseño Tumba
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(-700.0f, 0.0f, 1050.0f));
        modelOp = glm::rotate(modelOp, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(3.5f));
        staticShader.setMat4("model", modelOp);
        Tumba.Draw(staticShader);


        // Tumba 24 con diseño Tumba
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(750.0f, 0.0f, 1100.0f));
        modelOp = glm::rotate(modelOp, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(3.5f));
        staticShader.setMat4("model", modelOp);
        Tumba.Draw(staticShader);


        // Tumba 25 con diseño Tumba
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(1250.0f, 0.0f, 1050.0f));
        modelOp = glm::rotate(modelOp, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(3.5f));
        staticShader.setMat4("model", modelOp);
        Tumba.Draw(staticShader);


        // Tumba 1 con diseño Tumba2
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(-950.0f, 0.0f, -1300.0f));
        modelOp = glm::rotate(modelOp, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(8.0f));
        staticShader.setMat4("model", modelOp);
        Tumba2.Draw(staticShader);


        // Tumba 2 con diseño Tumba2
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(-450.0f, 0.0f, -1250.0f));
        modelOp = glm::rotate(modelOp, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(8.0f));
        staticShader.setMat4("model", modelOp);
        Tumba2.Draw(staticShader);


        // Tumba 3 con diseño Tumba2
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(150.0f, 0.0f, -1300.0f));
        modelOp = glm::rotate(modelOp, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(8.0f));
        staticShader.setMat4("model", modelOp);
        Tumba2.Draw(staticShader);


        // Tumba 4 con diseño Tumba2
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(650.0f, 0.0f, -1250.0f));
        modelOp = glm::rotate(modelOp, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(8.0f));
        staticShader.setMat4("model", modelOp);
        Tumba2.Draw(staticShader);


        // Tumba 5 con diseño Tumba2
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(1250.0f, 0.0f, -1350.0f));
        modelOp = glm::rotate(modelOp, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(8.0f));
        staticShader.setMat4("model", modelOp);
        Tumba2.Draw(staticShader);


        // Tumba 6 con diseño Tumba2
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(-1150.0f, 0.0f, -800.0f));
        modelOp = glm::rotate(modelOp, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(8.0f));
        staticShader.setMat4("model", modelOp);
        Tumba2.Draw(staticShader);


        // Tumba 7 con diseño Tumba2
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(-550.0f, 0.0f, -850.0f));
        modelOp = glm::rotate(modelOp, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(8.0f));
        staticShader.setMat4("model", modelOp);
        Tumba2.Draw(staticShader);


        // Tumba 8 con diseño Tumba2
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(350.0f, 0.0f, -800.0f));
        modelOp = glm::rotate(modelOp, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(8.0f));
        staticShader.setMat4("model", modelOp);
        Tumba2.Draw(staticShader);


        // Tumba 9 con diseño Tumba2
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(900.0f, 0.0f, -850.0f));
        modelOp = glm::rotate(modelOp, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(8.0f));
        staticShader.setMat4("model", modelOp);
        Tumba2.Draw(staticShader);


        // Tumba 10 con diseño Tumba2
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(1450.0f, 0.0f, -800.0f));
        modelOp = glm::rotate(modelOp, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(8.0f));
        staticShader.setMat4("model", modelOp);
        Tumba2.Draw(staticShader);


        // Tumba 11 con diseño Tumba2
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(-1350.0f, 0.0f, -350.0f));
        modelOp = glm::rotate(modelOp, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(8.0f));
        staticShader.setMat4("model", modelOp);
        Tumba2.Draw(staticShader);


        // Tumba 12 con diseño Tumba2
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(-850.0f, 0.0f, -400.0f));
        modelOp = glm::rotate(modelOp, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(8.0f));
        staticShader.setMat4("model", modelOp);
        Tumba2.Draw(staticShader);


        // Tumba 13 con diseño Tumba2
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(550.0f, 0.0f, -350.0f));
        modelOp = glm::rotate(modelOp, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(8.0f));
        staticShader.setMat4("model", modelOp);
        Tumba2.Draw(staticShader);


        // Tumba 14 con diseño Tumba2
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(1100.0f, 0.0f, -400.0f));
        modelOp = glm::rotate(modelOp, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(8.0f));
        staticShader.setMat4("model", modelOp);
        Tumba2.Draw(staticShader);


        // Tumba 15 con diseño Tumba2
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(-1050.0f, 0.0f, 150.0f));
        modelOp = glm::rotate(modelOp, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(8.0f));
        staticShader.setMat4("model", modelOp);
        Tumba2.Draw(staticShader);


        // Tumba 16 con diseño Tumba2
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(-450.0f, 0.0f, 100.0f));
        modelOp = glm::rotate(modelOp, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(8.0f));
        staticShader.setMat4("model", modelOp);
        Tumba2.Draw(staticShader);


        // Tumba 17 con diseño Tumba2
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(750.0f, 0.0f, 150.0f));
        modelOp = glm::rotate(modelOp, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(8.0f));
        staticShader.setMat4("model", modelOp);
        Tumba2.Draw(staticShader);


        // Tumba 18 con diseño Tumba2
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(1050.0f, 0.0f, 100.0f));
        modelOp = glm::rotate(modelOp, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(8.0f));
        staticShader.setMat4("model", modelOp);
        Tumba2.Draw(staticShader);


        // Tumba 19 con diseño Tumba2
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(-1350.0f, 0.0f, 650.0f));
        modelOp = glm::rotate(modelOp, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(8.0f));
        staticShader.setMat4("model", modelOp);
        Tumba2.Draw(staticShader);


        // Tumba 20 con diseño Tumba2
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(-750.0f, 0.0f, 600.0f));
        modelOp = glm::rotate(modelOp, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(8.0f));
        staticShader.setMat4("model", modelOp);
        Tumba2.Draw(staticShader);


        // Tumba 21 con diseño Tumba2
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(850.0f, 0.0f, 650.0f));
        modelOp = glm::rotate(modelOp, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(8.0f));
        staticShader.setMat4("model", modelOp);
        Tumba2.Draw(staticShader);


        // Tumba 22 con diseño Tumba2
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(1350.0f, 0.0f, 600.0f));
        modelOp = glm::rotate(modelOp, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(8.0f));
        staticShader.setMat4("model", modelOp);
        Tumba2.Draw(staticShader);


        // Tumba 23 con diseño Tumba2
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(-1050.0f, 0.0f, 1150.0f));
        modelOp = glm::rotate(modelOp, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(8.0f));
        staticShader.setMat4("model", modelOp);
        Tumba2.Draw(staticShader);


        // Tumba 24 con diseño Tumba2
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(-450.0f, 0.0f, 1100.0f));
        modelOp = glm::rotate(modelOp, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(8.0f));
        staticShader.setMat4("model", modelOp);
        Tumba2.Draw(staticShader);


        // Tumba 25 con diseño Tumba2
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(1000.0f, 0.0f, 1150.0f));
        modelOp = glm::rotate(modelOp, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(8.0f));
        staticShader.setMat4("model", modelOp);
        Tumba2.Draw(staticShader);


        // Tumba 1 con diseño Tumba3
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(-1450.0f, 0.0f, -1250.0f));
        modelOp = glm::rotate(modelOp, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(5.5f));
        staticShader.setMat4("model", modelOp);
        Tumba3.Draw(staticShader);


        // Tumba 2 con diseño Tumba3
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(-1100.0f, 0.0f, -1050.0f));
        modelOp = glm::rotate(modelOp, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(5.5f));
        staticShader.setMat4("model", modelOp);
        Tumba3.Draw(staticShader);


        // Tumba 3 con diseño Tumba3
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(-300.0f, 0.0f, -1000.0f));
        modelOp = glm::rotate(modelOp, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(5.5f));
        staticShader.setMat4("model", modelOp);
        Tumba3.Draw(staticShader);


        // Tumba 4 con diseño Tumba3
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(300.0f, 0.0f, -1050.0f));
        modelOp = glm::rotate(modelOp, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(5.5f));
        staticShader.setMat4("model", modelOp);
        Tumba3.Draw(staticShader);


        // Tumba 5 con diseño Tumba3
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(1050.0f, 0.0f, -1100.0f));
        modelOp = glm::rotate(modelOp, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(5.5f));
        staticShader.setMat4("model", modelOp);
        Tumba3.Draw(staticShader);


        // Tumba 6 con diseño Tumba3
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(-1350.0f, 0.0f, -600.0f));
        modelOp = glm::rotate(modelOp, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(5.5f));
        staticShader.setMat4("model", modelOp);
        Tumba3.Draw(staticShader);


        // Tumba 7 con diseño Tumba3
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(-750.0f, 0.0f, -650.0f));
        modelOp = glm::rotate(modelOp, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(5.5f));
        staticShader.setMat4("model", modelOp);
        Tumba3.Draw(staticShader);


        // Tumba 8 con diseño Tumba3
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(-150.0f, 0.0f, -600.0f));
        modelOp = glm::rotate(modelOp, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(5.5f));
        staticShader.setMat4("model", modelOp);
        Tumba3.Draw(staticShader);


        // Tumba 9 con diseño Tumba3
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(750.0f, 0.0f, -650.0f));
        modelOp = glm::rotate(modelOp, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(5.5f));
        staticShader.setMat4("model", modelOp);
        Tumba3.Draw(staticShader);


        // Tumba 10 con diseño Tumba3
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(1300.0f, 0.0f, -600.0f));
        modelOp = glm::rotate(modelOp, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(5.5f));
        staticShader.setMat4("model", modelOp);
        Tumba3.Draw(staticShader);


        // Tumba 11 con diseño Tumba3
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(-1150.0f, 0.0f, -150.0f));
        modelOp = glm::rotate(modelOp, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(5.5f));
        staticShader.setMat4("model", modelOp);
        Tumba3.Draw(staticShader);


        // Tumba 12 con diseño Tumba3
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(-650.0f, 0.0f, -200.0f));
        modelOp = glm::rotate(modelOp, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(5.5f));
        staticShader.setMat4("model", modelOp);
        Tumba3.Draw(staticShader);


        // Tumba 13 con diseño Tumba3
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(450.0f, 0.0f, -150.0f));
        modelOp = glm::rotate(modelOp, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(5.5f));
        staticShader.setMat4("model", modelOp);
        Tumba3.Draw(staticShader);


        // Tumba 14 con diseño Tumba3
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(1250.0f, 0.0f, -200.0f));
        modelOp = glm::rotate(modelOp, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(5.5f));
        staticShader.setMat4("model", modelOp);
        Tumba3.Draw(staticShader);


        // Tumba 15 con diseño Tumba3
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(-1250.0f, 0.0f, 350.0f));
        modelOp = glm::rotate(modelOp, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(5.5f));
        staticShader.setMat4("model", modelOp);
        Tumba3.Draw(staticShader);


        // Tumba 16 con diseño Tumba3
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(-850.0f, 0.0f, 300.0f));
        modelOp = glm::rotate(modelOp, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(5.5f));
        staticShader.setMat4("model", modelOp);
        Tumba3.Draw(staticShader);


        // Tumba 17 con diseño Tumba3
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(950.0f, 0.0f, 300.0f));
        modelOp = glm::rotate(modelOp, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(5.5f));
        staticShader.setMat4("model", modelOp);
        Tumba3.Draw(staticShader);


        // Tumba 18 con diseño Tumba3
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(1450.0f, 0.0f, 350.0f));
        modelOp = glm::rotate(modelOp, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(5.5f));
        staticShader.setMat4("model", modelOp);
        Tumba3.Draw(staticShader);


        // Tumba 19 con diseño Tumba3
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(-1150.0f, 0.0f, 850.0f));
        modelOp = glm::rotate(modelOp, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(5.5f));
        staticShader.setMat4("model", modelOp);
        Tumba3.Draw(staticShader);


        // Tumba 20 con diseño Tumba3
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(-600.0f, 0.0f, 800.0f));
        modelOp = glm::rotate(modelOp, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(5.5f));
        staticShader.setMat4("model", modelOp);
        Tumba3.Draw(staticShader);


        // Tumba 21 con diseño Tumba3
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(600.0f, 0.0f, 850.0f));
        modelOp = glm::rotate(modelOp, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(5.5f));
        staticShader.setMat4("model", modelOp);
        Tumba3.Draw(staticShader);


        // Tumba 22 con diseño Tumba3
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(1150.0f, 0.0f, 800.0f));
        modelOp = glm::rotate(modelOp, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(5.5f));
        staticShader.setMat4("model", modelOp);
        Tumba3.Draw(staticShader);


        // Tumba 23 con diseño Tumba3
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(-800.0f, 0.0f, 1300.0f));
        modelOp = glm::rotate(modelOp, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(5.5f));
        staticShader.setMat4("model", modelOp);
        Tumba3.Draw(staticShader);


        // Tumba 24 con diseño Tumba3
        modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(850.0f, 0.0f, 1300.0f));
        modelOp = glm::rotate(modelOp, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelOp = glm::scale(modelOp, glm::vec3(5.5f));
        staticShader.setMat4("model", modelOp);
        Tumba3.Draw(staticShader);
        

        // Skybox
        skyboxShader.use();
        glm::mat4 skyboxView = glm::rotate(viewOp, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        skybox.Draw(skyboxShader, skyboxView, projectionOp, camera);

        
        // Limitador de framerate a 60 FPS
        deltaTime = SDL_GetTicks() - lastFrame;
        if (deltaTime < LOOP_TIME) {
            SDL_Delay((int)(LOOP_TIME - deltaTime));
            deltaTime = LOOP_TIME;
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Para limpiar la memoria de audio antes de cerrar la ventana
    ma_sound_uninit(&musicaFondo);
    ma_engine_uninit(&engine);

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

    // Posiciones rápidas de cámara (teclas 1, 2 y 3)
    if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
        
        // 1) Puerta principal / Fachada exterior
        camera.Position = glm::vec3(0.0f, 50.0f, 1880.0f);
        camera.Yaw = -90.0f; //Orienta hacia el eje -Z (hacia el arco)
        camera.Pitch = -2.0f; //Ligera inclinación hacia la entrada
        camera.Front = glm::vec3(0.0f, 0.0f, -1.0f); //Vector de vista hacia el interior
    }
    if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
        
        // 2) Cerca de un elemento con animación
        camera.Position = glm::vec3(-30.0f, 25.0f, 20.0f);
    }
    if (key == GLFW_KEY_3 && action == GLFW_PRESS) {
        
        // 3) Cerca del elemento de gran detalle (Ofrenda central)
        camera.Position = glm::vec3(0.0f, 35.0f, 60.0f);
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
