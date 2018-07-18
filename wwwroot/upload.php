<!DOCTYPE html>
<html>
<head>
<title>
Particulates File Upload
</title>
<script src="https://d3js.org/d3.v4.min.js"></script>
</head>
<body>

<h2><a href="index.html">Particulates Matter!</a></h2>
<h1> Upload PM2.5 Data</h1>



<form action="upload.php" method = "post" enctype="multipart/form-data">
  <input type="file"   name = "fileToUpload" id = "fileToUpload" >
  <br />
  <input type="submit" value="Upload PM2.5 Data" name="submit" id="submit">
</form> 
<svg width="960" height="500"></svg>
<?php
require_once ('sqlconnect.php');
if(isset($_POST['submit'])) {
	$filename = addslashes($_FILES["fileToUpload"]["tmp_name"]);

echo "<script type='text/javascript'>\n";
# to build a multi-line string for d3 to parse, first build an
# array and then join it up.  Sigh. BB
echo "var rawpm25data = [\n"; # build up a long string for d3 to parse
echo "'datetime,pm25',\n";
$lines = file($filename);
foreach ($lines as $line_num => $line) {
    if ( $line_num > 3 ) { # skip 2 hdr lines and first two data lines
        $lineitems = str_getcsv($line);
        # [0] of the line is date-time "2018-7-2 14:44:59"
        # [5] of the line is pm2.5     "0.243"
        # echo "Line #<b>{$line_num}</b> : " . $lineitems[5] . "<br />\n";
        echo "'" . $lineitems[0] . ", " . $lineitems[5] . "',\n";
        # trailing comma after last entry is OK
    }
}
echo "].join('\\n');\n";
echo "</script>\n";
echo '<script src="makegraph.js"></script>';


	$pos = stripos(basename($_FILES["fileToUpload"]["name"]), "_");
	$monName = substr($_FILES["fileToUpload"]["name"], 0, $pos);
	$db=mysqli_connect($host, $user, $pwd, $conn);
	if($db) {        
		$sel = "SELECT `monitorName`, `monitorID` ".
    		   "FROM `monitor_info` WHERE monitorName = \"".$monName."\" ORDER BY locationDatetime DESC";
		#echo "The select query is: ".$sel."<br />";
        $result = mysqli_query($db, $sel);
		$row = $result->fetch_array();
		$monID = $row['monitorID'];
            # monitor software version 15 adds the navg field 7/2/18 BB
	    $sql = "LOAD DATA LOCAL INFILE  '".$filename."'
			INTO TABLE monitor_pm25
			FIELDS TERMINATED BY ',' 
			ENCLOSED BY '\"'
			LINES TERMINATED BY '\\r\\n'
			IGNORE 2 ROWS
			(@obsDatetime, millis, status, navg, pm01Std, pm25Std, pm10Std, pm01Atmos, pm25Atmos, pm10Atmos, pnc003, pnc005, pnc010, pnc025, pnc050, pnc100)
			SET monitorID = ".$monID.",
			obsDatetime = STR_TO_DATE(@obsDatetime, '%Y-%m-%d %H:%i:%s') ;";

		$result = mysqli_query($db, $sql);
		if ($result) {
			date_default_timezone_set('UTC');
			#cur_time = date('Y-m-d H:i:s', time());
			$dt = new Datetime(date('Y-m-d H:i:s'));
			$subh = new DateInterval('PT8H');
			$cur_time = $dt->sub($subh);
			
			#echo "The current time in PST: ".$cur_time."<br />";
			$upd = "UPDATE `monitor_info` SET `lastUpload` = '".$cur_time->format('Y-m-d H:i:s')."' WHERE `monitorID` = '".$monID."'" ;
			#echo "Upload query is: ".$upd."<br />";
			$res = mysqli_query($db, $upd);
			$message = "Data from ".$_FILES["fileToUpload"]["name"]." uploaded to database";
		}
		else { $message= "Data upload for ".$_FILES["fileToUpload"]["name"]." failed"; }
    }
	else {
		$message = "Could not open database.";
	}
	echo "<script type='text/javascript'>alert('$message');</script>";
}
?>


</body>
</html>
