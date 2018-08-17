<!DOCTYPE html>
<html>
<head>
<title>
Register Particulates Monitor
</title>

<meta charset="utf-8" />
<meta name="viewport" content="width=device-width, initial-scale=1.0">


</head>
<body>

<h2><a href="index.html">Particulates Matter!</a></h2>
<h1> Register a new software version for a PM2.5 monitor</h1>

 <form action="sw_version.php" method = POST>
  Monitor Name: <input type="text" name="monitor"><br />
  Monitor code version: <input type="text" name="codeVer"><br />
  <input name = "submit" id = "submit" type="submit" value="Save Data">
 </form>


<p> </p>
 
<?php
require_once ('sqlconnect.php');
if(isset($_POST['submit'])) {
  
	$db=mysqli_connect($host, $user, $pwd, $conn);
	if($db) {
		date_default_timezone_set('UTC');
		#cur_time = date('Y-m-d H:i:s', time());
		$dt = new Datetime(date('Y-m-d H:i:s'));
		$subh = new DateInterval('PT8H');
		$cur_time = $dt->sub($subh);
		#echo "Time: ".$cur_time->format('Y-m-d H:i:s');			
		$query = "INSERT INTO `monitor_info`(`monitorName`, `codeVersion`, `registerDatetime`  ) " .
                 '  VALUES ("' .$_POST['monitor'] .
                 '", "' . $_POST['codeVer'] .
		 '", "'.$cur_time->format('Y-m-d H:i:s').
                 '")';
		if (mysqli_query($db, $query)) { $message = $_POST['monitor']." version info added to database"; 
		}
		else { $message = "Error updating database."; 
		} 
	}
	else {
		$message = "Unable to connect to MySQL" .mysql_error();
	}

	echo "<script type='text/javascript'>alert('$message');</script>";		
}

?>

</body>
</html>
