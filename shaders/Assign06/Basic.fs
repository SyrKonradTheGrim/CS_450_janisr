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

void main() {
    // Normalize interNormal
    vec3 N = normalize(interNormal);

    // Calculate the light vector L
    vec3 L = normalize(vec3(light.pos - interPos));

    // Calculate the diffuse coefficient
    float diffuseCoefficient = max(0.0, dot(N, L));

    // Calculate diffuse color
    vec3 diffColor = diffuseCoefficient * vec3(vertexColor * light.color);

    // Calculate specular coefficient 
    float shininess = 10.0;
    vec3 R = reflect(-L, N);
    vec3 V = normalize(-interPos.xyz);
    float specularCoefficient = pow(max(dot(R, V), 0.0), shininess);//Missing Diffuse Coeffiecient mult///////////////////////////////////

    // Calculate specular color
    vec3 specularColor = diffuseCoefficient * light.color.rgb * specularCoefficient;
    
    // Set final color
    out_color = vec4(diffColor + specularColor, 1.0);
}
