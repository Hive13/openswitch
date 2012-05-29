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

  if(isSet($_REQUEST['temp'])) {
    // Add whatever was passed in to the text file
    echo "Saving temperature...\n";
    $safeTemp = mysql_real_escape_string($_REQUEST['temp']);
    $qry = "INSERT INTO tempEvents (dcTemp, vcLocation)
            VALUES ($safeTemp, 'DoorSwitch')";
    $res = mysql_query($qry,$dbh);
    mysql_close($dbh);
    echo "$safeTemp";
  }

  if(isSet($_REQUEST['tempReport'])) {
    if($_REQUEST['tempReport'] == '1') {
      // if = 1, read the last temperature
      echo "Temperature Report:\n";
      $qry = "SELECT dtEventDate, dcTemp FROM tempEvents ORDER BY dtEventDate DESC LIMIT 75";
      $res = mysql_query($qry, $dbh);
      while( $row = mysql_fetch_array($res, MYSQL_ASSOC)) {
        echo $row['dtEventDate'].",".$row['dcTemp']."\n";
      }
      mysql_close($dbh);
    }
  }

?>
