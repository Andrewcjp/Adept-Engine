#pragma once
#include "TDTypes.h"
#include "TDActor.h"
namespace TD
{
	class  TDShape
	{
	public:
		TD_API virtual ~TDShape();
		/**
		*\brief  Sets the Physical Material of this shape
		*\param mat to use
		*/
		TD_API void SetPhysicalMaterial(TDPhysicalMaterial mat);
		/**
		*\brief  Gets a const PTR to the shapes current physical material
		*\return Const PTR to a Physics material
		*/
		TD_API const TDPhysicalMaterial* GetPhysicalMaterial()const;
		/**
		*\brief Returns the type of this shape
		*/
		TD_API TDShapeType::Type GetShapeType()const;
		/**
		*\brief Sets the TDActor which owns this shape
		*/
		TD_API void SetOwner(TDActor* newowner);
		/**
		*\brief Get the TDActor which owns this shape
		*/
		TD_API TDActor* GetOwner();
		virtual glm::vec3 GetBoundBoxHExtents();
		template<class T>
		static T* CastShape(class TDShape* shape)
		{
			return dynamic_cast<T*>(shape);
		}
		glm::vec3 GetPos();
		TD_API TDShapeFlags& GetFlags()
		{
			return Flags;
		}
		void* UserData = nullptr;
		TDTransform* GetTransfrom();
	protected:
		TDShape();
		TDTransform Transfrom;
		TDShapeFlags Flags;
		TDShapeType::Type ShapeType = TDShapeType::eLimit;
		TDPhysicalMaterial ShapeMaterial = TDPhysicalMaterial();
		TDActor* Owner = nullptr;
	};

}