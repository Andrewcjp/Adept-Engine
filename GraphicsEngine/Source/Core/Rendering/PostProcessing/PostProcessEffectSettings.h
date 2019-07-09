#pragma once
struct ValueType
{
	int Value = 0;
	float FValue = 0.0f;
	enum EType
	{
		Integer,
		Floating,
		EnumType,
		Limit
	};
};
struct EPostProcessEffectType
{
	enum Type
	{
		SSAO,
		SSR,
		SSS,
		TAA,
		Bloom,
		MotionBlur,
		ColourGrading,
		Limit//etc.
	};
};
//This class holds the settings for a PostProcess effect
//the component edits instances of these and EffectRenderers will get from the blended setting. 
class PostProcessEffectSettings
{
public:
	PostProcessEffectSettings();
	~PostProcessEffectSettings();
	EPostProcessEffectType::Type GetType() const;
	void SetType(EPostProcessEffectType::Type val);
private:
	EPostProcessEffectType::Type SettingType = EPostProcessEffectType::Limit;

	std::map<std::string, ValueType> SettingMap;
};

