#pragma once
#include "TDTypes.h"
#include "TDActor.h"
namespace TD
{
	class TDShape
	{
	public:
		TDShape();
		~TDShape();
		/**
		*\brief  Sets the Physical Material of this shape
		*\param TDPhysicalMaterial to use
		*/
		void SetPhysicalMaterial(TDPhysicalMaterial mat);
		/**
		*\brief  Gets a const PTR to the shapes current physcal material
		*\return Const PTR to a Physics material
		*/
		const TDPhysicalMaterial* GetPhysicalMaterial()const;
		/**
		*\brief Returns the type of this shape 
		*/
		EShapeType::Type GetShapeType()const;
		/**
		*\brief Sets the TDActor which owns this shape
		*/
		void SetOwner(TDActor* newowner);
		/**
		*\brief Get the TDActor which owns this shape
		*/
		TDActor* GetOwner();
	protected:
		EShapeType::Type ShapeType = EShapeType::eLimit;
		TDPhysicalMaterial ShapeMaterial = TDPhysicalMaterial();
		TDActor* Owner = nullptr;
	};

}