#include <QTranslator>
#include <qtsingleapplication.h>
#include <QtGui/QApplication>
#include <QMessageBox>
#include <QDebug>
#include <QFile>

#include "mainUI.h"

#ifndef PREFIX
#define PREFIX QString("/usr/local/")
#endif

int main(int argc, char ** argv)
{
    QtSingleApplication a(argc, argv);
    if( a.isRunning() )
      return !(a.sendMessage("show"));
    
    QTranslator translator;
    QLocale mylocale;
    QString langCode = mylocale.name();
    
    if ( ! QFile::exists(PREFIX + "/share/pcbsd/i18n/pc-devicemanager_" + langCode + ".qm" ) )  langCode.truncate(langCode.indexOf("_"));
    translator.load( QString("pc-devicemanager_") + langCode, PREFIX + "/share/pcbsd/i18n/" );
    a.installTranslator( &translator );
    qDebug() << "Locale:" << langCode;
    

    mainUI w;
    QObject::connect(&a, SIGNAL(messageReceived(const QString&)), &w, SLOT(slotSingleInstance()) );
    w.show();
    //Check for root
    if (0 != geteuid())
    {
        QMessageBox msg;
        msg.setText(w.tr("This application requires administrator privileges for operation."));
        msg.exec();
        exit(2);
    }
    int retCode = a.exec();
    return retCode;
}
