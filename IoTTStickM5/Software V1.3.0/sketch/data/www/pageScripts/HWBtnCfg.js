var mainScrollBox;
var buttonTable;
//var listViewer;

var maxButtons = 0;

var btnStatus = ["off", "digital", "analog"];

function upgradeJSONVersion(jsonData)
{
	var thisVersion = jsonData.Version;
	console.log(thisVersion);
	if (thisVersion == "1.1.1")
		return;
	if ((isNaN(thisVersion)) || (thisVersion = undefined))
	{
		//upgrade from noversion to 1.1.1
		console.log("upgrade from noversion to 1.1.1");
		for (var i=0; i<jsonData.Buttons.length;i++)
		{
			if (jsonData.Buttons[i].EventMask == undefined)
				jsonData.Buttons[i].EventMask = 0x1F;
		}
		jsonData.Version = "1.1.1";
	}
}

function setThreshold(sender)
{
	if (sender.id == "holdthreshold")
		configWorkData.HoldThreshold = verifyNumber(sender.value, configWorkData.HoldThreshold);
	if (sender.id == "dblclkthreshold")
		configWorkData.DblClickThreshold = verifyNumber(sender.value, configWorkData.DblClickThreshold);
	if (sender.id == "baseaddress")
	{
		configWorkData.BoardBaseAddr = verifyNumber(sender.value, configWorkData.BoardBaseAddr);
		if (confirm("Set all Button Addresses?") == true)
			for (var i=0; i<configWorkData.Buttons.length;i++)
			{
				configWorkData.Buttons[i].ButtonAddr = configWorkData.BoardBaseAddr + i;
				var e = document.getElementById(buttonTable.id + "_" + i.toString() + "_" + "3");
				e.childNodes[0].value = configWorkData.Buttons[i].ButtonAddr;
			}
	}
}

function downloadSettings(sender)
{
	downloadConfig(0x0010); //send just this
}

function setButtonData(sender)
{
	var col = parseInt(sender.getAttribute("col"));
	var row = parseInt(sender.getAttribute("row"));
//	console.log(row, col);
	switch (col)
	{
		case 2:
//			console.log("2: ", btnStatus[sender.selectedIndex]);
			configWorkData.Buttons[row].ButtonType = btnStatus[sender.selectedIndex];
			break;
		case 3:
//			console.log("3: ", sender.value);
			configWorkData.Buttons[row].ButtonAddr = verifyNumber(sender.value, configWorkData.Buttons[row].ButtonAddr);
			break;
		case 5:
			var index = parseInt(sender.getAttribute("index"));
			if (sender.checked)
				configWorkData.Buttons[row].EventMask |= (0x01<<index);
			else
				configWorkData.Buttons[row].EventMask &= ~(0x01<<index);
			if ((configWorkData.Buttons[row].EventMask & 0x1F) == 0)
				configWorkData.Buttons[row].EventMask = 0x20; //set all zero bit
			else
				configWorkData.Buttons[row].EventMask &= 0x1F; //clear all zero bit
			break;
	}
//	console.log(configWorkData.Buttons[row]);
}

function loadTableData(thisTable, thisData)
{
	while (thisData.length > maxButtons)
		thisData.pop();
	while (thisData.length < maxButtons)
	{
		var newEntry = {"PortNr":0,"ButtonType": "off","ButtonAddr": 0, "EventMask":31}
		newEntry.PortNr = thisData.length;
		thisData.push(newEntry);
	}
//	console.log(thisData);
	var th = document.getElementById(buttonTable.id + "_head");
	var tb = document.getElementById(buttonTable.id + "_body");
	var numCols = th.childNodes[0].children.length;

	createDataTableLines(thisTable, [tfPos,tfText,tfBtnEvtSel,tfNumeric, tfText, tfBtnEvtMask], thisData.length, "setButtonData(this)");	
	for (var i=0; i<thisData.length;i++)
	{
		function selByName(prmVal)
		{
			return (prmVal == thisData[i].ButtonType);
		}

		var pinNr = Math.trunc(i/8) + 1; //nodeConfigData.BtnModConfig.DataPins[Math.trunc(i/16)];
		var pinPort = (i % 8) + 1;
		var e = document.getElementById(thisTable.id + "_" + i.toString() + "_" + "1");
		writeTextField(e.id, "Pin " + pinPort.toString() + " Port " + String.fromCharCode(64 + pinNr)); //A = 65
//		writeTextField(e.id, thisData[i].PortNr.toString() + " - Pin " + pinPort.toString() + " Group " + pinNr.toString());
		var e = document.getElementById(thisTable.id + "_" + i.toString() + "_" + "2");
		e.childNodes[0].selectedIndex = btnStatus.findIndex(selByName);
		var e = document.getElementById(thisTable.id + "_" + i.toString() + "_" + "3");
		e.childNodes[0].value = thisData[i].ButtonAddr;
		var e = document.getElementById(thisTable.id + "_" + i.toString() + "_" + "4");
		writeTextField(e.id, "");
		var e = document.getElementById(thisTable.id + "_" + i.toString() + "_" + "5");
		var eventMask = 0x01;
		for (var j = 0; j < 5; j++)
		{
			var t = document.getElementById("eventmask_"  + i.toString() + "_" + "5_" + j.toString());
			setVisibility(btnStatus.findIndex(selByName) != 2, t.parentElement);
			t.checked = ((thisData[i].EventMask & (eventMask<<j)) > 0);
		}
	}
}

function setButtonStatus(portType, portNr, portAddr, portValue)
{
	var e = document.getElementById(buttonTable.id + "_" + portNr.toString() + "_" + "4");
	switch (portType)
	{
		case 0: //button
			switch (portValue)
			{
				case 0: //down
					writeTextField(e.id, "Btn Down");
					e.style.backgroundColor = "hsl(39, 50%, 50%)";
					break;
				case 1: //up
					writeTextField(e.id, "Btn Up");
					e.style.backgroundColor = "hsl(116, 100%, 22%)";
					break;
				case 2: //click
					writeTextField(e.id, "Btn Click");
					e.style.backgroundColor = "hsl(116, 100%, 50%)";
					break;
				case 3: //hold
					writeTextField(e.id, "Btn Hold");
					e.style.backgroundColor = "hsl(0, 50%, 50%)";
					break;
				case 4: //dblclick
					writeTextField(e.id, "Btn Dbl Click");
					e.style.backgroundColor = "hsl(240, 50%, 50%)";
					break;
				default: //no status
					writeTextField(e.id, "Unkown Status");
					e.style.backgroundColor = "hsl(0, 0%, 50%)";
					break;
			}
			break;
		case 1: //analog
			writeTextField(e.id, "Analog " + portValue.toString());
			var hue = Math.round(240 * (1 - (portValue/4096)));
//			console.log(hue);
			e.style.backgroundColor = "hsl(" + hue.toString() + ", 100%, 50%)";
			break;
	}
}

function constructFooterContent(footerTab)
{
	var tempObj;
	tempObj = createEmptyDiv(footerTab, "div", "tile-1_4", "footerstatsdiv1");
		createDispText(tempObj, "", "Date / Time", "n/a", "sysdatetime");
		createDispText(tempObj, "", "System Uptime", "n/a", "uptime");
	tempObj = createEmptyDiv(footerTab, "div", "tile-1_4", "footerstatsdiv2");
		createDispText(tempObj, "", "IP Address", "n/a", "IPID");
		createDispText(tempObj, "", "Signal Strength", "n/a", "SigStrengthID");
	tempObj = createEmptyDiv(footerTab, "div", "tile-1_4", "footerstatsdiv3");
		createDispText(tempObj, "", "Firmware Version", "n/a", "firmware");
		createDispText(tempObj, "", "Available RAM/Flash", "n/a", "heapavail");
}

function constructPageContent(contentTab)
{
	var tempObj;
	mainScrollBox = createEmptyDiv(contentTab, "div", "pagetopicboxscroll-y", "btnconfigdiv");
		createPageTitle(mainScrollBox, "div", "tile-1", "", "h1", "Hardware Button Setup");
		createPageTitle(mainScrollBox, "div", "tile-1", "", "h2", "Basic Settings");
//		tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
//			createDispText(tempObj, "tile-1_4", "Address Pins", "n/a", "addrpins");
//			createDispText(tempObj, "tile-1_4", "Data Pins", "n/a", "datapins");
		tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
			createTextInput(tempObj, "tile-1_4", "Hold Threshold (ms)", "n/a", "holdthreshold", "setThreshold(this)");
			createTextInput(tempObj, "tile-1_4", "Dbl Clk Threshold (ms)", "n/a", "dblclkthreshold", "setThreshold(this)");
		tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "wificb");
			createTextInput(tempObj, "tile-1_4", "Board Base Address", "n/a", "baseaddress", "setThreshold(this)");

//		createPageTitle(mainScrollBox, "div", "tile-1", "BasicCfg_Title", "h2", "Local Board Button Events");
//		tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
//		listViewer = createListViewer(tempObj, "viewerbox", "eventlist");

		createPageTitle(mainScrollBox, "div", "tile-1", "", "h2", "Button Configuration");
		tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "wificb");
			createDispText(tempObj, "tile-1_4", "# of Buttons", "n/a", "maxbuttons");

		buttonTable = createDataTable(mainScrollBox, "tile-1", ["Pos","Port #", "HW Button Type", "LN Button Address", "Input Status", "Send Messages"], "btnconfig", "");

		tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
			createButton(tempObj, "", "Save & Restart", "btnSave", "saveSettings(this)");
			createButton(tempObj, "", "Cancel", "btnCancel", "cancelSettings(this)");
		tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
			createButton(tempObj, "", "Save to File", "btnDownload", "downloadSettings(this)");
}

function loadNodeDataFields(jsonData)
{
//	writeTextField("addrpins", jsonData.BtnModConfig.AddrPins);
//	writeTextField("datapins", jsonData.BtnModConfig.DataPins);
	maxButtons = 32; //16*jsonData.BtnModConfig.DataPins.length;
	writeTextField("maxbuttons", maxButtons);
}

function processStatsData(jsonData)
{
	writeTextField("sysdatetime", jsonData.systime);
	writeTextField("uptime", formatTime(Math.trunc(jsonData.uptime/1000)));
	writeTextField("IPID", jsonData.ipaddress);
	writeTextField("SigStrengthID", jsonData.sigstrength + " dBm");
	writeTextField("firmware", jsonData.version);
	writeTextField("heapavail", jsonData.freemem + " / " + jsonData.freedisk + " Bytes");
}

function loadDataFields(jsonData)
{
	upgradeJSONVersion(jsonData);
	writeInputField("holdthreshold", jsonData.HoldThreshold);
	writeInputField("dblclkthreshold", jsonData.DblClickThreshold);
	writeInputField("baseaddress", jsonData.BoardBaseAddr);
	loadTableData(buttonTable, jsonData.Buttons);
}

function processLocoNetInput(jsonData)
{
//	console.log(jsonData);
	setButtonStatus(jsonData[0],jsonData[1],jsonData[2],jsonData[3]); //type, portnr, addr, event
}
