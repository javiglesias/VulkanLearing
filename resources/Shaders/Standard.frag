#version 450
layout(binding=1) uniform sampler2D inTexture;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragPosition;
layout(location = 2) in vec2 texCoord;
layout(location = 3) in vec3 normal;

layout(location = 0) out vec4 outColor;

struct DirectionalLight 
{
	vec3 direction;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	vec3 color;
};

struct PointLight
{
	vec3 position;

	float k_constant;
	float k_linear;
	float k_quadratic;
};

struct SpotLight 
{
	float cutoff;
	float outer_cutoff;
};
vec3 LightPosition = vec3(1.f, 1.f, 1.f);
vec3 ViewerPosition = vec3(0.f, 0.f, 0.f);
vec3 Normal = vec3(0.f, 1.f, 0.f);
vec3 directional_light_calculations(vec3 _normal, vec3 _fragPos, vec3 _viewDir);
void main() 
{
	vec3 viewer_direction = normalize(ViewerPosition - fragPosition);
	vec3 result;
	result =  directional_light_calculations(Normal,  fragPosition,  viewer_direction);
    outColor = vec4(result, 1.0);
}

vec3 directional_light_calculations(vec3 _normal, vec3 _fragPos, vec3 _viewDir)
{
	vec3 light_dir = normalize(LightPosition - _fragPos);
	float diff = max(dot(_normal, light_dir), 0.0f);
	vec3 reflect_dir = reflect(light_dir,  _normal);
	float spec = pow(max(dot(_viewDir, reflect_dir), 0.0f), 32);//material.shininess);
	
	vec3 ambient = vec3(texture(inTexture, texCoord));
	vec3 diffuse = diff * vec3(texture(inTexture, texCoord));
	vec3 specular = spec * vec3(texture(inTexture, texCoord));
	return ambient + diffuse + specular;
}