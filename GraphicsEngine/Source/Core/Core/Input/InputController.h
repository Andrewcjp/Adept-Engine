#pragma once
//this is the interface between game code and hardware controllers.
//A controller might not support all functions E.g. tracking, touch pad etc.
//Each Implementation holds a list of button names that relate to the hardware specifics
//these can then be bound directly by the application (or the defaults used)
class InputController
{
public:
	InputController();
	~InputController();


};

