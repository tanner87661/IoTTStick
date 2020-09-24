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
var transferData; //multifile system to save large data structure in multiple files
var progressDlg = null;
var dlgTextDispArea;
var fileSendIndex = 0;

const targetSize = 10240; //10kb target length of file strings, should always fit into 16kB wsBuffer of asyncServer
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
						createMenueTabElement(menueTab, "button", "tablink", scriptList.Pages[i].ID, scriptList.Pages[i].Menue, false,"loadPage('" + scriptList.Pages[i].WebPage+ "')");
						
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

function downloadConfig(fileSelect) //get files from Stick and store to file
{
	ws.send("{\"Cmd\":\"CfgFiles\", \"Type\":" + fileSelect + "}");
}

function showMenueTabs(thisConfigData)
{
	for (i=0; i<scriptList.Pages.length;i++)
	{
		var isVisible = false;
		var tabElement = document.getElementById(scriptList.Pages[i].ID);
		if ((scriptList.Pages[i].ProdSel.indexOf(-1) >= 0) || (scriptList.Pages[i].CommSel.indexOf(-1) >= 0))
		{
			isVisible = true;
		}
		var thisId = parseInt(thisConfigData.HatTypeList[thisConfigData.HatIndex].HatId);
		if (scriptList.Pages[i].ProdSel.indexOf(thisId) >= 0)
		{
			isVisible = true;
		}
		thisId = parseInt(thisConfigData.InterfaceTypeList[thisConfigData.InterfaceIndex].IntfId);
		if (scriptList.Pages[i].CommSel.indexOf(thisId) >= 0)
		{
			isVisible = true;
		}
		for (var j=0; j < thisConfigData.ALMIndex.length; j++)
			if (scriptList.Pages[i].ALMSel.indexOf(thisConfigData.ALMIndex[j]) >= 0)
		{
			isVisible = true;
		}

		setVisibility(isVisible, tabElement);
	}
}

function loadPage(pageURL)
{
	window.location.href = pageURL;
}

function sendFileData(itemType, filenametype, filename, itemData, restart)
{
	var configStr = "{\"Cmd\":\"CfgUpdate\", \"FileNameType\": \"" + filenametype + "\", \"FileName\": \"" + filename + "\", \"Restart\": " + restart + ", \"Type\":\"" + itemType + "\", \"Data\":" + itemData  + "}";
	ws.send(configStr);
//	console.log(configStr);
}

function sendDeleteFile(itemType, filenametype, filename, startIndex)
{
	var configStr = "{\"Cmd\":\"CfgUpdate\", \"FileNameType\": \"" + filenametype + "\", \"FileName\": \"" + filename + "\", \"Type\":\"" + itemType + "\", \"Index\":" + startIndex  + "}";
	ws.send(configStr);
	console.log(configStr);
}

function startProgressDialog(parentObj)
{
	var mainDlg = document.createElement("div");
	mainDlg.setAttribute('class', "modal");
	
		var dlgDiv = document.createElement("div");
		dlgDiv.setAttribute('class', "modal-content");
		mainDlg.append(dlgDiv);
	
			var dlgSubDiv = document.createElement("div");
			dlgSubDiv.setAttribute('class', "modal-header");
			dlgDiv.append(dlgSubDiv);
				var dlgSpan = document.createElement("span");
				dlgSpan.setAttribute('class', "close");
				dlgSpan.innerHTML = "&times;";
				dlgSubDiv.append(dlgSpan);
				var dlgHeader = document.createElement("h2");
				dlgHeader.innerHTML = "Please wait... Configuration files are being transferred.";
				dlgSubDiv.append(dlgHeader);
	
			dlgSubDiv = document.createElement("div");
			dlgSubDiv.setAttribute('class', "modal-body");
			dlgDiv.append(dlgSubDiv);
				dlgTextDispArea = document.createElement("textarea"); //global var
				dlgTextDispArea.setAttribute('readonly', "true");
				dlgTextDispArea.setAttribute('rows', 10);
				dlgTextDispArea.setAttribute('style', "width:100%"); 
				dlgSubDiv.append(dlgTextDispArea);

			dlgSubDiv = document.createElement("div");
			dlgSubDiv.setAttribute('class', "modal-footer");
			dlgDiv.append(dlgSubDiv);
				var dlgHeader = document.createElement("h3");
				dlgHeader.innerHTML = "Thank you for using the IoTT Stick!";
				dlgSubDiv.append(dlgHeader);
	
	parentObj.append(mainDlg);
	var span = document.getElementsByClassName("close"); //get array of close elements, should only be 1
	span[0].onclick = function() {progressDlg.style.display = "none";}
	return mainDlg;
}

function closeDlg()
{
	wsInitData = true;
	setTimeout(function(){startWebsockets() }, 2500);
	if (progressDlg)
		progressDlg.style.display = "none";
}

function uploadConfig(fromJSON) //load config data files to IoTT Stick
{
	var fileTypeName = "";
	function findFileName(pageEntry)
	{
		return pageEntry.ID == fileTypeName;
	}
	
	transferData = [];
	
	for (var i = 0; i < fromJSON.length; i++)
	{
		if (fromJSON[i].FileName == undefined)
		{
			fileTypeName = fromJSON[i].Type;
			var thisIndex = scriptList.Pages.findIndex(findFileName);
			if (!isNaN(thisIndex))
				fromJSON[i].FileName = scriptList.Pages[thisIndex].FileName + ".cfg";
		}
		sendDeleteFile("pgDelete", fromJSON[i].FileNameType, fromJSON[i].FileName, 0); //delete the one file after the last, otherwise it will be loaded
		transferData.push(JSON.parse(JSON.stringify(fromJSON[i])));
	}
	if (progressDlg)	 
		dlgTextDispArea.innerHTML = "";
	fileSendIndex = 0;
	sendMultiFile();
}

function sendSingleFile()
{
//	console.log("Create Single File Dlg");
	if (progressDlg ==  null)
		progressDlg = startProgressDialog(document.getElementById("TabHolder"));
	progressDlg.style.display = "block";
		
		
	var thisFileName = transferData.FileName;
	if (fileSendIndex > 0)
		thisFileName += fileSendIndex.toString();
	thisFileName += ".cfg";
//	console.log("Send ", thisFileName);
	sendFileData(transferData.Type, transferData.FileName, thisFileName, JSON.stringify(transferData.FileList[fileSendIndex]), fileSendIndex == transferData.FileList.length - 1);
	dlgTextDispArea.innerHTML += "Send file " + thisFileName + " to IoTT Stick\n";
	fileSendIndex++; //now number of files sent so far
	if (fileSendIndex < transferData.FileList.length) //more to send
		setTimeout(function(){sendSingleFile() }, 500);
	else
	{
		setTimeout(function(){closeDlg() }, 2500);
	}
}

function sendMultiFile()
{
//	console.log("Create Multi File Dlg");
	if (progressDlg ==  null)
		progressDlg = startProgressDialog(document.getElementById("TabHolder"));
	progressDlg.style.display = "block";
	dlgTextDispArea.innerHTML += "Send file " + transferData[fileSendIndex].FileName + " to IoTT Stick\n";
	sendFileData(transferData[fileSendIndex].Type, transferData[fileSendIndex].FileNameType, transferData[fileSendIndex].FileName, JSON.stringify(transferData[fileSendIndex].Data), fileSendIndex == transferData.length - 1);
	fileSendIndex++;
	if (fileSendIndex < transferData.length)
	{
		setTimeout(function(){sendMultiFile() }, 500);
	}
	else
	{
		setTimeout(function(){closeDlg() }, 2500);
	}
}

function saveSettings(sender)
{
	transferData = {"Type": "", "FileName": "", "FileNameType": "", "FileList" : []};
	transferData.Type = scriptList.Pages[currentPage].ID;
	transferData.FileName = scriptList.Pages[currentPage].FileName;
	transferData.FileNameType = scriptList.Pages[currentPage].FileName;
	if (typeof prepareFileSeq == 'function') 
	{ 
		prepareFileSeq(); 
		if (transferData.FileList.length > 0)
			sendDeleteFile("pgDelete", transferData.FileNameType, transferData.FileName, transferData.FileList.length); //delete the one file after the last, otherwise it will be loaded
	}
	else
	{
		var objCopy = JSON.parse(JSON.stringify(configWorkData));
		transferData.FileList.push(objCopy);
	}
	
	if (progressDlg)	 
		dlgTextDispArea.innerHTML = "";
	fileSendIndex = 0;
	sendSingleFile();
}

function cancelSettings(sender)
{
	configWorkData = JSON.parse(JSON.stringify(configLoadData));
	loadDataFields(configWorkData);
}

function startWebsockets()
{
	ws = new WebSocket(serverIP);
	  
    ws.onopen = function() 
    {
		console.log("websock opening");
		if (wsInitData)
		{
			if (scriptList.Pages[currentPage].ID != "pgNodeCfg")
				ws.send("{\"Cmd\":\"CfgData\", \"Type\":\"pgNodeCfg\", \"FileName\": \"node\"}");
			ws.send("{\"Cmd\":\"CfgData\", \"Type\":\"" + scriptList.Pages[currentPage].ID + "\", \"FileName\":\"" + scriptList.Pages[currentPage].FileName+ "\"}");
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
			processLocoNetInput(myArr.Data);
  		if ((myArr.Cmd == "HWBtn") && (scriptList.Pages[currentPage].ID == "pgThrottleCfg"))
			processLocoNetInput(myArr.Data);
  		if ((myArr.Cmd == "LN") && (scriptList.Pages[currentPage].ID == "pgLNViewer"))
			processLocoNetInput(myArr.Data);
  		if ((myArr.Cmd == "OLCB") && (scriptList.Pages[currentPage].ID == "pgOLCBViewer"))
			processOLCBInput(myArr);
  		if ((myArr.Cmd == "DCC") && (scriptList.Pages[currentPage].ID == "pgDCCViewer"))
			processDCCInput(myArr.Data);
  		if ((String(myArr.Cmd).indexOf("MQTT") >= 0) && (scriptList.Pages[currentPage].ID == "pgMQTTViewer"))
			processMQTTInput(myArr);

  		if (myArr.Cmd == "CfgFiles")
  		{
//			console.log(myArr);
			switch (myArr.FileMode)
			{
				case 1: //add file to list
					dlgTextDispArea.innerHTML += "Receive file " + myArr.FileName + " from IoTT Stick\n";
					transferData.push(JSON.parse(JSON.stringify(myArr)));
					break;
				case 2: //delete list and restart
					transferData = [];
					if (progressDlg ==  null)
						progressDlg = startProgressDialog(document.getElementById("TabHolder"));
					dlgTextDispArea.innerHTML = "";
					progressDlg.style.display = "block";
					break;
				case 3: //write file to disk
					if (progressDlg)
						progressDlg.style.display = "none";
					const a = document.createElement("a");
					a.href = URL.createObjectURL(new Blob([JSON.stringify(transferData, null, 2)], {type: "text/plain"}));
					a.setAttribute("download", "M5Config.json");
					document.body.appendChild(a);
					a.click();
					document.body.removeChild(a);			
					break;
			}
		}

  		if (myArr.Cmd == "CfgData")
  		{
			console.log(myArr.Data);
			if (myArr.Type == "pgNodeCfg") //this is pgNodeCfg for all other pages
			{
				wsInitData = false;
				nodeConfigData = JSON.parse(JSON.stringify(myArr.Data));
				showMenueTabs(nodeConfigData);
				loadNodeDataFields(nodeConfigData);
			}
			if (myArr.Type == scriptList.Pages[currentPage].ID)
			{
				if (myArr.ResetData) //new data to be loaded
				{
					wsInitData = false;
					configLoadData = JSON.parse(JSON.stringify(myArr.Data));
					configWorkData = JSON.parse(JSON.stringify(myArr.Data));
				}
				else //data is additional file to current block
				{
					if (typeof addFileSeq == 'function')
						addFileSeq(myArr.Data);
				}
				loadDataFields(configWorkData);
			}
		}
	}
};

