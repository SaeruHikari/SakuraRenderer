/*****************************************************************************************
				             Copyrights   SaeruHikari
Description:	          
Details:		
*******************************************************************************************/
#pragma once
#include "Interface.hpp"
#include "ObjectCommonInclude.h"

typedef xg::Guid SGuid;

namespace SakuraCore
{
	class SakuraGraphicsManagerBase;
	class SSceneManager;
	SInterface ISSlientObject
	{
	protected:
		inline static SakuraGraphicsManagerBase* pGraphicsManager = nullptr;
		inline static SSceneManager* pSceneManager = nullptr;
	public:
		ISSlientObject()
		{
			ID = xg::newGuid();
		}
		virtual ~ISSlientObject() {};
		virtual SGuid GetID() { return ID; }
		virtual void SetID(SGuid id) { ID = id; }
	protected:
		SGuid ID;
	};

	SInterface ISTickObject : SImplements ISSlientObject
	{
	public:
		ISTickObject() = default;
		// Initialize function.
		virtual bool Initialize() = 0;
		// Finalize function.
		virtual void Finalize() = 0;
		// Tick function, be called per frame.
		virtual void Tick(double deltaTime) = 0;
	};
}


