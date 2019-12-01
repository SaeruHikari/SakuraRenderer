#pragma once

#include "SakuraViewportBase.h"
#include "ui_SakuraGBufferViewport.h"

class SakuraGBufferViewport : public SakuraViewportBase
{
	Q_OBJECT

public:
	SakuraGBufferViewport(QWidget *parent = Q_NULLPTR);
	~SakuraGBufferViewport();


	virtual bool Initialize() override;
	virtual void Tick(double deltaTime) override;
	virtual void Finalize() override;

private:
	Ui::SakuraGBufferViewport ui;
};
