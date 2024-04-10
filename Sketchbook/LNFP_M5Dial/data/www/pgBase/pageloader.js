var systemTime = new Date();
var serverIP = "ws://" + location.hostname + "/ws";  
var ws = null; // = new WebSocket(serverIP);
var urlString = window.location.href;
let paramString = urlString.split('?')[1];
let queryString = new URLSearchParams(paramString);
var pageParam = "";
var thisIntfID = 0; //= configData[nodeCfg].InterfaceTypeList[configData[nodeCfg].InterfaceIndex].IntfId;
var thisHatID = 0;
var thisServerList = [];
var thisALMList = [];

//console.log(queryString);
for (var px of queryString.entries()) 
{
    pageParam = px[0];
    break;
}


//var loadedScripts = [];
var scriptList;
var currentPage = 0;

var wsInitNode = true;
var wsInitData = true;
var flagCTS = true;
var ctsTimeout;

var	transferDataObj = {"Type": "", "FileName": "", "FileNameType": "", "FileList" : []};

var nodeCfg = 0;
var loadCfg = 1;
var workCfg = 2;
var configData = [{},{},{}];

var transferData; //multifile system to save large data structure in multiple files
var progressDlg = null;
var dlgTextDispArea;
var fileGroupIndex = 0;
var fileSendIndex = 0;

var storeConfigFiles = false;

var targetSize = 8192; //8kB target length of file strings, should always fit into 16kB wsBuffer of asyncServer
//var targetSize = 4096; //4kb target length of file strings, should always fit into 16kB wsBuffer of asyncServer
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
				for (i=0; i<scriptList.Pages.length;i++)
				{
					var paramOK = true;
					var paramStr = (scriptList.Pages[i].URLParam) ? "?" + scriptList.Pages[i].URLParam : "";
					if (pageParam.length > 0)
						paramOK = (scriptList.Pages[i].URLParam == pageParam);

					if ((scriptList.Pages[i].WebPage.indexOf(pageName) >= 0) && paramOK)
					{
//						console.log("Setting ID", pageName, scriptList.Pages[i].ID + paramStr);
						currentPage = i; //scriptList.Pages[i].ID;
						createMenueTabElement(menueTab, "button", "tablink", "main", scriptList.Pages[i].ID + paramStr, scriptList.Pages[i].Menue, true, "");
					}
					else
					{
						createMenueTabElement(menueTab, "button", "tablink", "main", scriptList.Pages[i].ID + paramStr, scriptList.Pages[i].Menue, false,"loadPage('" + scriptList.Pages[i].WebPage + paramStr + "')");
					}
//					console.log("Loading Page", pageName, scriptList.Pages[i].WebPage, scriptList.Pages[i].WebPage.indexOf(pageName), scriptList.Pages[i].ID + paramStr);
						
				}
				var paramStr = (scriptList.Pages[currentPage].URLParam) ? "?" + scriptList.Pages[currentPage].URLParam : "";
				updateMenueTabs("main", scriptList.Pages[currentPage].ID + paramStr, "grey");
				constructPageContent(contentTab);
				constructFooterContent(footerTab);
			}
		}
	}
	request.open("GET", "pageconfig.json", true);
	request.setRequestHeader('Cache-Control', 'no-cache');
	request.send()
}

function updateMenueTabs(menuName, pageID, color) 
{
	tablinks = document.getElementsByClassName("tablink");
	for (i = 0; i < tablinks.length; i++) 
	{
//		console.log(tablinks[i].id, pageID);
		if (tablinks[i].name == menuName)
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

function setServoPos(servoNr, servoPos)
{
//	console.log("{\"Cmd\":\"SetServo\", \"ServoNr\":[" + servoNr + "], \"ServoPos\":" + servoPos + "}");
	ws.send("{\"Cmd\":\"SetServo\", \"ServoNr\":[" + servoNr + "], \"ServoPos\":" + servoPos + "}");
}

function setSensorReportRate(rateMillis)
{
	ws.send("{\"Cmd\":\"SetSensor\", \"SubCmd\":\"RepRate\",\"Val\":" + rateMillis + "}");
}

function reqPageStats()
{
	ws.send("{\"Cmd\":\"ReqStats\"}");
}

function downloadConfig(fileSelect) //get files from Stick and store to file
{
	var moduleID = 0;
	if ((fileSelect & 0xFFFC) == 0x0200) //Greenhat, so we filter out module
	{
		moduleID = fileSelect & 0x0003;
		fileSelect &= 0x0200;
	}
	ws.send("{\"Cmd\":\"CfgFiles\", \"Type\":" + fileSelect + ", \"ModuleNr\":" + moduleID + "}");
}

function showMenueTabs(thisConfigData)
{
	for (i=0; i<scriptList.Pages.length;i++)
	{
		var isVisible = false;
		var paramStr = (scriptList.Pages[i].URLParam) ? "?" + scriptList.Pages[i].URLParam : "";
		var tabElement = document.getElementById(scriptList.Pages[i].ID + paramStr);
		var thisHatId = parseInt(thisConfigData.HatTypeList[thisConfigData.HatIndex].HatId);
		var thisInterfaceId = parseInt(thisConfigData.InterfaceTypeList[thisConfigData.InterfaceIndex].IntfId);
		
		if ((scriptList.Pages[i].ProdSel.indexOf(-1) >= 0) || (scriptList.Pages[i].CommSel.indexOf(-1) >= 0)) //-1 sets it true in every case
		{
			isVisible = true;
//			console.log(1);
		}
		if (scriptList.Pages[i].ProdSel.indexOf(thisHatId) >= 0)
		{
			isVisible = true;
//			console.log(2);
//			console.log(thisConfigData.HatIndex, thisConfigData.HatTypeList[thisConfigData.HatIndex].HatId, 2, thisId);
		}
		if (scriptList.Pages[i].CommSel.indexOf(thisInterfaceId) >= 0)
		{
			isVisible = true;
//			console.log(3);
		}
		for (var j=0; j < thisConfigData.ALMIndex.length; j++)
			if (scriptList.Pages[i].ALMSel.indexOf(thisConfigData.ALMIndex[j]) >= 0)
		{
			isVisible = true;
//			console.log(4);
		}
		for (var j=0; j < thisConfigData.ServerIndex.length; j++)
			if (scriptList.Pages[i].ServerSel.indexOf(thisConfigData.ServerIndex[j]) >= 0)
		{
			isVisible = true;
//			console.log(5);
		}
//		if (isVisible)
//			console.log(tabElement);
		setVisibility(isVisible, tabElement);

	}
}

function loadPage(pageURL)
{
	window.location.href = pageURL;
}

function sendFileData(itemType, filenametype, filename, itemData, restart)
{
	var configStr = "{\"Cmd\":\"CfgUpdate\", \"FileNameType\": \"" + filenametype + "\", \"FileName\": \"" + filename + "\", \"Restart\": " + restart + ", \"Type\":\"" + itemType + "\", \"Data\":" + JSON.stringify(itemData)  + "}";
	clearCTS();
	ws.send(configStr);
//	console.log(JSON.parse(configStr));
}

function sendDeleteFile(itemType, filenametype, filename, startIndex)
{
	if (flagCTS)
	{
//		if (dlgTextDispArea)
		{
			dlgTextDispArea.innerHTML += "Delete file " + filename + " from IoTT Stick\n";
			scrollTextArea();
		}
		var configStr = "{\"Cmd\":\"CfgUpdate\", \"FileNameType\": \"" + filenametype + "\", \"FileName\": \"" + filename + "\", \"Type\":\"" + itemType + "\", \"Index\":" + startIndex  + "}";
		clearCTS();
		ws.send(configStr);
//		console.log(JSON.parse(configStr));
	}
	else
	{
//		console.log("wait for flag 160");
		setTimeout(function(){sendDeleteFile(itemType, filenametype, filename, startIndex)}, 1000);
	}
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

function scrollTextArea()
{
	if (dlgTextDispArea)
		dlgTextDispArea.scrollTop = dlgTextDispArea.scrollHeight;
}

function closeDlg()
{
	wsInitNode = true;
	wsInitData = true;
	setTimeout(function(){startWebsockets() }, 2500);
	if (progressDlg)
		progressDlg.style.display = "none";
}

function uploadConfig(fromJSON) //load config data files (from disk file) to IoTT Stick
{
	if (flagCTS)
	{
		var fileTypeName = "";
		function findFileName(pageEntry)
		{
			return pageEntry.ID == fileTypeName;
		}
	
		transferData = [];

//		console.log("Create Upload Config File Dlg ", fileGroupIndex, fileSendIndex );
		if (progressDlg ==  null)
			progressDlg = startProgressDialog(document.getElementById("TabHolder"));
			
		if (progressDlg)	 
			dlgTextDispArea.innerHTML = "";
		progressDlg.style.display = "block";
	
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
		fileSendIndex = 0;
		sendMultiFile();
	}
	else
	{
//		console.log("wait for flag 239");
		setTimeout(function(){uploadConfig(fromJSON)}, 1000);
	}
}

function setCTS()
{
//	console.log("set CTS");
	flagCTS = true;
	clearTimeout(ctsTimeout);
}

function clearCTS()
{
//	console.log("clear CTS");
	flagCTS = false;
	ctsTimeout = setTimeout(function(){setCTS() }, 5000);
}

function sendSingleFile()
{
	if (flagCTS == true)
	{
//		console.log("Create Single File Dlg ", fileGroupIndex, fileSendIndex );
		if (progressDlg ==  null)
			progressDlg = startProgressDialog(document.getElementById("TabHolder"));
		progressDlg.style.display = "block";
	
		var thisTransferData = transferData[fileGroupIndex];
		var thisFileName = thisTransferData.FileName;
		if (fileSendIndex > 0)
			thisFileName += fileSendIndex.toString();
		thisFileName += ".cfg";
		var doRestart = (fileGroupIndex == transferData.length - 1) && (fileSendIndex == thisTransferData.FileList.length - 1);
		sendFileData(thisTransferData.Type, thisTransferData.FileName, thisFileName, JSON.stringify(thisTransferData.FileList[fileSendIndex]), doRestart);
		dlgTextDispArea.innerHTML += "Send file " + thisFileName + " to IoTT Stick\n";
		scrollTextArea();
		fileSendIndex++; //now number of files sent so far

		if (fileSendIndex < thisTransferData.FileList.length) //more to send
			setTimeout(function(){sendSingleFile() }, 4000);
		else
		{
			fileGroupIndex++; //now number of groups completed so far
			if (fileGroupIndex < transferData.length)
			{
				fileSendIndex = 0;
				setTimeout(function(){sendSingleFile() }, 4000);
			}
			else
			{
				if (typeof clearDisplay == 'function')
					clearDisplay();
				setTimeout(function(){closeDlg() }, 4000);
			}
		}
	}
	else
	{
//		console.log("wait for flag 297");
		setTimeout(function(){sendSingleFile()}, 1000);
	}
}

function sendMultiFile()
{
	if (flagCTS == true)
	{
//		console.log("Create Multi File Dlg");
		if (progressDlg ==  null)
			progressDlg = startProgressDialog(document.getElementById("TabHolder"));
		progressDlg.style.display = "block";
		dlgTextDispArea.innerHTML += "Send file " + transferData[fileSendIndex].FileName + " to IoTT Stick\n";
		scrollTextArea();
		sendFileData(transferData[fileSendIndex].Type, transferData[fileSendIndex].FileNameType, transferData[fileSendIndex].FileName, JSON.stringify(transferData[fileSendIndex].Data), fileSendIndex == transferData.length - 1);
		fileSendIndex++;
		if (fileSendIndex < transferData.length)
		{
			setTimeout(function(){sendMultiFile() }, 1000);
		}
		else
		{
			setTimeout(function(){closeDlg() }, 4000);
		}
	}
	else
	{
//		console.log("wait for flag 324");
		setTimeout(function(){sendMultiFile()}, 1000);
	}
}
	
function saveJSONConfig(fileType, fileName, configData, fileSequencer)
{
	if (flagCTS)
	{
		var thisIndex = transferData.push(JSON.parse(JSON.stringify({"Type": "", "FileName": "", "FileNameType": "", "FileList" : []}))) - 1;
		transferData[thisIndex].Type = fileType;
		transferData[thisIndex].FileName = fileName;
		transferData[thisIndex].FileNameType = fileName;
		if (progressDlg ==  null)
			progressDlg = startProgressDialog(document.getElementById("TabHolder"));
		progressDlg.style.display = "block";
		if (typeof fileSequencer == 'function') 
		{ 
			fileSequencer(configData, thisIndex); 
			if (transferData[thisIndex].FileList.length > 0)
				sendDeleteFile("pgDelete", transferData[thisIndex].FileNameType, transferData[thisIndex].FileName, transferData[thisIndex].FileList.length); //delete the one file after the last, otherwise it will be loaded
		}
		else
		{
			var objCopy = JSON.parse(JSON.stringify(configData));
			transferData[thisIndex].FileList.push(objCopy);
		}
	}
	else
	{
//		console.log("wait for flag 329");
		setTimeout(function(){saveJSONConfig(fileType, fileName, configData, fileSequencer)}, 1000);
	}
}

function saveSettings(sender)
{
	transferData = []; //empty array
	if (progressDlg)	 
		dlgTextDispArea.innerHTML = "";
	saveConfigFileSettings(); //this must be a function in each config.js
	fileSendIndex = 0;
	fileGroupIndex = 0;
//	console.log(transferData);
	sendSingleFile();
}

function cancelSettings(sender)
{
	configData[2] = JSON.parse(JSON.stringify(configData[1]));
	loadDataFields(configData[workCfg]);
}

function loadInitData()
{
//	console.log("loadInitData", wsInitNode, wsInitData);
	if (wsInitNode)
	{
		if (scriptList != undefined)
		{
			if (scriptList.Pages[currentPage].ID != "pgNodeCfg")
			{
				ws.send("{\"Cmd\":\"CfgData\", \"Type\":\"pgNodeCfg\", \"FileName\": \"node\"}");
//				console.log("{\"Cmd\":\"CfgData\", \"Type\":\"pgNodeCfg\", \"FileName\": \"node\"}");
			}
			else
				wsInitNode = false;
		}
		else
			console.log("Waiting for Script List");
		setTimeout(loadInitData, 1000);
	}
	else
		if (wsInitData)
		{
			ws.send("{\"Cmd\":\"CfgData\", \"Type\":\"" + scriptList.Pages[currentPage].ID + "\", \"FileName\":\"" + scriptList.Pages[currentPage].FileName+ "\"}");
//			console.log("{\"Cmd\":\"CfgData\", \"Type\":\"" + scriptList.Pages[currentPage].ID + "\", \"FileName\":\"" + scriptList.Pages[currentPage].FileName+ "\"}");
			setTimeout(reqPageStats, 1000);
		}
}

function writeDiskFile(newBlob, fileName)
{
	var a = document.createElement("a");
	a.href = URL.createObjectURL(newBlob);
	a.setAttribute("download", fileName);
	a.style.display = 'none';
	document.body.appendChild(a);
	a.click();
	document.body.removeChild(a);	
}

function startWebsockets()
{
//	console.log(serverIP);
	ws = new WebSocket(serverIP);
	  
    ws.onopen = function() 
    {
//		console.log("websock opening");
		
		if (wsInitData || wsInitNode)
			setTimeout(loadInitData, 1000);
    };
 
	ws.onclose = function (evt) 
	{
		console.log("websock close");
		wsInitNode = true;
		wsInitData = true;
		setTimeout(startWebsockets, 3000);
//		conStatusError();
	};

	ws.onerror = function (evt) 
	{
//		console.log(evt);
//		  conStatusError();
	};

    ws.onmessage = function(evt) 
    {
//		console.log(evt.data);
//		console.log(currentPage);
		try
		{
			var myArr = JSON.parse(evt.data);
		}
  		catch(err)
  		{
			console.log(err.message);
			return;
		}
  		if (myArr.Cmd == "CTS")
			setCTS();
  		
  		if (myArr.Cmd == "STATS")
  		{
//			console.log(JSON.stringify(myArr.Data));
			processStatsData(myArr.Data);
		}
  		if ((myArr.Cmd == "SensorData") && (scriptList.Pages[currentPage].ID == "pgPrplHatCfg"))
			processSensorInput(myArr.Data);
  		if ((myArr.Cmd == "SpeedTableData") && (scriptList.Pages[currentPage].ID == "pgPrplHatCfg"))
			processSpeedTableInput(myArr.Data, true); //smoothen curve
  		if ((myArr.Cmd == "ProgReturn") && (scriptList.Pages[currentPage].ID == "pgPrplHatCfg"))
			processProgrammerInput(myArr.Data);
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
  		if ((myArr.Cmd == "DCCPP") && (scriptList.Pages[currentPage].ID == "pgRedHatCfg"))
 			processDCCPPInput(myArr);
  		if ((myArr.Cmd == "FC") && (scriptList.Pages[currentPage].ID == "pgRedHatCfg"))
 			processFCInput(myArr);
  		if ((myArr.Cmd == "DCCAmp") && (scriptList.Pages[currentPage].ID == "pgRedHatCfg"))
 			processTrackDataInput(myArr);
  		if ((myArr.Cmd == "DCCAmp") && (scriptList.Pages[currentPage].ID == "pgSilverHatCfg"))
 			processTrackDataInput(myArr);
  		if ((myArr.Cmd == "SVR") && (scriptList.Pages[currentPage].ID == "pgSilverHatCfg"))
 			processConfigDataInput(myArr);
  		if ((myArr.Cmd == "ClientList") && ((scriptList.Pages[currentPage].ID == "pgWiCfg") || (scriptList.Pages[currentPage].ID == "pgLBSCfg")))
 			processClientList(myArr.Data);

  		if (myArr.Cmd == "CfgFiles")
  		{
//			console.log(myArr);
			switch (myArr.FileMode)
			{
				case 1: //add file to list
					dlgTextDispArea.innerHTML += "Receive file " + myArr.FileName + " from IoTT Stick\n";
					scrollTextArea();
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
						
						
					if (storeConfigFiles) //store individual config files
					{
//						console.log("SHIFT ", transferData.length);
						for (var i = 0; i < transferData.length; i++)
						{
							var a = document.createElement("a");
							a.href = URL.createObjectURL(new Blob([JSON.stringify(transferData[i].Data, null, 2)], {type: "text/plain"}));
							a.setAttribute("download", transferData[i].FileName);
							a.style.display = 'none';
							document.body.appendChild(a);
							a.click();
							document.body.removeChild(a);	
						}
					}
					else
					{
						var a = document.createElement("a");
						a.href = URL.createObjectURL(new Blob([JSON.stringify(transferData, null, 2)], {type: "text/plain"}));
						a.setAttribute("download", "IoTTConfig.json");
						a.style.display = 'none';
						document.body.appendChild(a);
						a.click();
						document.body.removeChild(a);	
					}
					break;
			}
		}
  		if (myArr.Cmd == "CfgData")
  		{
//			console.log(myArr);
			if (myArr.Type == "pgNodeCfg") //this is pgNodeCfg for all other pages
			{
				wsInitNode = false;
				configData[nodeCfg] = JSON.parse(JSON.stringify(myArr.Data));
				thisIntfID = configData[nodeCfg].InterfaceTypeList[configData[nodeCfg].InterfaceIndex].IntfId;
				thisHatID = configData[nodeCfg].ServerIndex;
				thisALMList = configData[nodeCfg].ALMIndex;
				showMenueTabs(configData[nodeCfg]);
				loadNodeDataFields(configData[nodeCfg]);
			}
			if (myArr.Type == scriptList.Pages[currentPage].ID)
			{
				if (myArr.ResetData) //new data to be loaded
				{
					wsInitData = false;
					configData[loadCfg] = JSON.parse(JSON.stringify(myArr.Data));
					configData[workCfg] = JSON.parse(JSON.stringify(myArr.Data));
				}
				else //data is additional file to current block
				{
					if (typeof addFileSeq == 'function')
						addFileSeq(myArr.Data, configData);
				}
				loadDataFields(configData[workCfg]);
			}
			else //additional config files for e.g. GreenHat
				if (myArr.Type != "pgNodeCfg") //pgNodeCfg already loaded above
				{
//					console.log(myArr.Type, scriptList.Pages[currentPage].ID);
					if (typeof addDataFile == 'function')
						addDataFile(myArr);
				}
		}
	}
};

function isInViewport(el) {
    const rect = el.getBoundingClientRect();
    return (
        rect.top >= 0 &&
        rect.left >= 0 &&
        rect.bottom <= (window.innerHeight || document.documentElement.clientHeight) &&
        rect.right <= (window.innerWidth || document.documentElement.clientWidth)

    );
}
