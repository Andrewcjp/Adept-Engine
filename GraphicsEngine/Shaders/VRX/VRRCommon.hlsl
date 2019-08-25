
#if 0
#define VRR_SHADER_PAYLOAD uint VRRIndex : SV_RenderTargetArrayIndex; uint VRRtIndex : SV_ViewportArrayIndex;
#define VRR_SHADER_MRTS 
#define VRR_BINDDATA  Texture2D<uint> VRRSSTexture : register( t65 ); cbuffer VRRData : register(b65){int VRRPerDrawRate;}
#define VRR_PROCESS(OutputData) OutputData.VRRIndex = 1; 
#else
#define VRR_SHADER_PAYLOAD 
#define VRR_BINDDATA 
#define VRR_PROCESS(OutputData)
#endif

//#define VRR_PROCESS(OutputData) if(VRRPerDrawRate == -1){ /*OutputData.VRRIndex =  VRRSSTexture.Sample(clampsampler,screenpos).r;*/ }else{ }OutputData.VRRIndex = 1; 