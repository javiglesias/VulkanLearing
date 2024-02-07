#version 450
layout(binding=1) uniform sampler2D inTexture[];

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragPosition;
layout(location = 2) in vec2 texCoord;
layout(location = 3) in vec3 normal;
layout(location = 4) in vec3 lightPosition;
layout(location = 5) in vec3 viewerPosition;

layout(location = 0) out vec4 outColor;

vec3 directional_light_calculations(vec3 _normal, vec3 _fragPos, vec3 _viewDir);

void main() 
{
	vec3 viewer_direction = normalize(viewerPosition - fragPosition);
	vec3 result;
	result =  directional_light_calculations(normal,  fragPosition,  viewer_direction);
    outColor = vec4(result, 1.0);
}

vec3 directional_light_calculations(vec3 _normal, vec3 _fragPos, vec3 _viewDir)
{
	vec3 light_dir = normalize(lightPosition - _fragPos);
	float diff = max(dot(_normal, light_dir), 0.0f);
	vec3 reflect_dir = reflect(light_dir,  _normal);
	float spec = pow(max(dot(_viewDir, reflect_dir), 0.0f), 32);//material.shininess);

	vec3 ambient = vec3(texture(inTexture[0], texCoord));
	vec3 diffuse = diff * vec3(texture(inTexture[1], texCoord));
	vec3 specular = spec * vec3(texture(inTexture[2], texCoord));
	return ambient + diffuse + specular;
}