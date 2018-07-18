<!DOCTYPE html>
<html>
<head>
<title>
Register Particulates Monitor
</title>

<meta charset="utf-8" />
<meta name="viewport" content="width=device-width, initial-scale=1.0">
	
<link rel="shortcut icon" type="image/x-icon" href="docs/images/favicon.ico" />

<link rel="stylesheet" href="https://unpkg.com/leaflet@1.3.1/dist/leaflet.css" integrity="sha512-Rksm5RenBEKSKFjgI3a41vrjkw4EVPlJ3+OiI65vTjIdo9brlAacEuKOiQ5OFh7cOI1bkDwLqdLw3Zg0cRJAAQ==" crossorigin=""/>
<script src="https://unpkg.com/leaflet@1.3.1/dist/leaflet.js" integrity="sha512-/Nsx9X4HebavoBvEBuyp3I7od5tA0UzAxs+j83KgC8PU0kgB4XiK4Lfe4y4cgBtaRJQEIFCW+oC506aPT2L1zw==" crossorigin=""></script>


</head>
<body>

<h2><a href="index.html">Particulates Matter!</a></h2>
<h1> Register a PM2.5 Monitor</h1>

 <form action="register.php" method = POST>
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
		
        $sel = "SELECT * FROM `monitor_info` WHERE `monitorName` = '".$_POST['monitor']."'";
        $result = mysqli_query($db, $sel);
		if ($result->num_rows > 0) {
			$message = "The monitor name ".$_POST['monitor']." already exists in the database. Please try a different name.";
        }
		else {	
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
			#echo $query;
			if (mysqli_query($db, $query)) { $message = $_POST['monitor']." info added to database"; 
			}
			else { $message = "Error updating database."; 
			} 
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
