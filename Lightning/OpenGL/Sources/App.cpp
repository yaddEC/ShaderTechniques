#include <App.h>
#include <ResourcesManager.h>

using namespace Core;
using namespace LowRenderer;

float rectangleVertices[] =
{
	// Coords    // texCoords
	 1.0f, -1.0f,  1.0f, 0.0f,
	-1.0f, -1.0f,  0.0f, 0.0f,
	-1.0f,  1.0f,  0.0f, 1.0f,

	 1.0f,  1.0f,  1.0f, 1.0f,
	 1.0f, -1.0f,  1.0f, 0.0f,
	-1.0f,  1.0f,  0.0f, 1.0f
};

void App::Init(AppInitializer init)
{
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, init.major);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, init.minor);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

	// glfw window creation
	// --------------------
	window = glfwCreateWindow(init.width, init.height, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, init.framebuffer_size_callback);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return;
	}

	skyboxShader.LoadShaders("Resources/Shaders/Skybox.vert", "Resources/Shaders/Skybox.frag");
	filtersShader.LoadShaders("Resources/Shaders/Filters.vert", "Resources/Shaders/Filters.frag");
	bloomShader.LoadShaders("Resources/Shaders/Filters.vert", "Resources/Shaders/Bloom.frag");

	glUseProgram(filtersShader.shaderProgram);
	glUniform1i(glGetUniformLocation(filtersShader.shaderProgram, "screenTexture"), 0);
	glUniform1f(glGetUniformLocation(filtersShader.shaderProgram, "gamma"), gamma);
	glUseProgram(bloomShader.shaderProgram);
	glUniform1i(glGetUniformLocation(bloomShader.shaderProgram, "screenTexture"), 0);
	
	GLint flags = 0;
	glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
	if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
	{
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(init.glDegugOutput, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
	}

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	// Prepare framebuffer rectangle VBO and VAO
	glGenVertexArrays(1, &rectVAO);
	glGenBuffers(1, &rectVBO);
	glBindVertexArray(rectVAO);
	glBindBuffer(GL_ARRAY_BUFFER, rectVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(rectangleVertices), &rectangleVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

	glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);

	// Create Framebuffer Texture
	glGenTextures(1, &postProcessingTexture);
	glBindTexture(GL_TEXTURE_2D, postProcessingTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, init.width, init.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // Prevents edge bleeding
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Prevents edge bleeding
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, postProcessingTexture, 0);

	

	// Create Render Buffer Object
	glGenRenderbuffers(1, &RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, init.width, init.height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);

	glGenTextures(1, &bloomTexture);
	glBindTexture(GL_TEXTURE_2D, bloomTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, init.width, init.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, bloomTexture, 0);

	unsigned int attachement[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
	glDrawBuffers(2, attachement);


	// Error checking framebuffer
	auto fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (fboStatus != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer error: " << fboStatus << std::endl;
	
	glGenFramebuffers(2, pingpongFBO);
	glGenTextures(2, pingpongBuffer);
	for (unsigned int i = 0; i < 2; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
		glBindTexture(GL_TEXTURE_2D, pingpongBuffer[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, init.width, init.height, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongBuffer[i], 0);
	}
}

void App::SphereColl()
{
	player1 = SphereCollider(&mesh[0]->pos, Sphere(1));

	for (int i = 1; i < mesh.size(); i++)
	{
		platform1 = OBBCollider(&mesh[i]->pos, OBB(&mesh[i]->scl, &mesh[i]->rot));
		platforms.push_back(Platform(platform1, &player2.colVisualisation));
	}

	player = Actor(player1, mesh[0], 0.05);
}

void App::Update(int shaderProgram)
{
	glfwPollEvents();
	// Bind the custom framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	processInput(window);
	player.Update(platforms);
	platform1.Update(); 

	if (Debug == false)
	{
		Vector3D direction = camera.camPos - player.model->pos;
		direction.Normalize();
		Vector3D target = camera.camPos + direction;
		camera.Update(target);
		SkyCam = camera.LookAt(camera.camPos, target, camera.camUP);
		PV = camera.projectionMatrix * camera.viewMatrix;
	}
	else
	{
		camera.camRight = camera.camUP.CrossProduct(camera.camFront);
		camera.camRight.Normalize();

		camera.viewMatrix = camera.LookAt(camera.camPos, camera.camPos + camera.camFront, camera.camUP);
		SkyCam = camera.LookAt(camera.camPos, camera.camPos + camera.camFront, camera.camUP);
		camera.projectionMatrix = matrix4.GetProjection(80.f, 0.005f, 1000.f);

		PV = camera.projectionMatrix * camera.viewMatrix;
	}

	glUseProgram(shaderProgram);


	OtherToShaders(shaderProgram);
	PointLightsToShaders(shaderProgram);
	DirectLightsToShaders(shaderProgram);
	SpotLightsToShaders(shaderProgram);
	
	if (player.model->pos.y <= -50)
	{
		player.model->pos = { 0, 10, 0 };
		camera.camPos.x = player.model->pos.x;
		camera.camPos.y = player.model->pos.y + 10;
		camera.camPos.z = player.model->pos.z + 30;
	}

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	if (ImGui::Begin("Debug", 0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove))
	{
		float fps = ImGui::GetIO().Framerate;
		ImGui::Text("FPS: %.1f", fps);
	}
	ImGui::End();

	if (MenuClose)
	{
		if(ImGui::Begin("Menu", 0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar| ImGuiWindowFlags_NoMove))
		{
			if (ImGui::Button("New Game", ImVec2(ImGui::GetWindowSize().x + 0.1f, ImGui::GetWindowSize().x * 0.1f)))
			{
				drawModel = true;
				MenuClose = false;
			}
			if (ImGui::Button("Option", ImVec2(ImGui::GetWindowSize().x + 0.1f, ImGui::GetWindowSize().x * 0.1f)))
			{
				MenuClose = false;
				Option = true;
			}
			if (ImGui::Button("Quit", ImVec2(ImGui::GetWindowSize().x + 0.1f, ImGui::GetWindowSize().x * 0.1f)))
			{
				glfwSetWindowShouldClose(window, true);
			}
		}
		ImGui::End();
	}

	if (Option)
	{
		if (ImGui::Begin("Settings", 0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove))
		{
			if (ImGui::Button("Control AZERTY", ImVec2(ImGui::GetWindowSize().x + 0.1f, ImGui::GetWindowSize().x * 0.1f)))
			{
				AZERTY = true;
				Option = false;
				MenuClose = true;
			}
			if (ImGui::Button("Control QWERTY", ImVec2(ImGui::GetWindowSize().x + 0.1f, ImGui::GetWindowSize().x * 0.1f)))
			{
				AZERTY = false;
				Option = false;
				MenuClose = true;
			}
		}
		ImGui::End();
	}

	if (drawModel)
	{
		Matrix4 temp;
		SkyCam.inverse(SkyCam, temp);
		Skybox.SkyboxUpdate(skyboxShader.shaderProgram, temp, camera.projectionMatrix.GetTransposeMat4());

		for (int i = 0; i < mesh.size(); i++)
		{
			mesh[i]->Update(shaderProgram, PV);
		}
	}


	static int selectItem = 0;
	std::vector<const char*> item;
	for (int i = 0; i < mesh.size(); i++)
	{
		if (i == 0) 
		{
			item.push_back("Player");
		}
		else 
		{
			item.push_back("Model");
		}
	}

	pointLights[0]->ambientColor.y = pointLights[0]->ambientColor.x;
	pointLights[0]->ambientColor.z = pointLights[0]->ambientColor.x;

	if (Debug && MenuClose == false)
	{

		if (ImGui::Begin("Config", 0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove))
		{
			ImGui::Combo("Model List", &selectItem, item.data(), item.size());

			if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::Indent(10);
				if (ImGui::CollapsingHeader("Position", ImGuiTreeNodeFlags_DefaultOpen))
				{
					ImGui::Indent(5);
					ImGui::SliderFloat("Position X", &mesh[selectItem]->pos.x, -200, 200);
					ImGui::SliderFloat("Position Y", &mesh[selectItem]->pos.y, -200, 200);
					ImGui::SliderFloat("Position Z", &mesh[selectItem]->pos.z, -200, 200);
					ImGui::Unindent(5);
				}
				if (ImGui::CollapsingHeader("Rotation", ImGuiTreeNodeFlags_DefaultOpen))
				{
					ImGui::Indent(5);
					ImGui::SliderFloat("Rotation X", &mesh[selectItem]->rot.x, 0, M_PI * 2);
					ImGui::SliderFloat("Rotation Y", &mesh[selectItem]->rot.y, 0, M_PI * 2);
					ImGui::SliderFloat("Rotation Z", &mesh[selectItem]->rot.z, 0, M_PI * 2);
					ImGui::Unindent(5);
				}
				
				if (ImGui::CollapsingHeader("Scale", ImGuiTreeNodeFlags_DefaultOpen))
				{
					ImGui::Indent(5);
					ImGui::SliderFloat("Scale X", &mesh[selectItem]->scl.x, 0, 50);
					ImGui::SliderFloat("Scale Y", &mesh[selectItem]->scl.y, 0, 50);
					ImGui::SliderFloat("Scale Z", &mesh[selectItem]->scl.z, 0, 50);
					ImGui::Unindent(5);
				}
				ImGui::Unindent(10);
			}

			if (ImGui::CollapsingHeader("Light", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::Indent(10);
				ImGui::Checkbox("Gamma", &Gamma);

				if (ImGui::CollapsingHeader("Directionnal Light", ImGuiTreeNodeFlags_DefaultOpen))
				{
					ImGui::Indent(5);
					ImGui::SliderFloat("Ambiante Color", &directLights[0]->ambientColor.x, -100, 100);
					ImGui::Unindent(5);
				}
				if (ImGui::CollapsingHeader("Point Light", ImGuiTreeNodeFlags_DefaultOpen))
				{
					ImGui::Indent(5);
					ImGui::SliderFloat("Point X", &pointLights[0]->position.x, -100, 100);
					ImGui::SliderFloat("Point Y", &pointLights[0]->position.y, -100, 100);
					ImGui::SliderFloat("Point Z", &pointLights[0]->position.z, -100, 100);

					ImGui::SliderFloat("Constant", &pointLights[0]->constant, -1, 1);
					ImGui::SliderFloat("Linear", &pointLights[0]->linear, -1, 1);
					ImGui::SliderFloat("Quadratic", &pointLights[0]->quadratic, -1, 1);

					ImGui::SliderFloat("Intensity", &pointLights[0]->ambientColor.x, 1, 10000);
					ImGui::Unindent(5);
				}
				if (ImGui::CollapsingHeader("Spot Light", ImGuiTreeNodeFlags_DefaultOpen))
				{
					ImGui::Indent(5);
					ImGui::SliderFloat("Spot X", &spotLights[0]->diffuseColor.x, -100, 100);
					ImGui::SliderFloat("Spot Y", &spotLights[0]->position.y, -100, 100);
					ImGui::SliderFloat("Spot Z", &spotLights[0]->position.z, -100, 100);
					ImGui::Unindent(5);
				}
				ImGui::Unindent(10);
			}
		}
		ImGui::End();
	}

	if (Pause)
	{
		if (ImGui::Begin("Pause", 0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove))
		{
			if (ImGui::Button("Resume", ImVec2(ImGui::GetWindowSize().x + 0.1f, ImGui::GetWindowSize().x * 0.1f)))
			{
				Pause = false;
			}
			if (ImGui::Button("Debug", ImVec2(ImGui::GetWindowSize().x + 0.1f, ImGui::GetWindowSize().x * 0.1f)))
			{
				Debug = !Debug;
			}
			if (ImGui::Button("Quit", ImVec2(ImGui::GetWindowSize().x + 0.1f, ImGui::GetWindowSize().x * 0.1f)))
			{
				glfwSetWindowShouldClose(window, true);
			}
		}
		ImGui::End();
	}

	bool horizontal = true;
	bool firstIteration = true;
	// Amount of time to bounce the blur
	int amount = 2;
	glUseProgram(bloomShader.shaderProgram);
	for (unsigned int i = 0; i < amount; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
		glUniform1i(glGetUniformLocation(bloomShader.shaderProgram, "horizontal"), horizontal);

		// In the first bounc we want to get the data from the bloomTexture
		if (firstIteration)
		{
			glBindTexture(GL_TEXTURE_2D, bloomTexture);
			firstIteration = false;
		}
		// Move the data between the pingPong textures
		else
		{
			glBindTexture(GL_TEXTURE_2D, pingpongBuffer[!horizontal]);
		}

		// Render the image
		glBindVertexArray(rectVAO);
		glDisable(GL_DEPTH_TEST);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// Switch between vertical and horizontal blurring
		horizontal = !horizontal;
	}
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glUseProgram(filtersShader.shaderProgram);
	glBindSampler(0, 0);
	glBindVertexArray(rectVAO);
	glDisable(GL_DEPTH_TEST);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, postProcessingTexture);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glEnable(GL_DEPTH_TEST);
	
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	glfwSwapBuffers(window);
}

void App::OtherToShaders(unsigned int shaderProgram)
{
	glUniform3f(glGetUniformLocation(shaderProgram, "camPos"), camera.camPos.x, camera.camPos.y, camera.camPos.z);
	glUniform1f(glGetUniformLocation(shaderProgram, "Gamma"), Gamma);
	glUniform1f(glGetUniformLocation(shaderProgram, "gamma"), gamma);
	
}

void App::DirectLightsToShaders(unsigned int shaderProgram)
{

	for (int i = 0; i < directLights.size(); i++)
	{
		diffuseColor  << "directLights[" << i << "].diffuseColor";
		ambientColor  << "directLights[" << i << "].ambientColor";
		specularColor << "directLights[" << i << "].specularColor";
		lightDir      << "directLights[" << i << "].direction";

		glUniform3f(glGetUniformLocation(shaderProgram, diffuseColor.str().c_str()), directLights[i]->diffuseColor.x, directLights[i]->diffuseColor.y, directLights[i]->diffuseColor.z);
		glUniform3f(glGetUniformLocation(shaderProgram, ambientColor.str().c_str()), directLights[i]->ambientColor.x, directLights[i]->ambientColor.y, directLights[i]->ambientColor.z);
		glUniform3f(glGetUniformLocation(shaderProgram, specularColor.str().c_str()), directLights[i]->specularColor.x, directLights[i]->specularColor.y, directLights[i]->specularColor.z);
		glUniform3f(glGetUniformLocation(shaderProgram, lightDir.str().c_str()), directLights[i]->direction.x, directLights[i]->direction.y, directLights[i]->direction.z);

		diffuseColor.str("");
		ambientColor.str("");
		specularColor.str("");
		lightDir.str("");
	}

	glUniform1i(glGetUniformLocation(shaderProgram, "directLightCount"), directLights.size());

}

void App::PointLightsToShaders(unsigned int shaderProgram)
{

	for (int i = 0; i < pointLights.size(); i++)
	{
		diffuseColor << "pointLights[" << i << "].diffuseColor";
		ambientColor << "pointLights[" << i << "].ambientColor";
		specularColor << "pointLights[" << i << "].specularColor";

		constant << "pointLights[" << i << "].constant";
		linear << "pointLights[" << i << "].linear";
		quadratic << "pointLights[" << i << "].quadratic";

		lightPos << "pointLights[" << i << "].position";

		glUniform3f(glGetUniformLocation(shaderProgram, diffuseColor.str().c_str()), pointLights[i]->diffuseColor.x, pointLights[i]->diffuseColor.y, pointLights[i]->diffuseColor.z);
		glUniform3f(glGetUniformLocation(shaderProgram, ambientColor.str().c_str()), pointLights[i]->ambientColor.x, pointLights[i]->ambientColor.y, pointLights[i]->ambientColor.z);
		glUniform3f(glGetUniformLocation(shaderProgram, specularColor.str().c_str()), pointLights[i]->specularColor.x, pointLights[i]->specularColor.y, pointLights[i]->specularColor.z);

		glUniform1f(glGetUniformLocation(shaderProgram, constant.str().c_str()), pointLights[i]->constant);
		glUniform1f(glGetUniformLocation(shaderProgram, linear.str().c_str()), pointLights[i]->linear);
		glUniform1f(glGetUniformLocation(shaderProgram, quadratic.str().c_str()), pointLights[i]->quadratic);

		glUniform3f(glGetUniformLocation(shaderProgram, lightPos.str().c_str()), pointLights[i]->position.x, pointLights[i]->position.y, pointLights[i]->position.z);

		diffuseColor.str("");
		ambientColor.str("");
		specularColor.str("");

		constant.str("");
		linear.str("");
		quadratic.str("");

		lightPos.str("");
	}

	glUniform1i(glGetUniformLocation(shaderProgram, "pointLightCount"), pointLights.size());

}

void App::SpotLightsToShaders(unsigned int shaderProgram)
{

	for (int i = 0; i < spotLights.size(); i++)
	{
		ambientColor << "spotLights[" << i << "].ambientColor";
		diffuseColor << "spotLights[" << i << "].diffuseColor";
		specularColor << "spotLights[" << i << "].specularColor";

		lightPos << "spotLights[" << i << "].position";
		lightDir << "spotLights[" << i << "].direction";

		cutOff << "spotLights[" << i << "].cutOff";
		outerCutOff << "spotLights[" << i << "].outerCutOff";

		glUniform3f(glGetUniformLocation(shaderProgram, diffuseColor.str().c_str()), spotLights[i]->diffuseColor.x, spotLights[i]->diffuseColor.y, spotLights[i]->diffuseColor.z);
		glUniform3f(glGetUniformLocation(shaderProgram, ambientColor.str().c_str()), spotLights[i]->ambientColor.x, spotLights[i]->ambientColor.y, spotLights[i]->ambientColor.z);
		glUniform3f(glGetUniformLocation(shaderProgram, specularColor.str().c_str()), spotLights[i]->specularColor.x, spotLights[i]->specularColor.y, spotLights[i]->specularColor.z);

		glUniform3f(glGetUniformLocation(shaderProgram, lightPos.str().c_str()), spotLights[i]->position.x, spotLights[i]->position.y, spotLights[i]->position.z);
		glUniform3f(glGetUniformLocation(shaderProgram, lightDir.str().c_str()), spotLights[i]->direction.x, spotLights[i]->direction.y, spotLights[i]->direction.z);

		glUniform1f(glGetUniformLocation(shaderProgram, cutOff.str().c_str()), spotLights[i]->cutOff);
		glUniform1f(glGetUniformLocation(shaderProgram, outerCutOff.str().c_str()), spotLights[i]->outerCutOff);

		diffuseColor.str("");
		ambientColor.str("");
		specularColor.str("");

		cutOff.str("");
		outerCutOff.str("");

		lightPos.str("");
		lightDir.str("");

	}

	glUniform1i(glGetUniformLocation(shaderProgram, "spotLightCount"), spotLights.size());
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void App::processInput(GLFWwindow* window)
{

	if (MenuClose == false)
	{
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		{
			Pause = true;
			camera.mouseMove = false;
			camera.firstMouse = true;
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

		}


		if (Debug && Pause == false)
		{

			if (AZERTY)
			{
				if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
					camera.camPos = camera.camPos + camera.moveSpeed * camera.camFront;

				if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
					camera.camPos = camera.camPos - camera.moveSpeed * camera.camFront;

				if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
					camera.camPos = camera.camPos + camera.moveSpeed * camera.camRight;

				if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
					camera.camPos = camera.camPos - camera.moveSpeed * camera.camRight;

			}
			else
			{
				if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
					camera.camPos = camera.camPos + camera.moveSpeed * camera.camFront;

				if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
					camera.camPos = camera.camPos - camera.moveSpeed * camera.camFront;

				if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
					camera.camPos = camera.camPos + camera.moveSpeed * camera.camRight;

				if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
					camera.camPos = camera.camPos - camera.moveSpeed * camera.camRight;

			}
			if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
				camera.camPos = camera.camPos + camera.moveSpeed * camera.camUP;

			if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE)
				camera.moveSpeed = 0.5f;

			if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
				camera.moveSpeed = 1.f;

			if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
				camera.camPos = camera.camPos - camera.moveSpeed * camera.camUP;

			if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
			{
				camera.mouseMove = true;
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			}
			if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE)
			{
				camera.mouseMove = false;
				camera.firstMouse = true;
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			}
		}

		if (Debug == false && Pause == false)
		{
			/*camera.camPos.x = mesh[0]->pos.x;
			camera.camPos.y = mesh[0]->pos.y + 10;
			camera.camPos.z = mesh[0]->pos.z + 30;
			camera.angle = 90;
			camera.pitch = 0;*/

			camera.camPos.y = player.model->pos.y + 10;
			if (AZERTY)
			{
				if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
				{
					mesh[0]->pos.x += 0.5f;
					mesh[0]->rot.y = 2;
					camera.camPos.x = mesh[0]->pos.x;
				}
				if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
				{
					mesh[0]->pos.x -= 0.5f;
					mesh[0]->rot.y = 5;
					camera.camPos.x = mesh[0]->pos.x;
				}
				if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
				{
					mesh[0]->pos.z -= 0.5f;
					mesh[0]->rot.y = 3.3f;
					camera.camPos.z = mesh[0]->pos.z + 30;
				}
				if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
				{
					mesh[0]->pos.z += 0.5f;
					mesh[0]->rot.y = 0;
					camera.camPos.z = mesh[0]->pos.z + 30;
				}
			}
			else
			{
				if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
				{
					mesh[0]->pos.x += 0.5f;
					mesh[0]->rot.y = 2;
				}
				if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
				{
					mesh[0]->pos.x -= 0.5f;
					mesh[0]->rot.y = 5;
				}
				if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
				{
					mesh[0]->pos.z -= 0.5f;
					mesh[0]->rot.y = 3.3f;
				}
				if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
				{
					mesh[0]->pos.z += 0.5f;
					mesh[0]->rot.y = 0;
				}
			}

			if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
			{
				camera.camPos = camera.camPos + camera.camRight * 1;
				if (camera.camPos.x >= player.model->pos.x + 50)
				{
					camera.camPos.x = player.model->pos.x + 50;
				}
				if (camera.camPos.z <= player.model->pos.z - 50)
				{
					camera.camPos.z = player.model->pos.z - 50;
				}
			}
			if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
			{
				camera.camPos = camera.camPos - camera.camRight * 1;
				if (camera.camPos.x <= player.model->pos.x - 50)
				{
					camera.camPos.x = player.model->pos.x - 50;
				}
				if (camera.camPos.z >= player.model->pos.z + 50)
				{
					camera.camPos.z = player.model->pos.z + 50;
				}
			}
			if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
			{
				camera.camPos = camera.camPos - camera.camFront * 1;
				if (camera.camPos.x <= player.model->pos.x - 50)
				{
					camera.camPos.x = player.model->pos.x - 50;
				}
				if (camera.camPos.z <= player.model->pos.z - 50)
				{
					camera.camPos.z = player.model->pos.z - 50;
				}
			}
			if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
			{
				camera.camPos = camera.camPos + camera.camFront * 1;
				if (camera.camPos.x >= player.model->pos.x + 50)
				{
					camera.camPos.x = player.model->pos.x + 50;
				}
				if (camera.camPos.z >= player.model->pos.z + 50)
				{
					camera.camPos.z = player.model->pos.z + 50;
				}
			}

			if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
			{
				if (player.isGrounded)
					player.Jump();
			}
		}

		double x, y;
		glfwGetCursorPos(window, &x, &y);
		camera.CameraWithMouse(window, x, y);
	}
}

App::~App()
{
	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
}