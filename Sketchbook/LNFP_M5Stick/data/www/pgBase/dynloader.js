var useCompressed = false;

var scrLoaded = 0;
var scrRequested = 0;

function loadjsfile(filename)
{
	var fileref = document.createElement('script')
	fileref.setAttribute("type","text/javascript")
	fileref.setAttribute("src", filename)
	if (typeof fileref != "undefined")
		document.getElementsByTagName("head")[0].appendChild(fileref)
}

function loadcssfile(filename)
{
	var fileref = document.createElement("link")
	fileref.setAttribute("rel", "stylesheet")
	fileref.setAttribute("type", "text/css")
	fileref.setAttribute("href", filename)
	if (typeof fileref != "undefined")
		document.getElementsByTagName("head")[0].appendChild(fileref)
}

function loadScripts(nextLib)
{
	if (useCompressed)
	{
		switch (nextLib)
		{
			case 0: loadcssfile("/pgBase/iottbase.css"); break;
			case 1: loadjsfile("/pgBase/pageloader_s.js"); break;
			case 2: loadjsfile("/pgBase/pagestitch_s.js"); break;
			case 3: loadjsfile("/pgBase/pagebuilder_1_s.js"); break;
			case 4: loadjsfile("/pgBase/pagebuilder_2_s.js"); break;
			case 5: loadjsfile("/pgBase/pagebuilder_3_s.js"); break;
			case 6: loadjsfile("/pgBase/pagebuilder_4_s.js"); break;
			case 7: loadjsfile("/pgBase/footer_s.js"); break;
		}
	}
	else
	{
		switch (nextLib)
		{
			case 0: loadcssfile("/pgBase/iottbase.css"); break;
			case 1: loadjsfile("/pgBase/pageloader.js"); break;
			case 2: loadjsfile("/pgBase/pagebuilder_1.js"); break;
			case 3: loadjsfile("/pgBase/pagebuilder_2.js"); break;
			case 4: loadjsfile("/pgBase/pagebuilder_3.js"); break;
			case 5: loadjsfile("/pgBase/pagebuilder_4.js"); break;
			case 6: loadjsfile("/pgBase/pagestitch.js"); break;
			case 7: loadjsfile("/pgBase/footer.js"); break;
		}
	}
}

function initPage()
{
	var funcType = typeof(constructPageContent) == 'function';
	scrLoaded = 0;
	funcType &= (document.styleSheets.length > 0);
	if (funcType) scrLoaded += 1;
	funcType &= (typeof(loadPageList) == 'function');
	if (funcType) scrLoaded += 1;
	funcType &= (typeof(createMenueTabElement) == 'function');
	if (funcType) scrLoaded += 1;
	funcType &= (typeof(tfSetCoordinate) == 'function');
	if (funcType) scrLoaded += 1;
	funcType &= (typeof(tfBtnCommandEditor) == 'function');
	if (funcType) scrLoaded += 1;
	funcType &= (typeof(enableInput) == 'function');
	if (funcType) scrLoaded += 1;
	funcType &= (typeof(upgradeJSONVersionGH) == 'function');
	if (funcType) scrLoaded += 1;
	funcType &= (typeof(constructFooterContent) == 'function');
	if (funcType) scrLoaded += 1;
	if (funcType == true)
	{
		loadPageList(fileName, document.getElementById("MenueHolder"), document.getElementById("TabHolder"), document.getElementById("FooterHolder"));
		setTimeout(function(){ startWebsockets() }, 50);
	}
	else
	{
		if (scrRequested == scrLoaded)
		{
			loadScripts(scrRequested);
			scrRequested += 1;
		}
		setTimeout(function(){ initPage() }, 50);
	}
}
