#include "TGComp.h"

#include <iostream>

TGComp::TGComp()
{
	std::cout << "hello " << std::endl;
}


TGComp::~TGComp()
{
}

void TGComp::BeginPlay()
{
}

void TGComp::Update(float delta)
{

}

void TGComp::InitComponent()
{

}

void TGComp::Serialise(rapidjson::Value & v)
{
}

void TGComp::Deserialise(rapidjson::Value & v)
{
}
