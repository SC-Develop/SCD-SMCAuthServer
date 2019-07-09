# SCD_SMCAuthServer
Smart Card Authentication Server

## Description

The SCD Smart Card Authentication Server allow you to read ATR code from your Smart Card and use it for login to remote web server or to acces to your own application. 
You can Ask to Server to read ATR Code from smart Card and manage the authentication, to check the matching of current ATR code with the authenticated code, or to check if the smart card is inserted into you card reader.

The authentication server can be work in two modality:

- <b>standalone</b>
- <b>embedded</b> on your application

Can be used with a web browser if , on login, an additional level of security is required, or as security key for your own application. Also, this server can be used embedding it on your own application.

## How to work

When started, the server open the port 10552, and wait for web socket connection.
Web socket client can connects to the server by the connection string: <b>"ws://localhost:10522"</b>.

This version of server supports only the unsecure connection.

The server accepts commands for:

- smart card ATR code reading
- management of authentication, 
- login consistency check
- card detection

Also, the sever reply with an error string in case of the smart card is not detected  (is not inserted into reeader), or no readers is detected.

### Server Commands:

- <b>Logincode:</b>  

  Require the <b>ATR</b> code: the server reply sending the <b>ATR</b> code readed from smart card.
  After sending this command, the validation controls will fails: you will need to log in again, authenticate and validate.
  See next command.
  
- <b>Authcode:ATR </b>

  Send to server the <b>ATR</b> code you have using for login, if match the <br>ATR</b> code of smart card currently inserted   into reader, the login authentication code is validated: the server store this code for next validation checking.
  
  Return reply:

    - <b>error string</b>  : the smart card is not detected  (is not inserted into reeader), or no readers is detected.

    - <b>Validated</b>     : validation success

    - <b>Notvalidated</b>  : the smart card <b>ATR</b> code do not match the <b>ATR</b> code validated

    - <b>AlreadyLogged</b> : not to need to autheticate: already logged in

- <b>Checkcode:</b>

  Detect smart card (if is it inserted into reader) and perform the validation check

  Return reply:

    - <b>error string</b>     : the smart card is not detected  (is not inserted into reeader), or no readers is detected.

    - <b>Validated</b>        : authentication valid

    - <b>Notvalidated</b>     : the smart card <b>ATR</b> code do not match the <b>ATR</b> code validated, you should to                                     logout.

    - <b>NotAuthenticated</b> : currently not authenticated: you should to log in.
    
    - <b>SessionTimeout</b>   : you should to logout.
    
  The validation check should be performad only after the authentication and validation of<b>ATR</b>code. It makes no sense     to check the validations if you are not logged in.
  If not validated or authenticated is safely and strictly  recomended to logout from your application andif need,  again log   in. If thea are an error you con wait until <b>SessionTimeout</b> is issued. 


- <b>Servertype:</b> 

  Return reply:
  
    - <b>Integrated</b>
    - <b>Standalone</b>

## Flow Diagram

<img src="diagram/Diagramma1.png" />

## Test the Server

Download the project typing on your linux terminal

```
$ git clone https://github.com/sc-develop/scd_smcauthserver
```
Run Qt Creator, load the project file build and run the server.

To test the Smart Card Autentication Server you can load the <b>atr.html</b> page on your browser. 
Connect your smartcard  reader to the USB connector of your PC/Laptop  and insert the smartcard into the reader. 
Now you can test the server by clicking the buttons on atr.html web page.

## How to use the server for authentication

The typical use is authentication on a remote server such as, for example, a WEB portal.
First you need to read the ATR code to associate with your user.
You can get the <b>ATR</b> code by using the <b>atr.html</b> page how explained above 
Now you can associate the <b>ATR</b> code with the username and password of your user in the authentication table of the users of your database server

```
 username : Tex
 password : Willer
 atr      : 3bff1800008131fe45006b05051017012101434e531031805e
 ```

The explanation of authentication on remote server by web page is beyond the scope of this document.

Now you are ready to use the SCD Smart Card Authentication Server full services.

The typical authentication session is shown on the diagram below.

The login web page should contain the <b>ATR</b> hidden field. This field will be automatic filled with the <b>ATR</b> code readed from smartcard.

1) The login web page ask to the server the ATR code by sending the command <b>"Logincode:"</b>, and set the ATR hidden field    with the readed ATR code.

2) When the user submits the login,  the <b>ATR</b> code is sent to web server togheter  username and passwor for the            authentication. 
   If the authentication succeeded (the ATR code match username and password) the command <b>Authcode:<ATR></b> will be sent    to the smart card server to validate the ATR code. 
   if your ATR code is <b>3bff1800008131fe45006b05051017012101434e531031805e</b> the command sent to the server will be          <b>"Authcode:3bff1800008131fe45006b05051017012101434e531031805e"</b>. 
   If the validation fails you should to return to login page.
Ad intervalli temporizzati, bisogna eseguire il controllo di validazione inviando il comando <b>"Checkcode:"</b> in caso di fallimento tornare al login, nel caso in cui per più di n-secondi non viene rilevata la smartcard od il lettore si torna al login.
Il controllo temporizzato non è necessario se si utilizza il browser proprietario, poichè se ne occupa il browser stesso.

Inoltre con il browser proprietario non è necessario controllare il valori restituiti da comando Authcode, poichè il browser stesso decide se disconnettersi o meno.
