#include "Stdafx.h"
#include "DxIncludeHandler.h"
#include <iostream>
#include <ios>
#include <stdio.h>
#include <fstream>
#include <string>
#include <string>
#include <iostream>
#include <filesystem>
#include "Core/Assets/AssetManager.h"
#include "Core/Utils/StringUtil.h"
DxIncludeHandler::DxIncludeHandler()
{}


DxIncludeHandler::~DxIncludeHandler()
{}

HRESULT DxIncludeHandler::Open(D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID * ppData, UINT * pBytes)
{
	std::string path = AssetManager::GetShaderDirPath();
	path.append(pFileName);
	std::ifstream myfile(path);
	if (myfile.is_open())
	{
		Data = std::string((std::istreambuf_iterator<char>(myfile)), std::istreambuf_iterator<char>());
		Widedata = StringUtils::ConvertStringToWide(Data);
		ppData = (LPCVOID*)(Widedata.c_str());
		length = (UINT)Widedata.length() *8;
		pBytes = &length;
		myfile.close();
		return S_OK;
	}
	return E_FAIL;
}

HRESULT DxIncludeHandler::Close(LPCVOID pData)
{
	Data.clear();
	return S_OK;
}
