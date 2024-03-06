#version 450
layout(set=0, binding=1) uniform sampler2D inDiffuseTexture;
layout(set=0, binding=2) uniform sampler2D inSpecularTexture;
layout(set=0, binding=3) uniform sampler2D inAmbientTexture;

layout(location = 0) in vec3 fragPosition;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec3 lightPosition;
layout(location = 4) in vec3 viewerPosition;
layout(location = 5) in vec3 lightColor;

layout(location = 0) out vec4 outColor;

vec3 directional_light_calculations(vec3 _viewDir);
vec3 BlinnPhong_calculations(vec3 _viewDir);

void main() 
{
	vec3 viewer_direction = normalize(viewerPosition - fragPosition);
	vec3 result =  directional_light_calculations( viewer_direction);
    outColor = vec4(result, 1.0);
}

vec3 directional_light_calculations(vec3 _viewDir)
{
	vec3  light_dir = normalize(lightPosition - fragPosition);
	float diff = max(dot(lightPosition, normal), 0.0);
	vec3 reflect_dir = reflect(light_dir,  normal);
	float specStrength = max(dot(_viewDir, reflect_dir), 0.0f);
	float spec = pow(specStrength, 32.0f);

	vec3 ambient = lightColor *  vec3(texture(inAmbientTexture, texCoord));
	vec3 diffuse = diff * vec3(texture(inDiffuseTexture, texCoord));
	vec3 specular = spec * vec3(texture(inSpecularTexture,
	texCoord));
	return ambient + diffuse + specular;
}

vec3 BlinnPhong_calculations(vec3 _viewDir)
{
	vec3  light_dir = lightPosition - fragPosition;
	float r = length(light_dir);
	light_dir = normalize(light_dir);

	float diff = max(dot(lightPosition, normal), 0.0);
	vec3 halfDir = normalize(_viewDir + light_dir);
	float specStrength = max(dot(halfDir, normal), 0.0f);
	float spec = pow(specStrength, 32.0f);

	vec3 ambient = lightColor * vec3(texture(inAmbientTexture, texCoord));
	vec3 diffuse = diff * vec3(texture(inDiffuseTexture, texCoord));
	vec3 specular = spec * vec3(texture(inSpecularTexture, texCoord));
	return ambient + diffuse;
}
