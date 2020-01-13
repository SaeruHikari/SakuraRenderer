/*****************************************************************************************
				             Copyrights   SaeruHikari
Description:	          
Details:		
*******************************************************************************************/
#pragma once
#include "Interface.hpp"
#include "ObjectCommonInclude.h"
#include "Framework/Core/Reflection/SakuraReflection.h"

typedef xg::Guid SGuid;

namespace SakuraCore
{
	typedef size_t SIndex;

	class SakuraGraphicsManagerBase;
	class SSceneManager;
	struct ISSilentObject
	{
	protected:
		inline static SakuraGraphicsManagerBase* pGraphicsManager = nullptr;
		inline static SSceneManager* pSceneManager = nullptr;
	public:
		ISSilentObject()
		{
			ID = xg::newGuid();
		}
		virtual ~ISSilentObject() {};
		virtual SGuid GetID() { return ID; }
		virtual void SetID(SGuid id) { ID = id; }
		virtual int Serialize();
		virtual char* GetSClassName() { return (char*)"ISSilentObject"; };
		REFLECTION_ENABLE()
	protected:
		SGuid ID;
	};

	SInterface ISTickObject : SImplements ISSilentObject
	{
	public:
		ISTickObject() = default;
		REFLECTION_ENABLE()
		virtual char* GetSClassName() override
		{
			return (char*)"ISSilentObject"; 
		};
		// Initialize function.
		virtual bool Initialize() = 0;
		// Finalize function.
		virtual void Finalize() = 0;
		// Tick function, be called per frame.
		virtual void Tick(double deltaTime) = 0;
	};
}


