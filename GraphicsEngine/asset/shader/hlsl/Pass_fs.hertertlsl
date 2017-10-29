//FXAA 
// adapted from http://developer.download.nvidia.com/assets/gamedev/files/sdk/11/FXAA_WhitePaper.pdf

#version 330 core

out vec4 FragColor;
in vec2 TexCoords;

// material parameters

uniform sampler2D R_filterTexture;
uniform vec3 R_inverseFilterTextureSize;
uniform float R_fxaaSpanMax;
uniform float R_fxaaReduceMin;
uniform float R_fxaaReduceMul;
uniform int EnableFXAA;

void FXAA(){
	vec2 texCoordOffset = R_inverseFilterTextureSize.xy;
	
	vec3 luma = vec3(0.299, 0.587, 0.114);	
	float lumaTL = dot(luma, texture2D(R_filterTexture, TexCoords.xy + (vec2(-1.0, -1.0) * texCoordOffset)).xyz);
	float lumaTR = dot(luma, texture2D(R_filterTexture, TexCoords.xy + (vec2(1.0, -1.0) * texCoordOffset)).xyz);
	float lumaBL = dot(luma, texture2D(R_filterTexture, TexCoords.xy + (vec2(-1.0, 1.0) * texCoordOffset)).xyz);
	float lumaBR = dot(luma, texture2D(R_filterTexture, TexCoords.xy + (vec2(1.0, 1.0) * texCoordOffset)).xyz);
	float lumaM  = dot(luma, texture2D(R_filterTexture, TexCoords.xy).xyz);

	vec2 dir;
	dir.x = -((lumaTL + lumaTR) - (lumaBL + lumaBR));
	dir.y = ((lumaTL + lumaBL) - (lumaTR + lumaBR));
	
	float dirReduce = max((lumaTL + lumaTR + lumaBL + lumaBR) * (R_fxaaReduceMul * 0.25), R_fxaaReduceMin);
	float inverseDirAdjustment = 1.0/(min(abs(dir.x), abs(dir.y)) + dirReduce);
	
	dir = min(vec2(R_fxaaSpanMax, R_fxaaSpanMax), 
		max(vec2(-R_fxaaSpanMax, -R_fxaaSpanMax), dir * inverseDirAdjustment));

	dir.x = dir.x * step(1.0, abs(dir.x));
 	dir.y = dir.y * step(1.0, abs(dir.y));
 	

 	dir = dir * texCoordOffset;
	vec3 result1 = (1.0/2.0) * (
		texture2D(R_filterTexture, TexCoords.xy + (dir * vec2(1.0/3.0 - 0.5))).xyz +
		texture2D(R_filterTexture, TexCoords.xy + (dir * vec2(2.0/3.0 - 0.5))).xyz);

	vec3 result2 = result1 * (1.0/2.0) + (1.0/4.0) * (
		texture2D(R_filterTexture, TexCoords.xy + (dir * vec2(0.0/3.0 - 0.5))).xyz +
		texture2D(R_filterTexture, TexCoords.xy + (dir * vec2(3.0/3.0 - 0.5))).xyz);

	float lumaMin = min(lumaM, min(min(lumaTL, lumaTR), min(lumaBL, lumaBR)));
	float lumaMax = max(lumaM, max(max(lumaTL, lumaTR), max(lumaBL, lumaBR)));
	float lumaResult2 = dot(luma, result2);
	
	//result2.x = 0;
	if(lumaResult2 < lumaMin || lumaResult2 > lumaMax)
		FragColor = vec4(result1, 1.0);
	else
		FragColor = vec4(result2, 1.0);

}
void main()
{			
	vec3 colour = texture(R_filterTexture,TexCoords).xyz;
	//colour.x = 0;
    FragColor = vec4(colour, 1.0);
	
	if(EnableFXAA == 1){	
		FXAA();	
	}
	
	 
}