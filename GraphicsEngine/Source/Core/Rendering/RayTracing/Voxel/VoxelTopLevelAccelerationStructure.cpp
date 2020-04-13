#include "VoxelTopLevelAccelerationStructure.h"
#include "VoxelBottomLevelAccelerationStructure.h"
#include "../../Shaders/Shader_Pair.h"
#include "RHI/RHITexture.h"
#include "VoxelTracingEngine.h"


VoxelTopLevelAccelerationStructure::VoxelTopLevelAccelerationStructure()
{
	Init();
}


VoxelTopLevelAccelerationStructure::~VoxelTopLevelAccelerationStructure()
{
}

void VoxelTopLevelAccelerationStructure::Init()
{
	const float size = 250;
	MapSize = glm::ivec3(size, 150, size);
	BuildTopLevelVXShader = new Shader_Pair(RHI::GetDefaultDevice(), { "Voxel\\BuildTopLevelVoxel" }, { EShaderType::SHADER_COMPUTE });
	RHITextureDesc2 TextureDesc;
	TextureDesc.Width = MapSize.x;
	TextureDesc.Height = MapSize.y;
	TextureDesc.Depth = MapSize.z;
	TextureDesc.MipCount = 1;
	TextureDesc.clearcolour = glm::vec4(0, 0, 0, 0);
	TextureDesc.AllowUnorderedAccess = true;
	TextureDesc.Dimension = DIMENSION_TEXTURE3D;
	TextureDesc.Format = FORMAT_R32G32B32A32_UINT;//FORMAT_R32_FLOAT;
	TextureDesc.InitalState = EResourceState::UAV;
	VoxelBuffer = RHI::GetRHIClass()->CreateTexture2();
	VoxelBuffer->Create(TextureDesc, Device);

	VoxelAlphaMap = RHI::GetRHIClass()->CreateTexture2();
	TextureDesc.Format = FORMAT_R16_FLOAT;
	TextureDesc.MipCount = 5;
	VoxelAlphaMap->Create(TextureDesc, Device);

	SturctureArray = RHI::CreateTextureArray(Device, MAX_STRUCT_COUNT);
	SturctureArray->SetTextureFormat(TextureDesc);

	InstanceBuffer = RHI::CreateRHIBuffer(ERHIBufferType::GPU, Device);
	RHIBufferDesc Desc;
	Desc.Stride = sizeof(GPUVXInstanceDesc);
	Desc.ElementCount = 150;
	Desc.Accesstype = EBufferAccessType::Dynamic;
	InstanceBuffer->CreateBuffer(Desc);

	ControlBuffer = RHI::CreateRHIBuffer(ERHIBufferType::Constant, Device);
	ControlBuffer->CreateConstantBuffer(sizeof(GPUcontrolData), 1);

	VoxelMapControlBuffer = RHI::CreateRHIBuffer(ERHIBufferType::Vertex);
	VoxelMapControlBuffer->CreateConstantBuffer(sizeof(VXData), 1, true);
	VXData ControlData = VoxelTracingEngine::Get()->ControlData;
	ControlData.VoxelRes = MapSize;
	//ControlData.VoxelSize = 0.25;
	ControlData.Update();
	//ControlData = VoxelTracingEngine::Get()->ControlData;
	VoxelMapControlBuffer->UpdateConstantBuffer(&ControlData);

}

void VoxelTopLevelAccelerationStructure::AddStructure(VoxelBottomLevelAccelerationStructureInstance * Structure)
{
	ContainedInstances.push_back(Structure);
}

void VoxelTopLevelAccelerationStructure::RemoveStructure(VoxelBottomLevelAccelerationStructureInstance * Structure)
{
	VectorUtils::Remove(ContainedInstances, Structure);
}

void VoxelTopLevelAccelerationStructure::BuildInstances()
{
	SturctureArray->Clear();

	Instances.clear();
	for (int i = 0; i < ContainedInstances.size(); i++)
	{
		GPUVXInstanceDesc Inst;
		Inst.InstanceIndex = i;
		Inst.Transform = ContainedInstances[i]->LinkedObject->GetTransform()->GetModel();
		Inst.Pos = glm::vec4(ContainedInstances[i]->LinkedObject->GetTransform()->GetPos(), 0.0f);
		glm::vec4 Size = glm::vec4(glm::vec3(ContainedInstances[i]->AcclerationData->GetMapSize())*VoxelTracingEngine::Get()->ControlData.VoxelSize, 0.0f);
		Inst.Size = glm::ivec4(Size);
		Inst.HalfSize = glm::vec4(Size / 2.0f);
		Instances.push_back(Inst);
		SturctureArray->AddTexture(ContainedInstances[i]->AcclerationData->GetVoxelBuffer(), i);
	}
	InstanceBuffer->UpdateBufferData(Instances.data(), Instances.size() * sizeof(GPUVXInstanceDesc), EBufferResourceState::Non_PixelShader);
	Controls.InstanceCount = ContainedInstances.size();
	ControlBuffer->UpdateConstantBuffer(&Controls);
}

void VoxelTopLevelAccelerationStructure::Build(RHICommandList * list)
{
	BuildInstances();
	VoxelBuffer->SetState(list, EResourceState::UAV);
	VoxelAlphaMap->SetState(list, EResourceState::UAV);
	RHIPipeLineStateDesc PSOD = RHIPipeLineStateDesc::CreateDefault(BuildTopLevelVXShader);
	list->SetPipelineStateDesc(PSOD);
	//list->SetBuffer(InstanceBuffer, "Descs");
	list->SetConstantBufferView(ControlBuffer, 0, "ControlData");
	list->SetUAV(VoxelBuffer, "TopLevelStructure");
	list->SetUAV(VoxelAlphaMap, "TopLevelAlphaMap");
	//list->SetTextureArray(SturctureArray, list->GetCurrnetPSO()->GetDesc().ShaderInUse->GetSlotForName("Structures"), RHIViewDesc::DefaultSRV());	
	//list->DispatchSized(MapSize.x, MapSize.y, MapSize.z);
	//list->UAVBarrier(VoxelBuffer);
	for (int i = 0; i < ContainedInstances.size(); i++)
	{
		BuildInstance(list, ContainedInstances[i], i);
	}
	list->UAVBarrier(VoxelBuffer);
	VoxelBuffer->SetState(list, EResourceState::Non_PixelShader);
	VoxelAlphaMap->SetState(list, EResourceState::Non_PixelShader);
}

void VoxelTopLevelAccelerationStructure::BuildInstance(RHICommandList* list, VoxelBottomLevelAccelerationStructureInstance* instance, int Index)
{
	glm::ivec3 InstanceSize = instance->AcclerationData->GetMapSize();
	list->SetTexture2(instance->AcclerationData->GetVoxelBuffer(), "VoxelData");
	glm::vec3 RootSize = glm::vec3(MapSize / 2);
	glm::vec3 LocalPos = instance->LinkedObject->GetTransform()->GetPos() / (VoxelTracingEngine::Get()->ControlData.VoxelSize * 2);
	LocalPos.y = -LocalPos.y;
	glm::ivec3 Offset = glm::ivec3(RootSize) - glm::ivec3(InstanceSize / 2) + glm::ivec3(LocalPos);
	glm::ivec4 Data = glm::ivec4(Offset, Index);
	list->SetRootConstant("IndexData", 3, &Data);
	list->DispatchSized(instance->AcclerationData->GetMapSize().x, instance->AcclerationData->GetMapSize().y, instance->AcclerationData->GetMapSize().z);

}