#include "MeshInstanceBuffer.h"
#include "Rendering/Core/Mesh/MeshBatch.h"
#include "Core/GameObject.h"
#include "../Material.h"
#include "RHI/RHIBufferGroup.h"


MeshInstanceBuffer::MeshInstanceBuffer()
{}

MeshInstanceBuffer::~MeshInstanceBuffer()
{}

void MeshInstanceBuffer::AddBatch(MeshBatch * Batch)
{
	containedBatches.push_back(Batch);
}

void MeshInstanceBuffer::RemoveBatch(MeshBatch * batch)
{
	VectorUtils::Remove(containedBatches, batch);
}

void MeshInstanceBuffer::UpdateBuffer()
{
	bool Culled = true;
	for (int i = 0; i < containedBatches.size(); i++)
	{
		if (!containedBatches[i]->MainPassCulled)
		{
			Culled = false;
			break;
		}
	}
	IsCulled = Culled;
	if (Culled)
	{
		return;
	}
	bool ForceUpdate = false;
	for (int i = containedBatches.size() - 1; i >= 0; i--)
	{
		if (!containedBatches[i]->IsValid)
		{
			containedBatches.erase(containedBatches.begin() + i);
			ForceUpdate = true;
		}
	}
	for (uint i = 0; i < containedBatches.size(); i++)
	{
		if (i > GetInstanceCount())
		{
			LogEnsureMsgf(false, "Array overflow in instancing buffer");
			break;
		}
		if (containedBatches[i]->Owner->GetLastMovedFrame() < RHI::GetFrameCount() && !ForceUpdate)
		{
			continue;
		}

		InstanceArgs B = {};
		B.M = containedBatches[i]->Owner->GetTransform()->GetModel();
		Buffer->UpdateConstantBuffer(&B, i);

		//Update material data
		void* ptr = containedBatches[i]->elements[0]->MaterialInUse->GetDataPtr();
		MateralDataBuffer->UpdateConstantBuffer(ptr, i);
	}
}

void MeshInstanceBuffer::Build()
{
	if (Buffer != nullptr)
	{
		EnqueueSafeRHIRelease(Buffer);
	}
	Buffer = new RHIBufferGroup();
	Stride = sizeof(InstanceArgs);
	Buffer->SetDebugName("Mesh Instance Buffer (Transfrom)" + std::to_string(GetInstanceCount()));
	Buffer->CreateConstantBuffer(Stride, GetInstanceCount());

	MateralDataBuffer = new RHIBufferGroup();
	MateralDataBuffer->SetDebugName("Mesh Instance Buffer (material data)" + std::to_string(GetInstanceCount()));
	MateralDataBuffer->CreateConstantBuffer(TargetMaterial->GetInstanceDataSize(), GetInstanceCount());
}

int MeshInstanceBuffer::GetInstanceCount()
{
	return (int)containedBatches.size();
}

RHIBufferGroup * MeshInstanceBuffer::GetBuffer()
{
	return Buffer;
}

RHIBufferGroup * MeshInstanceBuffer::GetMaterialBuffer()
{
	return MateralDataBuffer;
}

bool MeshInstanceBuffer::IsCompletelyCulled() const
{
	return IsCulled;
}
