#pragma once
class ITextInputReceiver
{
public:
	virtual ~ITextInputReceiver() {}
	//called once accept is called
	virtual void ReceiveCommitedText(const std::string& text) {};
	//called when a value char is added
	virtual void OnCharAdd(const char character) {};
	//the starting value for this edit
	virtual std::string GetStartValue() { return ""; };
	std::string DisplayStartChar = "";
	virtual void OnUpdate(const std::string & DisplayText){}
};

