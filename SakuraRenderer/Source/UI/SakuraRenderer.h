#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_SakuraRenderer.h"
#include "SakuraMainViewport.h"
#include "SakuraGBufferViewport.h"
#include <unordered_map>

class SakuraRenderer : public QMainWindow, SImplements ISTickObject
{
	Q_OBJECT

public:
	SakuraRenderer(QWidget *parent = Q_NULLPTR);

	virtual bool Initialize() override;
	virtual void Tick(double deltaTime) override;
	virtual void Finalize() override;

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
