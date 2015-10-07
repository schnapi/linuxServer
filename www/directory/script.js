$(document).ready(function(){
	var notes = [{"name":"C3", "value":"Do3"},
		{"name":"C3s", "value":"Do3#"},
		{"name":"D3", "value":"Ré3"},
		{"name":"D3s", "value":"Ré3#"},
		{"name":"E3", "value":"Mi3"},
		{"name":"F3", "value":"Fa3"},
		{"name":"F3s", "value":"Fa3#"},
		{"name":"G3", "value":"Sol3"},
		{"name":"G3s", "value":"Sol3#"},
		{"name":"A3", "value":"La3"},
		{"name":"A3s", "value":"La3#"},
		{"name":"B3", "value":"Si3"},
		{"name":"C4", "value":"Do4"},
		{"name":"C4s", "value":"Do4#"},
		{"name":"D4", "value":"Ré4"},
		{"name":"D4s", "value":"Ré4#"},
		{"name":"E4", "value":"Mi4"},
		{"name":"F4", "value":"Fa4"},
		{"name":"F4s", "value":"Fa4#"},
		{"name":"G4", "value":"Sol4"},
		{"name":"G4s", "value":"Sol4#"},
		{"name":"A4", "value":"La4"},
		{"name":"A4s", "value":"La4#"},
		{"name":"B4", "value":"Si4"},
		{"name":"C5", "value":"Do5"},
		{"name":"C5s", "value":"Do5#"},
		{"name":"D5", "value":"Ré5"},
		{"name":"D5s", "value":"Ré5#"},
		{"name":"E5", "value":"Mi5"},
		{"name":"F5", "value":"Fa5"},
		{"name":"F5s", "value":"Fa5#"},
		{"name":"G5", "value":"Sol5"},
		{"name":"G5s", "value":"Sol5#"},
		{"name":"A5", "value":"La5"},
		{"name":"A5s", "value":"La5#"},
		{"name":"B5", "value":"Si5"}];

	for(var i in notes)
	{
		note = notes[i];
		var classe = note.name.indexOf('s')> -1?"sharp":"regular";
		classe += " touche";
		var div = $('<div/>',{ class:classe, "note":note.name,"value":note.value});
		$("#piano").append(div);


		var audio = $('<audio>', {"id":note.name, "preload":"auto"});
		audio.append(
		$("<source>", {"src":"notes/"+note.name+".ogg", "type":"audio/ogg"})
		);
		$("#src").append(audio);
	}

	$(".touche").mousedown(function(){
		$(this).addClass("active");
		play($(this).attr("note"))
		$(this).mouseup(function(){
			$(this).removeClass("active");
			stop($(this).attr("note"))
		});
	})


	function play(note){
		document.getElementById(note).play()
	}


	function stop(note){
		document.getElementById(note).pause();
		document.getElementById(note).currentTime = 0;
	}
})

