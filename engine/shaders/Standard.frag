#version 460


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
layout(location = 6) in vec4 shadowCoord;
layout(location = 7) in float shadowBias;
layout(location = 8) in float projectShadow;
layout(location = 9) in float mipLevel;
layout(location = 10) in vec4 pointLightConstants;

layout(location = 0) out vec4 outColor;

vec3  DirectionalLight();
float ShadowCalculation(vec4 fragPosLightSpace, sampler2D uShadowMap);
float PointLight();

vec3 rgb_to_grayscale_luminosity(vec3 color) {
    float value = color.r * 0.21 + color.g * 0.71 + color.b * 0.07;
    return vec3(value);
}

void main() 
{
	vec3 color = textureLod(inAmbientTexture, texCoord, mipLevel).rgb;
	vec3 result = DirectionalLight();
	outColor = vec4(result, 1.0);
}

float PointLight()
{
	// d = distancia del fragmento
	// kc = constante
	// kl = constante linear
	// kq = constante cuadratica
	// atenuacion = 1/ (kc+ kl*d + kq*(d*d));
	vec3 dir = lightPosition - fragPosition;
	float d = length(dir);
	float att = 1.0
				/ (pointLightConstants[0]
					+ pointLightConstants[1] * d 
					+ pointLightConstants[2] * (d*d)
				);
	return att;
}

vec3 DirectionalLight()
{
	float att = PointLight();
	float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;
	float mipmapLevel = textureQueryLod(inAmbientTexture, texCoord).x;
	vec3 color = textureLod(inAmbientTexture, texCoord, mipLevel).rgb;
	vec3 norm = normalize(normal);
	vec3 lightDir = normalize(lightPosition - fragPosition);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * lightColor;
	float specularStrength = 0.5;
	vec3 viewDir = normalize(viewerPosition - fragPosition);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
	vec3 specular = specularStrength * spec * lightColor;
	// Caulculate shadows
	float shadow = ShadowCalculation(shadowCoord, inShadowTexture);
	ambient  *= att;
	diffuse  *= att;
	specular *= att;
	return (ambient + (1.0 - shadow) * (diffuse + specular)) * color;
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
