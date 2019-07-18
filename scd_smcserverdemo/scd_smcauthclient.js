/**
 * @description SCD Smart Card Authentication Server: client connection management class
 *  
 * https://github.com/SC-Develop/SCD_SMCAuthServer - git.sc.develop@gmail.com
 * 
 * Copyright (c) 2019 Ing. Salvatore Cerami (MIT) - dev.salvatore.cerami@gmail.com
 * 
 */

  /**
   * 
   * @class SCD_SmcAuthClient manage the connection with SCD Smartcard Authentication Server
   */
  class SCD_SmcAuthClient 
  {
    constructor(address,port)
    {
      this.opened = function() {};
      this.closed = function() {};
      this.error  = function() {};
      
      this.onAuthenticate   = function(e){};
      this.onValidate       = function(e){};
      this.onSessionExpired = function(e){};
      this.onServerType     = function(e){};
      this.onSetTimeout     = function(e){};
      this.onGetATR         = function(e){};      
      this.onSmcError       = function(e){};
                              
      this.onmessage = function(event)
      {
         var message = event.data.split('|');

         var appo = this;

         message[0] = message[0].trim().toUpperCase(); // command 
         message[1] = message[1].trim().toUpperCase(); // reply message
         
         var messageItems = message[1].trim().split(":");
         
         if (messageItems.length>1) // if message contains data or smartcard error
         {
            if (messageItems[0]==="ATR") 
            {
               var atrCode = messageItems[1];
               
               var event = new CustomEvent("getatr", { detail: {command: message[0], atr: atrCode} });
                
               this.dispatchEvent(event);
            }
            else if (messageItems[0]==="TIMEOUT")
            {
               var seconds = messageItems[1]; 
               
               var event = new CustomEvent("settimeout", { detail: {command: message[0], timeout: seconds} });
                
               this.dispatchEvent(event);
            }    
            else if (messageItems[0]==="ERROR")
            {
               var smcError = message[1]; 
               
               var event = new CustomEvent("smcerror", { detail: {command: message[0], error: smcError} });
                
               this.dispatchEvent(event);
            }    
            
            return;
         }    
         
         if (message[1]==="SESSIONEXPIRED")
         {
            var event = new CustomEvent("sessionexpired", { detail: {command: message[0]} });
                
            this.dispatchEvent(event); 
         }
         else if (message[1]==="VALIDATED")
         {
            var event = new CustomEvent("validated", { detail: {command: message[0], validated: true} });
                
            this.dispatchEvent(event);  
         }
         else if (message[1]==="NOTVALIDATED")
         {
            var event = new CustomEvent("validated", { detail: {command: message[0], validated: false} });
                
            this.dispatchEvent(event);  
         }
         else if (message[1]==="AUTHENTICATED")
         {
            var event = new CustomEvent("authenticated", { detail: {command: message[0], authenticated: true} });
                
            this.dispatchEvent(event);  
         }
         else if (message[1]==="NOTAUTHENTICATED")
         {
            var event = new CustomEvent("authenticated", { detail: {command: message[0], authenticated: false} });
                
            this.dispatchEvent(event);  
         }
         else if (message[1]==="INTEGRATED" || message[1].toUpperCase()==="STANDALONE")
         {
            var event = new CustomEvent("servertype", { detail: {command: message[0], type: message[1]} });
                
            this.dispatchEvent(event);  
         }         
      };
      
      this.url = "ws://" + address +":" + port;   
    }

    open()
    {
       this.socket = new WebSocket(this.url);     
       
       this.socket.onopen    = this.opened;                 
       this.socket.onmessage = this.onmessage;                
       this.socket.onclose   = this.closed;                 
       this.socket.onerror   = this.error;           
       
       this.socket.addEventListener("smcerror"      , this.onSmcError);      
       this.socket.addEventListener("getatr"        , this.onGetATR);      
       this.socket.addEventListener("validated"     , this.onValidate);      
       this.socket.addEventListener("authenticated" , this.onAuthenticate);      
       this.socket.addEventListener("settimeout"    , this.onSetTimeout);      
       this.socket.addEventListener("sessionexpired", this.onSessionExpired);      
       this.socket.addEventListener("servertype"    , this.onServerType);      
    }
    
    sendCommand(command) 
    {
       if (this.socket.readyState===1)
       {  
         this.socket.send(command);   
         
         return 1; // command sent
       }  
      
       return 0; // socket is closed
    }
      
    /**
     * Require to the server the ATR code 
     * 
     * @returns {Number}
     */ 
     getATR()
     {    
        return this.sendCommand("ATRCODE:");   
     }

     /**
      * Require to the server the authentication ATR code for login
      * 
      * @returns {Number}
      */ 
     getLoginCode()
     {         
        return this.sendCommand("LOGINCODE:");
     }

     /**
      * Require to the server to authenticate the ATR code, and register it as 
      * current  authenticated code. If success, after this call if server reads 
      * from smart card an ATR code which not match to the authenticated code, 
      * unvalidates the authentication and emit NotAuthenticated and 
      * SessionExpired messages
      * 
      * @param {string} atr
      * @returns {Number}
     */
     setAuthenticationCode(atr)
     {
        return this.sendCommand("Authcode:"+atr);    
     }

     /**
      * check authentication
      * 
      * @returns {Number}
      */ 
     checkAuthentication()
     {
        return this.sendCommand("Checkcode:");         
     }

     /**
      * require server type
      * 
      * @returns {Number}
      */ 
     getServerType()
     {
        return this.sendCommand("Servertype:");        
     }
  
     /**
      * 
      * @param {String} timeout 
      * @returns {Number|undefined}
      */
     setPollTimeout(timeout)
     {
        return this.sendCommand("PollTimeout:" + timeout.trim());        
     }    
  }