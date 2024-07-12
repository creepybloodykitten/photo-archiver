/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.7.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QPushButton *exit;
    QPushButton *minimaze;
    QPushButton *titlebar;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(950, 600);
        QSizePolicy sizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(MainWindow->sizePolicy().hasHeightForWidth());
        MainWindow->setSizePolicy(sizePolicy);
        MainWindow->setMouseTracking(true);
        MainWindow->setAutoFillBackground(false);
        MainWindow->setStyleSheet(QString::fromUtf8("\n"
"QMainWindow {\n"
"\n"
"	background-color: rgb(69, 69, 69);\n"
"}"));
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        centralwidget->setStyleSheet(QString::fromUtf8(""));
        exit = new QPushButton(centralwidget);
        exit->setObjectName("exit");
        exit->setGeometry(QRect(890, 0, 60, 30));
        exit->setStyleSheet(QString::fromUtf8("background-color: rgb(69, 69, 69);\n"
"font: 700 9pt \"Segoe UI\";\n"
"\n"
"color: rgb(255, 255, 255);"));
        exit->setFlat(true);
        minimaze = new QPushButton(centralwidget);
        minimaze->setObjectName("minimaze");
        minimaze->setGeometry(QRect(830, 0, 60, 30));
        minimaze->setStyleSheet(QString::fromUtf8("background-color: rgb(69, 69, 69);\n"
"font: 700 9pt \"Segoe UI\";\n"
"\n"
"color: rgb(255, 255, 255);"));
        minimaze->setFlat(true);
        titlebar = new QPushButton(centralwidget);
        titlebar->setObjectName("titlebar");
        titlebar->setGeometry(QRect(0, 0, 830, 30));
        titlebar->setAutoFillBackground(false);
        titlebar->setStyleSheet(QString::fromUtf8("QPushButton\n"
"{\n"
" text-align: left;\n"
"padding-left: 10px;\n"
"background-color:#454545;\n"
"border:none;\n"
"}\n"
"\n"
"QPushButton:hover:!pressed\n"
"{\n"
"background-color:#454545;\n"
"\n"
"}\n"
"\n"
"QPushButton:enabled\n"
"{\n"
"background-color:#454545;\n"
"\n"
"}\n"
"\n"
"QpushButton:disabled\n"
"{\n"
"background-color:#454545;\n"
"\n"
"}"));
        titlebar->setCheckable(false);
        titlebar->setAutoDefault(false);
        titlebar->setFlat(false);
        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 950, 25));
        menubar->setStyleSheet(QString::fromUtf8(""));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        statusbar->setSizeGripEnabled(false);
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        titlebar->setDefault(false);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "PhotoCompressor", nullptr));
        exit->setText(QCoreApplication::translate("MainWindow", "X", nullptr));
        minimaze->setText(QCoreApplication::translate("MainWindow", "__", nullptr));
        titlebar->setText(QCoreApplication::translate("MainWindow", "PhotoCompressor", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
