#pragma once

#include <QFrame>
#include "ui_SakuraViewportBase.h"

class SakuraViewportBase : public QFrame
{
	Q_OBJECT

public:
	SakuraViewportBase(QWidget *parent = Q_NULLPTR);
	~SakuraViewportBase();

private:
	Ui::SakuraViewportBase ui;
protected:
	virtual void resizeEvent(QResizeEvent* event);
	virtual void keyPressEvent(QKeyEvent* event);
	virtual void keyReleaseEvent(QKeyEvent* event);
	virtual void mousePressEvent(QMouseEvent* event);
	virtual void mouseReleaseEvent(QMouseEvent* event);
	virtual void mouseMoveEvent(QMouseEvent* event);
	virtual void wheelEvent(QWheelEvent* event);
};
