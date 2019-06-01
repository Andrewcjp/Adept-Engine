#include "MeshInstanceBuffer.h"
#include "Rendering/Core/Mesh/MeshBatch.h"
#include "Core/GameObject.h"


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
	for (int i = 0; i < containedBatches.size(); i++)
	{
		if (i > GetInstanceCount())
		{
			LogEnsureMsgf(false, "Array overflow in instancing buffer");
			break;
		}
		InstanceArgs B = {};
		B.M = containedBatches[i]->Owner->GetTransform()->GetModel();
		Buffer->UpdateConstantBuffer(&B.M[0], i);
	}
}

void MeshInstanceBuffer::Build()
{
	if (Buffer != nullptr)
	{
		EnqueueSafeRHIRelease(Buffer);
	}
	Buffer = RHI::CreateRHIBuffer(ERHIBufferType::Constant);
	Stride = sizeof(InstanceArgs);
	Buffer->SetDebugName("Mesh Instance Buffer " + std::to_string(GetInstanceCount()));
	Buffer->CreateConstantBuffer(Stride, GetInstanceCount());

}

int MeshInstanceBuffer::GetInstanceCount()
{
	return containedBatches.size();
}

RHIBuffer * MeshInstanceBuffer::GetBuffer()
{
	return Buffer;
}
