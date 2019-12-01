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
	return BindSakuraDbgWnd(HWND(this->winId()), this->size().width(), this->height());
}

void SakuraGBufferViewport::Tick(double deltaTime)
{

}

void SakuraGBufferViewport::Finalize()
{

}
