/********* Gallery.js ***********/
var background_images = new Array();
var totalImages = 6;

setup();
setBackground();

function setup(){
	for (var i = 0; i < totalImages
	if(i < 10)
		background_images[i] = '/images/backgrounds/back_0'+i+'.jpg';
	else
		background_images[1] = '/images/backgrounds/back_'+i+'.jpg';
}

function setBackground(){
	var image;
	var random = Math.floor(Math.random()*totalImages);
	alert("setting background image to: "+random);
	document.write("<style type='text/stylesheet'>\nbody{\nbackground: url('"+background_images[random]+"') top left no-repeat;\nbackground-attachment: fixed;\n}\n</style>\n"
}
