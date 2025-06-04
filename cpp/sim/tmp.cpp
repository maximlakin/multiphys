
#define GLM_ENABLE_EXPERIMENTAL

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <cmath>
#include <unistd.h>

#include <SDL.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "glm/gtx/rotate_vector.hpp"
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <shader.h>
#include <camera.h>

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

bool gQuit = false;

Camera camera(glm::vec3(1.0f, 4.0f, 7.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

glm::vec3 lightPos(0.0f, 3.0f, -1.0f);

float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

glm::vec2 oldMousePos;

SDL_Window* gGraphicsApplicationWindow = nullptr;
SDL_GLContext gOpenGLContext = nullptr;

void Cleanup(){
    SDL_DestroyWindow(gGraphicsApplicationWindow);
    gGraphicsApplicationWindow = nullptr;
    SDL_Quit();
}

void MouseLook(int mouseX, int mouseY){
    float xpos = static_cast<float>(mouseX);
    float ypos = static_cast<float>(mouseY);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);

}

void processKeyboardInput() {
    SDL_Event event; // create every time?

    static int mouseX = SCR_WIDTH/2;
    static int mouseY = SCR_HEIGHT/2;

    while(SDL_PollEvent(&event)!=0){
        if (event.type == SDL_QUIT){
            gQuit = true;
        }
        else if(event.type == SDL_MOUSEMOTION){
            mouseX += event.motion.xrel;
            mouseY += event.motion.yrel;
            MouseLook(mouseX, mouseY);
        }
        else if (event.type == SDL_MOUSEWHEEL) {
            int yoffset = event.wheel.y; 
            camera.ProcessMouseScroll(static_cast<float>(yoffset));
        }
    }

    const Uint8 *state = SDL_GetKeyboardState(NULL);
    if (state[SDL_SCANCODE_W]){
        camera.ProcessKeyboard(FORWARD, deltaTime);
    }else if (state[SDL_SCANCODE_S]){
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    }
    if (state[SDL_SCANCODE_A]){
        camera.ProcessKeyboard(LEFT, deltaTime);
    }else if (state[SDL_SCANCODE_D]){
        camera.ProcessKeyboard(RIGHT, deltaTime);
    }
}

const double sigma = 10.0;
const double rho = 28.0;
const double beta = 8.0 / 3.0;

const double dt = 0.01;
const int steps = 10000;

struct Point3D {
    float x, y, z;
};

std::vector<Point3D> computeLorenz(double x0, double y0, double z0) {
    std::vector<Point3D> trajectory;
    double x = x0, y = y0, z = z0;

    for (int i = 0; i < steps; ++i) {
        // Euler method for simplicity (you could use RK4 for better accuracy)
        double dx = sigma * (y - x) * dt;
        double dy = (x * (rho - z) - y) * dt;
        double dz = (x * y - beta * z) * dt;

        x += dx;
        y += dy;
        z += dz;

        trajectory.push_back({(float)x, (float)y, (float)z});
    }
    return trajectory;
}


int main(int argc, char* argv[]){
    if(SDL_Init(SDL_INIT_VIDEO)<0){
        std::cout<<"SDL could not initialize\n";
        exit(1);
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION,4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION,1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);


    gGraphicsApplicationWindow =  SDL_CreateWindow("Sim Engine", 0,0, SCR_WIDTH, SCR_HEIGHT, SDL_WINDOW_OPENGL);
    if (gGraphicsApplicationWindow==nullptr){
        std::cout<<"could not initialize gGraphicsApplicationWindow\n";
        exit(1);
    }
    gOpenGLContext =  SDL_GL_CreateContext(gGraphicsApplicationWindow);
    if (gOpenGLContext==nullptr){
        std::cout<<"could not initialize gOpenGLContext\n";
        exit(1);
    }
    if (!gladLoadGLLoader(SDL_GL_GetProcAddress)){
        std::cout<<"glad was not initialized\n";
        exit(1);
    }

    Shader ourShader("shader.vs", "shader.fs");
    Shader lineShader("line_shader.vs", "line_shader.fs");

    glEnable(GL_DEPTH_TEST);

    float cube_vertices[] = {
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
    };

    struct Vertex {
        float x, y, z;
    };

    float ground_vertices[] = {
        // positions          // norm
         0.5f,  0.5f, 0.0f,   0.0f, 0.0f, -1.0f, // top right
         0.5f, -0.5f, 0.0f,   0.0f, 0.0f, -1.0f, // bottom right
        -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, -1.0f, // bottom left
        -0.5f,  0.5f, 0.0f,   0.0f, 0.0f, -1.0f,  // top left 
    };
    unsigned int ground_indices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };

    // GROUND 
    unsigned int ground_VBO, ground_VAO, ground_EBO;
    glGenVertexArrays(1, &ground_VAO);
    glGenBuffers(1, &ground_VBO);
    glGenBuffers(1, &ground_EBO);

    glBindVertexArray(ground_VAO);

    glBindBuffer(GL_ARRAY_BUFFER, ground_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(ground_vertices), ground_vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ground_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(ground_indices), ground_indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // CUBE

    unsigned int cubeVBO, cubeVAO;

    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);

    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);

    glBindVertexArray(cubeVAO);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);


    // LINES

    std::vector<Point3D> trajectory = computeLorenz(1.0, 1.0, 1.0);

    unsigned int lineVBO, lineVAO;

    glGenVertexArrays(1, &lineVAO);
    glGenBuffers(1, &lineVBO);
    
    glBindVertexArray(lineVAO);
    glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
    glBufferData(GL_ARRAY_BUFFER, trajectory.size()*sizeof(Point3D), trajectory.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(lineVAO);


    SDL_WarpMouseInWindow(gGraphicsApplicationWindow, SCR_WIDTH/2, SCR_HEIGHT/2);
    SDL_SetRelativeMouseMode(SDL_TRUE);

    glLineWidth(3.0f);
    
    while (!gQuit){

        float currentFrame = SDL_GetTicks()/1000.0f;
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        
        processKeyboardInput();

        glDisable(GL_CULL_FACE);
        glViewport(0,0,SCR_WIDTH,SCR_HEIGHT);
        glClearColor(0.03f, 0.04f, 0.05f, 1.0f);
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

        ourShader.use();

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        ourShader.setMat4("projection", projection);
      
        glm::mat4 view = camera.GetViewMatrix();
        ourShader.setMat4("view", view);
        

        ourShader.setVec3("objectColor", 0.8f, 0.8f, 0.8f);
        ourShader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
        ourShader.setVec3("lightPos", lightPos);
        ourShader.setVec3("viewPos", camera.Position);

        // render ground
        glBindVertexArray(ground_VAO);

        for (unsigned int j = 0; j < 10; j++){
            for (unsigned int i = 0; i < 10; i++){
                // calculate the model matrix for each object and pass it to shader before drawing
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3( -2.54f+1.02f*j,  0.0f,  -2.54+1.02f*i));
                model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
                
                ourShader.setMat4("model", model);

                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            }
        }
        glm::mat4 model = glm::mat4(1.0f);

        // render cube 1

        glBindVertexArray(cubeVAO);
        ourShader.setVec3("objectColor", 0.0f, 0.5f, 0.31f);
        model = glm::translate(model, glm::vec3(4.0f,0.5f,0.0f));
        // model = glm::translate(model, glm::vec3(0.0f,1.0f+sin(currentFrame),0.0f));
        ourShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // render cube 2
        glBindVertexArray(cubeVAO);
        ourShader.setVec3("objectColor", 1.0f, 0.5f, 0.31f);
        // glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f,0.0f,4.0f));
        ourShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // render cube 3
        glBindVertexArray(cubeVAO);
        ourShader.setVec3("objectColor", 1.0f, 0.5f, 0.31f);
        // glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-4.0f,0.0f,0.0f));
        ourShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // render cube 4
        glBindVertexArray(cubeVAO);
        ourShader.setVec3("objectColor", 1.0f, 0.5f, 0.31f);
        // glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f,0.0f,-4.0f));
        ourShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // render trajectory
        glBindVertexArray(lineVAO);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3( 1.0f,  1.0f,  0.0f));
        model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));

        lineShader.use();
        lineShader.setMat4("view", view);
        lineShader.setMat4("projection", projection);
        lineShader.setMat4("model", model);

        lineShader.setVec3("objectColor", 1.0f, 1.0f, 1.0f);
        glDrawArrays(GL_LINE_STRIP, 0, trajectory.size());
        glBindVertexArray(0);

       
        SDL_GL_SwapWindow(gGraphicsApplicationWindow);
        
    }

    glDeleteVertexArrays(1, &ground_VAO);
    glDeleteBuffers(1, &ground_VBO);
    glDeleteBuffers(1, &ground_EBO);
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteBuffers(1, &cubeVBO);
    glDeleteVertexArrays(1, &lineVAO);
    glDeleteBuffers(1, &lineVBO);


    Cleanup();

    return 0;

}