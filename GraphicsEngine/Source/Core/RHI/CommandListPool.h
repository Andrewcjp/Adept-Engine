#pragma once
struct PoolThreadContext
{
	std::vector<RHICommandList*> commandLists;
	RHICommandList* CurrnetList = nullptr;
	void SetCurrentList(RHICommandList* list);
};
class CommandListPool
{
public:
	static RHICommandList* GetList(DeviceContext* con, ECommandListType::Type type = ECommandListType::Graphics);
	RHICommandList* GetCMDList(ECommandListType::Type type = ECommandListType::Graphics);
	void Flush();
	void Init(int Threadcount, DeviceContext* device);
	DeviceContext* Context = nullptr;
private:
	std::vector<PoolThreadContext> ThreadContext;
};

