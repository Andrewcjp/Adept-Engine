#pragma once

class DeviceContext;
namespace EGPUQueryType
{
	enum Type
	{
		Occlusion,
		BinaryOcclusion,
		Timestamp,
		Pipeline_Stats,
		Limit
	};
}

class RHIQuery
{
public:
	RHI_API RHIQuery(EGPUQueryType::Type Type,DeviceContext* Device);
	RHI_API virtual ~RHIQuery();
	RHI_API EGPUQueryType::Type GetQueryType() const;
protected:
	EGPUQueryType::Type QType = EGPUQueryType::Limit;
	DeviceContext* Device = nullptr;
};

