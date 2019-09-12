#pragma once
//base class for ALL C# bridge classes to extend off
public ref class CSObjectBase
{
public:
	CSObjectBase();
	virtual void Update();
	virtual void OnInit();
	virtual void OnDestruct();
};

