var element;
var folder;
var name;
function load(directory, FileName)
{
	folder = directory;
	name = FileName;
	element = document.getElementById('currentImage');
	/*element.style.background = "url('/images/elements/ajax-loader.gif') center center no-repeat";*/
  if (window.XMLHttpRequest)
    request = new XMLHttpRequest();
  else if (request == null)
    request = new ActiveXobject ("Microsoft.XMLHTTP");
  else
    alert ("This page requires XMLHTTP support. \nPlease upgrade your browser to view this site.");

	var dom = document.getElementById('query');
  if (request!=null)
  {
	  var url = "04Query.php"
		var query = "image=/images/gallery/"+folder+"/full/"+name;

    //open the connection.
    request.open ("POST", url, true);

    request.setRequestHeader("Content-type", "application/x-www-form-urlencoded");

    //set the onreadystatchange pointer.
    request.onreadystatechange=getPicture;
		request.send (query);
  }
}

function getPicture()
{
	element = document.getElementById('currentImage');
	element.innerHTML = "<image src='/images/elements/ajax-loader.gif' style='height: 50px; width:50px;'/>";
  if(request.readyState==4 ) //&& request.status==200)
  {
    element.innerHTML = "<img src='/images/gallery/"+folder+"/full/"+name+"'/>";
  }
	else if(request.readyState==4)
		alert("Error processing request.\n");
}