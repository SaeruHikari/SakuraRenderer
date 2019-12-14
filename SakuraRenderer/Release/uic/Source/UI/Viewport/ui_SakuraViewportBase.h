/********************************************************************************
** Form generated from reading UI file 'SakuraViewportBase.ui'
**
** Created by: Qt User Interface Compiler version 5.12.5
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SAKURAVIEWPORTBASE_H
#define UI_SAKURAVIEWPORTBASE_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFrame>

QT_BEGIN_NAMESPACE

class Ui_SakuraViewportBase
{
public:

    void setupUi(QFrame *SakuraViewportBase)
    {
        if (SakuraViewportBase->objectName().isEmpty())
            SakuraViewportBase->setObjectName(QString::fromUtf8("SakuraViewportBase"));
        SakuraViewportBase->resize(400, 300);

        retranslateUi(SakuraViewportBase);

        QMetaObject::connectSlotsByName(SakuraViewportBase);
    } // setupUi

    void retranslateUi(QFrame *SakuraViewportBase)
    {
        SakuraViewportBase->setWindowTitle(QApplication::translate("SakuraViewportBase", "SakuraViewportBase", nullptr));
    } // retranslateUi

};

namespace Ui {
    class SakuraViewportBase: public Ui_SakuraViewportBase {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SAKURAVIEWPORTBASE_H
