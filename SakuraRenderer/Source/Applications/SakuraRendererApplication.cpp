#include "SakuraRendererApplication.h"
#include "..\SakuraCore\Source\SakuraCore.h"


SakuraRendererApplication::SakuraRendererApplication(int argc, char* argv[])
	:QApplication(argc, argv)
{
	// Timer
	assert(mGlobalTimer == nullptr);
	mGlobalTimer = std::make_unique<QTimer>(this);
	mGlobalTimer->setTimerType(Qt::PreciseTimer);
	mGlobalTimer->stop();

	// Clock
	assert(mGlobalClock == nullptr);
	mGlobalClock = std::make_unique<QTime>();

	assert(mApp == nullptr);
	mApp = this;

	// window
	mRendererWindowHandle = std::make_unique<SakuraRenderer>();
	mRendererWindowHandle->setWindowTitle(mWindowTitle);

}

void SakuraRendererApplication::TickMaster()
{
	static double mLastTime = 0;
	
	double current_time_s = mGlobalClock->currentTime().second();
	double current_time_ms = mGlobalClock->currentTime().msec();

	double deltaTime = (current_time_s + current_time_ms/1000.f) - mLastTime;
	
	Tick(deltaTime); 

	double fps = 1.f / deltaTime;
	QString fpsString = QString::number(fps);
	mRendererWindowHandle->setWindowTitle(mWindowTitle + QString("   fps: ") + fpsString);

	mLastTime = (current_time_s + current_time_ms / 1000.f);
}

SakuraRendererApplication* SakuraRendererApplication::mApp = nullptr;
bool SakuraRendererApplication::Initialize()
{
	CreateSakuraCore(SAKURA_DRAW_WITH_D3D12);
	// Initialize Renderer.
	if (!InitRenderer())
		return false;
	if (mGlobalTimer == nullptr)
		assert(0);
	connect(mGlobalTimer.get(), SIGNAL(timeout()), this, SLOT(TickMaster()));
	return true;
}

void SakuraRendererApplication::Finalize()
{

}

void SakuraRendererApplication::Tick(double deltaTime)
{
	mRendererWindowHandle->Tick(deltaTime);
}

int SakuraRendererApplication::Run()
{
	mRendererWindowHandle->show();
	int DeltaMs;
	if (mUpdateRate == 0) DeltaMs = 0;
	else DeltaMs = (int)(1000.f / (double)(mUpdateRate));
	mGlobalTimer->start(DeltaMs);
	return this->exec();
}

HWND SakuraRendererApplication::MainWnd() const
{
	return (HWND)(mRendererWindowHandle->winId());
}

bool SakuraRendererApplication::InitRenderer()
{
	return mRendererWindowHandle->Initialize();
}
