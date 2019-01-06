#pragma once
/*!  \addtogroup Game_AI
* @{ */
class AIBase;
class BleedOut_Director;
//this class handles weather an AI can attack a Player object
class AttackController
{
public:
	AttackController();
	~AttackController();
	bool CanGetToken(AIBase* Base);
	void ReleaseToken(AIBase* Base);
	BleedOut_Director* OwningDirector = nullptr;
private:
	std::vector<AIBase*> CurrentAITokens;
};

