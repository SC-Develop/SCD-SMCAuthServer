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

### Comandi Server:

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

- <b>Checkcode:</b>=> verifica la presenza della Smart Card ed esegue il controllo di validazione.

  Return reply:

    - <b>error string</b>     : the smart card is not detected  (is not inserted into reeader), or no readers is detected.

    - <b>Validated</b>        : authentication valid

    - <b>Notvalidated</b>     : the smart card <b>ATR</b> code do not match the <b>ATR</b> code validated, you should to                                     logout.

    - <b>NotAuthenticated</b> : currently not authenticated: you should to log in.
    
    - <b>SessionTimeout</b>   : you should to logout.
    
The validation check should be performad only after the authentication and validation of<b>ATR</b>code. It makes no sense to check the validations if you are not logged in.
If not validated or authenticated is safely and strictly  recomended to logout from your application andif need,  again log in. If thea are an error you con wait until <b>SessionTimeout</b> is issued. 


- <b>Servertype:</b>restituisce "Integrated", "Standalone"

## Flow Diagram

<img src="diagram/Diagramma1.png" />

## Come comunicare col server per eseguire la procedura di autenticazione validazione e controllo</u>

Si premette che la pagina web di login deve oltre i parametri quali username, password etc. deve contenere un ulteriore campo hidden che conterrà il codice ATR letto dalla Smart Card inserita nel lettore.

La pagina web di login invia al server smart card tramite uno script javascript il comando <b>"Logincode:"</b> , ed ottiene il codice ATR da impostare nel campo hidden.
Il codice ATR viene inviato al server WEB per l'autenticazione. Se l'utente viene autenticato (il codice ATR corrisponde (match) con gli altri parametri di login, il codice viene reinviato indietro dal server web all'interno della pagina web (dentro una variabile javascript o un campo hidden, o come attributo di un tag etc.) che viene caricata se il login ha  avuto esito positivo,
Al termine del caricamento della pagina web un script javascritp invia il comando <b>"Authcode:<ATR>"</b> al server di gestione della smartcard per la validazione: es. <b>"Authcode:3bff1800008131fe45006b05051017012101434e531031805e"</b> se il codice non è validato bisogna tornare alla pagina di login
Ad intervalli temporizzati, bisogna eseguire il controllo di validazione inviando il comando <b>"Checkcode:"</b> in caso di fallimento tornare al login, nel caso in cui per più di n-secondi non viene rilevata la smartcard od il lettore si torna al login.
Il controllo temporizzato non è necessario se si utilizza il browser proprietario, poichè se ne occupa il browser stesso.

Inoltre con il browser proprietario non è necessario controllare il valori restituiti da comando Authcode, poichè il browser stesso decide se disconnettersi o meno.
