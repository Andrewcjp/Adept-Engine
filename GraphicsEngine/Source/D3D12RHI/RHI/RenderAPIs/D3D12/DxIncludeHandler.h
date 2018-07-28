#pragma once
#include <d3d12.h>
#define D3D_COMPILE_STANDARD_FILE_INCLUDE ((ID3DInclude*)(UINT_PTR)1)
class DxIncludeHandler:public ID3DInclude
{
public:
	DxIncludeHandler();
	~DxIncludeHandler();

	// Inherited via ID3DInclude
	virtual HRESULT Open(D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID * ppData, UINT * pBytes) override;
	virtual HRESULT Close(LPCVOID pData) override;
private:
	std::string Data;
	UINT length = 0;
	std::wstring Widedata;

};

