#pragma once

#ifndef _SerialHelpers_
#define _SerialHelpers_
#include "rapidjson\document.h"

namespace SerialHelpers
{
	template<typename T>
	void addVector(rapidjson::Value& jval, rapidjson::Document::AllocatorType& jallocator, const std::string& key, const T& value)
	{
		rapidjson::Value jkey;
		jkey.SetString(key.c_str(), (uint32_t)key.size(), jallocator);
		rapidjson::Value jvec(rapidjson::kArrayType);
		for (int32_t i = 0; i < value.length(); i++)
		{
			jvec.PushBack(value[i], jallocator);
		}

		jval.AddMember(jkey, jvec, jallocator);
	}
	void addJsonValue(rapidjson::Value& jval, rapidjson::Document::AllocatorType& jallocator, const std::string& key, rapidjson::Value& value);
	void addString(rapidjson::Value& jval, rapidjson::Document::AllocatorType& jallocator, const std::string& key, const std::string& value);
	template<typename T>
	void addLiteral(rapidjson::Value& jval, rapidjson::Document::AllocatorType& jallocator, const std::string& key, const T& value)
	{
		rapidjson::Value jkey;
		jkey.SetString(key.c_str(), (uint32_t)key.size(), jallocator);
		jval.AddMember(jkey, value, jallocator);
	}
	void addBool(rapidjson::Value& jval, rapidjson::Document::AllocatorType& jallocator, const std::string& key, bool isValue);


	template<uint32_t VecSize>
	bool getFloatVec(const rapidjson::Value& jsonVal, const std::string& desc, float vec[VecSize])
	{
		if (jsonVal.IsArray() == false)
		{
			//error("Trying to load a vector for " + desc + ", but JValue is not an array");
			return false;
		}

		if (jsonVal.Size() != VecSize)
		{
		/*	return error("Trying to load a vector for " + desc + ", but vector size mismatches. Required size is " + std::to_string(VecSize) + ", array size is " + std::to_string(jsonVal.Size()));*/
			return false;
		}

		for (uint32_t i = 0; i < jsonVal.Size(); i++)
		{
			if (jsonVal[i].IsNumber() == false)
			{
				//return error("Trying to load a vector for " + desc + ", but one the elements is not a number.");
				return false;
			}

			vec[i] = (float)(jsonVal[i].GetDouble());
		}
		return true;
	}
}
#endif