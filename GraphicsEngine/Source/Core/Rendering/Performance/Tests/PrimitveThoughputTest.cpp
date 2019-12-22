#include "PrimitveThoughputTest.h"
#include "Core/Assets/ShaderComplier.h"
#include "Core/Transform.h"
#include "Rendering/Core/Mesh.h"
#include "Rendering/Core/Mesh/MeshBatch.h"
#include "Rendering/Core/Mesh/MeshDrawCommand.h"
#include "Rendering/Core/SceneRenderer.h"
#include "Rendering/Shaders/Shader_PreZ.h"
#include "RHI/RHITimeManager.h"
#include "../../Shaders/Shader_Main.h"
#include "Core/Performance/PerfManager.h"
#include "Core/Utils/StringUtil.h"
const std::string PrimitveThoughputTest_Name = "PrimitveThoughputTest";

PrimitveThoughputTest::PrimitveThoughputTest()
{}


PrimitveThoughputTest::~PrimitveThoughputTest()
{}

void PrimitveThoughputTest::RunTest()
{
	List->ResetList();
	if (List->GetDeviceIndex() != 0)
	{
		List->GetDevice()->GetTimeManager()->StartTotalGPUTimer(List);
	}
	{
		DECALRE_SCOPEDGPUCOUNTER(List, PrimitveThoughputTest_Name);
		RHIPipeLineStateDesc desc;
		desc = RHIPipeLineStateDesc::CreateDefault(ShaderComplier::GetShader<Shader_PreZ>(), TestBuffer);
		desc.DepthStencilState.DepthCompareFunction = COMPARISON_FUNC_LESS_EQUAL;
		desc.DepthStencilState.DepthWrite = true;
		desc.DepthStencilState.DepthEnable = true;
		desc.RenderPassDesc = RHIRenderPassDesc(TestBuffer, ERenderPassLoadOp::Clear);
		List->SetPipelineStateDesc(desc);
		List->BeginRenderPass(RHIRenderPassDesc(TestBuffer, ERenderPassLoadOp::Clear));
		SceneRenderer::Get()->BindMvBufferB(List, MainShaderRSBinds::MVCBV);
		for (int i = 0; i < Batches.size(); i++)
		{
			MeshDrawCommand* C = Batches[i];			
			List->SetConstantBufferView(C->TransformUniformBuffer, 0, 0);
			List->SetVertexBuffer(C->Vertex);
			List->SetIndexBuffer(C->Index);
			List->DrawIndexedPrimitive(C->NumPrimitves, C->NumInstances, 0, 0, 0);
		}
		List->EndRenderPass();
	}
	if (List->GetDeviceIndex() != 0)
	{
		List->GetDevice()->GetTimeManager()->EndTotalGPUTimer(List);
	}
	List->Execute();
}

void PrimitveThoughputTest::GatherResults()
{
	TimerData* D = PerfManager::Get()->GetTimerData(PerfManager::Get()->GetTimerIDByName(PrimitveThoughputTest_Name + std::to_string(Device->GetDeviceIndex())));
	if (D != nullptr)
	{
		ResultData.TimeTaken = D->AVG->GetHighestValue();

		ResultData.PassedTest = true;
	}
}

void PrimitveThoughputTest::LogResults(GPUPerformanceTest* ZeroTest)
{
	float PCofZero = ZeroTest->ResultData.TimeTaken / ResultData.TimeTaken;
	Log::LogMessage("PrimitveThoughputTest Took " + StringUtils::ToString(ResultData.TimeTaken) + " " + StringUtils::ToString(PCofZero * 100) + "% of GPU 0");
}

void PrimitveThoughputTest::OnInit()
{
	MeshData = RHI::CreateMesh("\\models\\Sphere.obj");
#if NDEBUG
	const int Itor = 10;
#else
	const int Itor = 6;
#endif
	BuildBatches(Itor, glm::vec3(0, 5, 0), 2.0f);
	List = RHI::CreateCommandList(ECommandListType::Graphics, Device);
	RHIFrameBufferDesc Desc = RHIFrameBufferDesc::CreateColourDepth(1024, 1024);
	TestBuffer = RHI::CreateFrameBuffer(Device, Desc);
}

void PrimitveThoughputTest::OnDestory()
{
	MemoryUtils::RHIUtil::DeleteVector(TransfromBuffers);
}

void PrimitveThoughputTest::BuildBatches(int size, glm::vec3 startPos, float stride)
{
	for (int y = 0; y < size; y++)
	{
		for (int x = 0; x < size; x++)
		{
			for (int z = 0; z < size; z++)
			{
				AddObjectInstance(startPos + glm::vec3(x*stride, y*stride, z*stride));
			}
		}
	}
	ResultData.ItemsProcessed = size * size * size;
}

RHIBuffer* PrimitveThoughputTest::CreateTransfrom(glm::vec3 pos)
{
	RHIBuffer* PrimitiveTransfromBuffer = RHI::CreateRHIBuffer(ERHIBufferType::Constant);
	PrimitiveTransfromBuffer->CreateConstantBuffer(sizeof(MeshTransfromBuffer), 1, true);
	MeshTransfromBuffer SCB = {};
	Transform T = Transform();
	T.SetPos(pos);
	SCB.M = T.GetModel();
	PrimitiveTransfromBuffer->UpdateConstantBuffer(&SCB, 0);
	TransfromBuffers.push_back(PrimitiveTransfromBuffer);
	return PrimitiveTransfromBuffer;
}

void PrimitveThoughputTest::AddObjectInstance(glm::vec3 pos)
{
	MeshDrawCommand* Command = new MeshDrawCommand();
	Command->NumInstances = 1;
	Command->TransformUniformBuffer = CreateTransfrom(pos);
	Command->Vertex = MeshData->GetMeshBatch()->elements[0]->VertexBuffer;
	Command->Index = MeshData->GetMeshBatch()->elements[0]->IndexBuffer;
	Command->NumPrimitves = Command->Index->GetVertexCount();
	Batches.push_back(Command);
}
