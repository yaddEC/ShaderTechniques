#version 330 core

struct DirectionnalLight {
	vec3 direction;

	vec3 ambientColor;
	vec3 diffuseColor;
	vec3 specularColor;
};

struct PointLight {
	vec3 position;

	vec3 ambientColor;
	vec3 diffuseColor;
	vec3 specularColor;

	float constant;
	float linear;
	float quadratic;
};

struct SpotLight {
	vec3 position;
	vec3 direction;

	vec3 ambientColor;
	vec3 diffuseColor;
	vec3 specularColor;

	float cutOff;
	float outerCutOff;

	float angle;
};

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 bloomEffect;

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

uniform sampler2D texture1;

uniform vec3 camPos;
uniform bool Gamma;
uniform float gamma;

uniform PointLight pointLights[10];
uniform DirectionnalLight directLights[10];
uniform SpotLight spotLights[10];

uniform int pointLightCount;
uniform int directLightCount;
uniform int spotLightCount;

vec3 DirectLightCalcul(DirectionnalLight light, vec3 normal, vec3 viewDir)
{
	vec3 ambient = light.ambientColor;

	vec3 lightDir   = normalize(-light.direction);
	float diff      = max(dot(normal, lightDir), 0.0);
	vec3 reflectDir = reflect(lightDir, normal);
	float spec      = pow(max(dot(-viewDir, reflectDir), 0.0), 32);

	vec3 diffuse    = diff * light.diffuseColor;
	vec3 specular   = spec * light.specularColor;

	return clamp(ambient + diffuse + specular, 0, 1);
};

vec3 PointLightCalcul(PointLight light, vec3 normal, vec3 viewDir)
{
	vec3 ambient = light.ambientColor;

	vec3 lightDir   = normalize(light.position - FragPos);
	float diff      = max(dot(normal, lightDir), 0.0);
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec      = pow(max(dot(viewDir, reflectDir), 0.0), 32);

	vec3 diffuse  = diff * light.diffuseColor;
	vec3 specular = spec * light.specularColor;

	float distance = length(light.position - FragPos);
	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
	
	ambient  *= attenuation; 
	diffuse  *= attenuation;
	specular *= attenuation;  
	
	return ambient + diffuse + specular;

};

vec3 SpotLightCalcul(SpotLight light, vec3 normal, vec3 viewDir)
{
	vec3 ambient = light.ambientColor;

	vec3  lightDir = normalize(light.position - FragPos);
	float cutOff   = dot(lightDir, normalize(-light.direction));

	if (cutOff > light.outerCutOff)
	{
		float diff        = max(dot(normal, lightDir), 0.0);
		vec3  reflectDir  = reflect(lightDir, normal);
		float spec        = pow(max(dot(-viewDir, reflectDir), 0.0), 32);
		float attenuation = clamp((cutOff - light.outerCutOff) / (light.cutOff - light.outerCutOff), 0.0, 1.0);
		vec3  diffuse     = diff * light.diffuseColor;
		vec3  specular    = spec * light.specularColor;


		diffuse *= attenuation;
		specular *= attenuation;

		return clamp((ambient + diffuse + specular), 0 ,1);	
	}
	else
	{
		return ambient;
	}
};

void main()
{
	vec3 color = texture(texture1, TexCoord).rgb;
	vec3 norm = normalize(Normal);
	vec3 viewDir = normalize(camPos - FragPos);
	vec3 result = vec3(0, 0, 0);

	for (int i = 0; i < spotLightCount; i++)
		result += SpotLightCalcul(spotLights[i], norm, viewDir);

	for (int i = 0; i < pointLightCount; i++)
		result += PointLightCalcul(pointLights[i], norm, viewDir);

	for (int i = 0; i < directLightCount; i++)
		result += DirectLightCalcul(directLights[i], norm, viewDir);

	color *= result;

	if(Gamma)
		color = pow(color, vec3(1.0/gamma));
	
	FragColor = vec4(color, 1.0);
	
	float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
	if(brightness > 1.0f)
		bloomEffect = vec4(FragColor.rgb, 1.0f);
	else
		bloomEffect = vec4(0.0f, 0.0f, 0.0f, 1.0f); 
};