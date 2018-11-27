#pragma once
#include "TDTypes.h"
#include "TDActor.h"
namespace TD
{
	class TD_API TDShape
	{
	public:
		TDShape();
		virtual ~TDShape();
		/**
		*\brief  Sets the Physical Material of this shape
		*\param mat to use
		*/
		void SetPhysicalMaterial(TDPhysicalMaterial mat);
		/**
		*\brief  Gets a const PTR to the shapes current physical material
		*\return Const PTR to a Physics material
		*/
		const TDPhysicalMaterial* GetPhysicalMaterial()const;
		/**
		*\brief Returns the type of this shape 
		*/
		TDShapeType::Type GetShapeType()const;
		/**
		*\brief Sets the TDActor which owns this shape
		*/
		void SetOwner(TDActor* newowner);
		/**
		*\brief Get the TDActor which owns this shape
		*/
		TDActor* GetOwner();
		virtual glm::vec3 GetBoundBoxHExtents();
		template<class T>
		static T* CastShape(class TDShape* shape)
		{
			return dynamic_cast<T*>(shape);
		}
		glm::vec3 GetPos();
		TDShapeFlags& GetFlags(){return Falgs;}
	protected:
		TDShapeFlags Falgs;
		TDShapeType::Type ShapeType = TDShapeType::eLimit;
		TDPhysicalMaterial ShapeMaterial = TDPhysicalMaterial();
		TDActor* Owner = nullptr;
	};

}