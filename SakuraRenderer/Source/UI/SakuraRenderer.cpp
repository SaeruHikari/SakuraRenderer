#include "SakuraRenderer.h"

SakuraRenderer::SakuraRenderer(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	MainViewPort = ui.MainViewport;
	GBufferViewport = ui.GBufferViewport;
	Viewports["FinalColor"] = MainViewPort;
	Viewports["GBuffer"] = GBufferViewport;
	ui.PropertiesDock->setWindowTitle("Properties");
}

bool SakuraRenderer::Initialize()
{
	assert(Viewports["FinalColor"] != nullptr);
	if (!GBufferViewport->Initialize()) return false;
	return MainViewPort->Initialize();
}

void SakuraRenderer::Tick(double deltaTime)
{
	if (MainViewPort)
		MainViewPort->Tick(deltaTime);
	else
		// Should not tick before UI init complete.
		assert(0);
}

void SakuraRenderer::Finalize()
{

}
