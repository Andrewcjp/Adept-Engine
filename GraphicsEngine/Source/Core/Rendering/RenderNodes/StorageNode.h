#pragma once
namespace EStorageType
{
	enum Type
	{
		Buffer,//GPU buffers etc.
		Framebuffer,
		CPUData,//Data from a CPU system like lights for shadows
		SceneData,
		ShadowData,
		Limit
	};
}
class StorageNode
{
public:
	StorageNode();
	virtual ~StorageNode();
	EStorageType::Type StoreType = EStorageType::Limit;
	std::string DataFormat = "";
	void SetFormat(std::string format);
	void CreateNode();
	//Resets the node to its inital state for the frame.
	void Reset();
	//Mostly for CPU Systems to update their data 
	virtual void Update() {};
	//Called when the window is resized and for frame buffers to change size based on render scale.
	virtual void Resize() {};

	void SetDevice(DeviceContext* D);
protected:
	//called when the graph is built to create this resource (if needed)

	virtual void Create() {}; 
	DeviceContext* DeviceObject = nullptr;
private:
	std::string InitalFormat = "";
};

