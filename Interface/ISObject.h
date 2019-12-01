/*****************************************************************************************
				             Copyrights   SaeruHikari
Description:	          
Details:		
*******************************************************************************************/
#pragma once
#include "Interface.hpp"

SInterface ISSlientObject
{
public:
	virtual ~ISSlientObject() {};
};

SInterface ISDizzyObject : SImplements ISSlientObject
{
public:
	// Initialize function.
	virtual bool Initialize() = 0;
	// Finalize function.
	virtual void Finalize() = 0;
};

SInterface ISTickObject : SImplements ISDizzyObject
{
public:
	virtual ~ISTickObject() {};
	// Tick function, be called per frame.
	virtual void Tick(double deltaTime) = 0;
};
