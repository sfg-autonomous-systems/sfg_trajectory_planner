#version 330 core
in vec2 v_UV; 
in vec4 v_Color; 

out vec4 f_Color;

uniform sampler2D u_FontAtlas; 

void main() { 
    float alpha = texture(u_FontAtlas, v_UV).r;
    
    if(alpha < 0.1)
    {
        discard;
    }
    f_Color = vec4(v_Color.rgb, v_Color.a * alpha); 
}