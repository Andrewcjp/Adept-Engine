#version 330 core
layout (location = 0) in vec2 vertex; // vec2 pos & vec2 texure coods
layout (location = 1) in int back;
layout (location = 2) in vec3 FrontCol;
layout (location = 3) in vec3 BackCol;

flat out int UseBack;
uniform mat4 projection;

out vec3 FrontColour;
out vec3 BackColour;
void main()
{
   gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
   FrontColour = FrontCol;
   BackColour = BackCol;
   UseBack = back;
}  