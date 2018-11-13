
#include "SerialHelpers.h"

void SerialHelpers::addJsonValue(rapidjson::Value & jval, rapidjson::Document::AllocatorType & jallocator, const std::string & key, rapidjson::Value & value)
{
	rapidjson::Value jkey;
	jkey.SetString(key.c_str(), (uint32_t)key.size(), jallocator);
	jval.AddMember(jkey, value, jallocator);
}

void SerialHelpers::addString(rapidjson::Value & jval, rapidjson::Document::AllocatorType & jallocator, const std::string & key, const std::string & value)
{
	rapidjson::Value jstring, jkey;
	jstring.SetString(value.c_str(), (uint32_t)value.size(), jallocator);
	jkey.SetString(key.c_str(), (uint32_t)key.size(), jallocator);

	jval.AddMember(jkey, jstring, jallocator);
}


void SerialHelpers::addBool(rapidjson::Value & jval, rapidjson::Document::AllocatorType & jallocator, const std::string & key, bool isValue)
{
	rapidjson::Value jbool, jkey;
	jbool.SetBool(isValue);
	jkey.SetString(key.c_str(), (uint32_t)key.size(), jallocator);

	jval.AddMember(jkey, jbool, jallocator);
}

glm::vec3 SerialHelpers::GetFloatVec3(const rapidjson::Value & jsonVal)
{
	glm::vec3 out = glm::vec3(0, 0, 0);
	getFloatVec<3>(jsonVal, "Rot", &out[0]);
	return out;
}

glm::vec2 SerialHelpers::GetFloatVec2(const rapidjson::Value & jsonVal)
{
	glm::vec3 out = glm::vec3(0, 0, 0);
	getFloatVec<2>(jsonVal, "Rot", &out[0]);
	return out;
}
const std::string SerialHelpers::GetString(const rapidjson::Value & jsonVal)
{
	return jsonVal.GetString();
}
