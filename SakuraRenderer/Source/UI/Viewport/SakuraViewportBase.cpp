#include "SakuraViewportBase.h"
#include "SakuraCore.h"
#include "qevent.h"

SakuraViewportBase::SakuraViewportBase(QWidget *parent)
	: QFrame(parent)
{
	ui.setupUi(this);
	this->grabKeyboard();
}

SakuraViewportBase::~SakuraViewportBase()
{
}

void SakuraViewportBase::resizeEvent(QResizeEvent* event)
{
	MsgSakuraCore(SAKURA_CORE_COMPONENT_MASK_GRAPHICS,
		SAKURA_GRAPHICS_CORE_MSG_RESIZE,
		0, event->size().width(), event->size().height());

}

void SakuraViewportBase::keyPressEvent(QKeyEvent* event)
{

}

void SakuraViewportBase::keyReleaseEvent(QKeyEvent* event)
{

}

void SakuraViewportBase::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
		MsgSakuraCore(SAKURA_CORE_COMPONENT_MASK_GRAPHICS,
			SAKURA_GRAPHICS_CORE_MSG_MOUSEPRESS,
			SAKURA_INPUT_MOUSE_LBUTTON,
			event->x(), event->y());

	else if (event->button() == Qt::RightButton)
		MsgSakuraCore(SAKURA_CORE_COMPONENT_MASK_GRAPHICS,
			SAKURA_GRAPHICS_CORE_MSG_MOUSEPRESS,
			SAKURA_INPUT_MOUSE_RBUTTON,
			event->x(), event->y());
}

void SakuraViewportBase::mouseReleaseEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
		MsgSakuraCore(SAKURA_CORE_COMPONENT_MASK_GRAPHICS,
			SAKURA_GRAPHICS_CORE_MSG_MOUSERELEASE,
			SAKURA_INPUT_MOUSE_LBUTTON,
			event->x(), event->y());

	else if (event->button() == Qt::RightButton)
		MsgSakuraCore(SAKURA_CORE_COMPONENT_MASK_GRAPHICS,
			SAKURA_GRAPHICS_CORE_MSG_MOUSERELEASE,
			SAKURA_INPUT_MOUSE_RBUTTON,
			event->x(), event->y());
}

void SakuraViewportBase::mouseMoveEvent(QMouseEvent* event)
{
	if (event->buttons() & Qt::LeftButton)
		MsgSakuraCore(SAKURA_CORE_COMPONENT_MASK_GRAPHICS,
			SAKURA_GRAPHICS_CORE_MSG_MOUSEMOVE,
			SAKURA_INPUT_MOUSE_LBUTTON,
			event->x(), event->y());

	else if (event->buttons() & Qt::RightButton)
		MsgSakuraCore(SAKURA_CORE_COMPONENT_MASK_GRAPHICS,
			SAKURA_GRAPHICS_CORE_MSG_MOUSEMOVE,
			SAKURA_INPUT_MOUSE_RBUTTON,
			event->localPos().x(), event->localPos().y());
}

void SakuraViewportBase::wheelEvent(QWheelEvent* event)
{

}