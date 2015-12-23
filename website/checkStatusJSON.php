<?php
 header("Access-Control-Allow-Origin: *");
  require('func.php');
  $conf = loadconfig('doorlogger.config.php');
  $con = getdbhandle($conf);

  $sql="select * from doorEvents order by dtEventDate DESC limit 1";

  $res = mysql_query($sql);
  $row = mysql_fetch_array($res);
  if($row['bDoorIsOpen'] == 1) {
    echo '{ "isOpen" : 1 }';
  } else {
    echo '{ "isOpen" : 0 }';
  }
  mysql_close($con);
?>
