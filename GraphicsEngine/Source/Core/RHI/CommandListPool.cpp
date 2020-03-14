#include "CommandListPool.h"
#include "RHI/DeviceContext.h"
#include "RHICommandList.h"

RHICommandList * CommandListPool::GetList(DeviceContext * con, ECommandListType::Type type)
{
	return con->GetListPool()->GetCMDList(type);
}

RHICommandList * CommandListPool::GetCMDList(ECommandListType::Type type)
{
	int ThreadIndex = 0;
	PoolThreadContext* con = &ThreadContext[ThreadIndex];
	if (con->CurrnetList != nullptr)
	{
		if (con->CurrnetList->GetListType() == type)
		{
			return con->CurrnetList;
		}
	}
	RHICommandList* newlist = nullptr;
	for (int i = 0; i < con->commandLists.size(); i++)
	{
		if (con->commandLists[i]->GetListType() == type)
		{
			newlist = con->commandLists[i];
			con->SetCurrentList(newlist);
			if (!newlist->IsOpen())
			{
				newlist->ResetList();
			}
			return newlist;
		}
	}
	newlist = RHI::CreateCommandList(type, Context);
	con->commandLists.push_back(newlist);
	newlist->ResetList();
	con->SetCurrentList(newlist);
	return newlist;
}

void CommandListPool::Flush()
{
	int ThreadIndex = 0;
	PoolThreadContext* con = &ThreadContext[ThreadIndex];
	con->SetCurrentList(nullptr);
}

void CommandListPool::Init(int Threadcount, DeviceContext* device)
{
	ThreadContext.resize(Threadcount);
	Context = device;
}

void PoolThreadContext::SetCurrentList(RHICommandList * list)
{
	if (CurrnetList != nullptr)
	{
		CurrnetList->Execute();
	}
	CurrnetList = list;
}
