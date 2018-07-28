#version 330 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;
layout (location = 3) out vec3 gTangent;

in vec2 TexCoords;
in vec3 WorldPos;
in vec3 Normal;
in vec3 Tangent;
in mat3 TBN;
uniform sampler2D albedoMap;
uniform sampler2D texture_specular1;
uniform sampler2D NormalMap;
uniform sampler2D Displacementmap;
uniform int HasNormalMap;
uniform vec3 campos;
float height_scale = 0.06;
vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir)
{ 
	viewDir = viewDir * TBN;
	
    float height = (texture(Displacementmap, texCoords.xy).r * height_scale);// - (height_scale/2.0);  //bias  
	
    return texCoords.xy + viewDir.xy * height;    
} 
void main()
{    
    
    gPosition = WorldPos;
	vec2 newtexcoords = ParallaxMapping(TexCoords, normalize (campos - WorldPos));	
	vec3 TextureNormal = (TBN*(texture( NormalMap, newtexcoords ).rgb*2-1));
	if(HasNormalMap == 1){
	 gNormal = normalize(TextureNormal);
	 gAlbedoSpec.rgb = texture(albedoMap, newtexcoords).rgb;
	}else{		
	 gNormal = normalize(Normal);
	 gAlbedoSpec.rgb = texture(albedoMap,TexCoords ).rgb;
	}	
	gTangent = Tangent;
    
	gAlbedoSpec.a = 0.1;
}  