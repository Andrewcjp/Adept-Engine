#pragma once
class GameMode
{
public:
	CORE_API GameMode();
	CORE_API ~GameMode();
	CORE_API virtual void BeginPlay();
	CORE_API virtual void EndPlay();
	CORE_API virtual void Update();
};

