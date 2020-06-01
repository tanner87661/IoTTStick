var systemTime = new Date();
var serverIP = "ws://" + location.hostname + "/ws";  
var ws = null; // = new WebSocket(serverIP);
//var loadedScripts = [];
var scriptList;
var currentPage = 0;

var wsInitData = true;

var nodeConfigData;
var configLoadData;
var configWorkData;
//var configFileTransfer;

function loadPageList(pageName, menueTab, contentTab, footerTab)
{
    var request = new XMLHttpRequest();
	request.onreadystatechange = function()
	{
		if (this.readyState == 4) 
		{
			if (this.status == 200) 
			{
				scriptList = JSON.parse(this.response);
//				console.log(scriptList, scriptList.Pages.length);
				for (i=0; i<scriptList.Pages.length;i++)
				{
//					console.log("Loading Page", pageName, scriptList.Pages[i].WebPage );
					if (scriptList.Pages[i].WebPage == pageName)
					{
//						console.log("Setting ID", pageName, scriptList.Pages[i].ID );
						currentPage = i; //scriptList.Pages[i].ID;
						createMenueTabElement(menueTab, "button", "tablink", scriptList.Pages[i].ID, scriptList.Pages[i].Menue, true, "");
					}
					else
						createMenueTabElement(menueTab, "button", "tablink", scriptList.Pages[i].ID, scriptList.Pages[i].Menue, false, "loadPage('" + scriptList.Pages[i].WebPage+ "')");
						
				}
				updateMenueTabs(scriptList.Pages[currentPage].ID, "grey");
				constructPageContent(contentTab);
				constructFooterContent(footerTab);
			}
		}
	}
	request.open("GET", "pageconfig.json", true);
	request.setRequestHeader('Cache-Control', 'no-cache');
	request.send();
}

function updateMenueTabs(pageID, color) 
{
	tablinks = document.getElementsByClassName("tablink");
	for (i = 0; i < tablinks.length; i++) 
	{
//		console.log(tablinks[i].id, pageID);
		if (tablinks[i].id == pageID)
			tablinks[i].style.backgroundColor = color;
		else
			tablinks[i].style.backgroundColor = "";
	}
}

function setLEDTestDisplay(ledNr)
{
//	console.log("Setting LED: {\"Cmd\":\"SetLED\", \"LedNr\":[" + ledNr + "]}");
	ws.send("{\"Cmd\":\"SetLED\", \"LedNr\":[" + ledNr + "]}");
}

function downloadConfig(toFile) //get files from Stick and store to file
{
	ws.send("{\"Cmd\":\"CfgFiles\", \"Type\":\"AllConfig\"}");
}

function uploadConfig(fromJSON) //load config data files to IoTT Stick
{
	function sendFileData(item, restart)
	{
		var configStr = "{\"Cmd\":\"CfgUpdate\", \"Restart\": " + restart + ", \"Type\":\"" + item.Type + "\", \"Data\":" + JSON.stringify(item.Data)  + "}";
		ws.send(configStr);
	}
	
	for (var i = fromJSON.length; i > 0; i--)
	{
		sendFileData(fromJSON[i-1], i==1);
	}
	wsInitData = true;
	alert("Load configuration and restart device");
}

function showMenueTabs(thisConfigData)
{
	for (i=0; i<scriptList.Pages.length;i++)
	{
		var isVisible = false;
		var tabElement = document.getElementById(scriptList.Pages[i].ID);
		if ((scriptList.Pages[i].ProdSel.indexOf(-1) >= 0) || (scriptList.Pages[i].CommSel.indexOf(-1) >= 0))
			isVisible = true;
		var thisId = parseInt(thisConfigData.HatTypeList[thisConfigData.HatIndex].HatId);
		if (scriptList.Pages[i].ProdSel.indexOf(thisId) >= 0)
			isVisible = true;
		thisId = parseInt(thisConfigData.InterfaceTypeList[thisConfigData.InterfaceIndex].IntfId);
		if (scriptList.Pages[i].CommSel.indexOf(thisId) >= 0)
			isVisible = true;
		for (var j=0; j < thisConfigData.ALMIndex.length; j++)
			if (scriptList.Pages[i].ALMSel.indexOf(thisConfigData.ALMIndex[j]) >= 0)
				isVisible = true;
		setVisibility(isVisible, tabElement);
	}
}

function loadPage(pageURL)
{
	window.location.href = pageURL;
}

function saveSettings(sender)
{
	var configStr = "{\"Cmd\":\"CfgUpdate\", \"Restart\": true, \"Type\":\"" + scriptList.Pages[currentPage].ID + "\", \"Data\":" + JSON.stringify(configWorkData)  + "}";
	ws.send(configStr);
	alert("Save configuration and restart device");
	wsInitData = true;
	setTimeout(function(){ startWebsockets() }, 500);
}

function cancelSettings(sender)
{
	configWorkData = JSON.parse(JSON.stringify(configLoadData));
	loadDataFields(configWorkData);
}

//function sendUpdateFile(fileBlob)
//{
//	ws.binaryType = "blob";
//	ws.send(fileBlob);
//}

function startWebsockets()
{
	ws = new WebSocket(serverIP);
	  
    ws.onopen = function() 
    {
		console.log("websock opening");
		if (wsInitData)
		{
			if (scriptList.Pages[currentPage].ID != "pgNodeCfg")
				ws.send("{\"Cmd\":\"CfgData\", \"Type\":\"pgNodeCfg\"}");
			ws.send("{\"Cmd\":\"CfgData\", \"Type\":\"" + scriptList.Pages[currentPage].ID + "\"}");
		}
    };
 
	ws.onclose = function (evt) 
	{
		console.log("websock close");
		setTimeout(startWebsockets, 3000);
//		conStatusError();
	};

	ws.onerror = function (evt) 
	{
		console.log(evt);
//		  conStatusError();
	};

    ws.onmessage = function(evt) 
    {
//		console.log(evt.data);
//		console.log(currentPage);
  		var myArr = JSON.parse(evt.data);
  		if (myArr.Cmd == "STATS")
  		{
//			console.log(JSON.stringify(myArr.Data));
			processStatsData(myArr.Data);
		}
  		if ((myArr.Cmd == "HWBtn") && (scriptList.Pages[currentPage].ID == "pgHWBtnCfg"))
  		{
//			console.log(JSON.stringify(myArr.Data));
			processLocoNetInput(myArr.Data);
		}
  		if ((myArr.Cmd == "LN") && (scriptList.Pages[currentPage].ID == "pgLNViewer"))
			processLocoNetInput(myArr.Data);
  		if ((myArr.Cmd == "DCC") && (scriptList.Pages[currentPage].ID == "pgDCCViewer"))
			processDCCInput(myArr.Data);
  		if (myArr.Cmd == "CfgFiles")
  		{
//			console.log(myArr.FileEntries);
			const a = document.createElement("a");
			a.href = URL.createObjectURL(new Blob([JSON.stringify(myArr.FileEntries, null, 2)], {type: "text/plain"}));
			a.setAttribute("download", "M5Config.json");
			document.body.appendChild(a);
			a.click();
			document.body.removeChild(a);			
		}
  		if (myArr.Cmd == "CfgData")
  		{
			wsInitData = false;
			if (myArr.Type == scriptList.Pages[currentPage].ID)
			{
				configLoadData = JSON.parse(JSON.stringify(myArr.Data));
				configWorkData = JSON.parse(JSON.stringify(myArr.Data));
				loadDataFields(configWorkData);
			}
			if (myArr.Type == "pgNodeCfg") //this is pgNodeCfg for all other pages
			{
				nodeConfigData = JSON.parse(JSON.stringify(myArr.Data));
				showMenueTabs(nodeConfigData);
				loadNodeDataFields(nodeConfigData);
			}
		}
	}
};

