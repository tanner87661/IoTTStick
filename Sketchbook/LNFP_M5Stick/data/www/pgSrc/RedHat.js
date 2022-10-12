var mainScrollBox;

var tabConfig;
var tabInputs;
var tabButtons;
var tabGauges;

var updateCurrent = false;
var currTrack = 0;

var trackGauge;
var progGauge;

var ConfigOptionPanel;
var CurrentOptionPanel;
var InputPanel;
var ButtonPanel;

var sensorTable;
var turnoutTable;
var sensorTableDiv;
var turnoutTableDiv;
 
var newSensorTemplate = {"Id": 0, "PinNr": 1,"PNType": 0, "LNType": 0, "Logic": 1, "Par1": 0, "Par2": 0, "LNAddr": 1};
var newTurnoutTemplate = {"Id": 0,"PinNr": 1,"TOType": 3,"LNType": 0,"Logic": 1,"Startup": 0, "Par1": 0,"Par2": 0, "Prof": 2, "LNAddr": 1};

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
	var menueStr = ["Options", "Inputs", "Turnouts", "Monitor"];
	var tempObj;
	mainScrollBox = createEmptyDiv(contentTab, "div", "pagetopicboxscroll-y", "btnconfigdiv");

		var menueTab = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
		for (var i = 0; i < menueStr.length; i++)
			createMenueTabElement(menueTab, "button", "tablink", "RedHatSub", "cbsetup_" + i.toString(), menueStr[i], true, "setPageMode(this)");
		updateMenueTabs("RedHatSub", "cbsetup_0", "grey");
		tabConfig = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
			ConfigOptionPanel = createEmptyDiv(tabConfig, "div", "tile-1", "");
				tempObj= createEmptyDiv(ConfigOptionPanel, "div", "tile-1", "");
					createPageTitle(tempObj, "div", "tile-1", "BasicCfg_Title", "h2", "Command Station Settings");
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
					createPageTitle(tempObj, "div", "tile-1", "BasicCfg_Title", "h2", "LocoNet Options");
					createCheckbox(tempObj, "tile-1_4", "Switch Echo", "OpSw_56", "setOpSwCB(this)"); //IoTT opSw
					createCheckbox(tempObj, "tile-1_4", "Input Echo", "OpSw_57", "setOpSwCB(this)"); //IoTT opSw
					createCheckbox(tempObj, "tile-1_4", "Loco Echo", "OpSw_58", "setOpSwCB(this)"); //IoTT opSw
					createCheckbox(tempObj, "tile-1_4", "Power Status Echo", "OpSw_42", "setOpSwCB(this)");
				 tempObj= createEmptyDiv(ConfigOptionPanel, "div", "tile-1", "");
					createCheckbox(tempObj, "tile-1_4", "Disable Switch State Report", "OpSw_44", "setOpSwCB(this)");
//					createCheckbox(tempObj, "tile-1_4", "Disable Standard Switch Command", "OpSw_25", "setOpSwCB(this)");

//				 tempObj= createEmptyDiv(ConfigOptionPanel, "div", "tile-1", "");
//					createPageTitle(tempObj, "div", "tile-1", "BasicCfg_Title", "h2", "Communication Options");
//				 tempObj= createEmptyDiv(ConfigOptionPanel, "div", "tile-1", "");
//					createCheckbox(tempObj, "tile-1_4", "Provide Loconet TCP Server", "lbserver", "setServerOptions(this)");
//					createTextInput(tempObj, "tile-1_4", "on port:", "n/a", "lbserverport", "setServerOptions(this)");
//				 tempObj= createEmptyDiv(ConfigOptionPanel, "div", "tile-1", "");
//					createCheckbox(tempObj, "tile-1_4", "Provide WiThrottle Server", "withrottle", "setServerOptions(this)");
//					createTextInput(tempObj, "tile-1_4", "on port:", "n/a", "withrottleport", "setServerOptions(this)");

				 tempObj= createEmptyDiv(ConfigOptionPanel, "div", "tile-1", "");
					createPageTitle(tempObj, "div", "tile-1", "BasicCfg_Title", "h2", "DCC Generator Options");
					createDropdownselector(tempObj, "tile-1_4", "DCC Speed Steps:", ["128 Steps","28 Steps"], "OpSw_20", "setOpSwDD(this)");
					createDispText(tempObj, "tile-1_4", "DCC++ Refresh Slots:", "n/a", "refreshslots");
				 tempObj= createEmptyDiv(ConfigOptionPanel, "div", "tile-1", "");
					createPageTitle(tempObj, "div", "tile-1", "BasicCfg_Title", "h2", "Current Measurement Settings");
					createCheckbox(tempObj, "tile-1_4", "Report Main Track", "reportmain", "setCurrentOptions(this)");
					createCheckbox(tempObj, "tile-1_4", "Report Prog Track", "reportprog", "setCurrentOptions(this)");
				 tempObj= createEmptyDiv(ConfigOptionPanel, "div", "tile-1", "");
					createTextInput(tempObj, "tile-1_4", "Buffer Size:", "n/a", "currbuffsize", "setCurrentOptions(this)");
					createTextInput(tempObj, "tile-1_4", "Max. Value:", "n/a", "currmaxval", "setCurrentOptions(this)");
				 tempObj= createEmptyDiv(ConfigOptionPanel, "div", "tile-1", "");
					createTextInput(tempObj, "tile-1_4", "Major Ticks:", "n/a", "majorticks", "setCurrentOptions(this)");
				 tempObj= createEmptyDiv(ConfigOptionPanel, "div", "tile-1", "");
					createPageTitle(tempObj, "div", "tile-1", "BasicCfg_Title", "h2", "Programming Track Options");
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
					createPageTitle(sensorTableDiv, "div", "tile-1", "BasicCfg_Title", "h2", "Arduino Input Pins");
					sensorTable = createDataTable(sensorTableDiv, "tile-1_2", ["Pos","Add/Delete/Move", "ID", "DCC++", "Pin Nr", "Logic", "Message Type", "LocoNet Addr"], "sensorconfig", "");
				tempObj = createEmptyDiv(InputPanel, "div", "tile-1", "");
					createCheckbox(tempObj, "tile-1_4", "Store Input Settings to EEPROM", "sensoreeprom", "setProgOptions(this)");
		tabButtons = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
			ButtonPanel = createEmptyDiv(tabButtons, "div", "tile-1", "");
				turnoutTableDiv = createEmptyDiv(ButtonPanel, "div", "tile-1", "");
					createPageTitle(turnoutTableDiv, "div", "tile-1", "BasicCfg_Title", "h2", "Defined Turnouts and Pins");
					turnoutTable = createDataTable(turnoutTableDiv, "tile-1_2", ["Pos","Add/Delete/Move", "ID", "DCC++", "Turnout Address", "Turnout Type", "Logic", "Start-up", "Pin Nr", "Min. Position", "Max. Position", "Profile"], "turnoutconfig", "");
				tempObj = createEmptyDiv(ButtonPanel, "div", "tile-1", "");
					createSimpleText(tempObj,"tile-1", "(!) Not recommended. Use VPIN instead", "zpinwarning");
					createCheckbox(tempObj, "tile-1_4", "Store Turnout/Pin Settings to EEPROM", "turnouteeprom", "setProgOptions(this)");
		tabGauges = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
			GaugePanel = createEmptyDiv(tabGauges, "div", "tile-1", "");
				subGaugePanel1 = createEmptyDiv(GaugePanel, "div", "tile-1_4", "");
				subGaugePanel2 = createEmptyDiv(GaugePanel, "div", "tile-1_4", "");

			var trackCanvas = document.createElement("CANVAS");
			trackCanvas.setAttribute("id","trackCanvas"); 
			trackCanvas.setAttribute("style","tile-1_4"); 
			subGaugePanel1.append(trackCanvas);
//			var ctx = trackCanvas.getContext("2d");
//			ctx.fillStyle = "#FF0000";
//			ctx.fillRect(20, 20, 150, 100);
			trackGauge = new RadialGauge(
				{width: 200, 
				height: 200, 
				units: 'mA', 
				renderTo: "trackCanvas",
				title: "Main",
				value: 0,
				minValue: 0,
				maxValue: 3000,
				majorTicks: [0],
				minorTicks: 2,
				
				}).draw();

			var progCanvas = document.createElement("CANVAS");
			progCanvas.setAttribute("id","progCanvas"); 
			progCanvas.setAttribute("style","tile-1_4"); 
			subGaugePanel2.append(progCanvas);
//			var ctx = progCanvas.getContext("2d");
//			ctx.fillStyle = "#00FF00";
//			ctx.fillRect(20, 20, 150, 100);
			progGauge = new RadialGauge(
				{width: 200, 
				height: 200, 
				units: 'mA', 
				renderTo: "progCanvas",
				title: "Prog",
				value: 0,
				minValue: 0,
				maxValue: 3000,
				majorTicks: [0],
				minorTicks: 2,
				
				}).draw();
			setVisibility(false, trackCanvas);
			setVisibility(false, progCanvas);


/*
	ConfigOptionPanel = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
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

*/

	tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
		createButton(tempObj, "", "Save & Restart", "btnSave", "saveEEPROMSettings(this)");
		createButton(tempObj, "", "Cancel", "btnCancel", "cancelSettings(this)");
//		createButton(tempObj, "", "Request", "btnReq", "requestCurrent(this)");
	tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
			createButton(tempObj, "", "Save to File", "btnDownload", "downloadSettings(this)");
	setVisibility(false, tabInputs);
	setVisibility(false, tabButtons);
	setVisibility(false, tabGauges);

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
	console.log(cmdStr);
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

function setCurrentOptions(sender)
{
	if (sender.id == "reportmain")
		configData[workCfg].CurrentTracker.ReportMode = sender.checked ? configData[workCfg].CurrentTracker.ReportMode | 0x01 : configData[workCfg].CurrentTracker.ReportMode & ~0x01; 
	if (sender.id == "reportprog")
		configData[workCfg].CurrentTracker.ReportMode = sender.checked ? configData[workCfg].CurrentTracker.ReportMode | 0x02 : configData[workCfg].CurrentTracker.ReportMode & ~0x02; 
	if (sender.id == "currbuffsize")
		configData[workCfg].CurrentTracker.SampleSize = verifyNumber(sender.value, configData[workCfg].CurrentTracker.SampleSize); 
	if (sender.id == "currmaxval")
		configData[workCfg].CurrentTracker.MaxVal = verifyNumber(sender.value, configData[workCfg].CurrentTracker.MaxVal); 
	if (sender.id == "majorticks")
	{
		var newArray = verifyNumArray(sender.value, ",");
		if (newArray.length > 0)
		{
			configData[workCfg].CurrentTracker.MainTicks = []; //make sure this is an array
			for (var i = 0; i < newArray.length; i++)
				configData[workCfg].CurrentTracker.MainTicks.push(newArray[i]);
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
	setVisibility(false, tabGauges);
	setVisibility(false, document.getElementById("trackCanvas"));
	setVisibility(false, document.getElementById("progCanvas"));
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
			updateCurrent = true;
			setVisibility(true, tabGauges);
			setVisibility((configData[workCfg].CurrentTracker.ReportMode & 0x02) > 0, document.getElementById("progCanvas"));
			setVisibility((configData[workCfg].CurrentTracker.ReportMode & 0x01) > 0, document.getElementById("trackCanvas"));
			setTimeout(function(){requestCurrent(sender) }, 1000);
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

	createDataTableLines(thisTable, [tfPos,tfManipulatorBox, tfNumeric, tfText, tfNumeric, tfTurnoutTypeSel, tfEnablePosNegSel, tfStartupSel, tfNumeric, tfNumeric, tfNumeric, tfNumeric], thisData.length, "setTurnoutConfig(this)");	
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
	}
}

/*
function loadPinTable(thisTable, thisData)
{
	var th = document.getElementById(thisTable.id + "_head");
	var tb = document.getElementById(thisTable.id + "_body");
	var numCols = th.childNodes[0].children.length;

	var lineCount = 0;
	for (var i=0; i<thisData.length;i++)
		if (thisData[i].TOType > 0)
			lineCount++;

	createDataTableLines(thisTable, [tfPos,tfManipulatorBox, tfNumericLong,tfNumericLong, tfNumericLong, tfNumericLong, tfNumericLong], lineCount, "setColorData(this)");	
	for (var i=0; i<thisData.length;i++)
		if (thisData[i].TOType == 0)
		{
			var e = document.getElementById(thisTable.id + "_" + i.toString() + "_" + "2");
			e.childNodes[0].value = thisData[i].Id;
			var e = document.getElementById(thisTable.id + "_" + i.toString() + "_" + "3");
			e.childNodes[0].value = thisData[i].LNAddr;
			var e = document.getElementById(thisTable.id + "_" + i.toString() + "_" + "4");
			e.childNodes[0].value = thisData[i].TOType;
			var e = document.getElementById(thisTable.id + "_" + i.toString() + "_" + "5");
			e.childNodes[0].value = thisData[i].Logic;
			var e = document.getElementById(thisTable.id + "_" + i.toString() + "_" + "6");
			e.childNodes[0].value = thisData[i].PinNr;
			var e = document.getElementById(thisTable.id + "_" + i.toString() + "_" + "7");
			e.childNodes[0].value = thisData[i].Par1;
		}
}
*/

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
//	console.log(jsonData);
	configData[workCfg] = upgradeJSONVersionRH(jsonData);
	loadProgOptions(jsonData);
	writeCBInputField("sensoreeprom", jsonData.DevSettings.ConfigToEEPROM);
	writeCBInputField("turnouteeprom", jsonData.DevSettings.ConfigToEEPROM);
	document.getElementById("fcrate").selectedIndex = jsonData.DevSettings.FCRate;
	if (jsonData.DevSettings.CfgSlot != undefined)
		for (var i = 0; i < jsonData.DevSettings.CfgSlot.length; i++)
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
								var bitVal = (jsonData.DevSettings.CfgSlot[i] >> j) & bitMask;
							
//								console.log(numBits, bitVal, bitMask);
								guiEl.selectedIndex = bitVal;
								break;
							case "INPUT":
								guiEl.checked = ((jsonData.DevSettings.CfgSlot[i] & (0x01 << j)) > 0);
								break;
						}
				}
	loadSensorTable(sensorTable, jsonData.InputSettings.InpPins, true);
	loadTurnoutTable(turnoutTable, jsonData.TurnoutSettings.OutPins, true);

	trackGauge.update({
				maxValue: jsonData.CurrentTracker.MaxVal,
				majorTicks: jsonData.CurrentTracker.MainTicks,
	});
	progGauge.update({
				maxValue: jsonData.CurrentTracker.MaxVal,
				majorTicks: jsonData.CurrentTracker.MainTicks,
	});
	document.getElementById("currbuffsize").value = jsonData.CurrentTracker.SampleSize;
	document.getElementById("currmaxval").value = jsonData.CurrentTracker.MaxVal;
	document.getElementById("majorticks").value = jsonData.CurrentTracker.MainTicks;
	document.getElementById("reportmain").checked = (jsonData.CurrentTracker.ReportMode & 0x01) > 0;
	document.getElementById("reportprog").checked = (jsonData.CurrentTracker.ReportMode & 0x02) > 0;


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
		
	}
}

function processTrackDataInput(jsonData)
{
//	console.log(jsonData);
	var dispVal = 0;
	if ((jsonData.Data.Value >= 0) && (jsonData.Data.Value < 5000))
		dispVal = jsonData.Data.Value;
	if (jsonData.Data.Track == 0)
		trackGauge.value = dispVal;
	else
		progGauge.value = dispVal;
}

function processFCInput(jsonData)
{
//	console.log(jsonData);
	document.getElementById("fcrate").selectedIndex = jsonData.Data.Rate;
	var hrStr = Math.trunc(jsonData.Data.Time / 3600) % 24;
	var minStr = Math.trunc(jsonData.Data.Time / 60) % 60;
	document.getElementById("timepick").value = hrStr.toString().padStart(2, '0') + ":" + minStr.toString().padStart(2, '0');
}
