#version 450
layout(set=0, binding=1) uniform sampler2D inDiffuseTexture;
layout(set=0, binding=2) uniform sampler2D inSpecularTexture;
layout(set=0, binding=3) uniform sampler2D inAmbientTexture;
layout(set=0, binding=5) uniform sampler2D inShadowTexture;

layout(location = 0) in vec3 fragPosition;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec3 lightPosition;
layout(location = 4) in vec3 viewerPosition;
layout(location = 5) in vec3 lightColor;
layout(location = 6) in vec4 lightSpacePos;

layout(location = 0) out vec4 outColor;

vec3 directional_light_calculations();
vec3 BlinnPhong_calculations(vec3 _viewDir);
float compute_shadow_factor(vec4 light_space_pos, sampler2D shadow_map);

void main() 
{
	vec3 result =  directional_light_calculations();
    outColor = vec4(result, 1.0);
}

vec3 directional_light_calculations()
{
	vec3 color = texture(inAmbientTexture, texCoord).rgb;
	vec3 _normal = normalize(normal);
	// Ambient light
	vec3 ambient = 0.15 * lightColor;
	// diffuse light
	vec3 light_dir = normalize(lightPosition - fragPosition);
	float diff = max(dot(light_dir, _normal), 0.0f);
	vec3 diffuse = diff * lightColor;
	// Specular light
	vec3 viewer_direction = normalize(viewerPosition - fragPosition);
	vec3 reflect_dir = reflect(light_dir, _normal);
	vec3 halfwayDir = normalize(light_dir + viewer_direction);
	float spec = pow(max(dot(_normal, halfwayDir), 0.0), 64.0);
	vec3 specular = spec * lightColor;
	// Caulculate shadows
	float shadow = compute_shadow_factor(lightSpacePos,inShadowTexture);
	return (ambient + (1.0 - shadow) * (diffuse + specular)) * color;
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

float compute_shadow_factor(vec4 light_space_pos, sampler2D shadow_map)
{
	vec3 light_space_ndc = light_space_pos.xyz / light_space_pos.w; 
	light_space_ndc = light_space_ndc * 0.5 + 0.5;
	float closestDepth = texture(shadow_map, light_space_ndc.xy).r;
	float currentDepth = light_space_ndc.z;
	vec3 light_dir = normalize(lightPosition - fragPosition);
	float bias = max(0.0 * (1.0 - dot(normal, light_dir)), 0.005);
	float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
	return shadow;
}
