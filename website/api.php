<?php
  require('func.php');
  $conf = loadconfig('doorlogger.config.php');
  $con = getdbhandle($conf);


  //based on 'spaceAPI' found at:
  //  http://hackerspaces.nl/spaceapi/
  header('Access-Control-Allow-Origin: *');
  header('Cache-Control: no-cache, must-revalidate');
  header('Expires: Mon, 26 Jul 1997 05:00:00 GMT');
  header('Content-type: application/json');

  echo '{';
  echo '"api":"0.12",';
  echo '"space":"Hive13",';
  echo '"logo":"http://www.hive13.org/wp-uploads/2011/08/HIVE.jpg",';
  echo '"icon":{';
    echo '"open":"http://www.hive13.org/artwork/open-100x100.png",';
    echo '"closed":"http://www.hive13.org/artwork/closed-100x100.png" ';
  echo '},';
  echo '"url":"http://www.hive13.org/",';
  echo '"address":"2929 Spring Grove Ave. Cincinnati, OH 45225", ';
  echo '"cam":[';
    echo '"http://www.hive13.org/?page_id=16"';
  echo '], ';
  echo '"contact":{';
    echo '"twitter":"@hive13", ';
    echo '"irc":"irc://freenode.net/#hive13", ';
    echo '"ml":"http://groups.google.com/group/cincihackerspace/", ';
    echo '"email":"membership@hive13.org" ';
  echo '}, ';
  echo '"feeds":[';
    echo '{"name":"blog","type":"application/rss+xml","url":"http://www.hive13.org/?feed=rss2"},';
    echo '{"name":"blog","type":"application/rss+xml","url":"http://www.hive13.org/isOpen/RSS.php?temp=0"}';
  echo '], ';

  echo '"sensors":[';
  $sql="select dcTemp from tempEvents order by dtEventDate desc limit 1";
  $res = mysql_query($sql);
  $row = mysql_fetch_array($res);
    echo '{"temp":{"hive13":"';
    echo $row['dcTemp'];
    echo 'F"}}';
  echo '], ';

  $sql="select bDoorIsOpen, UNIX_TIMESTAMP(dtEventDate) AS timestamp from doorEvents order by dtEventDate DESC limit 1";

  $res = mysql_query($sql);
  $row = mysql_fetch_array($res);
  if($row['bDoorIsOpen'] == 1) {
    echo '"open":true';
  } else {
    echo '"open":false';
  }
  echo ', ';
  echo '"lastchange":';
  echo $row['timestamp'];

  mysql_close($con);

  echo '}';

?>
