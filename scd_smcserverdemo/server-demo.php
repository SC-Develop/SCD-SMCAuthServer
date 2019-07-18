<?php
  
  require_once 'authenticate.php';
  
  
  if (!empty($_POST))
  {
     $username = $_POST["username"];
     $password = $_POST["password"];
     $atr      = $_POST["atr"];
     
     if (!authenticateUser($username,$password,$atr))
     {
       header("location: server-demo.php");
       exit();
     }       
  }
  else 
  {
    $atr='';  
  }     
  
?>
<!doctype html> 

<html> 
    
<head>
    
  <title> Smart Card Authentication Demo Web Socket Client</title> 
  
  <script src="scd_smcauthclient.js" ></script> 
  
  <script src="server-demo.js" ></script>
  
  <script>
<?php
  
  if (!empty($_POST))
  {
    echo "login=true";
  }
  
?>  
  </script>
  
</head>

 <body onload="connect('<?=$atr?>')" style="font-family:sans-serif; margin:0px; background-color:#F0F0F0">
      
    <div style="background-color:#555555; color:#FFFFFF; margin:0px; padding:0.8em; padding-left:0.5em; border-radius:0px; border-bottom:1px solid #555555;">
      
      <h2 style="margin:0px; font-weight:normal;">SCD Smartcard Authentication Server - Demo</h2>   
        
    </div>        
      
    <div style="background-color:#888888; color:#FFFFFF; border-radius:0px;  padding:0.8em; padding-left:0.5em; margin:auto; margin-bottom:0em; border-bottom:1px solid #333333;">
        
      <button style="background-color: #555555; color:#FFFFFF; padding:0.5em; border-radius:3px; cursor:pointer; border:1px solid #333333;" type="button" onclick="client.getATR()"     >Get ATR Code     </button>
<?php
  
    if (empty($_POST))
    {
      
?>    <button style="background-color: #555555; color:#FFFFFF; padding:0.5em; border-radius:3px; cursor:pointer; border:1px solid #333333;" type="button" onclick="client.getServerType()">Check Server Type</button>
      <button style="background-color: #555555; color:#FFFFFF; padding:0.5em; border-radius:3px; cursor:pointer; border:1px solid #333333;" type="button" onclick="client.getLoginCode()" >Get Login Code   </button>
      <button style="background-color: #555555; color:#FFFFFF; padding:0.5em; border-radius:3px; cursor:pointer; border:1px solid #333333;" type="button" onclick="client.setAuthenticationCode(document.form1.atr.value)"    >Set Login Code   </button>
<?php
  
    }
?>    
      <button style="background-color: #555555; color:#FFFFFF; padding:0.5em; border-radius:3px; cursor:pointer; border:1px solid #333333;" type="button" onclick="client.checkAuthentication()"  >Check Login Code </button>
      <span style="color:#00FF00;padding-left:0.5em;padding-right:0.25em">Polling timer</span>
      <input style = "border-radius:3px; padding:0.3em; border:1px solid #333333; padding-left:0.25em; width:2em" id="timeout" value="1" />
      <button style="background-color: #555555; color:#FFFFFF; padding:0.5em; border-radius:3px; cursor:pointer; border:1px solid #333333;" type="button" onclick="client.setPollTimeout(document.getElementById('timeout').value)">Set Timeout</button>     
      
    </div>          
      
<?php
  
  if (empty($_POST))
  {
    
?>    
      
    <div style="width:31em;background-color: #337ab7; color:#FFFFFF; border-radius:3px; border:1px solid #333333; margin:0.5em; margin-bottom:1em; margin-top:1em; padding:1em">          
      <form name="form1" method="POST" action="server-demo.php">  
        <p style="margin-top: 0px;">Username</p>
        <input style = "border-radius:3px; padding:0.3em; border:1px solid #333333; padding-left:0.25em; width:98%" name="username" value="admin" />
        <p>Password</p>
        <input style = "border-radius:3px; padding:0.3em; border:1px solid #333333; padding-left:0.25em; width:98%" type="password" name="password" value="admin"/>
        <input type="hidden" name="atr" />
        <p id="reader">ATR:</p>          
        <div style="text-align:right">
          <button style="background-color: #555555; color:#FFFFFF; padding:0.5em; border-radius:3px; cursor:pointer; border:1px solid #333333;" type="submit" >
            Login
          </button>
        </div>
      </form>  
    </div>       
      
<?php
  
  }
  else
  {  
    
?> 
    <div id="auth" style="background-color:#337ab7; color:#FFFFFF; margin:0px; margin-bottom:0em; padding:0.8em; border-radius:0px; border-bottom:1px solid #555555;">
      <span>Authenticated user:</span> 
      <span id="username"><?=" ".$username;?></span>
      <span id="atr"><?=" - ATR: ".$atr;?></span>      
      <span style="text-align:right;float:right">
        <button onclick="location.href='server-demo.php'" style="cursor:pointer; padding:0.25em; border-radius:3px; border:1px solid #333333;" type="buttom" >
          Exit
        </button>
      <span>
    </div>        
     
      
<?php
  
  }
    
?>      
    <ul style="background-color:#FAFAFA; margin:0.5em; padding-top:1em; border-radius:0px; border:1px solid #333333; height:13em; overflow-y:scroll;" id="eventi_websocket"></ul>  
    
    <div style="background-color:#337ab7; color:#FFFFFF; margin:0px; padding:0.25em; border-radius:0px; border-top:1px solid #555555;">
      
      <span>Server Connection:</span><span style="color:orange; font-weight:bold;" id="data"> Opening...</span><span style="padding-left:0.5em;">|</span><span style="padding-left:0.5em;">Web Socket Error:</span><span style="color:orange; font-weight:bold; padding-left:0.5em" id="error">No error</span>
        
    </div>      
    
    <div style="background-color:#555555; color:#FFFFFF; margin:0px; padding:0.25em; border-radius:0px; border-top:1px solid #555555;">
      
      Copyright (c) 2019 Ing. Salvatore Cerami (MIT) - dev.salvatore.cerami@gmail.com - https://github.com/SC-Develop/SCD_SMCAuthServer - git.sc.develop@gmail.com
        
    </div>      
    
  </body> 
  
</html>
