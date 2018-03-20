<?php
function mysqli_connect_wrapper() {
  return mysqli_connect('hostname','username','userpass','dbname')
         or die('Error connecting to MySQL server.');
}
?>