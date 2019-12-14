/********************************************************************************
** Form generated from reading UI file 'SakuraRenderer.ui'
**
** Created by: Qt User Interface Compiler version 5.12.5
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SAKURARENDERER_H
#define UI_SAKURARENDERER_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QToolBox>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <SakuraGBufferViewport.h>
#include <SakuraMainViewport.h>

QT_BEGIN_NAMESPACE

class Ui_SakuraRendererClass
{
public:
    QWidget *centralWidget;
    QVBoxLayout *verticalLayout;
    QTabWidget *tabWidget;
    QWidget *FinalColorLabel;
    QVBoxLayout *verticalLayout_2;
    SakuraMainViewport *MainViewport;
    QWidget *GBufferLabel;
    QVBoxLayout *verticalLayout_6;
    SakuraGBufferViewport *GBufferViewport;
    QDockWidget *TopDock;
    QWidget *dockWidgetContents_5;
    QPushButton *pushButton;
    QDockWidget *PropertiesDock;
    QWidget *PropertiesWidget;
    QVBoxLayout *verticalLayout_3;
    QToolBox *PropertiesToolBox;
    QWidget *DescriptionPage;
    QVBoxLayout *verticalLayout_5;
    QGridLayout *gridLayout_2;
    QLabel *label_5;
    QLabel *label_4;
    QSpacerItem *verticalSpacer_2;
    QWidget *TransormPage;
    QVBoxLayout *verticalLayout_4;
    QGridLayout *gridLayout;
    QLineEdit *LocationY;
    QLineEdit *RotationZ;
    QLineEdit *LocationZ;
    QLineEdit *ScaleX;
    QLabel *label;
    QLineEdit *RotationX;
    QLineEdit *RotationY;
    QLineEdit *ScaleY;
    QLabel *label_3;
    QLabel *label_2;
    QLineEdit *ScaleZ;
    QLineEdit *LocationX;
    QSpacerItem *verticalSpacer;
    QWidget *MaterialPage;
    QDockWidget *ContentDock;
    QWidget *ContentWidget_2;
    QDockWidget *HierarchyDock;
    QWidget *dockWidgetContents;

    void setupUi(QMainWindow *SakuraRendererClass)
    {
        if (SakuraRendererClass->objectName().isEmpty())
            SakuraRendererClass->setObjectName(QString::fromUtf8("SakuraRendererClass"));
        SakuraRendererClass->resize(1375, 786);
        SakuraRendererClass->setStyleSheet(QString::fromUtf8("background-color: rgb(30, 30, 30);\n"
"color: rgb(155, 185, 255);\n"
"font: 12pt \"Bahnschrift Condensed\";"));
        centralWidget = new QWidget(SakuraRendererClass);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(centralWidget->sizePolicy().hasHeightForWidth());
        centralWidget->setSizePolicy(sizePolicy);
        centralWidget->setMinimumSize(QSize(800, 0));
        centralWidget->setMaximumSize(QSize(16777215, 16777215));
        verticalLayout = new QVBoxLayout(centralWidget);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        tabWidget = new QTabWidget(centralWidget);
        tabWidget->setObjectName(QString::fromUtf8("tabWidget"));
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(tabWidget->sizePolicy().hasHeightForWidth());
        tabWidget->setSizePolicy(sizePolicy1);
        tabWidget->setMinimumSize(QSize(780, 0));
        tabWidget->setMaximumSize(QSize(16777215, 16777215));
        tabWidget->setStyleSheet(QString::fromUtf8("background-color: rgb(70, 70, 80);\n"
"color: rgb(155, 185, 255);\n"
"font: 12pt \"Bahnschrift Condensed\";"));
        FinalColorLabel = new QWidget();
        FinalColorLabel->setObjectName(QString::fromUtf8("FinalColorLabel"));
        sizePolicy1.setHeightForWidth(FinalColorLabel->sizePolicy().hasHeightForWidth());
        FinalColorLabel->setSizePolicy(sizePolicy1);
        FinalColorLabel->setMinimumSize(QSize(780, 0));
        FinalColorLabel->setMaximumSize(QSize(16777215, 16777215));
        verticalLayout_2 = new QVBoxLayout(FinalColorLabel);
        verticalLayout_2->setSpacing(63);
        verticalLayout_2->setContentsMargins(11, 11, 11, 11);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        verticalLayout_2->setContentsMargins(0, 0, 0, 0);
        MainViewport = new SakuraMainViewport(FinalColorLabel);
        MainViewport->setObjectName(QString::fromUtf8("MainViewport"));
        sizePolicy1.setHeightForWidth(MainViewport->sizePolicy().hasHeightForWidth());
        MainViewport->setSizePolicy(sizePolicy1);
        MainViewport->setMinimumSize(QSize(770, 200));
        MainViewport->setMaximumSize(QSize(16777215, 16777215));
        MainViewport->setCursor(QCursor(Qt::ArrowCursor));
        MainViewport->setLayoutDirection(Qt::LeftToRight);
        MainViewport->setAutoFillBackground(false);
        MainViewport->setStyleSheet(QString::fromUtf8("background-color: rgb(60, 50, 60);\n"
"color: rgb(155, 185, 255);\n"
"font: 12pt \"Bahnschrift Condensed\";"));
        MainViewport->setFrameShape(QFrame::NoFrame);
        MainViewport->setFrameShadow(QFrame::Sunken);
        MainViewport->setLineWidth(0);
        MainViewport->setMidLineWidth(0);

        verticalLayout_2->addWidget(MainViewport);

        tabWidget->addTab(FinalColorLabel, QString());
        GBufferLabel = new QWidget();
        GBufferLabel->setObjectName(QString::fromUtf8("GBufferLabel"));
        GBufferLabel->setMaximumSize(QSize(16777215, 16777215));
        GBufferLabel->setStyleSheet(QString::fromUtf8("background-color: rgb(90, 90, 90);\n"
"color: rgb(155, 185, 255);\n"
"font: 12pt \"Bahnschrift Condensed\";"));
        verticalLayout_6 = new QVBoxLayout(GBufferLabel);
        verticalLayout_6->setSpacing(6);
        verticalLayout_6->setContentsMargins(11, 11, 11, 11);
        verticalLayout_6->setObjectName(QString::fromUtf8("verticalLayout_6"));
        verticalLayout_6->setContentsMargins(0, 0, 0, 0);
        GBufferViewport = new SakuraGBufferViewport(GBufferLabel);
        GBufferViewport->setObjectName(QString::fromUtf8("GBufferViewport"));
        sizePolicy1.setHeightForWidth(GBufferViewport->sizePolicy().hasHeightForWidth());
        GBufferViewport->setSizePolicy(sizePolicy1);
        GBufferViewport->setFrameShape(QFrame::StyledPanel);
        GBufferViewport->setFrameShadow(QFrame::Raised);

        verticalLayout_6->addWidget(GBufferViewport);

        tabWidget->addTab(GBufferLabel, QString());

        verticalLayout->addWidget(tabWidget);

        SakuraRendererClass->setCentralWidget(centralWidget);
        TopDock = new QDockWidget(SakuraRendererClass);
        TopDock->setObjectName(QString::fromUtf8("TopDock"));
        sizePolicy.setHeightForWidth(TopDock->sizePolicy().hasHeightForWidth());
        TopDock->setSizePolicy(sizePolicy);
        TopDock->setMinimumSize(QSize(834, 50));
        TopDock->setMaximumSize(QSize(524287, 50));
        TopDock->setAcceptDrops(true);
        TopDock->setStyleSheet(QString::fromUtf8("background-color: rgb(50, 50, 70);\n"
"color: rgb(255, 205, 225);\n"
"font: 9pt \"Adobe Gothic Std B\";"));
        TopDock->setFeatures(QDockWidget::NoDockWidgetFeatures);
        TopDock->setAllowedAreas(Qt::TopDockWidgetArea);
        dockWidgetContents_5 = new QWidget();
        dockWidgetContents_5->setObjectName(QString::fromUtf8("dockWidgetContents_5"));
        dockWidgetContents_5->setStyleSheet(QString::fromUtf8("background-color: rgb(50, 50, 70);\n"
"color: rgb(205, 205, 205);\n"
"font: 9pt \"Adobe Gothic Std B\";"));
        pushButton = new QPushButton(dockWidgetContents_5);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));
        pushButton->setGeometry(QRect(10, 0, 75, 23));
        QSizePolicy sizePolicy2(QSizePolicy::Minimum, QSizePolicy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(pushButton->sizePolicy().hasHeightForWidth());
        pushButton->setSizePolicy(sizePolicy2);
        pushButton->setStyleSheet(QString::fromUtf8("background-color: rgb(70, 70, 80);\n"
"color: rgb(155, 185, 255);\n"
"font: 11pt \"Bahnschrift Light SemiCondensed\";"));
        TopDock->setWidget(dockWidgetContents_5);
        SakuraRendererClass->addDockWidget(static_cast<Qt::DockWidgetArea>(4), TopDock);
        PropertiesDock = new QDockWidget(SakuraRendererClass);
        PropertiesDock->setObjectName(QString::fromUtf8("PropertiesDock"));
        QSizePolicy sizePolicy3(QSizePolicy::Minimum, QSizePolicy::Minimum);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(PropertiesDock->sizePolicy().hasHeightForWidth());
        PropertiesDock->setSizePolicy(sizePolicy3);
        PropertiesDock->setMinimumSize(QSize(280, 400));
        PropertiesDock->setStyleSheet(QString::fromUtf8("background-color: rgb(70, 70, 80);\n"
"color: rgb(155, 185, 255);\n"
"font: 12pt \"Bahnschrift Condensed\";"));
        PropertiesDock->setFeatures(QDockWidget::DockWidgetClosable|QDockWidget::DockWidgetMovable);
        PropertiesWidget = new QWidget();
        PropertiesWidget->setObjectName(QString::fromUtf8("PropertiesWidget"));
        QSizePolicy sizePolicy4(QSizePolicy::Minimum, QSizePolicy::Preferred);
        sizePolicy4.setHorizontalStretch(0);
        sizePolicy4.setVerticalStretch(0);
        sizePolicy4.setHeightForWidth(PropertiesWidget->sizePolicy().hasHeightForWidth());
        PropertiesWidget->setSizePolicy(sizePolicy4);
        PropertiesWidget->setMinimumSize(QSize(280, 0));
        verticalLayout_3 = new QVBoxLayout(PropertiesWidget);
        verticalLayout_3->setSpacing(6);
        verticalLayout_3->setContentsMargins(11, 11, 11, 11);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        PropertiesToolBox = new QToolBox(PropertiesWidget);
        PropertiesToolBox->setObjectName(QString::fromUtf8("PropertiesToolBox"));
        sizePolicy4.setHeightForWidth(PropertiesToolBox->sizePolicy().hasHeightForWidth());
        PropertiesToolBox->setSizePolicy(sizePolicy4);
        PropertiesToolBox->setMinimumSize(QSize(260, 0));
        DescriptionPage = new QWidget();
        DescriptionPage->setObjectName(QString::fromUtf8("DescriptionPage"));
        DescriptionPage->setGeometry(QRect(0, 0, 262, 374));
        sizePolicy4.setHeightForWidth(DescriptionPage->sizePolicy().hasHeightForWidth());
        DescriptionPage->setSizePolicy(sizePolicy4);
        DescriptionPage->setMinimumSize(QSize(260, 0));
        verticalLayout_5 = new QVBoxLayout(DescriptionPage);
        verticalLayout_5->setSpacing(6);
        verticalLayout_5->setContentsMargins(11, 11, 11, 11);
        verticalLayout_5->setObjectName(QString::fromUtf8("verticalLayout_5"));
        gridLayout_2 = new QGridLayout();
        gridLayout_2->setSpacing(6);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        label_5 = new QLabel(DescriptionPage);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        QSizePolicy sizePolicy5(QSizePolicy::Expanding, QSizePolicy::Preferred);
        sizePolicy5.setHorizontalStretch(0);
        sizePolicy5.setVerticalStretch(0);
        sizePolicy5.setHeightForWidth(label_5->sizePolicy().hasHeightForWidth());
        label_5->setSizePolicy(sizePolicy5);
        label_5->setScaledContents(false);
        label_5->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_2->addWidget(label_5, 0, 1, 1, 1);

        label_4 = new QLabel(DescriptionPage);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        sizePolicy4.setHeightForWidth(label_4->sizePolicy().hasHeightForWidth());
        label_4->setSizePolicy(sizePolicy4);
        label_4->setMinimumSize(QSize(50, 0));

        gridLayout_2->addWidget(label_4, 0, 0, 1, 1);


        verticalLayout_5->addLayout(gridLayout_2);

        verticalSpacer_2 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_5->addItem(verticalSpacer_2);

        PropertiesToolBox->addItem(DescriptionPage, QString::fromUtf8("Description"));
        TransormPage = new QWidget();
        TransormPage->setObjectName(QString::fromUtf8("TransormPage"));
        TransormPage->setGeometry(QRect(0, 0, 262, 374));
        sizePolicy4.setHeightForWidth(TransormPage->sizePolicy().hasHeightForWidth());
        TransormPage->setSizePolicy(sizePolicy4);
        TransormPage->setMinimumSize(QSize(260, 0));
        verticalLayout_4 = new QVBoxLayout(TransormPage);
        verticalLayout_4->setSpacing(6);
        verticalLayout_4->setContentsMargins(11, 11, 11, 11);
        verticalLayout_4->setObjectName(QString::fromUtf8("verticalLayout_4"));
        gridLayout = new QGridLayout();
        gridLayout->setSpacing(6);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        gridLayout->setHorizontalSpacing(0);
        gridLayout->setVerticalSpacing(2);
        LocationY = new QLineEdit(TransormPage);
        LocationY->setObjectName(QString::fromUtf8("LocationY"));
        QSizePolicy sizePolicy6(QSizePolicy::Ignored, QSizePolicy::Fixed);
        sizePolicy6.setHorizontalStretch(0);
        sizePolicy6.setVerticalStretch(0);
        sizePolicy6.setHeightForWidth(LocationY->sizePolicy().hasHeightForWidth());
        LocationY->setSizePolicy(sizePolicy6);
        LocationY->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(LocationY, 0, 2, 1, 1);

        RotationZ = new QLineEdit(TransormPage);
        RotationZ->setObjectName(QString::fromUtf8("RotationZ"));
        sizePolicy6.setHeightForWidth(RotationZ->sizePolicy().hasHeightForWidth());
        RotationZ->setSizePolicy(sizePolicy6);
        RotationZ->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(RotationZ, 1, 3, 1, 1);

        LocationZ = new QLineEdit(TransormPage);
        LocationZ->setObjectName(QString::fromUtf8("LocationZ"));
        sizePolicy6.setHeightForWidth(LocationZ->sizePolicy().hasHeightForWidth());
        LocationZ->setSizePolicy(sizePolicy6);
        LocationZ->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(LocationZ, 0, 3, 1, 1);

        ScaleX = new QLineEdit(TransormPage);
        ScaleX->setObjectName(QString::fromUtf8("ScaleX"));
        QSizePolicy sizePolicy7(QSizePolicy::Maximum, QSizePolicy::Fixed);
        sizePolicy7.setHorizontalStretch(0);
        sizePolicy7.setVerticalStretch(0);
        sizePolicy7.setHeightForWidth(ScaleX->sizePolicy().hasHeightForWidth());
        ScaleX->setSizePolicy(sizePolicy7);
        ScaleX->setMinimumSize(QSize(50, 0));
        ScaleX->setMaximumSize(QSize(50, 16777215));
        ScaleX->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(ScaleX, 2, 1, 1, 1);

        label = new QLabel(TransormPage);
        label->setObjectName(QString::fromUtf8("label"));
        sizePolicy3.setHeightForWidth(label->sizePolicy().hasHeightForWidth());
        label->setSizePolicy(sizePolicy3);

        gridLayout->addWidget(label, 0, 0, 1, 1);

        RotationX = new QLineEdit(TransormPage);
        RotationX->setObjectName(QString::fromUtf8("RotationX"));
        sizePolicy6.setHeightForWidth(RotationX->sizePolicy().hasHeightForWidth());
        RotationX->setSizePolicy(sizePolicy6);
        RotationX->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(RotationX, 1, 1, 1, 1);

        RotationY = new QLineEdit(TransormPage);
        RotationY->setObjectName(QString::fromUtf8("RotationY"));
        sizePolicy6.setHeightForWidth(RotationY->sizePolicy().hasHeightForWidth());
        RotationY->setSizePolicy(sizePolicy6);
        RotationY->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(RotationY, 1, 2, 1, 1);

        ScaleY = new QLineEdit(TransormPage);
        ScaleY->setObjectName(QString::fromUtf8("ScaleY"));
        sizePolicy7.setHeightForWidth(ScaleY->sizePolicy().hasHeightForWidth());
        ScaleY->setSizePolicy(sizePolicy7);
        ScaleY->setMinimumSize(QSize(50, 0));
        ScaleY->setMaximumSize(QSize(50, 16777215));
        ScaleY->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(ScaleY, 2, 2, 1, 1);

        label_3 = new QLabel(TransormPage);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        QSizePolicy sizePolicy8(QSizePolicy::Maximum, QSizePolicy::Preferred);
        sizePolicy8.setHorizontalStretch(50);
        sizePolicy8.setVerticalStretch(0);
        sizePolicy8.setHeightForWidth(label_3->sizePolicy().hasHeightForWidth());
        label_3->setSizePolicy(sizePolicy8);
        label_3->setMinimumSize(QSize(50, 0));

        gridLayout->addWidget(label_3, 2, 0, 1, 1);

        label_2 = new QLabel(TransormPage);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        gridLayout->addWidget(label_2, 1, 0, 1, 1);

        ScaleZ = new QLineEdit(TransormPage);
        ScaleZ->setObjectName(QString::fromUtf8("ScaleZ"));
        sizePolicy7.setHeightForWidth(ScaleZ->sizePolicy().hasHeightForWidth());
        ScaleZ->setSizePolicy(sizePolicy7);
        ScaleZ->setMinimumSize(QSize(50, 0));
        ScaleZ->setMaximumSize(QSize(50, 16777215));
        ScaleZ->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(ScaleZ, 2, 3, 1, 1);

        LocationX = new QLineEdit(TransormPage);
        LocationX->setObjectName(QString::fromUtf8("LocationX"));
        sizePolicy6.setHeightForWidth(LocationX->sizePolicy().hasHeightForWidth());
        LocationX->setSizePolicy(sizePolicy6);
        LocationX->setMinimumSize(QSize(20, 0));
        LocationX->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(LocationX, 0, 1, 1, 1);


        verticalLayout_4->addLayout(gridLayout);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_4->addItem(verticalSpacer);

        PropertiesToolBox->addItem(TransormPage, QString::fromUtf8("Transform"));
        MaterialPage = new QWidget();
        MaterialPage->setObjectName(QString::fromUtf8("MaterialPage"));
        MaterialPage->setGeometry(QRect(0, 0, 262, 374));
        sizePolicy4.setHeightForWidth(MaterialPage->sizePolicy().hasHeightForWidth());
        MaterialPage->setSizePolicy(sizePolicy4);
        MaterialPage->setMinimumSize(QSize(260, 0));
        PropertiesToolBox->addItem(MaterialPage, QString::fromUtf8("Material"));

        verticalLayout_3->addWidget(PropertiesToolBox);

        PropertiesDock->setWidget(PropertiesWidget);
        SakuraRendererClass->addDockWidget(static_cast<Qt::DockWidgetArea>(2), PropertiesDock);
        ContentDock = new QDockWidget(SakuraRendererClass);
        ContentDock->setObjectName(QString::fromUtf8("ContentDock"));
        ContentDock->setMinimumSize(QSize(61, 210));
        ContentDock->setStyleSheet(QString::fromUtf8("background-color: rgb(70, 70, 80);\n"
"color: rgb(155, 185, 255);\n"
"font: 12pt \"Bahnschrift Condensed\";"));
        ContentDock->setFeatures(QDockWidget::DockWidgetClosable|QDockWidget::DockWidgetMovable);
        ContentWidget_2 = new QWidget();
        ContentWidget_2->setObjectName(QString::fromUtf8("ContentWidget_2"));
        ContentDock->setWidget(ContentWidget_2);
        SakuraRendererClass->addDockWidget(static_cast<Qt::DockWidgetArea>(8), ContentDock);
        HierarchyDock = new QDockWidget(SakuraRendererClass);
        HierarchyDock->setObjectName(QString::fromUtf8("HierarchyDock"));
        HierarchyDock->setMinimumSize(QSize(270, 43));
        HierarchyDock->setStyleSheet(QString::fromUtf8("background-color: rgb(70, 70, 80);\n"
"color: rgb(155, 185, 255);\n"
"font: 12pt \"Bahnschrift Condensed\";"));
        HierarchyDock->setFeatures(QDockWidget::DockWidgetClosable|QDockWidget::DockWidgetMovable);
        dockWidgetContents = new QWidget();
        dockWidgetContents->setObjectName(QString::fromUtf8("dockWidgetContents"));
        HierarchyDock->setWidget(dockWidgetContents);
        SakuraRendererClass->addDockWidget(static_cast<Qt::DockWidgetArea>(1), HierarchyDock);

        retranslateUi(SakuraRendererClass);

        tabWidget->setCurrentIndex(0);
        PropertiesToolBox->setCurrentIndex(1);


        QMetaObject::connectSlotsByName(SakuraRendererClass);
    } // setupUi

    void retranslateUi(QMainWindow *SakuraRendererClass)
    {
        SakuraRendererClass->setWindowTitle(QApplication::translate("SakuraRendererClass", "SakuraRenderer", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(FinalColorLabel), QApplication::translate("SakuraRendererClass", "FinalColor", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(GBufferLabel), QApplication::translate("SakuraRendererClass", "GBuffer", nullptr));
        TopDock->setWindowTitle(QApplication::translate("SakuraRendererClass", "Sakura Renderer", nullptr));
        pushButton->setText(QApplication::translate("SakuraRendererClass", "PushButton", nullptr));
        PropertiesDock->setWindowTitle(QApplication::translate("SakuraRendererClass", "Properties", nullptr));
        label_5->setText(QApplication::translate("SakuraRendererClass", "ItemName", nullptr));
        label_4->setText(QApplication::translate("SakuraRendererClass", "Name", nullptr));
        PropertiesToolBox->setItemText(PropertiesToolBox->indexOf(DescriptionPage), QApplication::translate("SakuraRendererClass", "Description", nullptr));
        LocationY->setText(QApplication::translate("SakuraRendererClass", "0", nullptr));
        RotationZ->setText(QApplication::translate("SakuraRendererClass", "0", nullptr));
        LocationZ->setText(QApplication::translate("SakuraRendererClass", "0", nullptr));
        ScaleX->setText(QApplication::translate("SakuraRendererClass", "1", nullptr));
        label->setText(QApplication::translate("SakuraRendererClass", "Location", nullptr));
        RotationX->setText(QApplication::translate("SakuraRendererClass", "0", nullptr));
        RotationY->setText(QApplication::translate("SakuraRendererClass", "0", nullptr));
        ScaleY->setText(QApplication::translate("SakuraRendererClass", "1", nullptr));
        label_3->setText(QApplication::translate("SakuraRendererClass", "Scale", nullptr));
        label_2->setText(QApplication::translate("SakuraRendererClass", "Rotation", nullptr));
        ScaleZ->setText(QApplication::translate("SakuraRendererClass", "1", nullptr));
        LocationX->setText(QApplication::translate("SakuraRendererClass", "0", nullptr));
        PropertiesToolBox->setItemText(PropertiesToolBox->indexOf(TransormPage), QApplication::translate("SakuraRendererClass", "Transform", nullptr));
        PropertiesToolBox->setItemText(PropertiesToolBox->indexOf(MaterialPage), QApplication::translate("SakuraRendererClass", "Material", nullptr));
        ContentDock->setWindowTitle(QApplication::translate("SakuraRendererClass", "Content Browser", nullptr));
        HierarchyDock->setWindowTitle(QApplication::translate("SakuraRendererClass", "Hierarchy", nullptr));
    } // retranslateUi

};

namespace Ui {
    class SakuraRendererClass: public Ui_SakuraRendererClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SAKURARENDERER_H
