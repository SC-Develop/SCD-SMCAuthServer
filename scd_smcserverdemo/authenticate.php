<?php
 
  require_once 'authenticate.cfg';   
    
  /**
   * Authenticate the user: by default return always 1 (authenticated). 
   *                        otherwise you should implement your own authentication procedure.
   * 
   * @param string $username
   * @param string $password
   * @param string $atr
   * @return 1: authentication success, 0:failure
   */
  function authenticateUser($username,$password,$atr)
  {
    // you can write here your own server side authentication procedure
    
    GLOBAL $validUsername,$validPassword,$validAtr;
    
    if ($username==$validUsername && $password==$validPassword && $atr==$validAtr)
    {  
      return 1;
    }
    
    return 0;
  }  
  
?>  
  

