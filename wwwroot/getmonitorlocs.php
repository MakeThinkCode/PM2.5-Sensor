<?php
require_once ('sqlconnect.php');

if( 1==1 ) {
    $db=mysqli_connect($host, $user, $pwd, $conn);
    if($db) {		
        $sel = "SELECT * FROM `monitor_info`";
        # TO DO:  extend query to also give monitor hi/lo/avg and date range
        $result = mysqli_query($db, $sel);
        echo "[";
        $beenhere = 0;
        while ($row = mysqli_fetch_array($result)) {
            if ( $beenhere > 0 ) { echo ","; }
            echo "{";  // array elem
            echo "\"monitorName\":\"" . $row['monitorName'] . "\"";
            echo ",";
            echo "\"monitorID\":\"" . $row['monitorID'] . "\"";
            echo ",";
            echo "\"monLat\":"  . $row['monLat'];
            echo ",";
            echo "\"monLong\": " . $row['monLong'];
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
