#include <Mesh.h>

#define STB_IMAGE_IMPLEMENTATION
#include <STB_Image/stb_image.h>

LowRenderer::Mesh::Mesh(Model* model, Vector3D Position, Vector3D Rotation, Vector3D Scale, const char* fileName)
{
	modelOriginal = model;
	rot = Rotation;
	pos = Position;
	scl = Scale;

	int width, height, nrChannels;

	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load(fileName, &width, &height, &nrChannels, 0);

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(data);
}

void LowRenderer::Mesh::loadCubeMap(std::vector<std::string> faces)
{

	glGenTextures(1, &texture);


	glBindTexture(GL_TEXTURE_CUBE_MAP, texture);

	//glPixelStorei(GL_UNPACK_ALIGNMENT, 1); 

	for (unsigned i = 0; i < faces.size(); ++i)
	{
		int width, height, num_components;
		unsigned char* image_data = stbi_load(faces[i].c_str(), &width, &height, &num_components, 0);

		if (image_data)
		{
			GLenum format{};

			if (num_components == 1)
				format = GL_RED;
			else if (num_components == 3) // JPG images require GL_RGB
				format = GL_RGB;
			else if (num_components == 4) // PNG images require GL_RGBA
				format = GL_RGBA;

			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, image_data);
			std::cout << "   Cubemap texture image loaded successfully: " << faces[i] << "\n";
		}
		else
			std::cout << "   Cubemap texture image failed to load: " << faces[i] << "\n";

		stbi_image_free(image_data);
	}
	std::cout << "\n";
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

}

LowRenderer::Mesh::Mesh(Model* model, Vector3D Position, Vector3D Rotation, Vector3D Scale, GLuint texture)
{
	modelOriginal = model;
	rot = Rotation;
	pos = Position;
	scl = Scale;

	int width, height, nrChannels;


}




void LowRenderer::Mesh::Update(unsigned int shaderProgram, Matrix4 PVMatrix)
{

	Matrix4 modelMatrix = ModelMatrixPop();
	Matrix4 MVP = PVMatrix * modelMatrix;

	glUseProgram(shaderProgram);

	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "MVP"), 1, GL_FALSE, &MVP.matrixTab4[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "Model"), 1, GL_FALSE, &modelMatrix.matrixTab4[0][0]);

	// draw our first triangle
	glBindTextureUnit(0, texture);
	glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0); // set it manually

	glBindVertexArray(modelOriginal->VAO.vao); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized

	glDrawElements(GL_TRIANGLES, modelOriginal->vertexBFF.size(), GL_UNSIGNED_INT, 0);
}

void LowRenderer::Mesh::SkyboxUpdate(unsigned int shaderProgram, Matrix4 view, Matrix4 projection)
{
	glDepthMask(false);
	glUseProgram(shaderProgram);
	glDepthFunc(GL_LEQUAL);
	glUniform1i(glGetUniformLocation(shaderProgram, "skybox"), 0);
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, &projection.matrixTab4[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, &view.matrixTab4[0][0]);
	glBindTextureUnit(0, texture);
	glBindVertexArray(modelOriginal->VAO.vao);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
	glDrawElements(GL_TRIANGLES, modelOriginal->vertexBFF.size(), GL_UNSIGNED_INT, 0);
	glDepthMask(true);
	glDepthFunc(GL_LESS);


}

