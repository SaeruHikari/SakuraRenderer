#pragma once
#include <QFrame>
#include "ui_SakuraMainViewport.h"
#include "SakuraViewportBase.h"

class SakuraMainViewport : public SakuraViewportBase
{
	Q_OBJECT

public:
	SakuraMainViewport(QWidget *parent = Q_NULLPTR);
	~SakuraMainViewport();

	virtual bool Initialize() ;
	virtual void Tick(double deltaTime) ;
	virtual void Finalize() ;

private:
	Ui::SakuraMainViewport ui;
};
