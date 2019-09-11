#pragma once

class ICSharpContainerModule;
class CSharpContainer
{
public:
	CSharpContainer();
	~CSharpContainer();
	static void StartUp();
	static void ShutDown();
	static void Update(float DT);
	static CSharpContainer* Get();
private:
	ICSharpContainerModule* CSContainer = nullptr;
	static CSharpContainer* Instance;
};

