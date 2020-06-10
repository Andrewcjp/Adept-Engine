#pragma once

class RHITexture;
class GameObject;
class Mesh;
//this wraps both native and emulated sampler feedback 
class SamplerFeedbackEngine
{
public:
	static SamplerFeedbackEngine* Get()
	{
		if (Instance == nullptr)
		{
			Instance = new SamplerFeedbackEngine();
			Instance->Init();
		}
		return Instance;
	}
	static void Shutdown()
	{
		SafeDelete(Instance);
	}
	static void SetupPairedTexture(RHITexture* Target, RHIBuffer* ReadbackResouce = nullptr);
	static RHITexture* AccessParedTexture(RHITexture* target);
	void RenderTest(RHICommandList * list, FrameBuffer * Target);
	SamplerFeedbackEngine();
	void Init();
	void RenderQuad(RHICommandList * List);
	RHITexture* GetMipChainTexture(int size,int mipcount);
	static void ResolveAndReadback(RHICommandList* list, RHITexture* FeedbackMap, RHIBuffer* CPUData);
private:
	static SamplerFeedbackEngine* Instance;
	GameObject*  TMP = nullptr;
	Mesh* QuadMesh = nullptr;
	RHITexture* BaseTex;

	void CreateMipCheckingTexture(RHITexture * Texture, int Size, int Mipcount);
	std::map<uint64, RHITexture*> MipChainTextures;
	RHIBuffer* OutputData = nullptr;
};

