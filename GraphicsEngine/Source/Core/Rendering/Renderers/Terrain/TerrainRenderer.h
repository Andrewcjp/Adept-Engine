#pragma once

class Mesh;
class FrameBuffer;
class Shader_Pair;
//terrain components hold the data, this reads and renders a terrain or segment
class TerrainRenderer
{
public:
	TerrainRenderer();
	~TerrainRenderer();
	void Update();
	void RenderTerrainForDepth(FrameBuffer* Buffer, RHICommandList* list);
	void RenderTerrainGBuffer(FrameBuffer* Buffer, RHICommandList* list, FrameBuffer * DepthSource = nullptr);
	static TerrainRenderer* Get();
private:
	void RenderQuad(RHICommandList* list);
	GameObject*  TMP = nullptr;
	Mesh* QuadMesh = nullptr;
	BaseTextureRef HeightMap;
	BaseTextureRef BaseTex;
	static TerrainRenderer* Instance;
};

