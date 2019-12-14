/********************************************************************************
** Form generated from reading UI file 'SakuraGBufferViewport.ui'
**
** Created by: Qt User Interface Compiler version 5.12.5
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SAKURAGBUFFERVIEWPORT_H
#define UI_SAKURAGBUFFERVIEWPORT_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <sakuraviewportbase.h>

QT_BEGIN_NAMESPACE

class Ui_SakuraGBufferViewport
{
public:

    void setupUi(SakuraViewportBase *SakuraGBufferViewport)
    {
        if (SakuraGBufferViewport->objectName().isEmpty())
            SakuraGBufferViewport->setObjectName(QString::fromUtf8("SakuraGBufferViewport"));
        SakuraGBufferViewport->resize(400, 300);

        retranslateUi(SakuraGBufferViewport);

        QMetaObject::connectSlotsByName(SakuraGBufferViewport);
    } // setupUi

    void retranslateUi(SakuraViewportBase *SakuraGBufferViewport)
    {
        SakuraGBufferViewport->setWindowTitle(QApplication::translate("SakuraGBufferViewport", "SakuraGBufferViewport", nullptr));
    } // retranslateUi

};

namespace Ui {
    class SakuraGBufferViewport: public Ui_SakuraGBufferViewport {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SAKURAGBUFFERVIEWPORT_H
