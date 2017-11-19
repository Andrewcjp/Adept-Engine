#version 430

in vec4 FragPos;
out vec4 Colour;

uniform vec3 lightPos;
uniform float far_plane;
uniform int Ispoint;
void main()
{
	if(Ispoint == 1){
    // get distance between fragment and light source
    float lightDistance = length(FragPos.xyz - lightPos);
    // map to [0;1] range by dividing by far_plane
    lightDistance = lightDistance / far_plane;
    
    // Write this as modified depth
    gl_FragDepth = lightDistance;
	}else{
	gl_FragDepth = gl_FragCoord.z;
	}
	Colour = vec4(1);
	
}  