    var supported = (document.getElementById || document.all);
    var init = true;

    if (supported)
    {
        document.write("<STYLE TYPE='text/css'>");
        document.write(".para {display: none}");
        document.write("</STYLE>");

        var max = 14;
        var shown = new Array();
        for (var i=0;i<max;i++)
        {
            shown[i] = false;
        }
    }

    function blocking(i, isClick)
    {
        if(!init && !isClick)
        {
        	return;
        }

        init = false;

        if (!supported)
        {
            alert('This link does not work in your browser.');
            return;
        }
		shown[i] = !shown[i];
        current = (shown[i]) ? 'block' : 'none';
        if (document.getElementById)
        {
            if(document.getElementById('number'+i)!=null)
            {
            	document.getElementById('number'+i).style.display = current;

            	for (var j=0;j<max;j++)
				{
					if(j!=i)
					{
						shown[j] = false;
						if(document.getElementById('number'+j)!=null)
							document.getElementById('number'+j).style.display = 'none';
					}
        		}

            	if (!shown[i] && document.getElementById('link'+i))
					document.getElementById('link'+i).href = "#" + document.getElementById('link'+i).href.split("#")[1];
            }
        }
        else if (document.all)
        {
            document.all['number'+i].style.display = current;
        }
    }

	//main
	var url = window.location.href;
	var paramstring = url.split("?")[1];
	if (!paramstring) paramstring = "";
	var params = paramstring.split("&");
	if (!params) params = array();
	var parm, i;
	for (i = 0; i < params.length; i++)
	{
		//split(parm, "="); // [0] = key, [1] = value
		parm = params[i];

		if (parm.split("=")[0] == "block")
			blocking( parm.split("=")[1], false );
	}