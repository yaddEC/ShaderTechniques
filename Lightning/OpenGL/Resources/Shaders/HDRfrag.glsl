#version 330 core

out vec4 FragColor;
in vec2 TexCoord;

uniform sampler2D screenTexture;

const float offset_x = 1.0f / 800.0f;  
const float offset_y = 1.0f / 800.0f;  

vec2 offsets[9] = vec2[]
(
    vec2(-offset_x,  offset_y), vec2( 0.0f,    offset_y), vec2( offset_x,  offset_y),
    vec2(-offset_x,  0.0f),     vec2( 0.0f,    0.0f),     vec2( offset_x,  0.0f),
    vec2(-offset_x, -offset_y), vec2( 0.0f,   -offset_y), vec2( offset_x, -offset_y) 
);

float kernel[9] = float[]
(
    1,  1, 1,
    1, -7, 1,
    1,  1, 1
);

void main()
{
    /*vec3 color = vec3(0.0f);
    for(int i = 0; i < 9; i++)
        color += vec3(texture(screenTexture, TexCoord.st + offsets[i])) * kernel[i];
    FragColor = vec4(color, 1.0f);*/

    //FragColor = vec4(1.0f) - texture(screenTexture, TexCoord);

    /*vec4 tex = texture(screenTexture, TexCoord);
    float avg = (tex.x + tex.y + tex.z) / 3.0f;
    FragColor = vec4(avg, avg, avg, 1.0f);*/

    FragColor = texture(screenTexture, TexCoord);
}