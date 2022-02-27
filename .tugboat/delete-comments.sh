#!/usr/bin/env php
<?php
/**
 * Delete any previous comments in the PR about this preview.
 */

$curl_header = array(
  'Authorization: token ' . getenv('BACKDROP_GITHUB_TOKEN'),
  'Content-Type: application/json',
  'Accept: application/vnd.github.v3+json',
  'User-Agent: Backdrop CMS',
);
$text = 'Tugboat has finished building a preview for this pull request!';
$url = getenv('TUGBOAT_DEFAULT_SERVICE_URL');

// Get all comments in this PR.
$ch = curl_init('https://api.github.com/repos/' . getenv('TUGBOAT_GITHUB_OWNER') . '/' . getenv('TUGBOAT_GITHUB_REPO') . '/issues/' . getenv('TUGBOAT_GITHUB_PR') . '/comments');
curl_setopt($ch, CURLOPT_HTTPHEADER, $curl_header);
curl_setopt($ch, CURLOPT_RETURNTRANSFER, TRUE);
$response = curl_exec($ch);
$code = curl_getinfo($ch, CURLINFO_RESPONSE_CODE);
curl_close($ch);

// Find comments that match the new one we're about to add.
if ($code == 200) {
  foreach (json_decode($response) as $comment) {
    if (strpos($comment->body, $text) === 0 && strpos($comment->body, $url) !== FALSE) {
      // Delete comment.
      $ch = curl_init('https://api.github.com/repos/' . getenv('TUGBOAT_GITHUB_OWNER') . '/' . getenv('TUGBOAT_GITHUB_REPO') . '/issues/comments/' . $comment->id);
      curl_setopt($ch, CURLOPT_HTTPHEADER, $curl_header);
      curl_setopt($ch, CURLOPT_CUSTOMREQUEST, 'DELETE');
      curl_setopt($ch, CURLOPT_RETURNTRANSFER, TRUE);
      curl_exec($ch);
      curl_close($ch);
    }
  }
}
