#pragma once
#include <glad/glad.h>
#include <fstream>
#include <string>
#include <iostream>

namespace Resources
{
	class Shader
	{
	public:
		void LoadShaders(const char* vertexShaderLink = "Resources/Shaders/vertexShader.glsl", const char* fragShaderLink = "Resources/Shaders/fragShader.glsl");
		int shaderProgram;
	};
}

