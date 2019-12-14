/********************************************************************************
** Form generated from reading UI file 'SakuraMainViewport.ui'
**
** Created by: Qt User Interface Compiler version 5.12.5
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SAKURAMAINVIEWPORT_H
#define UI_SAKURAMAINVIEWPORT_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFrame>

QT_BEGIN_NAMESPACE

class Ui_SakuraMainViewport
{
public:

    void setupUi(QFrame *SakuraMainViewport)
    {
        if (SakuraMainViewport->objectName().isEmpty())
            SakuraMainViewport->setObjectName(QString::fromUtf8("SakuraMainViewport"));
        SakuraMainViewport->resize(400, 300);

        retranslateUi(SakuraMainViewport);

        QMetaObject::connectSlotsByName(SakuraMainViewport);
    } // setupUi

    void retranslateUi(QFrame *SakuraMainViewport)
    {
        SakuraMainViewport->setWindowTitle(QApplication::translate("SakuraMainViewport", "SakuraMainViewport", nullptr));
    } // retranslateUi

};

namespace Ui {
    class SakuraMainViewport: public Ui_SakuraMainViewport {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SAKURAMAINVIEWPORT_H
