#version 430 core

layout(location = 0) out vec4 out_color;

in vec4 vertexColor; // Now interpolated across face
in vec4 interPos;
in vec3 interNormal;

struct PointLight {
    vec4 pos;
    vec4 color;
};

uniform PointLight light;
uniform float metallic;
uniform float roughness;

const float PI = 3.14159265359;

vec3 getFresnelAtAngleZero(vec3 albedo, float metallic) {
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);
    return F0;
}

vec3 getFresnel(vec3 F0, vec3 L, vec3 H) {
    float cosAngle = max(0.0, dot(L, H));
    return F0 + (1.0 - F0) * pow(1.0 - cosAngle, 5.0);
}

float getNDF(vec3 H, vec3 N, float roughness) {
    float a2 = roughness * roughness;
    float NdotH = max(0.0, dot(N, H));
    float NdotH2 = NdotH * NdotH;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    return a2 / (PI * denom * denom);
}

float getSchlickGeo(vec3 B, vec3 N, float roughness) {
    float k = (roughness + 1.0) * (roughness + 1.0) / 8.0;
    return dot(N, B) / (dot(N, B) * (1.0 - k) + k);
}

float getGF(vec3 L, vec3 V, vec3 N, float roughness) {
    float GL = getSchlickGeo(L, N, roughness);
    float GV = getSchlickGeo(V, N, roughness);
    return GL * GV;
}

void main() {
    // Normalize interNormal
    vec3 N = normalize(interNormal);

    // Calculate the normalized view vector V
    vec3 V = normalize(-interPos.xyz);

    // Calculate F0 using getFresnelAtAngleZero
    vec3 F0 = getFresnelAtAngleZero(vec3(vertexColor), metallic);

    // Calculate the normalized half-vector H
    vec3 L = normalize(vec3(light.pos - interPos));
    vec3 H = normalize(L + V);

    // Calculate Fresnel reflectance F
    vec3 F = getFresnel(F0, L, H);

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    if (metallic > 0.0) {
        kD *= (1.0 - metallic) * vec3(vertexColor) / PI;
    }

    // Calculate the diffuse coefficient
    float diffuseCoefficient = max(0.0, dot(N, L));

    // Calculate diffuse color
    vec3 diffColor = diffuseCoefficient * vec3(vertexColor * light.color);

    // Calculate specular coefficient 
    float shininess = 10.0;
    vec3 R = reflect(-L, N);
   // vec3 V = normalize(-interPos.xyz);
    float specularCoefficient = pow(max(dot(R, V), 0.0), shininess);//Missing Diffuse Coeffiecient mult///////////////////////////////////

    // Calculate specular color
    vec3 specularColor = specularCoefficient * vec3(1.0, 1.0, 1.0);

    // Calculate specular reflection
    float NDF = getNDF(H, interNormal, roughness);
    float G = getGF(L, V, interNormal, roughness);
    vec3 specular = kS * NDF * G / (4.0 * max(0.0, dot(interNormal, L)) * max(0.0, dot(N, V)) + 0.0001);

    vec3 finalColor = (kD + specular) * vec3(light.color) * max(0.0, dot(interNormal, L));
    
    // Set final color
    out_color = vec4(finalColor, 1.0);
}
