<?php

$notesMicros = [0, 16129, 15152, 14286, 13514, 12821, 12195, 11364, 10870, 10204, 9615, 9091, 8621, 8065, 7692, 7246, 6849, 6410, 6098, 5747, 5376, 5102, 4808, 4545, 4274, 4065, 3817, 3597, 3401, 3205, 3030, 2857, 2703, 2551, 2404, 2273, 2146, 2024, 1908, 1805, 1701, 1608, 1515, 1433, 1351, 1276, 1205, 1136, 1073, 1012, 956, 903, 852, 804, 759, 716, 676, 638, 602, 568, 536, 506, 478, 451, 426, 402, 379, 358, 338, 319, 301, 284, 268, 253, 239, 226, 213, 201, 190, 179, 169, 159, 151, 142, 134, 127, 119, 113, 106, 100 ];

// Read file
$file = file_get_contents($argv[1]);
$file = explode("\n", $file);

$tracks = array();

// Split into tracks
for ($i=0; $i < sizeof($file); $i++) { 
	if ($file[$i] == 'MTrk') {
		for ($ii=$i; $ii < sizeof($file); $ii++) { 
			if($file[$ii] == 'TrkEnd') {
				array_push($tracks, array_slice($file, $i+1, $ii-$i-1));
				break;
			}
		}
		$i++;
	}
}


// The track of interest
$outputTrack = array();

function decomposeEvent($event) {
	$event = explode(' ', $event);
	$ret = array();
	@$ret['time'] = $event[0];
	@$ret['type'] = $event[1];
	@$ret['note'] = substr($event[3], 2);
	@$ret['velocity'] = substr($event[4], 2);
	return $ret;
}

$chosedTrack = $argv[2];

foreach ($tracks[$chosedTrack] as $event) {
	$ev = decomposeEvent($event);
	if($ev['type'] == 'Off' or ($ev['type'] == 'On' and $ev['velocity'] == 0)) {
		array_push($outputTrack, array('time'=>$ev['time'], 'action'=>'Off', 'note'=>$ev['note']));
	}
	else if($ev['type'] == 'On' and $ev['velocity'] != 0) {
		array_push($outputTrack, array('time'=>$ev['time'], 'action'=>'On', 'note'=>$ev['note']));
	}
}

// convert the tempo to relative instead of absolut
$lastEvent = 0;
foreach ($outputTrack as $key => $ev) {
	$outputTrack[$key]['time'] =  $ev['time'] - $lastEvent;
	$lastEvent = $ev['time'];
}


$notesArray =  "const PROGMEM uint16_t notes[] = {";
$tempoArray =  "const PROGMEM uint16_t tempo[] = {";
foreach ($outputTrack as $ev) {
	if ($ev['action'] == 'On') {
		// Adjust the notes from the midi source (Don't know why it is necessary)
		$ev['note'] -= 22;

		$notesArray .= $notesMicros[$ev['note']] .",";
	
		
		$tempoArray .= $ev['time'].",";
	}
	else if ($ev['action'] == 'Off') {
		// Adjust the notes from the midi source (Don't know why it is necessary)
		$ev['note'] -= 22;

		// Set the most significant bit to mark the deactivation of a note
		$notesArray .= $notesMicros[$ev['note']]+32768 .",";
		
		$tempoArray .= $ev['time'].",";
	}
}
$notesArray = substr($notesArray, 0, -1);
$tempoArray = substr($tempoArray, 0, -1);
$notesArray .= "};";
$tempoArray .= "};";

echo $notesArray."\n".$tempoArray."\n";