<?php
/* Minisforum N5 EC / IT5571 Driver — sensor data JSON endpoint.
 * Standalone PHP file (not a .page), served by PHP-FPM directly so the
 * frontend can poll it without going through template.php.
 * GPL-2.0-only
 */
header('Content-Type: application/json; charset=utf-8');
header('Cache-Control: no-store');

function hwmon_path() {
  foreach (glob('/sys/class/hwmon/hwmon*') ?: [] as $d) {
    $name = @file_get_contents("$d/name");
    if (trim($name) === 'minisforum_n5_it5571') return $d;
  }
  return false;
}

function read_attr($base, $attr) {
  $v = @file_get_contents("$base/$attr");
  return $v === false ? null : trim($v);
}

$h = hwmon_path();
$out = ['loaded' => is_dir($h), 'hwmon' => is_dir($h) ? basename($h) : null, 'version' => '0.1.0', 'pwm' => [], 'temp' => []];

if (is_dir($h)) {
  for ($i = 1; $i <= 4; $i++) {
    $pwm = read_attr($h, "pwm$i");
    $enable = read_attr($h, "pwm${i}_enable");
    $fan = read_attr($h, "fan${i}_input");
    $mode = match ($enable) { '1' => 'manual', '2' => 'bios', default => null };
    $out['pwm'][] = ['pwm' => $pwm, 'mode' => $mode, 'fan' => $fan];
  }
  for ($i = 1; $i <= 4; $i++) {
    $t = read_attr($h, "temp${i}_input");
    if ($t !== null) $out['temp'][] = round($t / 1000, 1);
  }
}

echo json_encode($out, JSON_UNESCAPED_UNICODE);
