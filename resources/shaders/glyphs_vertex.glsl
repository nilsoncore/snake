#version 330 core
layout (location = 0) in vec4 vertices; // <vec2 pos, vec2 tex>
out vec2 TexCoords;

uniform mat4 text_projection;

void main()
{
    gl_Position = text_projection * vec4(vertices.xy, 0.0, 1.0);
    TexCoords = vertices.zw;
}