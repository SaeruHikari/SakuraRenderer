/*******************************************************************************************************************************************************
									 Copyrights   SaeruHikari
CreateDate:									2019.10.21
Description:	      Interface of all run time modules, implement this to create entities.
Details:		Application manages the runtime module entities and batch them to call interface functions.
*********************************************************************************************************************************************************/
#pragma once
#include "ISObject.h"
#include <cassert>

SInterface IRuntimeModule : SImplements ISTickObject
{
public:
	virtual ~IRuntimeModule() {};
	// Initialize function.
	virtual bool Initialize() = 0;
	// Finalize function.
	virtual void Finalize() = 0;
	// Tick function, be called per frame.
	virtual void Tick(double deltaTime) = 0;
};
