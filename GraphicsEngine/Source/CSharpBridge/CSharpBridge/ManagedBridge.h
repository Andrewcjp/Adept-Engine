#pragma once
using namespace System;
public ref class EngineVersion
{
public:
	EngineVersion();
	virtual ~EngineVersion();
	static int GetVersion();
	static String^ GetFullVersionString();
private:

};
