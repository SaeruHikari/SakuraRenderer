/**************************************************************************************************************************
									 Copyrights   SaeruHikari
CreateDate:									2019.10.22
Description:			  Master class, base entity of a Sakura Application.
Details:		  Do the Common works for all platforms(windows only) and modes(renderer or editor)
****************************************************************************************************************************/
#pragma once
#include <QtWidgets/QApplication>
//
#include "../UI/SakuraRenderer.h"
#include <QTimer>
#include <QTime>
#include <windows.h>

// MSG loop source and application class.

class SakuraRendererApplication : public QApplication
{
	Q_OBJECT

public:
	SakuraRendererApplication(int argc, char* argv[]);
	SakuraRendererApplication() = delete;
	// ban copy construction.
	SakuraRendererApplication(const SakuraRendererApplication& rhs) = delete;
	// ban operator =
	SakuraRendererApplication& operator=(const SakuraRendererApplication& rhs) = delete;

public slots:
	void TickMaster();

public:
	virtual bool Initialize();
	virtual void Finalize();
	virtual void Tick(double deltaTime);
	virtual int Run();

	HWND MainWnd() const;

protected:
	bool InitRenderer();

	std::unique_ptr<QTimer> mGlobalTimer;
	std::unique_ptr<QTime> mGlobalClock;

	// Refresh rate
	UINT mUpdateRate = 0;

	std::unique_ptr<SakuraRenderer> mRendererWindowHandle;

	// Window
	QString mWindowTitle = "Sakura Renderer";
	bool mAppPaused = false;
	bool mMinimized = false;
	bool mMaximized = false;
	bool mResizing = false;
	bool mFullscreenState = false;

protected:
	static SakuraRendererApplication* mApp;
	
};