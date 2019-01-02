#pragma once

class AIBase;
class TestGame_Director;
//this class handles weather an AI can attack a Player object
class AttackController
{
public:
	AttackController();
	~AttackController();
	bool CanGetToken(AIBase* Base);
	void ReleaseToken(AIBase* Base);
	TestGame_Director* OwningDirector = nullptr;
private:
	std::vector<AIBase*> CurrentAITokens;
};

