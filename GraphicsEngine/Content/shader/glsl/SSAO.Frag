#version 330 core
out float FragColor;
in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D texNoise;

uniform vec3 samples[64];
uniform mat4 projection;
float radius = 0.5;
float bias = 0.025;
float kernelSize = 64;
uniform int height;
uniform int width;
// tile noise texture over screen based on screen dimensions divided by noise size
vec2 noiseScale = vec2(width/4.0, height/4.0); 

void main()
{
	vec3 fragPos   = texture(gPosition, TexCoords).rgb;
	vec3 normal    = normalize(texture(gNormal, TexCoords).rgb);
	vec3 randomVec = texture(texNoise, TexCoords * noiseScale).rgb*2.0-1.0; 
	vec3 tangent   = normalize(randomVec - normal * dot(randomVec, normal));
	vec3 bitangent = cross(normal, tangent);
	mat3 TBN       = mat3(tangent, bitangent, normal);  	
	float occlusion = 0.0;
	for(int i = 0; i < kernelSize; i++)
	{
		// get sample position:
		vec3 sample = TBN * samples[i];		
		sample = fragPos + sample * radius; 

		// project sample position:
		vec4 offset = vec4(sample, 1.0);
		offset = projection * offset;
		offset.xyz /= offset.w;
		offset.xyz = offset.xyz * 0.5 + 0.5;

		// get sample depth:
		float sampleDepth = texture(gPosition, offset.xy).z; 

		// range check & accumulate:
		//range check improves quality of AO
		float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
		occlusion   += (sampleDepth >= sample.z + bias ? 1.0 : 0.0) * rangeCheck;     
	}
	//sum occlusin over x samples
	occlusion = 1.0 - (occlusion / kernelSize);
	FragColor = occlusion;   
}