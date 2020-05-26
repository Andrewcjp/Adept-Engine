#include "MaterialAsset.h"
#include "Rendering/Core/Material.h"
#include "Asset_Shader.h"
#include "Rendering/ShaderGraph/ShaderGraph.h"
#include "Rendering/Core/Mesh/MaterialTypes.h"
#include "Core/Assets/Asset types/MaterialAsset.generated.h"

MaterialAsset::MaterialAsset()
{
	NeedsMetaFile = false;
	
	CALL_CONSTRUCTOR();
}

Material * MaterialAsset::CreateMaterial()
{
	if (!m_Shader.IsValid())
	{
		return nullptr;
	}
	Material* newmat = new Material(m_Shader.GetAsset());
	//Asset shader has shader bind set
	//this stores the settings of that set.
	//apply to material instance.
	//bind set contains data that is for evey instance of this shader 
	//an parameters branched out are just not bound.
	//ParmeterBindSet Set = m_Shader.GetAsset()->GetGraph()->GetParameters();
	//deserialse params;
	newmat->SetParmaters(AssetSet);
	return newmat;
}

void MaterialAsset::Sync(Material* mat)
{

}

void MaterialAsset::SeralizeText(Archive* A)
{
	SerializeThisText(A, m_RelfectionData.Data);
	AssetSet.SeralizeText(A);
}

void MaterialAsset::SetupFromShader()
{
	AssetSet = m_Shader.GetAsset()->GetGraph()->GetParameters();
	AssetSet.InitReflection();
	AssetSet.Validate();
}

void MaterialAsset::ProcessArchive(Archive* a)
{
	SeralizeText(a);
}
