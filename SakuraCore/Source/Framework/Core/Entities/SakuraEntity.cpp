#include "SakuraEntity.h"

bool SakuraCore::SakuraEntity::Initialize()
{
	return true;
}

void SakuraCore::SakuraEntity::Tick(double deltaTime)
{

}

void SakuraCore::SakuraEntity::Finalize()
{
	SakuraSceneNode::Finalize();
}
