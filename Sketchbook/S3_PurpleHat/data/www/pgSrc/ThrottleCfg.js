var mainScrollBox;
var buttonTable;
//var listViewer;
var mqttTitle;
var mqttBox;

jsonFileVersion = "1.1.3";

var maxButtons = 0;

var btnStatus = ["off", "digital", "analog"];

function saveConfigFileSettings()
{
	//step 1: save greenhat.cfg
	saveJSONConfig(scriptList.Pages[currentPage].ID, scriptList.Pages[currentPage].FileName, configData[workCfg], null);
}

function upgradeJSONVersion(jsonData)
{
	var thisVersion = jsonData.Version;
	console.log(thisVersion);
	if (thisVersion == jsonFileVersion)
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
	if (jsonData.Version == "1.1.1")
	{
		console.log("upgrading to 1.1.2");
		jsonData.MQTT = {"Subscribe": [{"Topic": "BTNASK","InclAddr": false}],"Publish": [{"Topic": "BTNREPORT","InclAddr": false}, {"Topic": "BTNREPLY","InclAddr": false}]};
		jsonData.Version = "1.1.2";
	}
	if (jsonData.Version == "1.1.2")
	{
		console.log("upgrading to 1.1.3");
		jsonData.RefreshInterval = 30000;
		jsonData.Sensitivity = 2;
		jsonData.Version = "1.1.3";
	}
}

function setThreshold(sender)
{
	if (sender.id == "holdthreshold")
		configData[2].HoldThreshold = verifyNumber(sender.value, configData[2].HoldThreshold);
	if (sender.id == "dblclkthreshold")
		configData[2].DblClickThreshold = verifyNumber(sender.value, configData[2].DblClickThreshold);
	if (sender.id == "analogrefresh")
		configData[2].RefreshInterval = verifyNumber(sender.value, configData[2].RefreshInterval);
	if (sender.id == "analogsensitivity")
		configData[2].Sensitivity = verifyNumber(sender.value, configData[2].Sensitivity);
	if (sender.id == "baseaddress")
	{
		configData[2].BoardBaseAddr = verifyNumber(sender.value, configData[2].BoardBaseAddr);
		if (confirm("Set all Button Addresses?") == true)
			for (var i=0; i<configData[2].Buttons.length;i++)
			{
				configData[2].Buttons[i].ButtonAddr = configData[2].BoardBaseAddr + i;
				var e = document.getElementById(buttonTable.id + "_" + i.toString() + "_" + "3");
				e.childNodes[0].value = configData[2].Buttons[i].ButtonAddr;
			}
	}
	if (sender.id == "mqbtnask")
		configData[2].MQTT.Subscribe[0].Topic = sender.value;
	if (sender.id == "inclbtnask")
		configData[2].MQTT.Subscribe[0].InclAddr = sender.checked;
	if (sender.id == "mqbtnreport")
		configData[2].MQTT.Publish[0].Topic = sender.value;
	if (sender.id == "inclbtnreport")
		configData[2].MQTT.Publish[0].InclAddr = sender.checked;
	if (sender.id == "mqbtnreply")
		configData[2].MQTT.Publish[1].Topic = sender.value;
	if (sender.id == "inclbtnreply")
		configData[2].MQTT.Publish[1].InclAddr = sender.checked;
}

function downloadSettings(sender)
{
	downloadConfig(0x0100); //send just this
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
			configData[2].Buttons[row].ButtonType = btnStatus[sender.selectedIndex];
			break;
		case 3:
//			console.log("3: ", sender.value);
			configData[2].Buttons[row].ButtonAddr = verifyNumber(sender.value, configData[2].Buttons[row].ButtonAddr);
			break;
		case 5:
			var index = parseInt(sender.getAttribute("index"));
			if (sender.checked)
				configData[2].Buttons[row].EventMask |= (0x01<<index);
			else
				configData[2].Buttons[row].EventMask &= ~(0x01<<index);
			if ((configData[2].Buttons[row].EventMask & 0x1F) == 0)
				configData[2].Buttons[row].EventMask = 0x20; //set all zero bit
			else
				configData[2].Buttons[row].EventMask &= 0x1F; //clear all zero bit
			break;
	}
//	console.log(configData[2].Buttons[row]);
}

function tfThrottleEvtMask(y, x, id, evtHandler)
{
	function createCB(appenddiv, index, disptext)
	{
		var thisId = "eventmask_" + y.toString() + "_" + x.toString() + "_" + index.toString();
		var cchBox = tfCheckBox(y, x, thisId, evtHandler);
		cchBox.childNodes[0].setAttribute("index", index);
		cchBox.childNodes[1].innerHTML = disptext;
		appenddiv.append(cchBox);
		return cchBox;
	}
		
	var evtMaskDiv = document.createElement("div");
	evtMaskDiv.setAttribute("class", "eventmask");
	tfSetCoordinate(evtMaskDiv, y, x, 0, id);
	createCB(evtMaskDiv, 0, "Bi-stable Switch");
	createCB(evtMaskDiv, 1, "Push Button");
	return evtMaskDiv;
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
	
	var pinNr = [0,26,36];

	createDataTableLines(thisTable, [tfPos,tfText,tfText,tfNumeric, tfText, tfBtnEvtMask], thisData.length, "setButtonData(this)");	
	for (var i=0; i<thisData.length;i++)
	{
		function selByName(prmVal)
		{
			return (prmVal == thisData[i].ButtonType);
		}

		var pinPort = pinNr[i];
		var e = document.getElementById(thisTable.id + "_" + i.toString() + "_" + "1");
		writeTextField(e.id, "Pin " + pinPort.toString()); 
		var e = document.getElementById(thisTable.id + "_" + i.toString() + "_" + "2");
		writeTextField(e.id, thisData[i].ButtonType);
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

function constructPageContent(contentTab)
{
	var tempObj;
	mainScrollBox = createEmptyDiv(contentTab, "div", "pagetopicboxscroll-y", "btnconfigdiv");
		createPageTitle(mainScrollBox, "div", "tile-1", "", "h1", "Smart Throttle Setup");
		createPageTitle(mainScrollBox, "div", "tile-1", "", "h2", "Basic Settings");
		tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
			createTextInput(tempObj, "tile-1_4", "Analog Refresh (ms)", "n/a", "analogrefresh", "setThreshold(this)");
			createTextInput(tempObj, "tile-1_4", "Analog Sensitivity (%)", "n/a", "analogsensitivity", "setThreshold(this)");
//		tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
//			createTextInput(tempObj, "tile-1_4", "Hold Threshold (ms)", "n/a", "holdthreshold", "setThreshold(this)");
//			createTextInput(tempObj, "tile-1_4", "Dbl Clk Threshold (ms)", "n/a", "dblclkthreshold", "setThreshold(this)");
//		tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "wificb");
//			createTextInput(tempObj, "tile-1_4", "Board Base Address", "n/a", "baseaddress", "setThreshold(this)");

		mqttTitle = createPageTitle(mainScrollBox, "div", "tile-1", "", "h2", "MQTT Settings");
		mqttBox = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
			tempObj = createEmptyDiv(mqttBox, "div", "tile-1", "");
			createTextInput(tempObj, "tile-1_4", "(S) Btn Query. Topic:", "n/a", "mqbtnask", "setThreshold(this)");
			createCheckbox(tempObj, "tile-1_4", "Include Btn #", "inclbtnask", "setThreshold(this)");
			tempObj = createEmptyDiv(mqttBox, "div", "tile-1", "");
			createTextInput(tempObj, "tile-1_4", "(P) Btn Status Topic:", "n/a", "mqbtnreport", "setThreshold(this)");
			createCheckbox(tempObj, "tile-1_4", "Include Btn #", "inclbtnreport", "setThreshold(this)");
			tempObj = createEmptyDiv(mqttBox, "div", "tile-1", "");
			createTextInput(tempObj, "tile-1_4", "(P) Btn Reply Topic:", "n/a", "mqbtnreply", "setThreshold(this)");
			createCheckbox(tempObj, "tile-1_4", "Include Btn #", "inclbtnreply", "setThreshold(this)");

		createPageTitle(mainScrollBox, "div", "tile-1", "", "h2", "Throttle Input Configuration");
//		tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "wificb");
//			createDispText(tempObj, "tile-1_4", "# of Buttons", "n/a", "maxbuttons");

		buttonTable = createDataTable(mainScrollBox, "tile-1", ["Pos","Stick Pin#", "Input Type", "LN Button Address", "Input Status", "Message Type"], "btnconfig", "");

		tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
			createButton(tempObj, "", "Save & Restart", "btnSave", "saveSettings(this)");
			createButton(tempObj, "", "Cancel", "btnCancel", "cancelSettings(this)");
		tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
			createButton(tempObj, "", "Save to File", "btnDownload", "downloadSettings(this)");
}

function loadNodeDataFields(jsonData)
{
	maxButtons = 3; //3 input data lines of hat interface
//	writeTextField("maxbuttons", maxButtons);
	var interfaceType = jsonData.InterfaceTypeList[jsonData.InterfaceIndex].Type;
	setVisibility(interfaceType == 3, mqttTitle);
	setVisibility(interfaceType == 3, mqttBox);
}

function loadDataFields(jsonData)
{
	upgradeJSONVersion(jsonData);
//	writeInputField("holdthreshold", jsonData.HoldThreshold);
//	writeInputField("dblclkthreshold", jsonData.DblClickThreshold);
//	writeInputField("baseaddress", jsonData.BoardBaseAddr);
	writeInputField("analogrefresh", jsonData.RefreshInterval);
	writeInputField("analogsensitivity", jsonData.Sensitivity);
	writeInputField("mqbtnask", jsonData.MQTT.Subscribe[0].Topic);
	writeCBInputField("inclbtnask", jsonData.MQTT.Subscribe[0].InclAddr);
	writeInputField("mqbtnreport", jsonData.MQTT.Publish[0].Topic);
	writeCBInputField("inclbtnreport", jsonData.MQTT.Publish[0].InclAddr);
	writeInputField("mqbtnreply", jsonData.MQTT.Publish[1].Topic);
	writeCBInputField("inclbtnreply", jsonData.MQTT.Publish[1].InclAddr);
	loadTableData(buttonTable, jsonData.Buttons);
}

function processLocoNetInput(jsonData)
{
//	console.log(jsonData);
	setButtonStatus(jsonData[0],jsonData[1],jsonData[2],jsonData[3]); //type, portnr, addr, event
}
