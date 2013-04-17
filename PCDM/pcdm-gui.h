/* PCDM Login Manager:
*  Written by Ken Moore (ken@pcbsd.org) 2012/2013
*  Copyright(c) 2013 by the PC-BSD Project
*  Available under the 3-clause BSD license
*/

#ifndef PCDMGUI_H
#define PCDMGUI_H

#include <QtGui/QWidget>
#include <QMessageBox>
#include <QFile>
#include <QGraphicsScene>
#include <QMainWindow>
#include <QToolBar>
#include <QToolButton>
#include <QMenu>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QLineEdit>
#include <QGridLayout>
#include <QSpacerItem>
#include <QProcessEnvironment>

#include "pcdm-backend.h"
#include "themeStruct.h"
#include "fancySwitcher.h"
#include "dialogKeyboard.h"
#include "dialogLocale.h"
#include "loginWidget.h"

#define TMPLANGFILE QString("/tmp/.PCDMLang")

class PCDMgui : public QMainWindow
{
    Q_OBJECT

public:
    PCDMgui();
    ~PCDMgui();
    void changeLang(QString code);
    void progInit();

private slots:
    void slotStartLogin(QString,QString);
    void slotLoginSuccess();
    void slotLoginFailure();
    void slotUserChanged(QString);
    void slotUserSelected(QString);
    void slotRestartComputer();
    void slotShutdownComputer();
    void slotClosePCDM();
    void slotChangeLocale();
    void slotChangeKeyboardLayout();
    void slotPushVirtKeyboard();    // Start xvkbd
    void slotLocaleChanged(QString);

private:
    //Objects
    LoginWidget* loginW;
    widgetKeyboard* wKey;
    widgetLocale* wLoc;
    QToolBar* toolbar;
    QAction *virtkeyboardButton, *localeButton, *keyboardButton;
    QToolButton *systemButton;
    QMenu* systemMenu;
    FancySwitcher* deSwitcher;

    QProcess* vkbd;
    ThemeStruct* currentTheme;

    QTranslator* m_translator;
    QString translationDir, lastUser, lastDE;
    
    QString hostname;
    //Functions
    void createGUIfromTheme();
    void retranslateUi();
    void loadTheme();
    void loadLastUser();
    void loadLastDE(QString);
    void saveLastLogin(QString, QString);

signals:
    void xLoginAttempt(QString, QString, QString, QString);

};

#endif // PCDMGUI_H
