<? 
  header("Content-Type: application/xml; charset=ISO-8859-1");
  
  require('func.php');
  
  $RSS_title="Hive13 RSS Feed";
  $RSS_description="RSS Feed for Hive13 events";
  $RSS_items=""; // Will be populated once we determine feed type.
    
  $conf = loadconfig('doorlogger.config.php');
  $dbh = getdbhandle($conf);

  if(isSet($_REQUEST['temp'])) {
    $reportIndex = $_REQUEST['temp'];
    $RSS_title="Hive13 Temperature Report";
    if($reportIndex == '0') { 
      $RSS_description="Feed of the Hive13 temperature for the last 24 hours.";
    
      $sql = "SELECT dtEventDate, dcTemp FROM tempEvents WHERE dtEventDate > DATE_SUB(NOW(), INTERVAL 24 HOUR) ORDER BY dtEventDate DESC";
    } else if($reportIndex == '1') {
      $RSS_description="Feed of hourly average temperatures for the last 24 hours.";
  
      $sql = "SELECT AVG(dcTemp) dcTemp, HOUR(dtEventDate) HourOfDay, dtEventDate, count(*) count 
              FROM tempEvents 
              WHERE DATE_SUB( dtEventDate, INTERVAL 1 HOUR ) and dtEventDate > DATE_SUB(NOW(), INTERVAL 24 HOUR) 
              GROUP BY HOUR(dtEventDate), DATE(dtEventDate) 
              ORDER BY dtEventDate DESC";
    }

    $res = mysql_query($sql, $dbh);

    while($row = mysql_fetch_array($res, MYSQL_ASSOC)) {
      $RSS_items .= '<item>';
      $RSS_items .= '  <title>'.$row['dcTemp'].'</title>';
      $RSS_items .= '  <link>http://www.hive13.org</link>';
      $RSS_items .= '  <description></description>';
      $RSS_items .= '  <pubDate>'.date(DATE_RSS, strtotime($row['dtEventDate'])).'</pubDate>';
      $RSS_items .= '</item>';
    }
  } else {
    $RSS_title="Hive13 Open Alerts";
    $RSS_description="This feed publishes alerts about the Hive13 hackerspace opening or closing.";
    // Generate the old, standard Open // Closed feed
    $sql = 'SELECT pk_DoorEventID, dtEventDate, bDoorIsOpen
          FROM doorEvents
          ORDER BY dtEventDate DESC';

    $res = mysql_query($sql, $dbh);

    while($row = mysql_fetch_array($res, MYSQL_ASSOC)) {
      $RSS_items .= '<item>';

      if($row['bDoorIsOpen'] == 1)
        $RSS_items .= '<title>Hive13 is Open!</title>';
      else
        $RSS_items .= '<title>Hive13 is closed.</title>';

      $RSS_items .= '<link>http://shell.hive13.org/webcam1/fullsize.jpg</link>
                <description></description>
                <pubDate>'.date(DATE_RSS, strtotime($row['dtEventDate'])).'</pubDate>
              </item>';
    }
  }
  
  // We are going to create a very basic dummy RSS feed here
  $feed = '<?xml version="1.0" encoding="ISO-8859-1" ?>
            <rss version="2.0">
              <channel>
                <title>'.$RSS_title.'</title>
                <link>http://www.hive13.org</link>
                <description>'.$RSS_description.'</description>
                <language>en-us</language>
                <pubDate>'.date(DATE_RSS).'</pubDate>';
  $feed .= $RSS_items;
  $feed .= '</channel></rss>';
  echo $feed;
?> 
