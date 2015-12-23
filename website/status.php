<?php

header("Access-Control-Allow-Origin: *");
//header("Content-type: text/json");
require('sql.php');

$res = array();
$conf = loadconfig('.htdb.conf');
$con = getdbhandle($conf);

$sql = "select bDoorIsOpen from doorEvents order by dtEventDate DESC limit 1";

$stmt = mysqli_prepare($con, $sql);
mysqli_stmt_execute($stmt);
mysqli_stmt_store_result($stmt);
mysqli_stmt_bind_result($stmt, $res["isOpen"]);
mysqli_stmt_fetch($stmt);
mysqli_stmt_close($stmt);
if ($res["isOpen"])
	$res["isOpenImg"] = "<img src=\"/artwork/open.png\" width=\"100px\" height=\"43px\">";
else
	$res["isOpenImg"] = "<img src=\"/artwork/closed.png\" width=\"100px\" height=\"43px\">";
	
echo json_encode($res);

?>
