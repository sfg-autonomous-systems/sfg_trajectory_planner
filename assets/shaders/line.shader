#pragma stage vertex
#version 330 core
layout(location = 0) in vec3 a_PositionWs;
layout(location = 1) in vec4 a_Color;

uniform mat4 u_WsToCsMatrix;

out vec4 v_Color;

void main() {
    v_Color = a_Color;
    gl_Position = u_WsToCsMatrix * vec4(a_PositionWs, 1.0);
}

#pragma stage fragment
#version 330 core
in vec4 v_Color;
out vec4 f_Color;

void main() {
    f_Color = v_Color;
}