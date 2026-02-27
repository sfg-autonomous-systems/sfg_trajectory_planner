#version 330 core
layout(location = 0) in vec3 a_PositionWs; 
layout(location = 1) in vec2 a_UV; 
layout(location = 2) in vec4 a_Color;

uniform mat4 u_WsToCsMatrix; 

out vec2 v_UV; 
out vec4 v_Color;

void main() { 
    v_UV = a_UV; 
    v_Color = a_Color; 
    gl_Position = u_WsToCsMatrix * vec4(a_PositionWs, 1.0); 
}