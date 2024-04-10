var mainScrollBox;

var tabConfig;
var tabInputs;
var tabButtons;
var tabGauges;
var tabTracks;
var tabAutomation;
var tabRoster;

var updateCurrent = false;
var currTrack = 0;

var trackGauges = [];
var trackGaugeDefs = [];

var ConfigOptionPanel;
var CurrentOptionPanel;
var InputPanel;
var ButtonPanel;
var GaugePanel;
var TrackManPanel;
var AutomationPanel;
var RosterPanel;

var sensorTable;
var turnoutTable;
var automationTable;
var rosterTable;
var trackTable;

var sensorTableDiv;
var turnoutTableDiv;
var automationTableDiv;
var rosterTableDiv;
var trackTableDiv;
 
var newSensorTemplate = {"Id": 0, "PinNr": 1,"PNType": 0, "LNType": 0, "Logic": 1, "Par1": 0, "Par2": 0, "LNAddr": 1, "Descr":""};
var newTurnoutTemplate = {"Id": 0,"PinNr": 1,"TOType": 3,"LNType": 0,"Logic": 1,"Startup": 0, "Par1": 0,"Par2": 0, "Prof": 2, "LNAddr": 1, "Descr":"IoTT"};
var newRouteTemplate = {"ID": 0, "Type": "","Descr": ""};
var newLocoTemplate = {"DCCAddr": 0, "Descr": "","FNMap": ""};

const newTrackTemplate = {"Status":0, "Addr" : 0};
const trackModeList = ["NONE", "MAIN", "PROG", "DC", "DCX"];

var swVersion;
var hwVersion;
var swDispStr;
var pageTitleStr = "Command Station Settings ";

function upgradeJSONVersion(jsonData)
{
	return upgradeJSONVersionRedHat(jsonData);
}

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
//	setVisibility(lnPanelVisible && IoTTMode, document.getElementById("pgHWBtnCfg"), false);
}

function constructPageContent(contentTab)
{
	var menueStr = ["Options", "Inputs", "Turnouts", "Routes", "Locomotives", "Tracks", "Monitor"];
	var tempObj;
	mainScrollBox = createEmptyDiv(contentTab, "div", "pagetopicboxscroll-y", "btnconfigdiv");

		var menueTab = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
		for (var i = 0; i < menueStr.length; i++)
			createMenueTabElement(menueTab, "button", "tablink", "RedHatSub", "cbsetup_" + i.toString(), menueStr[i], true, "setPageMode(this)");
		updateMenueTabs("RedHatSub", "cbsetup_0", "grey");
		tabConfig = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
			ConfigOptionPanel = createEmptyDiv(tabConfig, "div", "tile-1", "");
				tempObj= createEmptyDiv(ConfigOptionPanel, "div", "tile-1", "");
					createPageTitle(tempObj, "div", "tile-1", "BasicCfg_TitleMain", "h2", pageTitleStr);
					createCheckbox(tempObj, "tile-1_4", "Enable Purging", "OpSw_13", "setOpSwCB(this)");
				createDropdownselector(tempObj, "tile-1_4", "Purge Delay:", ["300 Sec.","600 Sec."], "OpSw_12", "setOpSwDD(this)");
					createDropdownselector(tempObj, "tile-1_4", "Purge Speed:", ["No speed change","Force loco to stop"], "OpSw_14", "setOpSwDD(this)");
					createDropdownselector(tempObj, "tile-1_4", "Track Power Up Status:", ["Set to ON","Set to Idle", "Set to OFF","Restore Previous State"], "OpSw_32", "setOpSwDD(this)");
				tempObj= createEmptyDiv(ConfigOptionPanel, "div", "tile-1", "");
					createCheckbox(tempObj, "tile-1_4", "Main Signal on Prog Track", "OpSw_46", "setOpSwCB(this)");
					createCheckbox(tempObj, "tile-1_4", "Allow No Slot Refresh", "OpSw_48", "setOpSwCB(this)"); //IoTT opSw
					createCheckbox(tempObj, "tile-1_4", "Disable Standard Switch Command", "OpSw_26", "setOpSwCB(this)");
					createCheckbox(tempObj, "tile-1_4", "Disable Power-ON Interrogation", "OpSw_27", "setOpSwCB(this)");
				tempObj = createEmptyDiv(ConfigOptionPanel, "div", "tile-1", "");
					createDispText(tempObj, "tile-1_4", "Fast Clock Settings", "Pick Time:", "fctext");
					var timePicker = document.createElement("input");
					timePicker.setAttribute("class", "pickerinput");
					timePicker.setAttribute("type", "time");
					timePicker.setAttribute("id","timepick");
					timePicker.setAttribute("name","timepick");
					timePicker.setAttribute("min","00:00");
					timePicker.setAttribute("max","23:59");
					timePicker.setAttribute("onChange","setPicker(this)");
					tempObj.append(timePicker);

					createDropdownselector(tempObj, "tile-1_4", "Clock Rate:", ["Off","Normal","2","3","4","5","6","7","8","9","10"], "fcrate", "setFCRate(this)");
//					createTextInput(tempObj, "tile-1_4", "Set Clock Rate:", "n/a", "fcrate", "setFCRate(this)");
//					createButton(tempObj, "", "Update Fast Clock", "fcUpdate", "setFCTime(this)");


				tempObj = createEmptyDiv(ConfigOptionPanel, "div", "tile-1", "");
					createButton(tempObj, "", "Reset Loco Slots", "btnSlotReset", "resetSlots(this)");

				 tempObj= createEmptyDiv(ConfigOptionPanel, "div", "tile-1", "");
					createPageTitle(tempObj, "div", "tile-1", "BasicCfg_TitleMainLN", "h2", "LocoNet Options");
					createCheckbox(tempObj, "tile-1_4", "Switch Echo", "OpSw_56", "setOpSwCB(this)"); //IoTT opSw
					createCheckbox(tempObj, "tile-1_4", "Input Echo", "OpSw_57", "setOpSwCB(this)"); //IoTT opSw
					createCheckbox(tempObj, "tile-1_4", "Loco Echo", "OpSw_58", "setOpSwCB(this)"); //IoTT opSw
					createCheckbox(tempObj, "tile-1_4", "Power Status Echo", "OpSw_42", "setOpSwCB(this)");
				 tempObj= createEmptyDiv(ConfigOptionPanel, "div", "tile-1", "");
					createCheckbox(tempObj, "tile-1_4", "Disable Switch State Report", "OpSw_44", "setOpSwCB(this)");
//					createCheckbox(tempObj, "tile-1_4", "Disable Standard Switch Command", "OpSw_25", "setOpSwCB(this)");

//				 tempObj= createEmptyDiv(ConfigOptionPanel, "div", "tile-1", "");
//					createPageTitle(tempObj, "div", "tile-1", "BasicCfg_TitleMainComm", "h2", "Communication Options");
//				 tempObj= createEmptyDiv(ConfigOptionPanel, "div", "tile-1", "");
//					createCheckbox(tempObj, "tile-1_4", "Provide Loconet TCP Server", "lbserver", "setServerOptions(this)");
//					createTextInput(tempObj, "tile-1_4", "on port:", "n/a", "lbserverport", "setServerOptions(this)");
//				 tempObj= createEmptyDiv(ConfigOptionPanel, "div", "tile-1", "");
//					createCheckbox(tempObj, "tile-1_4", "Provide WiThrottle Server", "withrottle", "setServerOptions(this)");
//					createTextInput(tempObj, "tile-1_4", "on port:", "n/a", "withrottleport", "setServerOptions(this)");

				 tempObj= createEmptyDiv(ConfigOptionPanel, "div", "tile-1", "");
					createPageTitle(tempObj, "div", "tile-1", "BasicCfg_TitleMainDCC", "h2", "DCC Generator Options");
					createDropdownselector(tempObj, "tile-1_4", "DCC Speed Steps:", ["128 Steps","28 Steps"], "OpSw_20", "setOpSwDD(this)");
					createDispText(tempObj, "tile-1_4", "DCC++ Refresh Slots:", "n/a", "refreshslots");
				 tempObj= createEmptyDiv(ConfigOptionPanel, "div", "tile-1", "");
					createPageTitle(tempObj, "div", "tile-1", "BasicCfg_TitleMainMeas", "h2", "Current Measurement Settings");
					createDropdownselector(tempObj, "tile-1_4", "Select Gauge:", ["Main","Prog"], "currgaugeselector", "setCurrentOptions(this)"); //add buttons later
				 tempObj= createEmptyDiv(ConfigOptionPanel, "div", "tile-1", "");
					createTextInput(tempObj, "tile-1_4", "Gauge Name:", "n/a", "currgaugename", "setCurrentOptions(this)");
					createCheckbox(tempObj, "tile-1_4", "Display Gauge", "showgauge", "setCurrentOptions(this)");
					createTextInput(tempObj, "tile-1_4", "Analog Pin:", "n/a", "currinppin", "setCurrentOptions(this)");
				 tempObj= createEmptyDiv(ConfigOptionPanel, "div", "tile-1", "");
					createTextInput(tempObj, "tile-1_4", "Buffer Size:", "n/a", "currbuffsize", "setCurrentOptions(this)");
					createTextInput(tempObj, "tile-1_4", "Multiplier:", "n/a", "currmultiplier", "setCurrentOptions(this)");
					createTextInput(tempObj, "tile-1_4", "Offset:", "n/a", "curroffset", "setCurrentOptions(this)");
				 tempObj= createEmptyDiv(ConfigOptionPanel, "div", "tile-1", "");
					createTextInput(tempObj, "tile-1_4", "Max. Value:", "n/a", "currmaxval", "setCurrentOptions(this)");
					createTextInput(tempObj, "tile-1_4", "Major Ticks:", "n/a", "majorticks", "setCurrentOptions(this)");
				 tempObj= createEmptyDiv(ConfigOptionPanel, "div", "tile-1", "");
					createPageTitle(tempObj, "div", "tile-1", "BasicCfg_TitleMainProg", "h2", "Programming Track Options");
					createTextInput(tempObj, "tile-1_4", "Min. ACK Pulse [mA]:", "n/a", "proglimit", "setProgOptions(this)");
					createTextInput(tempObj, "tile-1_4", "Min. ACK Time [\xB5s]:", "n/a", "progpulsemin", "setProgOptions(this)");
					createTextInput(tempObj, "tile-1_4", "Max. ACK Time [\xB5s]:", "n/a", "progpulsemax", "setProgOptions(this)");
				 tempObj= createEmptyDiv(ConfigOptionPanel, "div", "tile-1", "");
					createTextInput(tempObj, "tile-1_4", "# of Attempts:", "n/a", "progattempt", "setProgOptions(this)");
					createCheckbox(tempObj, "tile-1_4", "Boost Prog Current:", "progboost", "setProgOptions(this)");
				 tempObj= createEmptyDiv(ConfigOptionPanel, "div", "tile-1", "");
					createButton(tempObj, "", "Set Default Values", "btnSetDefault", "progDefault(this)");

			tempObj = createEmptyDiv(ConfigOptionPanel, "div", "tile-1", "");

		tabInputs = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
			InputPanel = createEmptyDiv(tabInputs, "div", "tile-1", "");
				sensorTableDiv = createEmptyDiv(InputPanel, "div", "tile-1", "");
					createPageTitle(sensorTableDiv, "div", "tile-1", "BasicCfg_TitleInput", "h2", "Arduino Input Pins");
					sensorTable = createDataTable(sensorTableDiv, "tile-1_2", ["Pos","Add/Delete/Move", "ID", "DCC EX", "Pin Nr", "Logic", "Message Type", "LocoNet Addr"], "sensorconfig", "");
				tempObj = createEmptyDiv(InputPanel, "div", "tile-1", "");
					createCheckbox(tempObj, "tile-1_4", "Store Input Settings to EEPROM", "sensoreeprom", "setProgOptions(this)");
		tabButtons = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
			ButtonPanel = createEmptyDiv(tabButtons, "div", "tile-1", "");
				turnoutTableDiv = createEmptyDiv(ButtonPanel, "div", "tile-1", "");
					createPageTitle(turnoutTableDiv, "div", "tile-1", "BasicCfg_TitleButton", "h2", "Defined Turnouts and Pins");
					turnoutTable = createDataTable(turnoutTableDiv, "tile-1_2", ["Pos","Add/Delete/Move", "ID", "DCC EX", "Turnout Address", "Turnout Type", "Logic", "Start-up", "Pin Nr", "Min. Position", "Max. Position", "Profile", "Description"], "turnoutconfig", "");
				tempObj = createEmptyDiv(ButtonPanel, "div", "tile-1", "");
					createSimpleText(tempObj,"tile-1", "(!) Not recommended. Use VPIN instead", "zpinwarning");
					createCheckbox(tempObj, "tile-1_4", "Store Turnout/Pin Settings to EEPROM", "turnouteeprom", "setProgOptions(this)");
		tabAutomation = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
			AutomationPanel = createEmptyDiv(tabAutomation, "div", "tile-1", "");
				automationTableDiv = createEmptyDiv(AutomationPanel, "div", "tile-1", "");
					createPageTitle(automationTableDiv, "div", "tile-1", "BasicCfg_TitleAuto", "h2", "Routes");
					automationTable = createDataTable(automationTableDiv, "tile-1_2", ["Pos", "Delete", "DCC EX", "ID", "Type", "Description"], "automationconfig", "");

		tabRoster = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
			RosterPanel = createEmptyDiv(tabRoster, "div", "tile-1", "");
				rosterTableDiv = createEmptyDiv(RosterPanel, "div", "tile-1", "");
					createPageTitle(rosterTableDiv, "div", "tile-1", "BasicCfg_TitleRoster", "h2", "Locomotives");
//					rosterTable = createDataTable(rosterTableDiv, "tile-1_2", ["Pos","Add/Delete/Move", "DCC Addr", "Description", "Function Map"], "rosterconfig", "");
					rosterTable = createDataTable(rosterTableDiv, "tile-1_2", ["Pos", "Delete", "DCC EX", "DCC Addr", "Description", "Function Map"], "rosterconfig", "");

		tabTracks = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
			TrackManPanel = createEmptyDiv(tabTracks, "div", "tile-1", "");
				trackTableDiv = createEmptyDiv(TrackManPanel, "div", "tile-1", "");
					createPageTitle(trackTableDiv, "div", "tile-1", "BasicCfg_TitleTrack", "h2", "Track Manager Settings");
					trackTable = createDataTable(trackTableDiv, "tile-1_2", ["Track", "DCC EX", "Mode", "DCC Addr"], "trackconfig", "");

		tabGauges = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
			GaugePanel = createEmptyDiv(tabGauges, "div", "tile-1", "");

	tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
		createButton(tempObj, "", "Save & Restart", "btnSave", "saveEEPROMSettings(this)");
		createButton(tempObj, "", "Cancel", "btnCancel", "cancelSettings(this)");
//		createButton(tempObj, "", "Request", "btnReq", "requestCurrent(this)");
	tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
			createButton(tempObj, "", "Save to File", "btnDownload", "downloadSettings(this)");
	setVisibility(false, tabInputs);
	setVisibility(false, tabButtons);
	setVisibility(false, tabGauges);
	setVisibility(false, tabAutomation);
	setVisibility(false, tabRoster);
	setVisibility(false, tabTracks);
	setVisibility(false, document.getElementById("cbsetup_5")); //only show menu item if needed
	setVisibility(false, document.getElementById("cbsetup_6")); //only show menu item if needed
//	setVisibility(false, document.getElementById("OpSw_46").parentElement); //only show menu item if needed
	

}

function saveEEPROMSettings(sender)
{
	ws.send("{\"Cmd\":\"SetDCCPP\", \"SubCmd\":\"SendCmd\",\"OpCode\": \"e\"}");
	if (readCBInputField("sensoreeprom") || readCBInputField("turnouteeprom"))
	if (configData[workCfg].DevSettings.ConfigToEEPROM)
	{
		for (var i = 0; i < configData[2].InputSettings.InpPins.length; i++)
		{
			writeSensorToDCC(i);
		}
		for (var i = 0; i < configData[2].TurnoutSettings.OutPins.length; i++)
		{
			writeTurnoutToDCC(i);
		}
		ws.send("{\"Cmd\":\"SetDCCPP\", \"SubCmd\":\"SendCmd\",\"OpCode\": \"E\"}");
	}
	saveSettings(sender);
}

function requestCurrent(sender)
{
	try 
	{
		ws.send("{\"Cmd\":\"SetDCCPP\", \"SubCmd\":\"GetCurrent\",\"OpCode\": " + currTrack + "}");
	}
	catch  (error) 
	{
		console.error(error);
	}
	currTrack = Math.abs(currTrack - 1);
	if (updateCurrent)
		setTimeout(function(){requestCurrent(null)}, 500);
}

function progDefault(sender)
{
	configData[workCfg].DevSettings.ProgTrack.CurrLimit = 50;
	configData[workCfg].DevSettings.ProgTrack.MinAckP = 200;
	configData[workCfg].DevSettings.ProgTrack.MaxAckP = 2000;
	configData[workCfg].DevSettings.ProgTrack.NumTry = 3;
	configData[workCfg].DevSettings.ProgTrack.Boost= false ;
	loadProgOptions(configData[workCfg]);
}

function setPicker(sender)
{
//	console.log("setPicker");
	timeArray = document.getElementById("timepick").value.split(":");
	var timeVal = parseInt(timeArray[0]) * 3600 + parseInt(timeArray[1]) * 60; //time in seconds
	setFCTime(timeVal, configData[workCfg].DevSettings.FCRate);
}

function setFCRate(sender)
{
//	console.log("SetRate");
	configData[workCfg].DevSettings.FCRate = sender.selectedIndex;
	setPicker(sender);
}

function setFCTime(timeVal, clockRate)
{
//	console.log("setFCTime");
	cmdStr = "{\"Cmd\":\"SetFC\", \"FCTime\":\"" + timeVal + "\",\"FCRate\": \"" + clockRate + "\"}";
//	console.log(cmdStr);
	ws.send(cmdStr);
}

//function setServerOptions(sender)
//{
//	if (sender.id == "lbserver")
//		configData[workCfg].ServerSettings.LbServer.Active = sender.checked;
//	if (sender.id == "lbserverport")
//		configData[workCfg].ServerSettings.LbServer.Port = verifyNumber(sender.value, configData[workCfg].ServerSettings.LbServer.Port); 
//	if (sender.id == "withrottle")
//		configData[workCfg].ServerSettings.wiThrottle.Active = sender.checked;
//	if (sender.id == "withrottleport")
//		configData[workCfg].ServerSettings.wiThrottle.Port = verifyNumber(sender.value, configData[workCfg].ServerSettings.wiThrottle.Port); 
//}

function loadGaugeData(gaugeNr)
{
	if (gaugeNr < 0)
		gaugeNr = document.getElementById("currgaugeselector").selectedIndex;
	if (gaugeNr >= 0)
	{
		document.getElementById("showgauge").checked = configData[workCfg].CurrentTracker[gaugeNr].ShowGauge;
		document.getElementById("currgaugename").value = configData[workCfg].CurrentTracker[gaugeNr].GaugeName;
		document.getElementById("currinppin").value = configData[workCfg].CurrentTracker[gaugeNr].PinNr;
		document.getElementById("currbuffsize").value = configData[workCfg].CurrentTracker[gaugeNr].SampleSize;
		document.getElementById("currmultiplier").value = configData[workCfg].CurrentTracker[gaugeNr].Multiplier;
		document.getElementById("curroffset").value = configData[workCfg].CurrentTracker[gaugeNr].Offset;
		document.getElementById("currmaxval").value = configData[workCfg].CurrentTracker[gaugeNr].MaxVal;
		document.getElementById("majorticks").value = configData[workCfg].CurrentTracker[gaugeNr].MainTicks;
	}
}

function setCurrentOptions(sender)
{
	var activeTrack = document.getElementById("currgaugeselector").selectedIndex;
	if (activeTrack >= 0)
	{
		if (sender.id == "currgaugeselector")
			loadGaugeData(sender.selectedIndex);
		if (sender.id == "showgauge")
			configData[workCfg].CurrentTracker[activeTrack].ShowGauge = sender.checked;
		if (sender.id == "currgaugename")
			configData[workCfg].CurrentTracker[activeTrack].GaugeName = sender.value; 
		if (sender.id == "currinppin")
			configData[workCfg].CurrentTracker[activeTrack].PinNr = verifyNumber(sender.value, configData[workCfg].CurrentTracker[activeTrack].PinNr); 
		if (sender.id == "currbuffsize")
			configData[workCfg].CurrentTracker[activeTrack].SampleSize = verifyNumber(sender.value, configData[workCfg].CurrentTracker[activeTrack].SampleSize); 
		if (sender.id == "currmultiplier")
			configData[workCfg].CurrentTracker[activeTrack].Multiplier = verifyNumber(sender.value, configData[workCfg].CurrentTracker[activeTrack].Multiplier); 
		if (sender.id == "curroffset")
			configData[workCfg].CurrentTracker[activeTrack].Offset = verifyNumber(sender.value, configData[workCfg].CurrentTracker[activeTrack].Offset); 
		if (sender.id == "currmaxval")
			configData[workCfg].CurrentTracker[activeTrack].MaxVal = verifyNumber(sender.value, configData[workCfg].CurrentTracker[activeTrack].MaxVal); 
		if (sender.id == "majorticks")
		{
			var newArray = verifyNumArray(sender.value, ",");
			if (newArray.length > 0)
			{
				configData[workCfg].CurrentTracker[activeTrack].MainTicks = []; //make sure this is an array
				for (var i = 0; i < newArray.length; i++)
					configData[workCfg].CurrentTracker[activeTrack].MainTicks.push(newArray[i]);
			}
		}
	}
}

function setProgOptions(sender)
{
	if (sender.id == "proglimit")
		configData[workCfg].DevSettings.ProgTrack.CurrLimit = verifyNumber(sender.value, configData[workCfg].DevSettings.ProgTrack.CurrLimit); 
	if (sender.id == "progpulsemin")
		configData[workCfg].DevSettings.ProgTrack.MinAckP = verifyNumber(sender.value, configData[workCfg].DevSettings.ProgTrack.MinAckP); 
	if (sender.id == "progpulsemax")
		configData[workCfg].DevSettings.ProgTrack.MaxAckP = verifyNumber(sender.value, configData[workCfg].DevSettings.ProgTrack.MaxAckP); 
	if (sender.id == "progattempt")
		configData[workCfg].DevSettings.ProgTrack.NumTry = verifyNumber(sender.value, configData[workCfg].DevSettings.ProgTrack.NumTry); 
	if (sender.id == "progboost")
		configData[workCfg].DevSettings.ProgTrack.Boost = sender.checked;
	if (sender.id == "sensoreeprom")
	{
		configData[workCfg].DevSettings.ConfigToEEPROM = sender.checked;
		document.getElementById("turnouteeprom").checked = sender.checked;
	}
	if (sender.id == "turnouteeprom")
	{
		configData[workCfg].DevSettings.ConfigToEEPROM = sender.checked;
		document.getElementById("sensoreeprom").checked = sender.checked;
	}
}

function resetSlots(sender)
{
//	console.log("resetSlots");
	cmdStr = "{\"Cmd\":\"ResetSlots\"}";
//	console.log(cmdStr);
	ws.send(cmdStr);
}

function setOpSwCB(sender) //check box
{
	//calculate startbit
	var opsNr = parseInt(sender.id.substr("OpSw_".length, sender.id.length));
	var byteNr = opsNr >> 3;
	if (byteNr > 3)
		byteNr++; //byte for is track power
	var bitNr = opsNr & 0x07;
	var bitMask = 0x01 << bitNr;
	if (sender.checked)//set bit value
		configData[workCfg].DevSettings.CfgSlot[byteNr] = configData[workCfg].DevSettings.CfgSlot[byteNr] | bitMask;
	else	
		configData[workCfg].DevSettings.CfgSlot[byteNr] = configData[workCfg].DevSettings.CfgSlot[byteNr] & ~bitMask;
//	console.log(opsNr, byteNr, bitNr, bitMask, configData[workCfg].DevSettings.CfgSlot[byteNr]);
}

function setOpSwDD(sender) //drop down
{
	var opsNr = parseInt(sender.id.substr("OpSw_".length, sender.id.length));
	var byteNr = opsNr >> 3;
	if (byteNr > 3)
		byteNr++; //byte for is track power
	var bitNr = opsNr & 0x07;
	var numBits = parseInt((Math.log(sender.options.length) / Math.log(2)));
	var bitMask = (Math.pow(2, numBits) - 1) << bitNr;
	var bitVal = sender.selectedIndex << bitNr;
	configData[workCfg].DevSettings.CfgSlot[byteNr] = ((configData[workCfg].DevSettings.CfgSlot[byteNr] & ~bitMask) | bitVal);
//	console.log(opsNr, byteNr, bitNr, bitMask, bitVal, configData[workCfg].DevSettings.CfgSlot[byteNr]);
}

function setPageMode(sender)
{
	setVisibility(false, tabConfig);
	setVisibility(false, tabInputs);
	setVisibility(false, tabButtons);
	setVisibility(false, tabAutomation);
	setVisibility(false, tabRoster);
	setVisibility(false, tabTracks);
	setVisibility(false, tabGauges);
	for (var i = 0; i < trackGauges.length; i++)
		setVisibility(false, trackGauges[i]);
	updateCurrent = false;
	switch (sender.id)
	{
		case "cbsetup_0":
//			writeRBInputField("cbsetup", 0);
			setVisibility(true, tabConfig);
			break;
		case "cbsetup_1":
//			writeRBInputField("cbsetup", 1);
			setVisibility(true, tabInputs);
			break;
		case "cbsetup_2":
//			writeRBInputField("cbsetup", 2);
			setVisibility(true, tabButtons);
			break;
		case "cbsetup_3":
//			writeRBInputField("cbsetup", 2);
			setVisibility(true, tabAutomation);
			break;
		case "cbsetup_4":
//			writeRBInputField("cbsetup", 2);
			setVisibility(true, tabRoster);
			break;
		case "cbsetup_5":
//			writeRBInputField("cbsetup", 2);
			setVisibility(true, tabTracks);
			break;
		case "cbsetup_6":
//			console.log(configData[workCfg].CurrentTracker);
			if (trackGauges.length > 0)
			{
				updateCurrent = true;
				setVisibility(true, tabGauges);
//				console.log(trackGauges.length);
				for (var i = 0; i < trackGauges.length; i++)
				{
					setVisibility(configData[workCfg].CurrentTracker[i].ShowGauge, trackGauges[i]);
				}
				setTimeout(function(){requestCurrent(sender) }, 1000);
			}
			break;
	}
	updateMenueTabs("RedHatSub", sender.id, "grey");
}

function verifyUniqueID(thisID)
{
//	console.log("check ", thisID);
	for (var i = 0; i < configData[workCfg].InputSettings.InpPins.length; i++)
		if (thisID == configData[workCfg].InputSettings.InpPins[i].Id)
			return false;
	for (var i = 0; i < configData[workCfg].TurnoutSettings.OutPins.length; i++)
		if (thisID == configData[workCfg].TurnoutSettings.OutPins[i].Id)
			return false;
//	console.log(i, " is unique");
	return true;
}

function getUniqueID()
{
	var maxVal = configData[2].InputSettings.InpPins.length + configData[2].TurnoutSettings.OutPins.length + 10;
//	console.log("max ", maxVal);
	for (var i = 0; i < maxVal; i++)
		if (verifyUniqueID(i))
		{
//			console.log(i, " new id");
			return i;
		}
	return -1;
}

function setSensorConfig(sender)
{
	var thisRow = parseInt(sender.getAttribute("row"));
	var thisCol = parseInt(sender.getAttribute("col"));
	var thisIndex = parseInt(sender.getAttribute("index"));
//	console.log(thisRow, thisCol, thisIndex);
	switch (thisCol)
	{
		case -1: //empty table, create first entry
			configData[2].InputSettings.InpPins.push(JSON.parse(JSON.stringify(newSensorTemplate)));
			configData[2].InputSettings.InpPins[0].Id = getUniqueID();
			writeSensorToDCC(0);
			loadSensorTable(sensorTable, configData[2].InputSettings.InpPins);
			break;
		case 1:
			switch (thisIndex)
			{
				case 1:
					configData[2].InputSettings.InpPins.splice(thisRow+1, 0, JSON.parse(JSON.stringify(newSensorTemplate)));
					configData[2].InputSettings.InpPins[thisRow+1].Id = getUniqueID();
					writeSensorToDCC(thisRow+1);
					break;
				case 2:
					DeleteSensorFromDCC(thisRow);
					configData[2].InputSettings.InpPins.splice(thisRow, 1);
					break;
				case 3:
					if (thisRow > 0)
					{
						thisElement = configData[2].InputSettings.InpPins.splice(thisRow, 1);
						configData[2].InputSettings.InpPins.splice(thisRow-1,0, thisElement[0]);
					}
					break;
				case 4:
					if (thisRow < configData[2].InputSettings.InpPins.length)
					{
						thisElement = configData[2].InputSettings.InpPins.splice(thisRow, 1);
						configData[2].InputSettings.InpPins.splice(thisRow+1,0, thisElement[0]);
					}
					break;
			}
			loadSensorTable(sensorTable, configData[2].InputSettings.InpPins);
			break;
		case 2: //new ID
			var newID = verifyNumber(sender.value, configData[workCfg].InputSettings.InpPins[thisRow].Id);
			if (newID != configData[workCfg].InputSettings.InpPins[thisRow].Id)
				if (verifyUniqueID(newID))
				{
					DeleteSensorFromDCC(thisRow);
					configData[workCfg].InputSettings.InpPins[thisRow].Id = newID; 
					writeSensorToDCC(thisRow);
				}
				else
				{
					alert("ID already used");
					sender.value = configData[workCfg].InputSettings.InpPins[thisRow].Id;
				}
			break;
		case 4: configData[workCfg].InputSettings.InpPins[thisRow].PinNr = verifyNumber(sender.value, configData[workCfg].InputSettings.InpPins[thisRow].PinNr); 
				writeSensorToDCC(thisRow);
			break;
		case 5: configData[workCfg].InputSettings.InpPins[thisRow].Logic = sender.selectedIndex;
				writeSensorToDCC(thisRow);
			break;
		case 6: switch (sender.selectedIndex)
				{
					case 1: configData[workCfg].InputSettings.InpPins[thisRow].LNType = 9; break;
					case 2: configData[workCfg].InputSettings.InpPins[thisRow].LNType = 5; break;
					default: configData[workCfg].InputSettings.InpPins[thisRow].LNType = sender.selectedIndex; break;
				}
			break;
		case 7: configData[workCfg].InputSettings.InpPins[thisRow].LNAddr = verifyNumber(sender.value, configData[workCfg].InputSettings.InpPins[thisRow].LNAddr); 
			break;
	}
}

function setRosterConfig(sender)
{
	var thisRow = parseInt(sender.getAttribute("row"));
	var thisCol = parseInt(sender.getAttribute("col"));
	var thisIndex = parseInt(sender.getAttribute("index"));
	switch (thisCol)
	{
		case 1:
			switch (thisIndex)
			{
				case 2:
					configData[2].RosterSettings.Locos.splice(thisRow, 1);
					break;
				default:
					alert("Roster entries are read only and must be defined in EX RAIL"); 
			}
			loadRosterTable(rosterTable, configData[2].RosterSettings.Locos);
			break;
	}
}

function setAutomationConfig(sender)
{
	var thisRow = parseInt(sender.getAttribute("row"));
	var thisCol = parseInt(sender.getAttribute("col"));
	var thisIndex = parseInt(sender.getAttribute("index"));
//	console.log(thisRow, thisCol, thisIndex);
	switch (thisCol)
	{
		case 1:
			switch (thisIndex)
			{
				case 2:
					configData[2].AutomationSettings.Routes.splice(thisRow, 1);
					break;
				default:
					alert("Routes and Automations are read only and must be defined in EX RAIL"); 
			}
			loadAutomationTable(automationTable, configData[2].AutomationSettings.Routes);
			break;
	}
}

function setTrackConfig(sender)
{
	var thisRow = parseInt(sender.getAttribute("row"));
	var thisCol = parseInt(sender.getAttribute("col"));
	var thisIndex = parseInt(sender.getAttribute("index"));
	switch (thisCol)
	{
		case 2:
			configData[workCfg].TrackSettings.TrackModes[thisRow].Status = sender.selectedIndex;
			writeTrackToDCC(thisRow);
			break;
		case 3:
			configData[workCfg].TrackSettings.TrackModes[thisRow].Addr = verifyNumber(sender.value, configData[workCfg].TrackSettings.TrackModes[thisRow].Addr);
			writeTrackToDCC(thisRow);
			break;
	}
}

function setTurnoutConfig(sender)
{
	var thisRow = parseInt(sender.getAttribute("row"));
	var thisCol = parseInt(sender.getAttribute("col"));
	var thisIndex = parseInt(sender.getAttribute("index"));
//	console.log(thisRow, thisCol, thisIndex);
	switch (thisCol)
	{
		case -1: //empty table, create first entry
			configData[2].TurnoutSettings.OutPins.push(JSON.parse(JSON.stringify(newTurnoutTemplate)));
			configData[2].TurnoutSettings.OutPins[thisRow+1].Id = getUniqueID();
			writeTurnoutToDCC(0);
			loadTurnoutTable(turnoutTable, configData[2].TurnoutSettings.OutPins);
			break;
		case 1:
			switch (thisIndex)
			{
				case 1:
					configData[2].TurnoutSettings.OutPins.splice(thisRow+1, 0, JSON.parse(JSON.stringify(newTurnoutTemplate)));
					configData[2].TurnoutSettings.OutPins[thisRow+1].Id = getUniqueID();
					writeTurnoutToDCC(thisRow+1);
					break;
				case 2:
					DeleteTurnoutFromDCC(thisRow);
					configData[2].TurnoutSettings.OutPins.splice(thisRow, 1);
					break;
				case 3:
					if (thisRow > 0)
					{
						thisElement = configData[2].TurnoutSettings.OutPins.splice(thisRow, 1);
						configData[2].TurnoutSettings.OutPins.splice(thisRow-1,0, thisElement[0]);
					}
					break;
				case 4:
					if (thisRow < configData[2].TurnoutSettings.OutPins.length)
					{
						thisElement = configData[2].TurnoutSettings.OutPins.splice(thisRow, 1);
						configData[2].TurnoutSettings.OutPins.splice(thisRow+1,0, thisElement[0]);
					}
					break;
			}
			loadTurnoutTable(turnoutTable, configData[2].TurnoutSettings.OutPins);
			break;
		case 2: //configData[workCfg].TurnoutSettings.OutPins[thisRow].Id = verifyNumber(sender.value, configData[workCfg].TurnoutSettings.OutPins[thisRow].Id); 
			var newID = verifyNumber(sender.value, configData[workCfg].TurnoutSettings.OutPins[thisRow].Id);
			if (newID != configData[workCfg].TurnoutSettings.OutPins[thisRow].Id)
				if (verifyUniqueID(newID))
				{
					DeleteTurnoutFromDCC(thisRow);
					configData[workCfg].TurnoutSettings.OutPins[thisRow].Id = newID; 
					writeTurnoutToDCC(thisRow);
				}
				else
				{
					alert("ID already used");
					sender.value = configData[workCfg].TurnoutSettings.OutPins[thisRow].Id;
				}
			break;
		case 4: configData[workCfg].TurnoutSettings.OutPins[thisRow].LNAddr = verifyNumber(sender.value, configData[workCfg].TurnoutSettings.OutPins[thisRow].LNAddr); 
				if ([1].indexOf(configData[workCfg].TurnoutSettings.OutPins[thisRow].TOType) >= 0)
					writeTurnoutToDCC(thisRow);
			break;
		case 5: if ((sender.value == 0) || (configData[workCfg].TurnoutSettings.OutPins[thisRow].TOType == 0))
					DeleteTurnoutFromDCC(thisRow); //if changing from/to ZPIN, delete to avoid doubling IDs
				configData[workCfg].TurnoutSettings.OutPins[thisRow].TOType = verifyNumber(sender.value, configData[workCfg].TurnoutSettings.OutPins[thisRow].TOType); 
				writeTurnoutToDCC(thisRow);
			break;
		case 6: configData[workCfg].TurnoutSettings.OutPins[thisRow].Logic = sender.selectedIndex;
				if ([0].indexOf(configData[workCfg].TurnoutSettings.OutPins[thisRow].TOType) >= 0)
					writeTurnoutToDCC(thisRow);
			break;
		case 7: configData[workCfg].TurnoutSettings.OutPins[thisRow].Startup = sender.selectedIndex;
				if ([0].indexOf(configData[workCfg].TurnoutSettings.OutPins[thisRow].TOType) >= 0)
					writeTurnoutToDCC(thisRow);
			break;
		case 8: configData[workCfg].TurnoutSettings.OutPins[thisRow].PinNr = verifyNumber(sender.value, configData[workCfg].TurnoutSettings.OutPins[thisRow].PinNr); 
				if ([0,2,3].indexOf(configData[workCfg].TurnoutSettings.OutPins[thisRow].TOType) >= 0)
					writeTurnoutToDCC(thisRow);
			break;
		case 9: configData[workCfg].TurnoutSettings.OutPins[thisRow].Par1 = verifyNumber(sender.value, configData[workCfg].TurnoutSettings.OutPins[thisRow].Par1); 
				if ([2].indexOf(configData[workCfg].TurnoutSettings.OutPins[thisRow].TOType) >= 0)
					writeTurnoutToDCC(thisRow);
			break;
		case 10: configData[workCfg].TurnoutSettings.OutPins[thisRow].Par2 = verifyNumber(sender.value, configData[workCfg].TurnoutSettings.OutPins[thisRow].Par2); 
				if ([2].indexOf(configData[workCfg].TurnoutSettings.OutPins[thisRow].TOType) >= 0)
					writeTurnoutToDCC(thisRow);
			break;
		case 11: configData[workCfg].TurnoutSettings.OutPins[thisRow].Prof = verifyNumber(sender.value, configData[workCfg].TurnoutSettings.OutPins[thisRow].Prof); 
				if ([2].indexOf(configData[workCfg].TurnoutSettings.OutPins[thisRow].TOType) >= 0)
					writeTurnoutToDCC(thisRow);
			break;
	}
}

function writeTurnoutToDCC(turnoutNr)
{
	var turnoutData = configData[workCfg].TurnoutSettings.OutPins[turnoutNr];
	var cmdStr;
	switch (turnoutData.TOType)
	{
		case 0: var iFlags;
				switch (turnoutData.Startup)
				{
					case 0: iFlags = turnoutData.Logic; break;
					case 1: iFlags = 6 + turnoutData.Logic; break;
					case 2: iFlags = 2 + turnoutData.Logic; break;
				}
				cmdStr = "Z " + turnoutData.Id + " " + turnoutData.PinNr + " " + iFlags; break;
		case 1: cmdStr = "T " + turnoutData.Id + " DCC " + turnoutData.LNAddr; break;
		case 2: cmdStr = "T " + turnoutData.Id + " SERVO " + turnoutData.PinNr + " " + turnoutData.Par1 + " " + turnoutData.Par2 + " " + turnoutData.Prof; break;
		case 3: cmdStr = "T " + turnoutData.Id + " VPIN " + turnoutData.PinNr; break;
		default: return;
	}
	cmdStr = "{\"Cmd\":\"SetDCCPP\", \"SubCmd\":\"SendCmd\",\"OpCode\": \"" + cmdStr + "\"}";
//	console.log(cmdStr);
	ws.send(cmdStr);
}

function DeleteTurnoutFromDCC(sensorNr)
{
	var turnoutData = configData[workCfg].TurnoutSettings.OutPins[sensorNr];
	var cmdStr = (turnoutData.TOType == 0 ? "Z " : "T ") + turnoutData.Id;
	cmdStr = "{\"Cmd\":\"SetDCCPP\", \"SubCmd\":\"SendCmd\",\"OpCode\": \"" + cmdStr + "\"}";
//	console.log(cmdStr);
	ws.send(cmdStr);
}

function writeSensorToDCC(sensorNr)
{
	var sensorData = configData[workCfg].InputSettings.InpPins[sensorNr];
	var cmdStr = "S " + sensorData.Id + " " + sensorData.PinNr + " " + sensorData.Logic;
	cmdStr = "{\"Cmd\":\"SetDCCPP\", \"SubCmd\":\"SendCmd\",\"OpCode\": \"" + cmdStr + "\"}";
//	console.log(cmdStr);
	ws.send(cmdStr);
}

function DeleteSensorFromDCC(sensorNr)
{
	var sensorData = configData[workCfg].InputSettings.InpPins[sensorNr];
	var cmdStr = "S " + sensorData.Id;
	cmdStr = "{\"Cmd\":\"SetDCCPP\", \"SubCmd\":\"SendCmd\",\"OpCode\": \"" + cmdStr + "\"}";
//	console.log(cmdStr);
	ws.send(cmdStr);
}

function writeTrackToDCC(trackNr)
{
	var trackData = configData[workCfg].TrackSettings.TrackModes[trackNr];
	var cmdStr = "= " + String.fromCharCode(trackNr+65) + " " + trackModeList[trackData.Status];
	if (trackData.Status > 2)
	{
		cmdStr += " ";
		cmdStr += trackData.Addr > 0 ? trackData.Addr : 1; //DCC EX only accepting Addr > 0
	}
	cmdStr = "{\"Cmd\":\"SetDCCPP\", \"SubCmd\":\"SendCmd\",\"OpCode\": \"" + cmdStr + "\"}";
//	console.log(cmdStr);
	ws.send(cmdStr);
}

function loadSensorTable(thisTable, thisData, clrConf)
{
	var th = document.getElementById(thisTable.id + "_head");
	var tb = document.getElementById(thisTable.id + "_body");
	var numCols = th.childNodes[0].children.length;

	createDataTableLines(thisTable, [tfPos,tfManipulatorBox, tfNumeric, tfText, tfNumeric, tfEnablePosNegSel, tfMessageTypeSel, tfNumeric], thisData.length, "setSensorConfig(this)");	
	for (var i=0; i<thisData.length;i++)
	{
		var e = document.getElementById(thisTable.id + "_" + i.toString() + "_" + "2");
		e.childNodes[0].value = thisData[i].Id;
		var e = document.getElementById(thisTable.id + "_" + i.toString() + "_" + "3");
		if (clrConf)
			thisData[i].confirmed = false;
		e.childNodes[0].innerHTML = thisData[i].confirmed ? "ok" : "not set";
		var e = document.getElementById(thisTable.id + "_" + i.toString() + "_" + "4");
		e.childNodes[0].value = thisData[i].PinNr;
		var e = document.getElementById(thisTable.id + "_" + i.toString() + "_" + "5");
		e.childNodes[0].value = thisData[i].Logic;
		var e = document.getElementById(thisTable.id + "_" + i.toString() + "_" + "6");
		switch (thisData[i].LNType)
		{
			case 5: e.childNodes[0].value = 2; break;
			case 9: e.childNodes[0].value = 1; break;
			default: e.childNodes[0].value = thisData[i].LNType; break;
		}
		var e = document.getElementById(thisTable.id + "_" + i.toString() + "_" + "7");
		e.childNodes[0].value = thisData[i].LNAddr;
	}
}

function loadTurnoutTable(thisTable, thisData, clrConf)
{
	var th = document.getElementById(thisTable.id + "_head");
	var tb = document.getElementById(thisTable.id + "_body");
	var numCols = th.childNodes[0].children.length;

	createDataTableLines(thisTable, [tfPos,tfManipulatorBox, tfNumeric, tfText, tfNumeric, tfTurnoutTypeSel, tfEnablePosNegSel, tfStartupSel, tfNumeric, tfNumeric, tfNumeric, tfNumeric, tfText], thisData.length, "setTurnoutConfig(this)");	
	for (var i=0; i<thisData.length;i++)
	{
		var e = document.getElementById(thisTable.id + "_" + i.toString() + "_" + "2");
		e.childNodes[0].value = thisData[i].Id;
		var e = document.getElementById(thisTable.id + "_" + i.toString() + "_" + "3");
		if (clrConf)
			thisData[i].confirmed = false;
		e.childNodes[0].innerHTML = thisData[i].confirmed ? "ok" : "not set";
		var e = document.getElementById(thisTable.id + "_" + i.toString() + "_" + "4");
		e.childNodes[0].value = thisData[i].LNAddr;
		var e = document.getElementById(thisTable.id + "_" + i.toString() + "_" + "5");
		e.childNodes[0].value = thisData[i].TOType;
		var e = document.getElementById(thisTable.id + "_" + i.toString() + "_" + "6");
		e.childNodes[0].value = thisData[i].Logic;
		var e = document.getElementById(thisTable.id + "_" + i.toString() + "_" + "7");
		e.childNodes[0].value = thisData[i].Startup;
		var e = document.getElementById(thisTable.id + "_" + i.toString() + "_" + "8");
		e.childNodes[0].value = thisData[i].PinNr;
		var e = document.getElementById(thisTable.id + "_" + i.toString() + "_" + "9");
		e.childNodes[0].value = thisData[i].Par1;
		var e = document.getElementById(thisTable.id + "_" + i.toString() + "_" + "10");
		e.childNodes[0].value = thisData[i].Par2;
		var e = document.getElementById(thisTable.id + "_" + i.toString() + "_" + "11");
		e.childNodes[0].value = thisData[i].Prof;
		var e = document.getElementById(thisTable.id + "_" + i.toString() + "_" + "12");
		e.childNodes[0].innerHTML = thisData[i].Descr;
	}
}

function loadAutomationTable(thisTable, thisData, clrConf)
{
	var th = document.getElementById(thisTable.id + "_head");
	var tb = document.getElementById(thisTable.id + "_body");
	var numCols = th.childNodes[0].children.length;

	createDataTableLines(thisTable, [tfPos, tfManipulatorBox, tfText, tfText, tfText, tfText], thisData.length, "setAutomationConfig(this)");	
	for (var i=0; i<thisData.length;i++)
	{
		var e = document.getElementById(thisTable.id + "_" + i.toString() + "_" + "2");
		if (clrConf)
			thisData[i].confirmed = false;
		e.childNodes[0].innerHTML = thisData[i].confirmed ? "ok" : "not set";
		var e = document.getElementById(thisTable.id + "_" + i.toString() + "_" + "3");
		e.childNodes[0].innerHTML = thisData[i].ID;
		var e = document.getElementById(thisTable.id + "_" + i.toString() + "_" + "4");
		e.childNodes[0].innerHTML = thisData[i].Type;
		var e = document.getElementById(thisTable.id + "_" + i.toString() + "_" + "5");
		e.childNodes[0].innerHTML = thisData[i].Descr;
	}
}

function loadRosterTable(thisTable, thisData, clrConf)
{
	var th = document.getElementById(thisTable.id + "_head");
	var tb = document.getElementById(thisTable.id + "_body");
	var numCols = th.childNodes[0].children.length;

	createDataTableLines(thisTable, [tfPos, tfManipulatorBox, tfText, tfText, tfText, tfText], thisData.length, "setRosterConfig(this)");	
	for (var i=0; i<thisData.length;i++)
	{
		var e = document.getElementById(thisTable.id + "_" + i.toString() + "_" + "2");
		if (clrConf)
			thisData[i].confirmed = false;
		e.childNodes[0].innerHTML = thisData[i].confirmed ? "ok" : "not set";
		var e = document.getElementById(thisTable.id + "_" + i.toString() + "_" + "3");
		e.childNodes[0].innerHTML = thisData[i].DCCAddr;
		var e = document.getElementById(thisTable.id + "_" + i.toString() + "_" + "4");
		e.childNodes[0].innerHTML = thisData[i].Descr;
		var e = document.getElementById(thisTable.id + "_" + i.toString() + "_" + "5");
		e.childNodes[0].innerHTML = thisData[i].FNMap;
	}
}

function loadTrackTable(thisTable, thisData, clrConf)
{
	var th = document.getElementById(thisTable.id + "_head");
	var tb = document.getElementById(thisTable.id + "_body");
	var numCols = th.childNodes[0].children.length;
	if (hwVersion != undefined)
	{
		if (hwVersion.indexOf("MEGA") >= 0)
			createDataTableLines(thisTable, [tfText, tfText, tfTrackSelMEGA, tfNumeric], thisData.length, "setTrackConfig(this)");	
		else
			createDataTableLines(thisTable, [tfText, tfText, tfTrackSelUNO, tfText], thisData.length, "setTrackConfig(this)");
	}	
	else
		createDataTableLines(thisTable, [tfPos, tfManipulatorBox, tfText, tfText, tfTrackSelUNO, tfText], thisData.length, "setTrackConfig(this)");	
	for (var i=0; i<thisData.length;i++)
	{
		var e = document.getElementById(thisTable.id + "_" + i.toString() + "_" + "0");
		e.childNodes[0].innerHTML = String.fromCharCode(i+65);

		var e = document.getElementById(thisTable.id + "_" + i.toString() + "_" + "1");
		if (clrConf)
			thisData[i].confirmed = false;
		e.childNodes[0].innerHTML = thisData[i].confirmed ? "ok" : "not set";

		var e = document.getElementById(thisTable.id + "_" + i.toString() + "_" + "2");
		e.childNodes[0].selectedIndex = thisData[i].Status;
		var e = document.getElementById(thisTable.id + "_" + i.toString() + "_" + "3");
		if (hwVersion != undefined)
			if (hwVersion.indexOf("MEGA") >= 0)
				e.childNodes[0].value = thisData[i].Addr;
		setVisibility(thisData[i].Status > 2, e);
	}
}

function confirmSensorPin(jsonData)
{
	var sensorId = jsonData.Data.Msg[1].d;
	var pinNr = jsonData.Data.Msg[2].d;
	var pinLogic = jsonData.Data.Msg[3].d;
	var sensorPos = configData[workCfg].InputSettings.InpPins.findIndex(element => element.Id == sensorId);
	
//	console.log("Confirm Sensor");
	if (sensorPos < 0)
	{
		sensorPos = configData[2].InputSettings.InpPins.push(JSON.parse(JSON.stringify(newSensorTemplate))) - 1;
		configData[workCfg].InputSettings.InpPins[sensorPos].Id = sensorId;
	}
	configData[workCfg].InputSettings.InpPins[sensorPos].PinNr = pinNr;
	configData[workCfg].InputSettings.InpPins[sensorPos].Logic = pinLogic;
	configData[workCfg].InputSettings.InpPins[sensorPos].confirmed = true;
	loadSensorTable(sensorTable, configData[workCfg].InputSettings.InpPins);
}

function updateZPin(thisRow, jsonData)
{ // <Y ID PIN IFLAG STATE>
//	console.log("Confirm Z Pin");
	var pinNr = jsonData.Data.Msg[2].d;
	var iFlag = jsonData.Data.Msg[3].d;
	configData[workCfg].TurnoutSettings.OutPins[thisRow].PinNr = pinNr;
	configData[workCfg].TurnoutSettings.OutPins[thisRow].TOType = 0;
	configData[workCfg].TurnoutSettings.OutPins[thisRow].Logic = iFlag & 0x01;
	configData[workCfg].TurnoutSettings.OutPins[thisRow].Startup = 0; //default restore
	switch (iFlag & 0x06)
	{
		case 2:	configData[workCfg].TurnoutSettings.OutPins[thisRow].Startup = 1; break;
		case 6:	configData[workCfg].TurnoutSettings.OutPins[thisRow].Startup = 2; break;
	}
}

function updateDCCPin(thisRow, jsonData)
{ //<H ID DCC ADDRESS SUBADDRESS THROWN>
//	console.log("Confirm DCC Pin");
	configData[workCfg].TurnoutSettings.OutPins[thisRow].LNAddr = (4 * (jsonData.Data.Msg[3].d-1)) + jsonData.Data.Msg[4].d + 1;
	configData[workCfg].TurnoutSettings.OutPins[thisRow].TOType = 1;
}

function updateServoPin(thisRow, jsonData)
{//<H ID SERVO PIN THROWNPOSITION CLOSEDPOSITION PROFILE THROWN>
//	console.log("Confirm Servo");
	configData[workCfg].TurnoutSettings.OutPins[thisRow].PinNr = jsonData.Data.Msg[3].d;
	configData[workCfg].TurnoutSettings.OutPins[thisRow].TOType = 2;
	configData[workCfg].TurnoutSettings.OutPins[thisRow].Par1 = jsonData.Data.Msg[4].d;
	configData[workCfg].TurnoutSettings.OutPins[thisRow].Par2 = jsonData.Data.Msg[5].d;
	configData[workCfg].TurnoutSettings.OutPins[thisRow].Prof = jsonData.Data.Msg[6].d;
}

function updateVPin(thisRow, jsonData)
{//<H ID VPIN PIN THROWN>
//	console.log("Confirm VPin");
	configData[workCfg].TurnoutSettings.OutPins[thisRow].PinNr = jsonData.Data.Msg[3].d;
	configData[workCfg].TurnoutSettings.OutPins[thisRow].TOType = 3;
}

function confirmTurnoutPin(jsonData)
{
//	console.log(jsonData); // <Y ID PIN IFLAG STATE> or <H ID ADDRESS SUBADDRESS THROWN>

	var turnoutId = jsonData.Data.Msg[1].d;
	var turnoutPos = configData[workCfg].TurnoutSettings.OutPins.findIndex(element => element.Id == turnoutId);
	if (turnoutPos < 0)
	{
		turnoutPos = configData[2].TurnoutSettings.OutPins.push(JSON.parse(JSON.stringify(newTurnoutTemplate))) - 1;
		configData[workCfg].TurnoutSettings.OutPins[turnoutPos].Id = turnoutId;
	}

	var opCode =  jsonData.Data.Msg[0].d;
	switch (opCode)
	{
		case "Y" : updateZPin(turnoutPos, jsonData); break;
		case "H" :
			var pinType = jsonData.Data.Msg[2].d;
			switch (pinType)
			{
				case "DCC": updateDCCPin(turnoutPos, jsonData); break;
				case "SERV": updateServoPin(turnoutPos, jsonData); break;
				case "VPIN": updateVPin(turnoutPos, jsonData); break;
				default: console.log("Unknown type"); break;
			}
			break;
	}
	configData[workCfg].TurnoutSettings.OutPins[turnoutPos].confirmed = true;
	loadTurnoutTable(turnoutTable, configData[workCfg].TurnoutSettings.OutPins);
}

function confirmTurnout(jsonData)
{
	var turnoutId = jsonData.Data.Msg[2].d;
	var turnoutPos = configData[workCfg].TurnoutSettings.OutPins.findIndex(element => element.Id == turnoutId);
	
	if (turnoutPos < 0)
	{
		turnoutPos = configData[2].TurnoutSettings.OutPins.push(JSON.parse(JSON.stringify(newTurnoutTemplate))) - 1;
		configData[workCfg].TurnoutSettings.OutPins[turnoutPos].Id = turnoutId;
	}
//	configData[workCfg].TurnoutSettings.OutPins[turnoutPos].Type = jsonData.Data.Msg[3].d;
	configData[workCfg].TurnoutSettings.OutPins[turnoutPos].Descr = jsonData.Data.Msg[4].d;
	configData[workCfg].TurnoutSettings.OutPins[turnoutPos].confirmed = true;
	loadTurnoutTable(turnoutTable, configData[workCfg].TurnoutSettings.OutPins);
}

function confirmAutomation(jsonData)
{
//	console.log(jsonData);
	var routeID = jsonData.Data.Msg[2].d;
	var routePos = configData[workCfg].AutomationSettings.Routes.findIndex(element => element.ID == routeID);
	
	if (routePos < 0)
	{
		routePos = configData[2].AutomationSettings.Routes.push(JSON.parse(JSON.stringify(newRouteTemplate))) - 1;
		configData[workCfg].AutomationSettings.Routes[routePos].ID = routeID;
	}
	configData[workCfg].AutomationSettings.Routes[routePos].Type = jsonData.Data.Msg[3].d;
	configData[workCfg].AutomationSettings.Routes[routePos].Descr = jsonData.Data.Msg[4].d;
	configData[workCfg].AutomationSettings.Routes[routePos].confirmed = true;
	loadAutomationTable(automationTable, configData[workCfg].AutomationSettings.Routes, false);
}

function confirmRoster(jsonData)
{
//	console.log(jsonData);
	var locoID = jsonData.Data.Msg[2].d;
	var locoPos = configData[workCfg].RosterSettings.Locos.findIndex(element => element.DCCAddr == locoID);
	
	if (locoPos < 0)
	{
		locoPos = configData[2].RosterSettings.Locos.push(JSON.parse(JSON.stringify(newLocoTemplate))) - 1;
		configData[workCfg].RosterSettings.Locos[locoPos].DCCAddr = locoID;
	}
	configData[workCfg].RosterSettings.Locos[locoPos].Descr = jsonData.Data.Msg[3].d;
	configData[workCfg].RosterSettings.Locos[locoPos].FNMap = jsonData.Data.Msg[4].d;
	configData[workCfg].RosterSettings.Locos[locoPos].confirmed = true;
	loadRosterTable(rosterTable, configData[workCfg].RosterSettings.Locos, false);
}

function confirmTrack(jsonData)
{
//	console.log(jsonData);
	var trackID = jsonData.Data.Msg[1].d.charCodeAt(0)-65;
	var trackMode = jsonData.Data.Msg[2].d;
	var dccAddr = jsonData.Data.Msg.length > 3 ? jsonData.Data.Msg[3].d : 0; 
	while (configData[workCfg].TrackSettings.TrackModes.length <= trackID)
		configData[workCfg].TrackSettings.TrackModes.push(JSON.parse(JSON.stringify(newTrackTemplate)));
	configData[workCfg].TrackSettings.TrackModes[trackID].Status = trackModeList.indexOf(trackMode);
	configData[workCfg].TrackSettings.TrackModes[trackID].Addr = dccAddr;
	configData[workCfg].TrackSettings.TrackModes[trackID].confirmed = true;
//	console.log(configData[workCfg].TrackSettings.TrackModes[trackID], dccAddr);
//	console.log("TT B", trackTable.parentNode.parentNode.parentNode.parentNode.parentNode);
	loadTrackTable(trackTable, configData[workCfg].TrackSettings.TrackModes, false);
}

function loadNodeDataFields(jsonData)
{
//	console.log(jsonData);
		
}

function loadProgOptions(jsonData)
{
	writeInputField("proglimit", jsonData.DevSettings.ProgTrack.CurrLimit);
	writeInputField("progpulsemin", jsonData.DevSettings.ProgTrack.MinAckP);
	writeInputField("progpulsemax", jsonData.DevSettings.ProgTrack.MaxAckP);
	writeInputField("progattempt", jsonData.DevSettings.ProgTrack.NumTry);
	writeCBInputField("progboost", jsonData.DevSettings.ProgTrack.Boost);
}

function loadDataFields(jsonData)
{
	configData[workCfg] = upgradeJSONVersion(jsonData);
//	console.log(configData[workCfg]);
	loadProgOptions(configData[workCfg]);
	writeCBInputField("sensoreeprom", configData[workCfg].DevSettings.ConfigToEEPROM);
	writeCBInputField("turnouteeprom", configData[workCfg].DevSettings.ConfigToEEPROM);
	document.getElementById("fcrate").selectedIndex = configData[workCfg].DevSettings.FCRate;
	if (configData[workCfg].DevSettings.CfgSlot != undefined)
		for (var i = 0; i < configData[workCfg].DevSettings.CfgSlot.length; i++)
			if (i != 4)
				for (var j = 0; j < 8; j++)
				{
					var byteNr = i;
					if (byteNr > 3)
						byteNr--; //byte for is track power
					var opSwId = "OpSw_" + ((8 * byteNr) + j).toString();
					guiEl = document.getElementById(opSwId);
					if (guiEl != null)
						switch (guiEl.nodeName)
						{
							case "SELECT":
								var numBits = parseInt((Math.log(guiEl.options.length) / Math.log(2)));
								var bitMask = Math.pow(2, numBits) - 1;
								var bitVal = (configData[workCfg].DevSettings.CfgSlot[i] >> j) & bitMask;
							
//								console.log(numBits, bitVal, bitMask);
								guiEl.selectedIndex = bitVal;
								break;
							case "INPUT":
								guiEl.checked = ((configData[workCfg].DevSettings.CfgSlot[i] & (0x01 << j)) > 0);
								break;
						}
				}
	try
	{
//		console.log("TT C", trackTable.parentNode);
		loadTrackTable(trackTable, configData[workCfg].TrackSettings.TrackModes, true);
	}
	catch (error) {console.log(error)};

	try
	{
		loadSensorTable(sensorTable, configData[workCfg].InputSettings.InpPins, true);
	}
	catch (error) {};
	try
	{
		loadTurnoutTable(turnoutTable, configData[workCfg].TurnoutSettings.OutPins, true);
	}
	catch (error) {};
	try
	{
		loadAutomationTable(automationTable, configData[workCfg].AutomationSettings.Routes, true);
	}
	catch (error) {};
	try
	{
		loadRosterTable(rosterTable, configData[workCfg].RosterSettings.Locos, true);
	}
	catch (error) {};

	while (GaugePanel.lastElementChild) 
		GaugePanel.removeChild(GaugePanel.lastElementChild);

	for (var i = 0; i < configData[workCfg].CurrentTracker.length; i++)
	{
		var subGaugePanel = createEmptyDiv(GaugePanel, "div", "tile-1_4", "");
		var trackCanvas = document.createElement("CANVAS");
		trackCanvas.setAttribute("id","trackCanvas_" + i.toString()); 
		trackCanvas.setAttribute("style","tile-1_4"); 
		subGaugePanel.append(trackCanvas);
		trackGauges.push(trackCanvas);
		var trackGauge = new RadialGauge(
			{width: 200, 
			height: 200, 
			units: 'mA', 
			renderTo: "trackCanvas_" + i.toString(),
			title: configData[workCfg].CurrentTracker[i].GaugeName,
			value: 0,
			minValue: 0,
			maxValue: configData[workCfg].CurrentTracker[i].MaxVal,
			majorTicks: configData[workCfg].CurrentTracker[i].MainTicks,
			minorTicks: 2,
		}).draw();
		trackGaugeDefs.push(trackGauge);
	}

	document.getElementById("currgaugeselector").selectedIndex = 0;
	loadGaugeData(0);
	setVisibility(true, document.getElementById("cbsetup_6")); //only show menu item if needed


	setPanelVisibility();
	cmdStr = "{\"Cmd\":\"SetDCCPP\", \"SubCmd\":\"GetConfig\",\"Filter\": 0}";
//	console.log(cmdStr);
	ws.send(cmdStr);
	cmdStr = "{\"Cmd\":\"GetFC\"}";
//	console.log(cmdStr);
	ws.send(cmdStr);
}

function processDCCPPInput(jsonData)
{
//	console.log("processDCCPPInput");
//	console.log(jsonData);
	if (jsonData.Data.Msg[0].t == 1) //opCode
	{
		var opCode = jsonData.Data.Msg[0].d;
		if ((opCode == '#') && (jsonData.Data.Msg.length == 2))
			writeTextField("refreshslots", jsonData.Data.Msg[1].d); //refresh slot update
		if (opCode == 'H') //VPin
		{
			confirmTurnoutPin(jsonData);
		}
		if (opCode == 'Q') //Input
		{
			confirmSensorPin(jsonData);
		}
		if (opCode == 'Y') //ZPin
		{
			confirmTurnoutPin(jsonData);
		}
		if (opCode == 'i') //Arduino info
		{
			swDispStr = jsonData.Data.Msg[1].d + " " + jsonData.Data.Msg[2].d + " on Arduino " + jsonData.Data.Msg[4].d;
			swVersion = jsonData.Data.Msg[2].d;
			hwVersion = jsonData.Data.Msg[4].d;
			document.getElementById("BasicCfg_TitleMain").innerHTML = "<h2>" + pageTitleStr + "(" + swDispStr + ")";
//			setVisibility(swVersion.indexOf("V-4") >= 0, document.getElementById("OpSw_46").parentElement); //only show menu item if needed
			setVisibility(swVersion.indexOf("V-5") >= 0, document.getElementById("cbsetup_5")); //only show menu item if needed
//			setVisibility(swVersion.indexOf("V-5") >= 0, tabTracks); //only show menu item if needed
//			console.log("set Vis tabTracks");
		}
		if (opCode == 'j') //Automation or Roster
		{
			if (jsonData.Data.Msg[1].d == 'T') //Automation
			{
				confirmTurnout(jsonData);
			}
			if (jsonData.Data.Msg[1].d == 'A') //Automation
			{
				confirmAutomation(jsonData);
			}
			if (jsonData.Data.Msg[1].d == 'R') //Automation
			{
				confirmRoster(jsonData);
			}
		}
		if (opCode == '=') //Track data
		{
			confirmTrack(jsonData);
		}
	}
}

function processTrackDataInput(jsonData)
{
//	console.log(jsonData);
	var dispVal = 0;
	for (var i = 0; i < trackGauges.length; i++)
	{
		if (configData[workCfg].CurrentTracker[i].PinNr == jsonData.Data.Track)
		{
			dispVal = Math.min(jsonData.Data.Value, configData[workCfg].CurrentTracker[i].MaxVal);
			trackGaugeDefs[i].value = dispVal;
			break;
		}
	}
}

function processFCInput(jsonData)
{
//	console.log(jsonData);
	document.getElementById("fcrate").selectedIndex = jsonData.Data.Rate;
	var hrStr = Math.trunc(jsonData.Data.Time / 3600) % 24;
	var minStr = Math.trunc(jsonData.Data.Time / 60) % 60;
	document.getElementById("timepick").value = hrStr.toString().padStart(2, '0') + ":" + minStr.toString().padStart(2, '0');
}
