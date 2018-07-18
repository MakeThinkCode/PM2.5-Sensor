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
<h1> Update a monitor's location</h1>

 <form action="loc.php"  name="locationform" method = POST>
  Monitor Name: <input type="text" name="monitor"><br />
  Monitor Location: <br />
  &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Latitude: <input type="text" name="lat"><br />
  &nbsp;&nbsp;Longitude: <input type="text" name="long"><br /><br />
  Date and time monitor moved to this location: <br />
  &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Date (yyyy-mm-dd): <input type="text" name="ldate"><br />
  &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
  Time (hh:mm): <input type="text" name="ltime"><br /><br />
  <input name = "submit" id = "submit" type="submit" value="Save Data">
 </form>


<p> </p>
 
<div id="mapid" style="width: 600px; height: 400px;"></div>
<script>

	var mymap = L.map('mapid').setView([45.505, -122.682], 12);

	L.tileLayer('https://api.tiles.mapbox.com/v4/{id}/{z}/{x}/{y}.png?access_token=pk.eyJ1IjoibWFwYm94IiwiYSI6ImNpejY4NXVycTA2emYycXBndHRqcmZ3N3gifQ.rJcFIG214AriISLbB6B5aw', {
		maxZoom: 18,
		attribution: 'Map data &copy; <a href="https://www.openstreetmap.org/">OpenStreetMap</a> contributors, ' +
			'<a href="https://creativecommons.org/licenses/by-sa/2.0/">CC-BY-SA</a>, ' +
			'Imagery © <a href="https://www.mapbox.com/">Mapbox</a>',
		id: 'mapbox.streets'
	}).addTo(mymap);

	var popup = L.popup();
	function onMapClick(e) {
		popup
			.setLatLng(e.latlng)
			.setContent("You clicked the map at " + e.latlng.toString())
			.openOn(mymap);
                document.forms["locationform"]["lat"].value  = e.latlng.lat;
                document.forms["locationform"]["long"].value = e.latlng.lng;
	}

	mymap.on('click', onMapClick);



</script>
<?php
require_once ('sqlconnect.php');
if(isset($_POST['submit'])) {
	$db=mysqli_connect($host, $user, $pwd, $conn);
	if($db) {
        $sel = "SELECT `monitorName`, `monLat`, `monLong`, `codeVersion`, `registerDatetime`, `locationDatetime` ".
    		   "FROM `monitor_info` WHERE monitorName = \"".$_POST['monitor']."\" ORDER BY locationDatetime DESC";
		#echo "The select query is: ".$sel."<br />";
        $result = mysqli_query($db, $sel);

		if ($result->num_rows < 1) {
			$message = "Please register monitor before setting its location.";
		} else  {
					$row = $result->fetch_array();
					if ($row['monLat'] == 0){			
					   $query = "UPDATE `monitor_info` SET `monLat` = ".$_POST['lat'].", `monLong` = ".$_POST['long'].
                          ", `locationDatetime` = \"".$_POST['ldate']." ".$_POST['ltime']."\" WHERE `monitorName` = '".$_POST['monitor']."'" ;
					   #echo $query."<br />";
					   $result = mysqli_query($db, $query);
					   #echo $result."<br />";
					   $message = "Location information for monitor ".$_POST['monitor']." added to database.";
					} else {
						$sql = "INSERT INTO `monitor_info` (`monitorName`, `monLat`, `monLong`, `registerDatetime`, `codeVersion`,  `locationDatetime`)".
						       " VALUES (\"".$_POST['monitor']."\", ".$_POST['lat'].", ".$_POST['long'].
							   ", \"".$row['registerDatetime']."\", ".$row['codeVersion'].", \"".$_POST['ldate']." ".$_POST['ltime']."\")";
					    #echo $sql."<br />";
						$result = mysqli_query($db, $sql);
						if ($result) { $message = "New location information for monitor ".$_POST['monitor']." added to database.";}
						else {$message = "Could not set new location for mointor".$_POST['monitor'];}
					}
				}
	} 
	else { $message = "Error connecting to database."; 
	} 
    echo "<script type='text/javascript'>alert('$message');</script>";		
}

?>

</body>
</html>
