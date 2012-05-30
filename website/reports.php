<? 
  header('Content-Type: text/plain; charset=utf-8');
  
  require('func.php');
  $conf = loadconfig('doorlogger.config.php');
  $dbh = getdbhandle($conf);
  
  $reportText = "";
  
  if(isSet($_REQUEST['temp'])) {
    $reportIndex = $_REQUEST['temp'];
    
    if($reportIndex == '0') {
      // Only show the latest temperature
      $qry = "SELECT dtEventDate, dcTemp FROM tempEvents ORDER BY dtEventDate DESC LIMIT 1";
    } else if($reportIndex == '1') {
      // Generate a report for the last 24 hours
      // TODO: 288 is rough, lets do an actual last 24 hours w/ dtEventDate
      $qry = "SELECT dtEventDate, dcTemp FROM tempEvents ORDER BY dtEventDate DESC LIMIT 288";
    } else if($reportIndex == '2') {
      // They want it all... so grab all temperatures, this time show order ASC
      $qry = "SELECT dtEventDate, dcTemp FROM tempEvents ORDER BY dtEventDate ASC";
    }
    $res = mysql_query($qry, $dbh);
    while( $row = mysql_fetch_array($res, MYSQL_ASSOC)) {
      echo $row['dtEventDate'].",".$row['dcTemp']."\n";
    }
    mysql_close($dbh);
  }
 
  
  echo $reportText;
?> 