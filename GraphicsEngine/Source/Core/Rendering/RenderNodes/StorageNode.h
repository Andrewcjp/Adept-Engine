#pragma once

class RenderGraph;
class RenderNode;
namespace EStorageType
{
	enum Type
	{
		Buffer,//GPU buffers etc.
		Framebuffer,
		CPUData,//Data from a CPU system like lights for shadows
		SceneData,
		ShadowData,
		InterGPUStagingResource,
		Limit
	};
}
class StorageNode
{
public:
	StorageNode(const std::string& name = "");
	virtual ~StorageNode();
	EStorageType::Type StoreType = EStorageType::Limit;
	std::string DataFormat = "";
	void SetFormat(const std::string & format);
	void CreateNode();
	//Resets the node to its initial state for the frame.
	void Reset();
	//Mostly for CPU Systems to update their data 
	virtual void Update() {};
	//Called when the window is resized and for frame buffers to change size based on render scale.
	virtual void Resize() {};

	void SetDevice(DeviceContext* D);
	template<class T>
	static T* NodeCast(StorageNode* node)
	{
		return dynamic_cast<T*>(node);
	}
	std::string Name = "";
	RenderGraph* OwnerGraph = nullptr;
	bool GetIsTransisent() const { return IsTransisent; }
	void SetRetained() { IsTransisent = false; }
	DeviceContext* GetDeviceObject() const { return DeviceObject; }
	//Once This node has run the resource is not used again
	void DiscardAtEndOfNode(RenderNode* node);

protected:
	//called when the graph is built to create this resource (if needed)

	virtual void Create() {}; 
	DeviceContext* DeviceObject = nullptr;
	bool IsTransisent = true;
	RenderNode* DiscardingNode = nullptr;
private:
	std::string InitalFormat = "";
};

