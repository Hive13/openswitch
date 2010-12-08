<?php
  header('Content-Type: text/html; charset=utf-8');
?>

<?php
if(isSet($_REQUEST['switch'])) {
  if($_REQUEST['switch'] == '0') {
    echo "The space is closed";
  } else if($_REQUEST['switch'] == '1') {
    echo "The space is open";
  } else {
    echo "Unknown: ".$_REQUEST['switch'];
  }  
}
?>