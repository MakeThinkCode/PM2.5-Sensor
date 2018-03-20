
<?php
include 'sqlconnect.php';

$target_dir = "uploads/";
$target_file = $target_dir . basename($_FILES["fileToUpload"]["name"]);

echo "09<br>";


if (move_uploaded_file($_FILES["fileToUpload"]["tmp_name"], $target_file)) {
    echo "The file ". basename( $_FILES["fileToUpload"]["name"]). " has been uploaded.";
    echo "<br>";
    $myfile = fopen($target_file, "r") or die("Unable to open file!");
    // throw into db
    $db = mysqli_connect_wrapper();
    $readingformat13 = 0;
    while(!feof($myfile)) {
        $oneline = fgets($myfile);
        if ($readingformat13) {
            $items = explode(",",$oneline);    // need to get rid of white space too
            $next_id = 'NULL';
            $query = "INSERT INTO `mtctest2`(`ID`, `col1`, `col2`, `col3`  ) " .
                     "  VALUES (" . $next_id .
                     ', "' . $items[0] . '", "' . $items[1] . '","' . $items[2] .
                     '")';
            echo  "query:  " . $query . "<br>";
            mysqli_query($db, $query) or die('Error updating database.');
        } else {
            // we're not reading format13; check to see if we have a start tag for
            // this format
            if ( strpos($oneline, "plantower_13 starting up") >=  0 ) {
                ....
            }
        }
    }
    mysqli_close($db);
    fclose($myfile);
} else {
    echo "Sorry, there was an error uploading your file.";
}


echo "the DB now contains: <br> ID, col1, col2, col3<br>";

$db = mysqli_connect_wrapper();
$query = "SELECT * FROM `mtctest2` ";
($result = mysqli_query($db, $query)) or die('Error querying database.');
while ($row = mysqli_fetch_array($result)) {
    $id = strtolower($row['ID']);
    $c1 = strtolower($row['col1']);
    $c2 = strtolower($row['col2']);
    $c3 = strtolower($row['col3']);
    echo $id . ", " . $c1 . ", " . $c2 . ", " . $c3 . ", " . "<br>";
}

mysqli_close($db);
echo "---- that's all--- <br>";

?>
