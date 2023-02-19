#!/usr/bin/env php
<?php
/**
 * Set an expiration time for the preview using Tugboat's API.
 */

// Get 'expires' date/time.
$expires_date = date_create('+2 months', timezone_open('UTC'));
$expires = $expires_date->format('c');

// Set 'expires' date/time via Tugboat API.
// @see https://api.tugboat.qa/v3#tag/Previews/paths/~1previews~1{id}/patch
$ch = curl_init('https://api.tugboat.qa/v3/previews/' . getenv('TUGBOAT_PREVIEW_ID'));
curl_setopt($ch, CURLOPT_HTTPHEADER, array(
  'Authorization: Bearer ' . getenv('BACKDROP_TUGBOAT_TOKEN'),
  'Content-Type: application/json',
  'Accept: application/json',
));
curl_setopt($ch, CURLOPT_HEADER, TRUE);
curl_setopt($ch, CURLOPT_RETURNTRANSFER, TRUE);
curl_setopt($ch, CURLOPT_FOLLOWLOCATION, FALSE);
curl_setopt($ch, CURLOPT_CUSTOMREQUEST, 'PATCH');
curl_setopt($ch, CURLOPT_POSTFIELDS, json_encode(array('expires' => $expires)));
curl_exec($ch);
curl_close($ch);
