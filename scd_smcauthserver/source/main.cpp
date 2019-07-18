#include <QCoreApplication>
#include <QWebSocketServer>
#include <QWebSocket>
#include <QSettings>

#include "scd_smartcardserver.h"

#define  echo QTextStream(stderr) <<

/**
 * @brief main
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char *argv[])
{
   QCoreApplication a(argc, argv);

   echo "SC-Develop Smart Card Web Socket Server v1.0\n";
   echo "Copyright (c) 2019 (MIT) Ing. Salvatore Cerami - dev.salvatore.cerami@gmail.com\n";
   echo "https://github.com/sc-develop - git.sc.develop@gmail.com\n\n";

   QSettings config(QCoreApplication::applicationDirPath() + "/config.cfg",QSettings::IniFormat);

   qint16 port = config.value("port",10522).toInt();

   config.setValue("port",port);

   config.sync();

   SCD_SmartCardServer server;

   if (server.start())
   {
     return a.exec();
   }

   return 0;
}
