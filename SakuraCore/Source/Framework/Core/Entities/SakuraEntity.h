/*****************************************************************************************
								 Copyrights   SaeruHikari
CreateDate:								2019.10.30
Description:				Sakura entity master class.
Details:			Entity is a concrete class that can spawn in the scene.
*******************************************************************************************/
#pragma once
#include "../Scene/SakuraSceneNode.hpp"

namespace SakuraCore
{
	class SakuraEntity : public SakuraSceneNode
	{
		friend class SakuraEntityFactory;
		// Cause this is a concrete class, the interfaces must be implemented.

	public:
		virtual bool Initialize() override;
		virtual void Tick(double deltaTime) override;
		virtual void Finalize() override;
	};
}
