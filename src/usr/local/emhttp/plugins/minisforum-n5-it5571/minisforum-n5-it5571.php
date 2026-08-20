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
$out = ['loaded' => is_dir($h), 'hwmon' => is_dir($h) ? basename($h) : null, 'version' => '0.2.0', 'pwm' => [], 'temp' => []];

$pwm_map = [1 => ['zh' => 'CPU 风扇', 'en' => 'CPU Fan'],
            2 => ['zh' => 'SSD 风扇', 'en' => 'SSD Fan'],
            3 => ['zh' => 'HDD 风扇组', 'en' => 'HDD Fan Group'],
            4 => ['zh' => 'PCIe 风扇', 'en' => 'PCIe Fan']];
$temp_map = [1 => ['zh' => 'CPU 温度', 'en' => 'CPU Temp'],
             2 => ['zh' => '系统温度', 'en' => 'System Temp'],
             3 => ['zh' => '主板温度', 'en' => 'Board Temp'],
             4 => ['zh' => '环境温度', 'en' => 'Ambient Temp']];

if (is_dir($h)) {
  for ($i = 1; $i <= 4; $i++) {
    $pwm = read_attr($h, "pwm$i");
    $enable = read_attr($h, "pwm${i}_enable");
    $fan = read_attr($h, "fan${i}_input");
    $mode = match ($enable) { '1' => 'manual', '2' => 'bios', default => null };
    $out['pwm'][] = ['label' => $pwm_map[$i], 'pwm' => $pwm, 'mode' => $mode, 'fan' => $fan];
  }
  for ($i = 1; $i <= 4; $i++) {
    $t = read_attr($h, "temp${i}_input");
    if ($t !== null) $out['temp'][] = ['label' => $temp_map[$i], 'c' => round($t / 1000, 1)];
  }
}

echo json_encode($out, JSON_UNESCAPED_UNICODE);
