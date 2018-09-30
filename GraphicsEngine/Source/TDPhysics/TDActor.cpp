#include "TDActor.h"

namespace TD
{

	TDActor::TDActor()
	{}
	
	TDActor::~TDActor()
	{}

	void TDActor::Init()
	{

	}

	TDActorType::Type TDActor::GetActorType() const
	{
		return ActorType;
	}

	TDScene * TDActor::GetScene() const
	{
		return OwningScene;
	}

	void TDActor::Release()
	{

	}
	TDTransform * TDActor::GetTransfrom() 
	{
		return &Transform;
	}
}