/**
 * @description lib function for Smart Card Auth Server Demo - atr.hmtl
 * 
 * https://github.com/SC-Develop/SCD_SMCAuthServer - git.sc.develop@gmail.com
 * 
 * Copyright (c) 2019 Ing. Salvatore Cerami (MIT) - dev.salvatore.cerami@gmail.com
 * 
 */

  var atrCode=0;
  var maxEle=9;
  var client;
  var login=false;
 
  /**
   * Remove older excess  elments from top of list. The list can contain at most maxElements element.
   * 
   * @param {int} maxElements 
   * @returns {undefined}
   */  
  clear = function(maxElements)
  {
     var ul = document.getElementById("eventi_websocket");

     var elements = document.getElementsByTagName("LI");

     var len = elements.length;

     if (len<maxElements)
     {
       return;
     }  

     for (n=0; n<(len-maxElements); n++)
     {
        ul.removeChild(elements[0]);
     }
  };

  /**
   * Append the text to list
   * 
   * @param {tstring} text
   * @returns {undefined}
   */
  function append(text)
  {
     var ul = document.getElementById("eventi_websocket");

     ul.insertAdjacentHTML('beforeend', "<li>" + text + "</li>");        
  };

  /**
   * Set the text of 'data' element to data
   * 
   * @param {type} data
   * @returns {undefined}
   */
  function setData(data)
  {
     var id = document.getElementById("data");

     id.innerHTML = data;
  }

  /**
   * Set the text of error element to 'error'
   * 
   * @param {string} error
   * @returns {undefined}
   */ 
  function setError(error)
  {
     var id = document.getElementById("error");

     id.innerHTML = error;
  }
 
    
  function onOpened()
  {
    setData(" Opened");

    append("Connection opened");

    client.getLoginCode();    
  }
  
  /**
   * Open connection to SCD Smart Card Authentication Server
   * @param {string} atr
   * @returns {undefined}
   */
  
  function connect(atr)
  {  
    client = new SCD_SmcAuthClient('localhost',10522); 
    
    client.onSmcError = function(e)
    {
       clear(maxEle); 
       
       append("Command: " + e.detail.command + " => error: " + e.detail.error);
       
       if (e.detail.command==="LOGINCODE")
       {
          var id = document.getElementById("reader");

          if (id)
          { 
             id.innerText = "ATR: ";

             document.form1.atr.value = ''; // set ATR authentication code into hiddeb field
          } 
       }
    };
        
    client.onGetATR = function(e)
    {
       clear(maxEle);  
       
       append("Command: " + e.detail.command + " => Atr: " + e.detail.atr);
       
       if (e.detail.command==="LOGINCODE")
       {
          var id = document.getElementById("reader");

          if (id)
          { 
              id.innerText = "ATR: " + e.detail.atr;

              document.form1.atr.value = e.detail.atr; // set ATR authentication code into hiddeb field
          }
       }    
    };
    
    client.onValidate = function(e)
    {
        clear(maxEle); 
        
        append("Command: " + e.detail.command + " => Validated: " + e.detail.validated);
        
        if (!e.detail.validated && login)
        {
           alert('Not validated: continue to logout!');
            
           window.location.href='server-demo.php';
        }    
    };
    
    client.onAuthenticate = function(e)
    {
        clear(maxEle); 
        
        append("Command: " + e.detail.command + " => Authenticated: " + e.detail.authenticated);
        
        if (e.detail.authenticated)
        {
          client.checkAuthentication();  
        }    
        
        if (!e.detail.authenticated && login)
        {
          alert('Not validated: continue to logout!');
            
          window.location.href='server-demo.php';
        }  
    };
    
    client.onSessionExpired = function(e)
    {
        clear(maxEle); 
        
        append("Command: " + e.detail.command + " => Session Expired");
        
        if (login)
        {
          alert('Session expired: continue to logout!');
            
          window.location.href='server-demo.php';  
        }    
    };
    
    client.onServerType = function(e)
    {
        clear(maxEle); 
        
        append("Command: " + e.detail.command + " => Type: " + e.detail.type);
    };
    
    client.onSetTimeout = function(e)
    {
        clear(maxEle); 
        
        append("Command: " + e.detail.command + " => Timeout: " + e.detail.timeout);
    };
    
    if (atr)
    {
        client.opened = function()
        {
           setData(" Opened");

           append("Connection opened");
           
           append("Set authentication code..."); 

           client.setAuthenticationCode(atr);    
        };
    }    
    else
    {    
       client.opened = function(e)
       {
          setData(" Opened");

          append("Connection opened");
          
          append("Get login code..."); 

          client.getLoginCode();    
       };
    }   
    
    client.closed = function(e)
    {
       setData("Closed");
       
       append("Connection closed");  
    };
    
    client.error = function(e)
    {
       let message; 
       
       if (!e.message) 
       {
         message = "Open connection failure";  
       }    
       else
       {
         message = e.message;  
       }
       
       setError(message); 
       
       append("Web socket error: " + message);  
    };
      
    client.open();
  }   