#pragma once
class Game
{
public:
	Game();
	~Game();
	virtual void  Update();
	virtual void BeginPlay();
	virtual void EndPlay();
	float GetTickRate();
	void SetTickRate(float NewRate);
	class ExtraComponentRegister* GetECR();
protected:
	ExtraComponentRegister* ECR = nullptr;
private:
	float TickRate = 60.0f;
	
};

