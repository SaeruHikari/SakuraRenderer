#include "SakuraGBufferViewport.h"
#include "SakuraCore.h"
#include "qevent.h"

SakuraGBufferViewport::SakuraGBufferViewport(QWidget *parent)
	: SakuraViewportBase(parent)
{
	ui.setupUi(this);
}

SakuraGBufferViewport::~SakuraGBufferViewport()
{
}

bool SakuraGBufferViewport::Initialize()
{
	return true;
}

void SakuraGBufferViewport::Tick(double deltaTime)
{

}

void SakuraGBufferViewport::Finalize()
{

}
