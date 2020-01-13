#include "ISObject.h"

namespace SakuraCore
{
	int SakuraCore::ISSilentObject::Serialize()
	{
		return 2;
	}

	REFLECTION_EDITOR
	(
		registration::class_<ISSilentObject>("ISSilentObject")
		.constructor<>()
		.method("GetID", &ISSilentObject::GetID)
		.method("SetID", &ISSilentObject::SetID)
		.method("Serialize", &ISSilentObject::Serialize);
	)
}


