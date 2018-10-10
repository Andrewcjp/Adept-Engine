/*******************************************************************************
The content of this file includes portions of the AUDIOKINETIC Wwise Technology
released in source code form as part of the SDK installer package.

Commercial License Usage

Licensees holding valid commercial licenses to the AUDIOKINETIC Wwise Technology
may use this file in accordance with the end user license agreement provided
with the software or, alternatively, in accordance with the terms contained in a
written agreement between you and Audiokinetic Inc.

Version: v2018.1.2  Build: 6762
Copyright (c) 2006-2018 Audiokinetic Inc.
*******************************************************************************/

#pragma once

#include <string>
#include "AK/WwiseAuthoringAPI/AkAutobahn/AkJson.h"

namespace AK
{
	namespace WwiseAuthoringAPI
	{
		class JsonProvider
		{
		public:

			JsonProvider(const AkJson& in_json)
				: m_json(in_json) {}

			const AkJson& GetAkJson() const;
			std::string GetJsonString() const;

		private:
			const AkJson& m_json;
		};
	}
}
