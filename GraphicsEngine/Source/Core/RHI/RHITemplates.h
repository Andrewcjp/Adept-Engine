#pragma once

//Interface for multiple objects linking to the same "object" (texture or buffer) on different devices
template <class T>
class IRHISharedDeviceObject
{
public:
	virtual ~IRHISharedDeviceObject()
	{
		//for (int i = 0; i < SharedObjects.size(); i++)
		//{
		//	EnqueueSafeRHIRelease(SharedObjects[i]);
		//}
	}
	//#RHI_Multi: ref count both sides for now just link
	void RegisterOtherDeviceTexture(T * Other)
	{
		SharedObjects.push_back(Other);
	}
	T * GetOnOtherDevice(DeviceContext * Con)
	{
		for (int i = 0; i < SharedObjects.size(); i++)
		{
			if (Con == SharedObjects[i]->GetContext())
			{
				return SharedObjects[i];
			}
		}
		return nullptr;
	}
	template<class R>
	static R* GetObject(T* buffer, DeviceContext* Device)
	{
		if (!(((R*)buffer)->CheckDevice(Device->GetDeviceIndex())))
		{
			//Hack!
			T* otherdevicebuffer = buffer->GetOnOtherDevice(Device);
			if (otherdevicebuffer == nullptr)
			{
				Log::LogMessage("Failed to Bind object on Device", Log::Error);
				return (R*)buffer;
			}
			return (R*)otherdevicebuffer;
		}
		return (R*)buffer;
	}
private:
	std::vector<T*> SharedObjects;
};