// Auto-generated embedded shaders — no external .glsl files needed at runtime
#pragma once

namespace EmbeddedShaders {

inline const char* vertexShader = R"glsl(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 vertexColor;

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    gl_PointSize = 3.0;
    vertexColor = aColor;
}
)glsl";

inline const char* fragmentShader = R"glsl(
#version 330 core

in vec3 vertexColor;
out vec4 FragColor;

void main() {
    FragColor = vec4(vertexColor, 1.0);
}
)glsl";

} // namespace EmbeddedShaders
