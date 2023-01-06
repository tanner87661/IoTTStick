var mainScrollBox;
var pingDelay;
var wiConfigOptions;
var listActiveClients;
var clientDisplay;

function upgradeJSONVersion(jsonData)
{
	return upgradeJSONVersionLBServer(jsonData);
}

function saveConfigFileSettings()
{
	//step 1: save greenhat.cfg
	saveJSONConfig(scriptList.Pages[currentPage].ID, scriptList.Pages[currentPage].FileName, configData[workCfg], null);
}

function downloadSettings(sender)
{
	if (currentPage == 5)
		downloadConfig(0x0400); //send just this for lbServer
	if (currentPage == 6)
		downloadConfig(0x4000); //send just this for WiThrottle
}

function getClientList()
{
	if (currentPage == 5)
		ws.send("{\"Cmd\":\"GetClients\", \"SubCmd\":\"LN\"}");
	if (currentPage == 6)
		ws.send("{\"Cmd\":\"GetClients\", \"SubCmd\":\"WI\"}");
}

function setLbServer(sender)
{
//	console.log(sender);
	if (sender.id == "serverip")
		configData[workCfg].ServerIP = sender.value; 
	if (sender.id == "serverport")
		if (pageParam == 's')
			configData[workCfg].ServerPortNr = sender.value; 
		else
			configData[workCfg].PortNr = sender.value; 
}

function setTurnoutList(sender)
{
	var inpData = sender.value.split(',');
	var validCount = 0;
	configData[workCfg].Turnouts = [];
	for (var i = 0; i < inpData.length; i++)
		if (!isNaN(inpData[i]))
		{
			configData[workCfg].Turnouts[validCount] = inpData[i];
			validCount++;
		}
//	console.log(configData[workCfg].Turnouts);
	writeInputField("turnoutlist", configData[2].Turnouts.toString());
}

//function setFC(sender)
//{
//	configData[workCfg].UpdateFC = sender.checked;
//}

function setPowerMode(sender, id)
{
//	console.log(id);
	if (sender.id == "selectpowermode_0")
		configData[2].PowerMode = 0;
	if (sender.id == "selectpowermode_1")
		configData[2].PowerMode = 1;
	if (sender.id == "selectpowermode_2")
		configData[2].PowerMode = 2;
}

function constructPageContent(contentTab)
{
	var tempObj;
	mainScrollBox = createEmptyDiv(contentTab, "div", "pagetopicboxscroll-y", "nodeconfigdiv");
		tempObj = createPageTitle(mainScrollBox, "div", "tile-1", "BasicCfg_Title", "h1", "LocoNet over TCP Server Configuration");
		tempObj.setAttribute("id", "ServerTitle");
		setVisibility(false, tempObj);
		tempObj = createPageTitle(mainScrollBox, "div", "tile-1", "BasicCfg_Title", "h1", "WiThrottle Server Configuration");
		tempObj.setAttribute("id", "WiServerTitle");
		setVisibility(false, tempObj);
		tempObj = createPageTitle(mainScrollBox, "div", "tile-1", "BasicCfg_Title", "h1", "LocoNet over TCP Client Configuration");
		tempObj.setAttribute("id", "ClientTitle");
		setVisibility(false, tempObj);
		tempObj = createPageTitle(mainScrollBox, "div", "tile-1", "BasicCfg_Title", "h1", "WiThrottle Client Configuration");
		tempObj.setAttribute("id", "WiClientTitle");
		setVisibility(false, tempObj);

//		createPageTitle(mainScrollBox, "div", "tile-1", "", "h2", "Basic Settings");

		tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
		tempObj.setAttribute("id", "ServerIPDiv");
		setVisibility(false, tempObj);
			createTextInput(tempObj, "tile-1_4", "Server IP/Name:", "n/a", "serverip", "setLbServer(this)");
		tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
		tempObj.setAttribute("id", "ServerPortDiv");
			createTextInput(tempObj, "tile-1_4", "Port #:", "n/a", "serverport", "setLbServer(this)");

		wiConfigOptions = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
			setVisibility(false, wiConfigOptions);
			tempObj = createPageTitle(wiConfigOptions, "div", "tile-1", "BasicCfg_Title", "h2", "WiThrottle Options");
			tempObj = createEmptyDiv(wiConfigOptions, "div", "tile-1", "");
			createRadiobox(tempObj, "tile-1_2", "Allow Power Commands:", ["Display only", "Toggle ON - Idle", "Toggle ON - OFF"], "selectpowermode", "setPowerMode(this, id)");
			tempObj = createEmptyDiv(wiConfigOptions, "div", "tile-1", "");
			createTextInput(tempObj, "tile-1_2", "Include Turnouts:", "n/a", "turnoutlist", "setTurnoutList(this)");
//			tempObj = createEmptyDiv(wiConfigOptions, "div", "tile-1", "");
//			createCheckbox(tempObj, "tile-1_4", "Update FastClock", "cbUpdateFC", "setFC(this)");

		clientDisplay = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
			var leftDiv = createEmptyDiv(clientDisplay, "div", "tile-1_2", "leftside");
				createPageTitle(leftDiv, "div", "tile-1", "", "h2", "Connected Clients");
				listActiveClients = createListViewer(leftDiv, "clientbox", "ClientList");

		tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
			createButton(tempObj, "", "Save & Restart", "btnSave", "saveSettings(this)");
			createButton(tempObj, "", "Cancel", "btnCancel", "cancelSettings(this)");
		tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
			createButton(tempObj, "", "Save to File", "btnDownload", "downloadSettings(this)");
		setVisibility(false, clientDisplay);
}

function processClientList(jsonData)
{
//	console.log(jsonData);
	setVisibility(true, clientDisplay);
	if (((currentPage == 5) && !jsonData.WIType) || ((currentPage == 6) && jsonData.WIType))
	{
		while (listActiveClients.childNodes.length > 0)
			listActiveClients.removeChild(listActiveClients.childNodes[0]);
		for (var i=0; i < jsonData.cl.length; i++)
		{
			newLine = document.createElement("li");
			newLine.innerHTML = jsonData.cl[i].ip;
			if (jsonData.WIType)
			{
				newLine.innerHTML += " - " + jsonData.cl[i].name;
				if (jsonData.cl[i].t0.length > 0)
				{
					newLine.innerHTML += " Th 1: ";
					for (var j = 0; j < jsonData.cl[i].t0.length; j++)
					{
						if (j > 0)
							newLine.innerHTML += ", ";
						newLine.innerHTML += jsonData.cl[i].t0[j].toString();
					}
				}
				if (jsonData.cl[i].t1.length > 0)
				{
					newLine.innerHTML += " Th 2: ";
					for (var j = 0; j < jsonData.cl[i].t1.length; j++)
					{
						if (j > 0)
							newLine.innerHTML += ", ";
						newLine.innerHTML += jsonData.cl[i].t1[j].toString();
					}
				}
				
			}
			listActiveClients.append(newLine);
		}
	}
}

function loadNodeDataFields(jsonData)
{
//	console.log(jsonData);
}

function loadDataFields(jsonData)
{
	var error;
	configData[workCfg] = upgradeJSONVersion(jsonData);
//	console.log(currentPage);
//	console.log(configData[nodeCfg]);
	if (pageParam == 's')
		writeInputField("serverport", jsonData.ServerPortNr);
	else
		writeInputField("serverport", jsonData.PortNr);
	writeInputField("serverip", jsonData.ServerIP);
//	try
//	{
//		document.getElementById("cbUpdateFC").checked = configData[workCfg].UpdateFC;
//	}
//	catch (error) {};
	try
	{
		writeRBInputField("selectpowermode", configData[2].PowerMode);
	}
	catch (error) {};
	try
	{
		if (Array.isArray(configData[2].Turnouts))
			writeInputField("turnoutlist", configData[2].Turnouts.toString());
	}
	catch (error) {};

	setVisibility((configData[nodeCfg].ServerIndex.indexOf(1) >= 0) && (pageParam == 's') && (currentPage == 5), document.getElementById("ServerTitle"));
	setVisibility((configData[nodeCfg].ServerIndex.indexOf(2) >= 0) && (pageParam == 's') && (currentPage == 6), document.getElementById("WiServerTitle"));
	setVisibility((pageParam == 'c'), document.getElementById("ServerIPDiv"));
//	setVisibility(([12].indexOf(thisIntfID) && (pageParam == 'c'), document.getElementById("ServerIPDiv"));
	setVisibility(([12].indexOf(thisIntfID) >= 0) && (pageParam == 'c'), document.getElementById("ClientTitle"));
	setVisibility(([17].indexOf(thisIntfID) >= 0) && (pageParam == 'c'), document.getElementById("WiClientTitle"));
	setVisibility(((configData[nodeCfg].ServerIndex.indexOf(2) >= 0) && (pageParam == 's') && (currentPage == 6)), wiConfigOptions);

	getClientList();
}
