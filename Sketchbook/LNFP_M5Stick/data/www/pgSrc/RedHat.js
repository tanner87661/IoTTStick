var mainScrollBox;

var tabConfig;
var tabInputs;
var tabButtons;

var ConfigOptionPanel;
var InputPanel;
var ButtonPanel;

var sensorTable;
var turnoutTable;
var sensorTableDiv;
var turnoutTableDiv;
 
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
	var menueStr = ["Options", "Inputs", "Turnouts"];
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
					createButton(tempObj, "", "Reset Loco Slots", "btnSlotReset", "setOpSw(this)");

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
					turnoutTable = createDataTable(turnoutTableDiv, "tile-1_2", ["Pos","Add/Delete/Move", "ID", "DCC++", "Turnout Address", "Turnout Type", "Logic", "Start-up", "Pin Nr", "Min. Position", "Max. Position"], "turnoutconfig", "");
				tempObj = createEmptyDiv(ButtonPanel, "div", "tile-1", "");
					createSimpleText(tempObj,"tile-1", "(!) Not recommended. Use VPIN instead", "zpinwarning");
					createCheckbox(tempObj, "tile-1_4", "Store Turnout/Pin Settings to EEPROM", "turnouteeprom", "setProgOptions(this)");


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
	tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
			createButton(tempObj, "", "Save to File", "btnDownload", "downloadSettings(this)");
	setVisibility(false, tabInputs);
	setVisibility(false, tabButtons);

}

function saveEEPROMSettings(sender)
{
	if (readCBInputField("sensoreeprom") || readCBInputField("turnouteeprom"))
		ws.send("{\"Cmd\":\"SetDCCPP\", \"SubCmd\":\"SendCmd\",\"OpCode\": \"E\"}");
	saveSettings(sender);
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
		configData[workCfg].DevSettings.ConfigSensorIO = !sender.checked;
	if (sender.id == "turnouteeprom")
		configData[workCfg].DevSettings.ConfigTurnoutIO = !sender.checked;
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
	}
	updateMenueTabs("RedHatSub", sender.id, "grey");
}

function setSensorConfig(sender)
{
	var thisRow = parseInt(sender.getAttribute("row"));
	var thisCol = parseInt(sender.getAttribute("col"));
	var thisIndex = parseInt(sender.getAttribute("index"));
	console.log(thisRow, thisCol, thisIndex);
	switch (thisCol)
	{
		case 1:
			switch (thisIndex)
			{
				case 1:
				case 2:
				case 3:
				case 4:
			}
			break;
		case 2: configData[workCfg].InputSettings.InpPins[thisRow].Id = verifyNumber(sender.value, configData[workCfg].InputSettings.InpPins[thisRow].Id); 
			break;
		case 4: configData[workCfg].InputSettings.InpPins[thisRow].PinNr = verifyNumber(sender.value, configData[workCfg].InputSettings.InpPins[thisRow].PinNr); 
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
	console.log(thisRow, thisCol, thisIndex);
	switch (thisCol)
	{
		case 1:
			switch (thisIndex)
			{
				case 1:
				case 2:
				case 3:
				case 4:
			}
			break;
		case 2: configData[workCfg].TurnoutSettings.OutPins[thisRow].Id = verifyNumber(sender.value, configData[workCfg].TurnoutSettings.OutPins[thisRow].Id); 
			break;
		case 4: configData[workCfg].TurnoutSettings.OutPins[thisRow].LNAddr = verifyNumber(sender.value, configData[workCfg].TurnoutSettings.OutPins[thisRow].LNAddr); 
			break;
		case 5: configData[workCfg].TurnoutSettings.OutPins[thisRow].TOType = verifyNumber(sender.value, configData[workCfg].TurnoutSettings.OutPins[thisRow].TOType); 
			break;
		case 6: configData[workCfg].TurnoutSettings.OutPins[thisRow].Logic = (configData[workCfg].TurnoutSettings.OutPins[thisRow].Logic & 0x06) | sender.selectedIndex;
				writeTurnoutToDCC(thisRow);
			break;
		case 7: configData[workCfg].TurnoutSettings.OutPins[thisRow].Logic = (configData[workCfg].TurnoutSettings.OutPins[thisRow].Logic & 0x01) | (sender.selectedIndex << 1);
			break;
		case 8: configData[workCfg].TurnoutSettings.OutPins[thisRow].PinNr = verifyNumber(sender.value, configData[workCfg].TurnoutSettings.OutPins[thisRow].PinNr); 
			break;
		case 9: configData[workCfg].TurnoutSettings.OutPins[thisRow].Par1 = verifyNumber(sender.value, configData[workCfg].TurnoutSettings.OutPins[thisRow].Par1); 
			break;
		case 10: configData[workCfg].TurnoutSettings.OutPins[thisRow].Par2 = verifyNumber(sender.value, configData[workCfg].TurnoutSettings.OutPins[thisRow].Par2); 
			break;
	}
}

function writeTurnoutToDCC(turnoutNr)
{
	var turnoutData = configData[workCfg].TurnoutSettings.OutPins[turnoutNr];
	var cmdStr;
	switch (turnoutData.TOType)
	{
		case 0: cmdStr = "Z " + turnoutData.Id + " " + turnoutData.PinNr + " " + turnoutData.Logic; break;
		case 1: cmdStr = "T " + turnoutData.Id + " DCC " + turnoutData.LNAddr; break;
		case 2: cmdStr = "T " + turnoutData.Id + " SERVO " + turnoutData.PinNr + " " + turnoutData.Par1 + " " + turnoutData.Par2 + " " + turnoutData.Logic; break;
		case 3: cmdStr = "T " + turnoutData.Id + " VPIN " + turnoutData.PinNr; break;
		default: return;
	}
	cmdStr = "{\"Cmd\":\"SetDCCPP\", \"SubCmd\":\"SendCmd\",\"OpCode\": \"" + cmdStr + "\"}";
	console.log(cmdStr);
	ws.send(cmdStr);
}

function writeSensorToDCC(sensorNr)
{
	var sensorData = configData[workCfg].InputSettings.InpPins[sensorNr];
	var cmdStr = "S " + sensorData.Id + " " + sensorData.PinNr + " " + sensorData.Logic;
	cmdStr = "{\"Cmd\":\"SetDCCPP\", \"SubCmd\":\"SendCmd\",\"OpCode\": \"" + cmdStr + "\"}";
	console.log(cmdStr);
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

	createDataTableLines(thisTable, [tfPos,tfManipulatorBox, tfNumeric, tfText, tfNumeric, tfTurnoutTypeSel, tfEnablePosNegSel, tfStartupSel, tfNumeric, tfNumeric, tfNumeric], thisData.length, "setTurnoutConfig(this)");	
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
		e.childNodes[0].value = (thisData[i].Logic & 0x01);
		var e = document.getElementById(thisTable.id + "_" + i.toString() + "_" + "7");
		e.childNodes[0].value = ((thisData[i].Logic & 0x06) >> 1);
		var e = document.getElementById(thisTable.id + "_" + i.toString() + "_" + "8");
		e.childNodes[0].value = thisData[i].PinNr;
		var e = document.getElementById(thisTable.id + "_" + i.toString() + "_" + "9");
		e.childNodes[0].value = thisData[i].Par1;
		var e = document.getElementById(thisTable.id + "_" + i.toString() + "_" + "10");
		e.childNodes[0].value = thisData[i].Par2;
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
	
//	console.log(sensorPos);
	if (sensorPos >= 0)
	{
		configData[workCfg].InputSettings.InpPins[sensorPos].confirmed = true;
		loadSensorTable(sensorTable, configData[workCfg].InputSettings.InpPins);
	}
}

function confirmTurnoutPin(jsonData)
{
//	console.log(jsonData);
	var turnoutId = jsonData.Data.Msg[1].d;
	var pinNr = jsonData.Data.Msg[2].d;
	var pinLogic = jsonData.Data.Msg[3].d;
	var turnoutPos = configData[workCfg].TurnoutSettings.OutPins.findIndex(element => element.Id == turnoutId);
	if (turnoutPos >= 0)
	{
		configData[workCfg].TurnoutSettings.OutPins[turnoutPos].confirmed = true;
		loadTurnoutTable(turnoutTable, configData[workCfg].TurnoutSettings.OutPins);
	}
}

function loadNodeDataFields(jsonData)
{
	console.log(jsonData);
		
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
	console.log(jsonData);
	configData[workCfg] = upgradeJSONVersionRH(jsonData);
	loadProgOptions(jsonData);
	writeCBInputField("sensoreeprom", jsonData.DevSettings.ConfigSensorIO);
	writeCBInputField("turnouteeprom", jsonData.DevSettings.ConfigTurnoutIO);

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
	setPanelVisibility();
//	ws.send("{\"Cmd\":\"SetDCCPP\", \"SubCmd\":\"GetConfig\",\"Filter\": 255}");
	ws.send("{\"Cmd\":\"SetDCCPP\", \"SubCmd\":\"SendCmd\",\"OpCode\": \"T\"}");
	ws.send("{\"Cmd\":\"SetDCCPP\", \"SubCmd\":\"SendCmd\",\"OpCode\": \"S\"}");
	ws.send("{\"Cmd\":\"SetDCCPP\", \"SubCmd\":\"SendCmd\",\"OpCode\": \"Z\"}");
	ws.send("{\"Cmd\":\"SetDCCPP\", \"SubCmd\":\"SendCmd\",\"OpCode\": \"#\"}");
	
}

function processDCCPPInput(jsonData)
{
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
