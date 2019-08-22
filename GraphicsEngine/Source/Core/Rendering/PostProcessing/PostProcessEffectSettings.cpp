
#include "PostProcessEffectSettings.h"


PostProcessEffectSettings::PostProcessEffectSettings()
{}


PostProcessEffectSettings::~PostProcessEffectSettings()
{}

EPostProcessEffectType::Type PostProcessEffectSettings::GetType() const
{
	return SettingType;
}

void PostProcessEffectSettings::SetType(EPostProcessEffectType::Type val)
{
	SettingType = val;
}
