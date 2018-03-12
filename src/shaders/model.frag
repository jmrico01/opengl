#version 330 core

in vec3 normalCamSpace;

out vec4 outColor;

void main()
{
    vec3 ambientColor = vec3(0.1, 0.1, 0.1);
    vec3 modelColor = vec3(0.7, 0.7, 0.7);

    vec3 lightDirCamSpace = vec3(0.0, 0.0, -1.0);
    vec3 normal = normalize(normalCamSpace);
    float cosTheta = dot(normalCamSpace, -lightDirCamSpace);

    outColor = vec4(ambientColor + modelColor * cosTheta, 1.0);
}