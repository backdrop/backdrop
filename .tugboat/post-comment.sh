#!/usr/bin/env php
<?php
/**
 * Post a comment in the PR with login details.
 */

// Don't run this if comment updating is disabled.
// @see https://dashboard.tugboatqa.com/5f9cffcf6c879bf4534e573e/settings/
if (!getenv('UPDATE_COMMENTS')) {
  return;
}

$password = file_get_contents(getenv('TUGBOAT_ROOT') . '/.tugboat/password');
$expires_date = date_create('+2 months', timezone_open('UTC'));
$comment = 'Tugboat has finished building a preview for this pull request!

Website: ' . getenv('TUGBOAT_DEFAULT_SERVICE_URL') . '
Username: admin
Password: ' . $password . '

This preview will automatically expire on the ' . $expires_date->format('jS \of F, Y') . '.';

// Post comment to GitHub.
$ch = curl_init('https://api.github.com/repos/' . getenv('TUGBOAT_GITHUB_OWNER') . '/' . getenv('TUGBOAT_GITHUB_REPO') . '/issues/' . getenv('TUGBOAT_GITHUB_PR') . '/comments');
curl_setopt($ch, CURLOPT_HTTPHEADER, array(
  'Authorization: token ' . getenv('BACKDROP_GITHUB_TOKEN'),
  'Content-Type: application/json',
  'Accept: application/vnd.github.v3+json',
  'User-Agent: Backdrop CMS',
));
curl_setopt($ch, CURLOPT_POST, TRUE);
curl_setopt($ch, CURLOPT_POSTFIELDS, json_encode(array('body' => $comment)));
curl_setopt($ch, CURLOPT_RETURNTRANSFER, TRUE);
curl_exec($ch);
curl_close($ch);
