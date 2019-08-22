#pragma once
#include "..\GPUPerformanceTest.h"

class MeshDrawCommand;
//this needs to skip most of the scene renderer overhead etc.
//as tests should NOT effect the state of the renderer.
class PrimitveThoughputTest : public GPUPerformanceTest
{
public:
	PrimitveThoughputTest();
	~PrimitveThoughputTest();

	virtual void RunTest() override;


protected:
	virtual void OnInit() override;
	virtual void OnDestory() override;
private:
	
	void BuildBatches(int size, glm::vec3 startPos, float stride);
	RHIBuffer * CreateTransfrom(glm::vec3 pos);
	void AddObjectInstance(glm::vec3 pos);
	//skip meshes etc. pre-gen the draw commands directly
	std::vector<MeshDrawCommand*> Batches;
	Mesh* MeshData;
	std::vector<RHIBuffer*> TransfromBuffers;

	FrameBuffer* TestBuffer = nullptr;
	RHICommandList* List = nullptr;
};

