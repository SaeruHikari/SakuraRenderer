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

	virtual bool Initialize() override;
	virtual void Tick(double deltaTime) override;
	virtual void Finalize() override;

private:
	Ui::SakuraMainViewport ui;
};
