#version 460

layout(set=0, binding=1) uniform sampler2D inTextures[8];

layout(set=0, binding=3) uniform DirLightBufferObject
{
	mat4 lightView;
	mat4 lightProj;
	vec4 lightPosition;
	vec4 lightColor;
	vec4 lightOpts; // 0: lightType, 1: shadow 
	vec4 additionalLightOpts; // Kc, Kl, Kq
} dirLight;
layout(set=0, binding=4) uniform PointLightBufferObject // 6,7,8,9
{
	mat4 lightView;
	mat4 lightProj;
	vec4 lightPosition;
	vec4 lightColor;
	vec4 lightOpts; // 0: lightType, 1: shadow 
	vec4 additionalLightOpts; // Kc, Kl, Kq
} PLight;
layout(set=0, binding=5) uniform LightBufferObject // 6,7,8,9
{
	mat4 lightView;
	mat4 lightProj;
	vec4 lightPosition;
	vec4 lightColor;
	vec4 lightOpts; // 0: lightType, 1: shadow 
	vec4 additionalLightOpts; // Kc, Kl, Kq
} libO[2];

layout(location = 0) in vec3 fragPosition;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec3 viewerPosition;
layout(location = 4) in vec4 shadowCoord;
layout(location = 5) in float mipLevel;
layout(location = 6) in float nLights;
layout(location = 7) in float renderDepth;
layout(location = 8) in float renderSpecular;
layout(location = 9) in float renderNormals;

layout(location = 0) out vec4 outColor;

vec3 lightPosition 	= dirLight.lightPosition.xyz;
vec3 lightColor 	= dirLight.lightColor.xyz;
float shadowBias 	= dirLight.lightOpts.y;
float projectShadow = dirLight.lightOpts.x;
vec4 pointLightConstants = dirLight.additionalLightOpts;

vec3  DirectionalLight(vec3 _color);
float ShadowCalculation(vec4 fragPosLightSpace, sampler2D uShadowMap);
float PointLight();

vec3 rgb_to_grayscale_luminosity(vec3 color) {
    float value = color.r * 0.21 + color.g * 0.71 + color.b * 0.07;
    return vec3(value);
}

void main() 
{
	// vec4 shadowCoordFinal = shadowCoord * libO[2].lightProj * libO[0].lightView;
	vec3 color = textureLod(inTextures[0], texCoord, mipLevel).rgb;
	vec3 result = DirectionalLight(color);
	outColor = vec4(result, 1.0);
}

float PointLight()
{
	// d = distancia del fragmento
	// kc = constante
	// kl = constante linear
	// kq = constante cuadratica
	// atenuacion = 1/ (kc+ kl*d + kq*(d*d));
	float att = 1.0;	
	att = 0.0;
	vec3 lightPos = libO[0].lightPosition.xyz;
	vec4 pointLightC = libO[0].additionalLightOpts;
	vec3 dir = lightPos - fragPosition;
	float d = length(dir);
	att += 1.0
		/ (pointLightC[0]
			+ pointLightC[1] * d 
			+ pointLightC[2] * (d*d)
		);
	return att;
}

vec3 DirectionalLight(vec3 _color)
{
	vec3 _normal = texture(inTextures[6], texCoord).rgb;
	float att = PointLight();
	float ambientStrength = 0.05;
    vec3 ambient = _color * ambientStrength;
	
	vec3 norm = normalize(normal * 2.0 - 1.0); // transform normal vector to range [-1,1]
	vec3 lightDir = normalize(lightPosition - fragPosition);
	vec3 diffuse = max(dot(normal, lightDir), 0.0) * _color;
	
	float specularStrength = 0.5;
	vec3 viewDir = normalize(viewerPosition - fragPosition);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
	vec3 specular = specularStrength * spec * _color;
	
	// Caulculate shadows
	vec4 fraglight = dirLight.lightProj * vec4(shadowCoord);
	float shadow = ShadowCalculation(fraglight, inTextures[7]);
	ambient  *= att;
	diffuse  *= att;
	specular *= att;
	return ((ambient + ( 1 - shadow)) * diffuse + specular) * _color;
	// return (diffuse + specular) * _color;

}
const float bias = 0.005;
float ShadowCalculation(vec4 fragPosLightSpace, sampler2D uShadowMap) 
{

    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // Remap to [0.0, 1.0]
    projCoords = projCoords * 0.5 + 0.5;

    // Return no shadow when outside of clipping planes
    if (projCoords.z > 1.0) { return 0.0; }

    float closestDepth = texture(uShadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;

    float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

    return shadow;
}
