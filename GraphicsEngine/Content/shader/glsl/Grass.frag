#version 330

in vec2 vTexCoord;
//smooth in vec3 vNormal;


out vec4 outputColor;

uniform sampler2D gSampler;
uniform vec4 vColor;

uniform vec3 vEyePosition;

//uniform Material matActive;

uniform float fAlphaTest;
uniform float fAlphaMultiplier;
uniform vec3 Colour;
void main()
{
	vec4 vTexColor = texture2D(gSampler, vTexCoord);
	float fNewAlpha = vTexColor.a*fAlphaMultiplier;               
	if(fNewAlpha < fAlphaTest)
		discard;
	
	//if(fNewAlpha > 1.0f)
	//	fNewAlpha = 1.0f;
	fNewAlpha = clamp(fNewAlpha,0.0f,1.0f);		
		
	vec4 vMixedColor =vTexColor; //mix(vTexColor,vec4(color,1.0),0.5) ; 
	
	outputColor = vec4(vMixedColor.xyz, fNewAlpha);
//	outputColor = vec4(color,fNewAlpha);
	//outputColor = texture2D(gSampler, vTexCoord);
	//outputColor = vec4(1,1,1,1);
	//outputColor.a = 1.0f;
}