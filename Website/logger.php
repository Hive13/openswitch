<?php

  header('Content-Type: text/plain; charset=utf-8');
  
  require('func.php');
  $conf = loadconfig('doorlogger.config.php');
  $dbh = getdbhandle($conf);

  if(isSet($_REQUEST['switch'])) {
    if($_REQUEST['switch'] == '0' ||
       $_REQUEST['switch'] == '1') {
      echo "Event logged";
      // Insert a space closed
      
      $sql = "INSERT INTO doorEvents (bDoorIsOpen)
              VALUES (".$_REQUEST['switch'].")";
      $res = mysql_query($sql,$dbh);
      mysql_close($dbh);
      // Check result of the insert?
    } else {
      echo "Cool stuff coming here.";
      // Show a list of door transactions.
      // Date, Door Status
      /* $sql = "SELECT dtEventDate, bDoorIsOpen
              FROM doorEvents
              ORDER BY dtEventDate ASC";
      $res = mysql_query($sql,$dbh);
      while($row = mysql_fetch_array(*/
      
    }  
  }
?>
