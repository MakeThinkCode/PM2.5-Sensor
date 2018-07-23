<?php
require_once ('sqlconnect.php');

if(isset($_POST['monitorId'])) {

##### TO DO:  security on the monitorId parameter; it should be an integer.
    $monid = $_POST['monitorId'];

    $db=mysqli_connect($host, $user, $pwd, $conn);
    if($db) {		
        $sel = "SELECT * FROM `monitor_pm25` where monitorID=" . $monid;
        #echo $sel;
        $result = mysqli_query($db, $sel);
        echo "[";
        $beenhere = 0;
        while ($row = mysqli_fetch_array($result)) {
            if ( $beenhere > 0 ) { echo ","; }
            echo "{";  // array elem
            echo "\"obsDatetime\":\"" . $row['obsDatetime'] . "\"";
            echo ",";
            echo "\"pm25Std\":"  . $row['pm25Std'];
            echo "}";
            $beenhere = 1;
        }
        mysqli_close($db);
        echo "]";
    } else {
        # $message = "Unable to connect to MySQL: " . mysql_error();
    }
} else {
    echo "missing _POST data";
}

?>
