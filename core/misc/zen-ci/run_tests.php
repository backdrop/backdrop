<?php
  
$home = getenv('HOME');


$sitepath = $home . '/www';

$data = array(
  'state' => 'pending',
  'message' => 'Processing Tests',
);

putRequest($data);

chdir($sitepath);

$cmd = 'php core/scripts/run-tests.sh --url http://localhost --verbose --cache --force --all --concurrency 10 --color --verbose --summary /tmp/summary';
$proc = popen($cmd, 'r');

while (!feof($proc)) {
  echo fread($proc, 4096);
  @flush();
}

$status = pclose($proc);

$content = file_get_contents('/tmp/summary');

if($status) {
  $content = explode("\n", $content);
  
  $message = $content[0];
  unset($content[0]);
  $summary = implode("\n", $content);
  //Test failed
  $data = array(
    'state' => 'error',
    'message' => $message,
    'summary' => $summary,
  );
  putRequest($data);
  exit(1);
}
else {
  //success
  $data = array(
    'state' => 'success',
    'message' => $content,
  );
  exit(0);
}



function putRequest($data) {

  $token = getenv('GITLC_API_TOKEN');
  $status_url = getenv('GITLC_STATUS_URL');
  
  $data = json_encode($data);
  
  $ch = curl_init();

  curl_setopt($ch, CURLOPT_URL, $status_url);
  curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
  curl_setopt($ch, CURLOPT_POST, true);
  curl_setopt($ch, CURLOPT_CUSTOMREQUEST, "PUT"); // note the PUT here
  
  curl_setopt($ch, CURLOPT_POSTFIELDS, $data);
  curl_setopt($ch, CURLOPT_HEADER, true);
  
  curl_setopt($ch, CURLOPT_HTTPHEADER, array(                                                                          
      'Content-Type: application/json',
      'Token: ' . $token,                                                                                
      'Content-Length: ' . strlen($data)                                                                       
  )); 
  curl_exec($ch);
  curl_close($ch);
}
