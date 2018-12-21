#include "Source/Core/Stdafx.h"
#include "TD_ConstraintInstance.h"

TD_ConstraintInstance::TD_ConstraintInstance(TD::TDConstraint * con, ConstaintSetup setup):GenericConstraint(setup)
{
	constraint = con;
}

TD_ConstraintInstance::~TD_ConstraintInstance()
{}

void TD_ConstraintInstance::UpdateConstraintData()
{
	throw std::logic_error("The method or operation is not implemented.");
}
