#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <iostream>
#include <sstream>
#include <Maths.h>
#include <Mesh.h>
#include <Light.h>
#include <Collider.h>
#include <Transform.h>
#include <Camera.h>
#include <Actor.h>

namespace Core
{

    struct AppInitializer
    {
        unsigned int width;
        unsigned int height;
        unsigned int major;
        unsigned int minor;
        const char* name;
        void (*framebuffer_size_callback)(GLFWwindow* window, int width, int height);
        void (*glDegugOutput)(GLenum source, GLenum type, GLuint id, GLenum severity,
            GLsizei lengh, const GLchar* message, const void* userParam);
    };

    class App
    {

    public:
        GLFWwindow* window;
        SphereCollider player1;
        SphereCollider player2;
        OBBCollider platform1;
        Camera camera;
        Actor player;
        Matrix4 PV;
        Matrix4 SkyCam;
        Matrix4 matrix4;
        std::vector<Platform> platforms;
        std::vector<LowRenderer::Mesh*> mesh;
        std::vector<LowRenderer::PointLight*> pointLights;
        std::vector<LowRenderer::DirectionnalLight*> directLights;
        std::vector<LowRenderer::SpotLight*> spotLights;

    public:
        bool drawModel = false;
        bool DirectionalLight = true;
        bool PointLight = true;
        bool SpotLight = true;
        bool Debug = true;
        bool Pause = false;
        bool MenuClose = true;
        bool AZERTY = false;
        bool Option = false;
        bool Gamma = true;
        float gamma = 2.2;
        float smoothSpeed = 0.15f;
        unsigned int cubemapTexture;
        Mesh Skybox;
        Shader skyboxShader;

    private:
        std::stringstream diffuseColor;
        std::stringstream ambientColor;
        std::stringstream specularColor;
        std::stringstream constant;
        std::stringstream linear;
        std::stringstream quadratic;
        std::stringstream lightPos;
        std::stringstream lightDir;
        std::stringstream lightAng;


    public:

        ~App();

        void Init(AppInitializer init);
        void Update(int shaderProgram);
        void OtherToShaders(unsigned int shaderProgram);
        void DirectLightsToShaders(unsigned int shaderProgram);
        void PointLightsToShaders(unsigned int shaderProgram);
        void SpotLightsToShaders(unsigned int shaderProgram);
        void processInput(GLFWwindow* window);
        void SphereColl();

    private:

    };

}