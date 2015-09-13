<?php

/**
 * @file
 * CTools plugin declaration for FieldDefaultTokenEmptyAlter behavior.
 */

$plugin = array(
  'title' => t('Allows tokens as field default value'),
  'description' => t('Allows using tokens while selecting the default value of the field.'),
  'class' => 'FieldDefaultTokenEmptyAlter',
  'behavior type' => 'field',
  'force enabled' => TRUE,
  'weight' => 100,
);
