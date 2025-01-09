#version 440 core

in vec3 Normal;
in vec2 TexCoords;
in vec3 FragPos;

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

layout(std140, binding = 0) uniform MaterialBlock {
    Material material;
};

uniform int numOfLights;

layout(std140, binding = 1) uniform LightBlock {
    Light Lights[20];
};

float CalcAttenuation(float d, float falloffStart, float falloffEnd) {
    return clamp((falloffEnd-d) / (falloffEnd - falloffStart), 0.0, 1.0);
}

vec3 CalcFrensel(vec3 Fresnel, vec3 normal, vec3 lightVec) {

    float cosIncidentAngle = clamp(dot(normal, lightVec), 0.0, 1.0);

    float f0 = 1.0f - cosIncidentAngle;
    vec3 reflectPercent = Fresnel + (1.0f - Fresnel)*(pow(f0, 4));

    return reflectPercent;
}

vec3 BlinnPhong(vec3 lightStrength, vec3 lightVec, vec3 normal, vec3 toEye, Material mat) {
    const float m = mat.Shininess * 256.0f;
    vec3 halfVec = normalize(toEye + lightVec);

    float shineFactor = (m + 8.0f)*pow(max(dot(halfVec, normal), 0.0f), m) / 8.0f;
    vec3 fresnelFactor = CalcFrensel(mat.Fresnel, halfVec, lightVec);

    vec3 specAlbedo = fresnelFactor*shineFactor;

    specAlbedo = specAlbedo / (specAlbedo + 1.0f);

    return (mat.Diffuse.rgb + specAlbedo) * lightStrength;
}

vec3 DirectionalLight(Light L, Material mat, vec3 normal, vec3 toEye) {
    vec3 lightVec = -L.Direction;

    float lambert = max(dot(lightVec, normal), 0.0f);

    vec3 lightStrength = L.Strength * lambert;

    return BlinnPhong(lightStrength, lightVec, normal, toEye, mat);
}

vec3 PointLight(Light L, Material mat, vec3 normal, vec3 toEye) {
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

    return BlinnPhong(lightStrength, lightVec, normal, toEye, mat);
}

vec4 CalcLight(vec3 normal, vec3 viewDir) {

    vec3 result = vec3(0.f, 0.f, 0.f);

    for(int i = 0; i < numOfLights; i++) {
        if(Lights[i].Type == 1) {
            result += DirectionalLight(Lights[i], material, normal, viewDir);
        } else if(Lights[i].Type == 2) {
            result += PointLight(Lights[i], material, normal, viewDir);
        }
    }

    return vec4(result, 1.f);
}

void main() {
    if (material.Diffuse.a < 0.1f) {
        discard;
    }

    vec3 norm = normalize(Normal);

    vec4 ambient = vec4(ambientColor, 1.f) * material.Diffuse;

    vec3 viewDir = normalize(cameraPos - FragPos);

    vec4 directLight = CalcLight(norm, viewDir);

    vec4 pixelColor = ambient + directLight;

    FragColor = pixelColor;
}