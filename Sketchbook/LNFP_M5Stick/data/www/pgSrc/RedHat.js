var mainScrollBox;
var pingDelay;
var pwrSettings;
var modeSettings;
var hwSettings;
var BoosterPanel;
var CmdStationPanel;
var DCCPPPanel;
var LocoNetPanel;
var cbLNMaster;

var hwOptionsList = ["Arduino Board"];
//var hwOptionsList = ["IoTT Board", "Arduino Board"];
var devOptionsList = ["Command Station", "DCC Booster"];
var pwrOptionsList = ["2.5 Amp.", "4 Amp.", "8 Amp.", "10 Amp."];

function saveConfigFileSettings()
{
	//step 1: save rhcfg.cfg
	saveJSONConfig(scriptList.Pages[currentPage].ID, scriptList.Pages[currentPage].FileName, configData[workCfg], null);
}

function downloadSettings(sender)
{
	downloadConfig(0x1000); //send just this
}

function setPanelVisibility()
{
	var currMode = (configData[workCfg].DevSettings.HWMode<<1) + configData[workCfg].DevSettings.WorkMode;
	console.log(currMode);
	setVisibility(currMode == 0, CmdStationPanel, false);
	setVisibility(currMode == 1, BoosterPanel, false);
	setVisibility(currMode == 2, DCCPPPanel, false);
	var IoTTMode = currMode < 2;
	var lnPanelVisible = (currMode == 0) || (configData[workCfg].DevSettings.UseLocoNet == 1);
	setVisibility(IoTTMode, DeviceModePanel, false);
	setVisibility(lnPanelVisible, LocoNetPanel, false);
	setVisibility((configData[workCfg].DevSettings.WorkMode == 1) && configData[workCfg].DevSettings.UseLocoNet && IoTTMode, cbLNMaster, false);
	setVisibility(lnPanelVisible && IoTTMode, document.getElementById("pgHWBtnCfg"), false);
	setVisibility(lnPanelVisible && IoTTMode, document.getElementById("pgLEDCfg"), false);
}

function constructPageContent(contentTab)
{
	var tempObj;
	mainScrollBox = createEmptyDiv(contentTab, "div", "pagetopicboxscroll-y", "nodeconfigdiv");
	tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
		createPageTitle(tempObj, "div", "tile-1", "BasicCfg_Title", "h2", "Hardware Selection");
	tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
		hwSettings = createDropdownselector(tempObj, "tile-1_4", "DCC++ hardware:", hwOptionsList, "hwmode_id", "setHWMode(this)");
	DeviceModePanel = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
		createPageTitle(DeviceModePanel, "div", "tile-1", "BasicCfg_Title", "h2", "Device Workmode");
		tempObj = createEmptyDiv(DeviceModePanel, "div", "tile-1", "");
			modeSettings = createDropdownselector(tempObj, "tile-1_4", "Output Type:", devOptionsList, "devmode_id", "setDeviceMode(this)");
			pwrSettings = createDropdownselector(tempObj, "tile-1_4", "Track Power:", pwrOptionsList, "pwrmode_id", "setDeviceMode(this)");
	tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
	CmdStationPanel = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
		createPageTitle(CmdStationPanel, "div", "tile-1", "BasicCfg_Title", "h2", "Command Station Configuration");
	tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
	DCCPPPanel = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
		createPageTitle(DCCPPPanel, "div", "tile-1", "DCCExCfg_Title", "h2", "DCC++Ex Arduino Configuration");
	BoosterPanel = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
		createPageTitle(BoosterPanel, "div", "tile-1", "BoosterCfg_Title", "h2", "Booster Configuration");
		createCheckbox(BoosterPanel, "tile-1_4", "Use LocoNet", "incllnreport", "setDeviceMode(this)");
		cbLNMaster = createCheckbox(BoosterPanel, "tile-1_4", "as Limited Master", "islnmaster", "setDeviceMode(this)");
	LocoNetPanel = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
		createPageTitle(LocoNetPanel, "div", "tile-1", "BasicCfg_Title", "h2", "LocoNet Device Configuration");



	tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
		createButton(tempObj, "", "Save & Restart", "btnSave", "saveSettings(this)");
		createButton(tempObj, "", "Cancel", "btnCancel", "cancelSettings(this)");
	tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
			createButton(tempObj, "", "Save to File", "btnDownload", "downloadSettings(this)");

}

function setHWMode(sender)
{
	if (sender.id == "hwmode_id")
	{
		configData[workCfg].DevSettings.HWMode = sender.selectedIndex; //IoTT Board = 0, Arduino = 1
		if (sender.selectedIndex == 1)
		{
			configData[workCfg].DevSettings.WorkMode = 0; //command station only
			setDropdownValue("devmode_id", configData[workCfg].DevSettings.WorkMode);
			configData[workCfg].DevSettings.UseLocoNet = 0;
			writeCBInputField("incllnreport", configData[workCfg].DevSettings.UseLocoNet);
		}
	}
	setPanelVisibility();
}

function setDeviceMode(sender)
{
	if (sender.id == "devmode_id")
		configData[workCfg].DevSettings.WorkMode = sender.selectedIndex; //Command Station = 0, Booster = 1
	if (sender.id == "pwrmode_id")
		configData[workCfg].DevSettings.PowerMode = sender.selectedIndex; //Amperage
	if (sender.id == "incllnreport")
		configData[workCfg].DevSettings.UseLocoNet = sender.checked;
	if (sender.id == "islnmaster")
		configData[workCfg].DevSettings.LNMaster = sender.checked;
	setPanelVisibility();
}

function loadNodeDataFields(jsonData)
{
	console.log(jsonData);
}

function loadDataFields(jsonData)
{
	console.log(jsonData);
	configData[workCfg] = upgradeJSONVersionRH(jsonData);
	setDropdownValue("hwmode_id", configData[workCfg].DevSettings.HWMode);
	setDropdownValue("devmode_id", configData[workCfg].DevSettings.WorkMode);
	setDropdownValue("pwrmode_id", configData[workCfg].DevSettings.PowerMode);
	writeCBInputField("incllnreport", configData[workCfg].DevSettings.UseLocoNet);
	writeCBInputField("islnmaster", configData[workCfg].DevSettings.LNMaster);
	
	setPanelVisibility();
}
