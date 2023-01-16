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
		void LoadShaders(const char* vertexShaderLink, const char* fragShaderLink);
		int shaderProgram;
	};
}

