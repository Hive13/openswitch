<? 
  header("Content-Type: application/xml; charset=ISO-8859-1");
  
  require('func.php');
  $conf = loadconfig('doorlogger.config.php');
  $dbh = getdbhandle($conf);
  
  // We are going to create a very basic dummy RSS feed here
  $feed = '<?xml version="1.0" encoding="ISO-8859-1" ?>
            <rss version="2.0">
              <channel>
                <title>Hive13 Open Alerts</title>
                <link>http://www.hive13.org</link>
                <description>This feed publishes alerts about the
                             Hive13 hackerspace opening or closing.</description>
                <language>en-us</language>
                <pubDate>'.date(DATE_RSS).'</pubDate>';
  
  $sql = 'SELECT pk_DoorEventID, dtEventDate, bDoorIsOpen
          FROM doorEvents
          ORDER BY dtEventDate DESC';
  
  $res = mysql_query($sql, $dbh);
  
  while($row = mysql_fetch_array($res, MYSQL_ASSOC)) {
    $feed .= '<item>';
    
    if($row['bDoorIsOpen'] == 1)
      $feed .= '<title>Hive13 is Open!</title>';
    else
      $feed .= '<title>Hive13 is closed.</title>';
    
    $feed .= '<link>http://www.hive13.org</link>
              <description></description>
              <pubDate>'.date(DATE_RSS, strtotime($row['dtEventDate'])).'</pubDate>
            </item>';
  }
  $feed .= '</channel></rss>';
  echo $feed;
?> 