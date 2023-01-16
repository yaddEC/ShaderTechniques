#version 330 core

out vec4 FragColor;
in vec2 TexCoord;

uniform sampler2D screenTexture;
uniform sampler2D bloomTexture;
uniform float gamma;

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
    1, -8, 1,
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

    //FragColor = texture(screenTexture, TexCoord);

    /*vec3 hdrColor = texture(screenTexture, TexCoords).rgb;
    FragColor = vec4(hdrColor, 1.0);*/

    /*const float gamma = 2.2;
    vec3 hdrColor = texture(screenTexture, TexCoords).rgb;

    // reinhard tone mapping
    vec3 mapped = hdrColor / (hdrColor + vec3(1.0));
    // gamma correction 
    mapped = pow(mapped, vec3(1.0 / gamma));

    FragColor = vec4(mapped, 1.0);*/

    vec3 fragment = texture(screenTexture, TexCoord).rgb;
    vec3 bloom = texture(bloomTexture, TexCoord).rgb;
    
    //fragment += bloom;

    float exposure = 5.0f;
    vec3 toneMapped = vec3(1.0f) - exp(-fragment * exposure);

    toneMapped = pow(toneMapped, vec3(1.0 / gamma));
    FragColor = vec4(toneMapped, 1.0);

}