#include "stdafx.h"
#include "RefChecker.h"
#if DETECT_MEMORY_LEAKS
RefCheckerContainer* RefCheckerContainer::Instance = nullptr;
int RefCheckerContainer::LogRefs()
{
	return -1;
}

void RefCheckerContainer::LogAllRefCounters()
{
	if (Instance)
	{
		for (int i = 0; i < Instance->Checkers.size(); i++)
		{
			Instance->Checkers[i]->LogRefs();
		}
	}
}

void RefCheckerContainer::Add(RefCheckerContainer * a)
{
	if (Instance == nullptr)
	{
		Instance = new RefCheckerContainer();
	}
	Instance->Checkers.push_back(a);
}
#endif
