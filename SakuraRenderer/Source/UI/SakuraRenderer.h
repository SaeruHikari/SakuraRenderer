#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_SakuraRenderer.h"
#include "SakuraMainViewport.h"
#include "SakuraGBufferViewport.h"
#include <unordered_map>

class SakuraRenderer : public QMainWindow
{
	Q_OBJECT

public:
	SakuraRenderer(QWidget *parent = Q_NULLPTR);

	virtual bool Initialize();
	virtual void Tick(double deltaTime);
	virtual void Finalize();

	WId GetMainViewportWinId() {
		//assert(MainViewPort != nullptr);
		//return MainViewPort->winId();
	}

private:
	Ui::SakuraRendererClass ui;
	std::unordered_map<std::string, QWidget*> Viewports;
	SakuraMainViewport* MainViewPort = nullptr;
	SakuraGBufferViewport* GBufferViewport = nullptr;
};
