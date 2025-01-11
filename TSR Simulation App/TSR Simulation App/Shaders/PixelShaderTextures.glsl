#version 440 core

in vec3 Normal;
in vec2 TexCoords;
in vec3 FragPos;
in vec4 FragPosLightSpace;

uniform vec3 cameraPos;
uniform vec3 ambientColor;

out vec4 FragColor;

struct Material {
    vec4 Diffuse;
    vec3 Fresnel;
    float Shininess;
};

struct Light {
    float FalloffStart;
    float FalloffEnd;
    int Type;
    int padding4;
    vec3 Strength;
    vec3 Direction;
    vec3 Position;
};

uniform sampler2D texture_diffuse;
uniform sampler2D shadowMap;

layout(std140, binding = 0) uniform MaterialBlock {
    Material material;
};

uniform int numOfLights;

layout(std140, binding = 1) uniform LightBlock {
    Light Lights[20];
};

float ShadowCalculation(vec4 fragPosLightSpace) {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    float currentDepth = projCoords.z;
    float bias = 0.001;  
    
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x) {
        for(int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;

    if(projCoords.z > 1.0)
        shadow = 0.0;

    return shadow;
}  


float CalcAttenuation(float d, float falloffStart, float falloffEnd) {
    return clamp((falloffEnd-d) / (falloffEnd - falloffStart), 0.0, 1.0);
}

vec3 CalcFrensel(vec3 Fresnel, vec3 normal, vec3 lightVec) {

    float cosIncidentAngle = clamp(dot(normal, lightVec), 0.0, 1.0);

    float f0 = 1.0f - cosIncidentAngle;
    vec3 reflectPercent = Fresnel + (1.0f - Fresnel)*(pow(f0, 4));

    return reflectPercent;
}

vec3 BlinnPhong(vec3 lightStrength, vec3 lightVec, vec3 normal, vec3 toEye, Material mat, vec4 textureColor) {
    const float m = mat.Shininess * 256.0f;
    vec3 halfVec = normalize(toEye + lightVec);

    float shineFactor = (m + 8.0f)*pow(max(dot(halfVec, normal), 0.0f), m) / 8.0f;
    vec3 fresnelFactor = CalcFrensel(mat.Fresnel, halfVec, lightVec);

    vec3 specAlbedo = fresnelFactor*shineFactor;

    specAlbedo = specAlbedo / (specAlbedo + 1.0f);

    return (textureColor.rgb + specAlbedo) * lightStrength;
}

vec3 DirectionalLight(Light L, Material mat, vec3 normal, vec3 toEye, vec4 textureColor) {
    vec3 lightVec = -L.Direction;

    float lambert = max(dot(lightVec, normal), 0.0f);

    vec3 lightStrength = L.Strength * lambert;

    return BlinnPhong(lightStrength, lightVec, normal, toEye, mat, textureColor);
}

vec3 PointLight(Light L, Material mat, vec3 normal, vec3 toEye, vec4 textureColor) {
    vec3 lightVec = L.Position - FragPos;

    float d = length(lightVec);

    if(d > L.FalloffEnd) {
        return vec3(0.0f, 0.0f, 0.0f);
    }

    lightVec /= d;

    float lambert = max(dot(lightVec, normal), 0.0f);

    vec3 lightStrength = L.Strength * lambert;

    float att = CalcAttenuation(d, L.FalloffStart, L.FalloffEnd);
    lightStrength *= att;

    return BlinnPhong(lightStrength, lightVec, normal, toEye, mat, textureColor);
}

vec4 CalcLight(vec3 normal, vec3 viewDir, vec4 textureColor) {

    vec3 result = vec3(0.f, 0.f, 0.f);

    for(int i = 0; i < numOfLights; i++) {
        if(Lights[i].Type == 1) {
            result += DirectionalLight(Lights[i], material, normal, viewDir, textureColor);
        } else if(Lights[i].Type == 2) {
            result += PointLight(Lights[i], material, normal, viewDir, textureColor);
        }
    }

    return vec4(result, 1.f);
}

void main() {
    vec3 norm = normalize(Normal);

    vec4 textureColor = texture(texture_diffuse, TexCoords);

    if (textureColor.a < 0.1f) {
        discard;
    }

    vec4 ambient = vec4(ambientColor, 1.f) * textureColor * material.Diffuse;

    vec3 viewDir = normalize(cameraPos - FragPos);

    vec4 directLight = CalcLight(norm, viewDir, textureColor);

    float shadow = ShadowCalculation(FragPosLightSpace);

    vec4 pixelColor = ambient + (directLight * (1.0 - shadow)) * 0.4;

    FragColor = pixelColor;
}