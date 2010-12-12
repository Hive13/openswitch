<?php

$con = mysql_connect('localhost', 'username','password');
if(!$con) {
   die("Bork Bork! " . mysql_error());
}
mysql_select_db("channellog", $con);

$sql="select * from doorEvents order by dtEventDate DESC limit 1";

$res = mysql_query($sql);
$row = mysql_fetch_array($res);
if($row['bDoorIsOpen'] == 1) {
	echo '<img src=open.jpg width=100px height=100px>';
} else {
	echo '<img src=closed.jpg width=100px height=100px>';
}
mysql_close($con);
?>
