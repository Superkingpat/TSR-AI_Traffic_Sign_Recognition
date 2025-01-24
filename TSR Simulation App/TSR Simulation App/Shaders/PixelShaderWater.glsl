#version 440 core

out vec4 FragColor;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;
in vec4 FragPosLightSpace;

uniform sampler2D texture_diffuse;
uniform sampler2D shadowMap;
uniform vec3 cameraPos;
uniform float time;

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

layout(std140, binding = 1) uniform LightBlock {
    Light Lights[20];
};

uniform int numOfLights;
uniform vec3 ambientColor;

float ShadowCalculation(vec4 fragPosLightSpace) {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    if (projCoords.z > 1.0)
        return 0.0;

    float closestDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;
    float bias = 0.0005;

    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;

    return shadow;
}

void main() {
    // Animated texture coordinates
    vec2 movingUV = TexCoords;
    movingUV.x += sin(time * 0.5 + TexCoords.y * 4.0) * 0.03;
    movingUV.y += cos(time * 0.5 + TexCoords.x * 4.0) * 0.03;

    vec4 texColor = texture(texture_diffuse, movingUV);

    // Wave height effect
    vec3 modifiedNormal = Normal;
    modifiedNormal.y += sin(time + FragPos.x * 2.0) * 0.1;
    modifiedNormal = normalize(modifiedNormal);

    // Lighting calculation
    vec3 viewDir = normalize(cameraPos - FragPos);
    vec3 result = ambientColor * material.Diffuse.rgb;

    for (int i = 0; i < numOfLights; i++) {
        vec3 lightDir = normalize(Lights[i].Position - FragPos);
        vec3 halfwayDir = normalize(lightDir + viewDir);

        float diff = max(dot(modifiedNormal, lightDir), 0.0);
        float spec = pow(max(dot(modifiedNormal, halfwayDir), 0.0), material.Shininess);

        vec3 ambient = Lights[i].Strength * material.Diffuse.rgb * 0.1; // Assuming ambient contribution is 10%
        vec3 diffuse = Lights[i].Strength * diff * material.Diffuse.rgb;
        vec3 specular = Lights[i].Strength * spec * material.Fresnel;

        float shadow = ShadowCalculation(FragPosLightSpace);
        result += ambient + (1.0 - shadow) * (diffuse + specular);
    }

    // Water effects
    float fresnel = pow(1.0 - max(dot(modifiedNormal, viewDir), 0.0), 2.0);
    vec3 waterColor = mix(texColor.rgb, vec3(0.0, 0.3, 0.5), 0.5);
    result = mix(result * waterColor, vec3(0.8, 0.9, 1.0), fresnel * 0.5);

    // Ripple effect
    float ripple = sin(time * 2.0 + FragPos.x * 4.0 + FragPos.z * 4.0) * 0.1;
    result += vec3(ripple);

    FragColor = vec4(result, texColor.a);
}

