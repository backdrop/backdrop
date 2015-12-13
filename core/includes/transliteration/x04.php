<?php

$base = array(
  0x00 => 'E', 'E', 'D', 'G', 'E', 'Z', 'I', 'I', 'J', 'L', 'N', 'C', 'K', 'I', 'U', 'D',
  0x10 => 'A', 'B', 'V', 'G', 'D', 'E', 'Z', 'Z', 'I', 'I', 'K', 'L', 'M', 'N', 'O', 'P',
  0x20 => 'R', 'S', 'T', 'U', 'F', 'H', 'C', 'C', 'S', 'S', '', 'Y', '', 'E', 'U', 'A',
  0x30 => 'a', 'b', 'v', 'g', 'd', 'e', 'z', 'z', 'i', 'i', 'k', 'l', 'm', 'n', 'o', 'p',
  0x40 => 'r', 's', 't', 'u', 'f', 'h', 'c', 'c', 's', 's', '', 'y', '', 'e', 'u', 'a',
  0x50 => 'e', 'e', 'd', 'g', 'e', 'z', 'i', 'i', 'j', 'l', 'n', 'c', 'k', 'i', 'u', 'd',
  0x60 => 'O', 'o', 'E', 'e', 'Ie', 'ie', 'E', 'e', 'Ie', 'ie', 'O', 'o', 'Io', 'io', 'Ks', 'ks',
  0x70 => 'Ps', 'ps', 'F', 'f', 'Y', 'y', 'Y', 'y', 'u', 'u', 'O', 'o', 'O', 'o', 'Ot', 'ot',
  0x80 => 'Q', 'q', '*1000*', '', '', '', '', NULL, '*100.000*', '*1.000.000*', NULL, NULL, '"', '"', 'R\'', 'r\'',
  0x90 => 'G', 'g', 'G', 'g', 'G', 'g', 'Zh\'', 'zh\'', 'Z', 'z', 'K\'', 'k\'', 'K\'', 'k\'', 'K\'', 'k\'',
  0xA0 => 'K\'', 'k\'', 'N\'', 'n\'', 'Ng', 'ng', 'P\'', 'p\'', 'Kh', 'kh', 'S\'', 's\'', 'T\'', 't\'', 'U', 'u',
  0xB0 => 'U\'', 'u\'', 'Kh\'', 'kh\'', 'Tts', 'tts', 'Ch\'', 'ch\'', 'Ch\'', 'ch\'', 'H', 'h', 'Ch', 'ch', 'Ch\'', 'ch\'',
  0xC0 => '`', 'Z', 'z', 'K\'', 'k\'', NULL, NULL, 'N\'', 'n\'', NULL, NULL, 'Ch', 'ch', NULL, NULL, NULL,
  0xD0 => 'A', 'a', 'A', 'a', 'AE', 'ae', 'E', 'e', '@', '@', '@', '@', 'Z', 'z', 'Z', 'z',
  0xE0 => 'Dz', 'dz', 'I', 'i', 'I', 'i', 'O', 'o', 'O', 'o', 'O', 'o', 'E', 'e', 'U', 'u',
  0xF0 => 'U', 'u', 'U', 'u', 'C', 'c', NULL, NULL, 'Y', 'y', NULL, NULL, NULL, NULL, NULL, NULL,
);

// Overrides for Kyrgyz input.
$variant['kg'] = array(
  0x01 => 'E',
  0x16 => 'C',
  0x19 => 'J',
  0x25 => 'X',
  0x26 => 'TS',
  0x29 => 'SCH',
  0x2E => 'JU',
  0x2F => 'JA',
  0x36 => 'c',
  0x39 => 'j',
  0x45 => 'x',
  0x46 => 'ts',
  0x49 => 'sch',
  0x4E => 'ju',
  0x4F => 'ja',
  0x51 => 'e',
  0xA2 => 'H',
  0xA3 => 'h',
  0xAE => 'W',
  0xAF => 'w',
  0xE8 => 'Q',
  0xE9 => 'q',
);

// Overrides for Ukrainian input.
$variant['uk'] = array(
  0x90 => 'G',
  0x91 => 'g',
  0x04 => 'YE',
  0x54 => 'ye',
  0x18 => 'Y',
  0x38 => 'y',
);
