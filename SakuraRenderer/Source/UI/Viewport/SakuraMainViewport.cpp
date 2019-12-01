#include "SakuraMainViewport.h"
#include "SakuraCore.h"
#include "qevent.h"

SakuraMainViewport::SakuraMainViewport(QWidget* parent)
	: SakuraViewportBase(parent)
{
	ui.setupUi(this);


	setAttribute(Qt::WA_PaintOnScreen, true);

	setAttribute(Qt::WA_OpaquePaintEvent, true);

	setAttribute(Qt::WA_MSWindowsUseDirect3D, true);


}

SakuraMainViewport::~SakuraMainViewport()
{

}

bool SakuraMainViewport::Initialize()
{
	return InitSakuraGraphicsCore(HWND(this->winId()), this->size().width(), this->height());
}

void SakuraMainViewport::Tick(double deltaTime)
{
	TickSakuraCore(deltaTime);
}

void SakuraMainViewport::Finalize()
{

}



