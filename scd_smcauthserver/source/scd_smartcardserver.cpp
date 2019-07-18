#include "scd_smartcardserver.h"

/**
 * @brief SCD_SmartCardServer::SCD_SmartCardServer
 * @param port
 * @param parent
 */

SCD_SmartCardServer::SCD_SmartCardServer(qint16 port, ServerType type, QObject *parent) : QObject(parent), type(type), port(port)
{
   messages.insert(SM_AUTHENTICATED   , "Authenticated");
   messages.insert(SM_NOTAUTHENTICATED, "NotAuthenticated");
   messages.insert(SM_VALIDATED,        "Validated");
   messages.insert(SM_NOTVALIDATED,     "NotValidated");
   messages.insert(SM_ALREADYAUTH,      "AlreadyAuthenticated");
   messages.insert(SM_SESSIONTIMEOUT,   "SessionExpired");
   messages.insert(SM_UNKNOWNCOMMAND,   "UnknownCommand");
   messages.insert(SM_INTEGRATED,       "Integrated");
   messages.insert(SM_STANDALONE,       "Standalone");
   messages.insert(SM_UNKNOWN,          "Unknown");

   commands.insert(C_SERVERTYPE, "SERVERTYPE");
   commands.insert(C_ATR,        "ATRCODE");
   commands.insert(C_LOGIN,      "LOGINCODE");
   commands.insert(C_CHECK,      "CHECKCODE");
   commands.insert(C_AUTH,       "AUTHCODE");
   commands.insert(C_TIMEOUT,    "POLLTIMEOUT");

   connect(&pollTimer,SIGNAL(timeout()),this,SLOT(onPolling()));

   pollTimer.setInterval(1500);
}

/**
 * @brief SCD_SmartCardServer::start
 * @return
 */
int SCD_SmartCardServer::start()
{
   cardServer = new QWebSocketServer("ATR",QWebSocketServer::NonSecureMode,NULL);

   connect(cardServer, SIGNAL(newConnection()), this, SLOT(onConnect()));
   connect(cardServer, SIGNAL(closed()),        cardServer, SLOT(deleteLater()));

   cardServer->setMaxPendingConnections(1);

   if (cardServer->listen(QHostAddress::LocalHost,port))
   {
      lastError =  "Server listening on port: " + QString::number(port) + "\n";

      qDebug() << lastError;

      return 1;
   }

   lastError = "unable to start server omn port: " + QString::number(port) + "\n";

   qDebug() << lastError;

   return 1;
}

/**
 * @brief SCD_SmartCardServer::stop
 * @return
 */
void SCD_SmartCardServer::stop()
{
   cardServer->close();
}

/**
 * @brief SCD_SmartCardServer::onConnect
 */
void SCD_SmartCardServer::onConnect()
{
   socket = cardServer->nextPendingConnection(); // get the new connection

   connect(socket, SIGNAL(textMessageReceived(QString)), this, SLOT(onCheckCardMessageReceived(QString)));
   connect(socket, SIGNAL(disconnected()), socket, SLOT(deleteLater()));

   lastCardError.clear();

   lastPollStatus = SM_UNKNOWN;
   pollMode   = PM_NONE;
}

/**
 * @brief SCD_SmartCardServer::onCheckCardMessageReceived
 * @param message
 */
void SCD_SmartCardServer::onCheckCardMessageReceived(const QString &message)
{
   QWebSocket *socket = static_cast<QWebSocket *>(sender());

   currentPollMode = pollMode;

   pollMode       = PM_NONE;
   lastPollStatus = SM_UNKNOWN;

   lastCardError.clear();

   messageParse(socket, message);
}

/**
 * @brief SCD_SmartCardServer::messageParse
 * @param socket
 * @param message
 */
void SCD_SmartCardServer::messageParse(QWebSocket *socket, const QString &message)
{
   QByteArray code;

   int err;

   qDebug() << "Message Received: " << message << "\n";

   QStringList msg=message.toUpper().split(":");

   if (msg.count()!=2)
   {
      qDebug() << messages.at(SM_UNKNOWNCOMMAND) << "\n";

      emit error(msg[0],messages.at(SM_UNKNOWNCOMMAND));

      socket->close(); // close suspect connection

      return;
   }

   // Require to change polling tmeout interval -------------------------------------------

   if (msg[0]==commands.at(C_TIMEOUT))
   {
      int msec = msg[1].trimmed().toInt() * 1000;

      if (msec>=0)
      {
         pollTimer.stop(); // pause current polling (if any)

         pollTimer.setInterval(msec);
         socket->sendTextMessage(msg[0] + "|Timeout: " + msg[1].trimmed());  // reply to client

         restartPolling();
      }
      else
      {
         socket->sendTextMessage(msg[0] + "|ERROR:invalid timeout => " + msg[1].trimmed());  // send server type to client
      }

      return;
   }

   // Require server type ------------------------------------------------------------------

   if (msg[0]==commands.at(C_SERVERTYPE))
   {
      pollTimer.stop();

      emit serverType(type);

      if (type==ST_STANDALONE)
      {
         qDebug() << messages.at(SM_STANDALONE) << "\n";

         socket->sendTextMessage(msg[0] + "|" + messages.at(SM_STANDALONE));  // send server type to client
      }
      else
      if (type==ST_INTEGRATED)
      {
         qDebug() << messages.at(SM_INTEGRATED) << "\n";

         socket->sendTextMessage(msg[0] + "|" + messages.at(SM_INTEGRATED)); // send server type to client
      }
      else
      {
         qDebug() << messages.at(SM_UNKNOWN) << "\n";

         socket->sendTextMessage(msg[0] + "|ERROR:" + messages.at(SM_UNKNOWN));
      }

      restartPolling();

      return;
   }

   // Read the ATR code (for diagnostic use, or code detection)

   if (msg[0]==commands.at(C_ATR))
   {
      //stopPolling();

      pollTimer.stop();

      data = cardReader.CheckCard();

      if (data.atrvalid)           // if readed ATR code is valid
      {
         code = getCardCode(&data,&err);

         qDebug() << "Login: " << code << "\n";

         socket->sendTextMessage(msg[0] + "|atr:" + code); // send ATR to client
      }
      else
      {
         qDebug() << data.errmsg << "\n";

         emit error(msg[0],data.errmsg);

         socket->sendTextMessage(msg[0] + "|" + data.errmsg);
      }

      restartPolling();

      return;
   }

   // Require ATR authentication code -----------------------------------------------------

   if (msg[0]==commands.at(C_LOGIN))
   {
      pollTimer.stop();

      data = cardReader.CheckCard();

      resetAuthentication();  // unvalidate authentication

      emit status(msg[0], SM_SESSIONTIMEOUT, true); // emit session timeout signal

      if (data.atrvalid) // if readed ATR is valid
      {
         code = getCardCode(&data,&err);

         qDebug() << "Login: " << code << "\n";

         emit loginCode(code);

         if (lastPollStatus!=SM_SESSIONTIMEOUT)
         {
            socket->sendTextMessage(msg[0] + "|" + messages.at(SM_SESSIONTIMEOUT)); // send login ATR to client
            socket->sendTextMessage(msg[0] + "|" + "atr:" + code); // send login ATR to client
         }

         lastPollStatus = SM_SESSIONTIMEOUT;
         lastError.clear();
      }
      else
      {
         qDebug() << data.errmsg << "\n";

         emit error(msg[0],data.errmsg);

         if (lastPollStatus!=SM_ERROR || (lastPollStatus==SM_ERROR && lastError!=data.errmsg))
         {
            socket->sendTextMessage(msg[0] + "|" + messages.at(SM_SESSIONTIMEOUT)); // send login ATR to client
            socket->sendTextMessage(msg[0] + "|" + data.errmsg);
         }

         lastPollStatus = SM_ERROR;
         lastError  = data.errmsg;
      }

      startPolling(PM_LOGIN);

      return;
   }

   // Authentication check ----------------------------------------------------------------

   if (msg[0]==commands.at(C_CHECK))
   {
      pollTimer.stop();

      if (isAuthenticated)
      {
         data = cardReader.CheckCard();

         code = getCardCode(&data, &err); // Get Hex ATR code

         if (data.atrvalid)   // if readed ATR is valid
         {
            timer=0;

            if (atr==code)    // if readed ATR match authenticated ATR
            {
               qDebug() << "Check success => Card code: " << code << " => " << atr << "\n";

               emit status(msg[0],SM_VALIDATED,false);

               if (lastPollStatus!=SM_VALIDATED)
               {
                  socket->sendTextMessage(msg[0] + "|" + messages.at(SM_VALIDATED));
               }

               lastPollStatus = SM_VALIDATED;
               lastError.clear();
            }
            else // validation failure
            {
               qDebug() << "Check failure => Card code: " << code << " => " << atr << "\n";

               resetAuthentication(); // unvalidate authentication

               emit status(msg[0], SM_NOTVALIDATED, true);

               if (lastPollStatus!=SM_NOTVALIDATED)
               {
                  socket->sendTextMessage(msg[0] + "|" + messages.at(SM_NOTVALIDATED));
                  socket->sendTextMessage(msg[0] + "|" + messages.at(SM_SESSIONTIMEOUT));
               }

               lastPollStatus = SM_NOTVALIDATED;
               lastError.clear();
            }
         }
         else  // on reading card error
         {
            timer++;

            if (timer>3) // wait tree times
            {
               timer=0;

               resetAuthentication(); // unvalidate authentication            

               qDebug() << messages.at(SM_SESSIONTIMEOUT) << "\n";

               emit status(msg[0], SM_SESSIONTIMEOUT, true);

               if (lastError!=SM_SESSIONTIMEOUT)
               {
                  socket->sendTextMessage(msg[0] + "|" + messages.at(SM_SESSIONTIMEOUT));
               }

               lastPollStatus = SM_SESSIONTIMEOUT;
               lastError.clear();;
            }
            else
            {
               qDebug() << "Check error" << data.errmsg <<"\n";

               QString errMsg = QString(data.errmsg) + " (" + QString::number(timer) + ")";

               emit error(msg[0],errMsg);

               //if (lastPollStatus!=SM_ERROR || (lastPollStatus==SM_ERROR && lastError!=data.errmsg))
               //{
                  socket->sendTextMessage(msg[0] + "|" + errMsg);
               //}

               lastPollStatus = SM_ERROR;
               lastError  = data.errmsg;
            }
         }

         startPolling(PM_CHECK);
      }
      else
      {
         // if it is not authenticated, do not start again the polling,
         // becose the auhentication do not change if it do not to try to authenticate again.
         // the client must be send a LOGINCODE command.

         qDebug() << messages.at(SM_NOTAUTHENTICATED) << "\n";

         emit status(msg[0],SM_NOTAUTHENTICATED,true);

         if (lastPollStatus!=SM_NOTAUTHENTICATED)
         {
            socket->sendTextMessage(msg[0] + "|" + messages.at(SM_NOTAUTHENTICATED));
         }

         lastPollStatus = SM_NOTAUTHENTICATED;
         lastError.clear();         
      }

      return;
   }

   // Require to authenticate ATR code --------------------------------------------------

   if (msg[0]==commands.at(C_AUTH))
   {
      stopPolling();

      if (isAuthenticated)
      {
         qDebug() << messages.at(SM_ALREADYAUTH) << "\n";

         emit status(msg[0],SM_ALREADYAUTH,false);

         socket->sendTextMessage(msg[0] + "|" + messages.at(SM_ALREADYAUTH));
      }
      else  // try to authenticate
      {
         data = cardReader.CheckCard();

         code = getCardCode(&data, &err);

         if (data.atrvalid)   // reading ATR code success
         {
            if (msg[1]==code) // authentication success
            {
               atr = code;

               isAuthenticated = 1;

               qDebug() << messages.at(SM_AUTHENTICATED) << "\n";

               emit status(msg[0],SM_AUTHENTICATED,false);

               socket->sendTextMessage(msg[0] + "|" + messages.at(SM_AUTHENTICATED));
            }
            else
            {
               resetAuthentication();

               qDebug() << messages.at(SM_NOTAUTHENTICATED) << "\n";

               emit status(msg[0],SM_NOTAUTHENTICATED,true);

               socket->sendTextMessage(msg[0] + "|" + messages.at(SM_NOTAUTHENTICATED));
            }
         }
         else // on reading card error
         {
            qDebug() << data.errmsg << "\n";

            emit error(msg[0],data.errmsg);

            socket->sendTextMessage(msg[0] + "|" + data.errmsg);

            socket->sendTextMessage(msg[0] + "|" + messages.at(SM_NOTAUTHENTICATED));
         }
      }

      return;
   }

   qDebug() << messages.at(SM_UNKNOWNCOMMAND) << "\n";

   emit error(msg[0],messages.at(SM_UNKNOWNCOMMAND));

   socket->close(); // close suspect connection
}

/**
 * @brief SCD_SmartCardServer::onPolling
 */
void SCD_SmartCardServer::onPolling()
{
   switch (pollMode)
   {
      case PM_LOGIN:
        messageParse(socket,commands[C_LOGIN] + ":");
      break;

      case PM_CHECK:
        messageParse(socket,commands[C_CHECK] + ":");
      break;

      default:
        stopPolling();
      break;
   }
}

/**
 * @brief SCD_SmartCardServer::getCardCode
 * @param data
 * @param err
 * @return
 */
QByteArray SCD_SmartCardServer::getCardCode(SCD_PCSC::card_data *data, int *err)
{
   QByteArray cdata;

   *err = data->error;

   if (data->error)
   {
      cdata = data->errmsg;

      return cdata;
   }

   cdata = QByteArray::fromRawData((char *)data->data, data->datalen);

   return cdata.toHex().toUpper();
}

/**
 * @brief SCD_SmartCardServer::resetAuthentication
 */
void SCD_SmartCardServer::resetAuthentication()
{
   isAuthenticated = 0;
   atr = "";
}

/**
 * @brief SCD_SmartCardServer::startPolling
 * @param mode
 */
void SCD_SmartCardServer::startPolling(SCD_SmartCardServer::PollingMode mode)
{
   if (mode!=PM_NONE)
   {
      pollMode  = mode;
      pollTimer.setSingleShot(true);

      if (pollTimer.interval()>0)
      {
         pollTimer.start();
      }
      else
      {
         pollTimer.stop();
      }
   }
}

/**
 * Restart paused current polling
 *
 * @brief SCD_SmartCardServer::restartPolling
 */
void SCD_SmartCardServer::restartPolling()
{
  startPolling(currentPollMode);
}

/**
 * @brief SCD_SmartCardServer::stopPolling
 */
void SCD_SmartCardServer::stopPolling()
{
   pollTimer.stop();    // stop polling
   lastPollStatus = SM_UNKNOWN;
   pollMode = PM_NONE;   
   lastCardError.clear();
}
