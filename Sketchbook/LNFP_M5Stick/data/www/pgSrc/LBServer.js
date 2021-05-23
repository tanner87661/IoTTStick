var mainScrollBox;
var pingDelay;

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
	downloadConfig(0x0400); //send just this
}

function setLbServer(sender)
{
//	console.log(sender);
	if (sender.id == "serverip")
		configData[workCfg].ServerIP = sender.value; 
	if (sender.id == "serverport")
		configData[workCfg].PortNr = sender.value; 
}

function constructPageContent(contentTab)
{
	var tempObj;
	mainScrollBox = createEmptyDiv(contentTab, "div", "pagetopicboxscroll-y", "nodeconfigdiv");
		tempObj = createPageTitle(mainScrollBox, "div", "tile-1", "BasicCfg_Title", "h1", "LocoNet over TCP Server Configuration");
		tempObj.setAttribute("id", "ServerTitle");
		setVisibility(false, tempObj);
		tempObj = createPageTitle(mainScrollBox, "div", "tile-1", "BasicCfg_Title", "h1", "LocoNet over TCP Client Configuration");
		tempObj.setAttribute("id", "ClientTitle");
		setVisibility(false, tempObj);

//		createPageTitle(mainScrollBox, "div", "tile-1", "", "h2", "Basic Settings");

		tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
		tempObj.setAttribute("id", "ServerIPDiv");
		setVisibility(false, tempObj);
			createTextInput(tempObj, "tile-1_4", "Server IP/Name:", "n/a", "serverip", "setLbServer(this)");
		tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
		tempObj.setAttribute("id", "ServerPortDiv");
			createTextInput(tempObj, "tile-1_4", "Port #:", "n/a", "serverport", "setLbServer(this)");


		tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
			createButton(tempObj, "", "Save & Restart", "btnSave", "saveSettings(this)");
			createButton(tempObj, "", "Cancel", "btnCancel", "cancelSettings(this)");
		tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
			createButton(tempObj, "", "Save to File", "btnDownload", "downloadSettings(this)");

}

function loadNodeDataFields(jsonData)
{
}

function loadDataFields(jsonData)
{
	configData[workCfg] = upgradeJSONVersion(jsonData);
//	console.log(jsonData);
//	console.log(configData[nodeCfg]);
	writeInputField("serverport", jsonData.PortNr);
	writeInputField("serverip", jsonData.ServerIP);
	setVisibility([8,9,10,11].indexOf(configData[nodeCfg].InterfaceIndex) >= 0, document.getElementById("ServerTitle"));
	setVisibility(configData[nodeCfg].InterfaceIndex == 12, document.getElementById("ServerIPDiv"));
	setVisibility(configData[nodeCfg].InterfaceIndex == 12, document.getElementById("ClientTitle"));
}
