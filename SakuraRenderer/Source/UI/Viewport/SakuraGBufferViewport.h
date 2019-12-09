#pragma once
#include "SakuraViewportBase.h"
#include "ui_SakuraGBufferViewport.h"

class SakuraGBufferViewport : public SakuraViewportBase
{
	Q_OBJECT

public:
	SakuraGBufferViewport(QWidget *parent = Q_NULLPTR);
	~SakuraGBufferViewport();


	virtual bool Initialize() ;
	virtual void Tick(double deltaTime) ;
	virtual void Finalize() ;

private:
	Ui::SakuraGBufferViewport ui;
};
