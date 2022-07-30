var mainScrollBox;
var buttonTable;
//var listViewer;
var mqttTitle;
var mqttBox;
var setupText;
var setupBox;
//var hatType; //used to control display of input fields
var interfaceType;

var maxButtons = 0;

var btnStatus = ["off", "digital", "analog", "sensor", "swireport"];

var templateDlg = null;
var level0Div;
var level1Div;
var level2Div;
var level3Div;
var level4Div;


function saveConfigFileSettings()
{
	//step 1: save greenhat.cfg
	saveJSONConfig(scriptList.Pages[currentPage].ID, scriptList.Pages[currentPage].FileName, configData[workCfg], null);
}

function runTemplate(sender)
{
	function incrementAddr(thisAddr, incrBy)
	{
		var resArray = [];
		if (Array.isArray(thisAddr))
		{
			for (var i = 0; i < thisAddr.length; i++)
				resArray.push(parseInt(thisAddr[i]) + incrBy);
		}
		else
			if (thisAddr != undefined)
				resArray = thisAddr + incrBy;
		return resArray;
	}
	
	var templId = templateDlg.getAttribute('templchid') - 1;
	var templBtn = JSON.parse(JSON.stringify(configData[workCfg].Buttons[templId]));
	var startIndex = Math.min(32, Math.max(1, document.getElementById("startchannel").value));
	var endIndex = Math.max(1, Math.min(32, document.getElementById("endchannel").value));
	var incrBtn =  parseInt(document.getElementById("mainaddrincr").value);
	for (var i = startIndex; i <= endIndex; i++)
	{
		configData[workCfg].Buttons[i-1].ButtonType = templBtn.ButtonType;
		configData[workCfg].Buttons[i-1].EventMask = templBtn.EventMask;
		configData[workCfg].Buttons[i-1].ButtonAddr = templBtn.ButtonAddr + ((i-startIndex) * incrBtn);
	}
	loadTableData(buttonTable, configData[workCfg].Buttons);
	templateDlg.style.display = "none";
}

function cancelTemplate(sender)
{
	templateDlg.style.display = "none";
}

function startTemplateDialog(parentObj, templateChannel)
{
	var mainDlg = document.createElement("div");
	mainDlg.setAttribute('class', "modal");
	mainDlg.setAttribute('templchid', templateChannel);
	
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
				dlgHeader.innerHTML = "Configure Address Assignment, then click Run Template"; 
				dlgSubDiv.append(dlgHeader);
	
			dlgSubDiv = document.createElement("div");
			dlgSubDiv.setAttribute('class', "modal-body");
			dlgDiv.append(dlgSubDiv);
				dlgTextDispArea = document.createElement("div"); //global var
				dlgTextDispArea.setAttribute('readonly', "true");
				dlgTextDispArea.setAttribute('style', "height: 250px; width:100%"); 

				level0Div = document.createElement("div"); //global var
				level0Div.setAttribute('style', "height: 50px; width:100%"); 
				level0Div.setAttribute('id', "temptitle"); 
				level0Div.innerHTML = "Set input characterstics based on settings of input #" + templateChannel.toString();
				dlgTextDispArea.append(level0Div);

				level1Div = document.createElement("div"); //global var
				level1Div.setAttribute('style', "height: 50px; width:100%"); 
				dlgTextDispArea.append(level1Div);
				var tempField = createTextInput(level1Div, "tile-1_2", "Run from Input:", templateChannel.toString(), "startchannel", "");
				tempField = createTextInput(level1Div, "tile-1_2", "to Input:", Math.min(16, templateChannel+5).toString(), "endchannel", "");

				level2Div = document.createElement("div"); //global var
				level2Div.setAttribute('style', "height: 50px; width:100%"); 
				dlgTextDispArea.append(level2Div);
				createTextInput(level2Div, "tile-1_2", "Increment Input Address by:", "1", "mainaddrincr", "");

			dlgSubDiv.append(dlgTextDispArea);
			var footerDiv = document.createElement("div");
			footerDiv.setAttribute('class', "modal-footer");
			dlgDiv.append(footerDiv);
			var dlgHeader = document.createElement("div");
				dlgHeader.setAttribute('style', "height: 70px; width:100%"); 
				dlgHeader.innerHTML = "&nbsp;";
				createButton(dlgHeader, "", "Run Template", "btnRun", "runTemplate(this)");
				createButton(dlgHeader, "", "Cancel", "btnCancel", "cancelTemplate(this)");
			footerDiv.append(dlgHeader);
	parentObj.append(mainDlg);
	var span = document.getElementsByClassName("close"); //get array of close elements, should only be 1
	span[0].onclick = function() {templateDlg.style.display = "none";}
	return mainDlg;
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
	downloadConfig(0x0010); //send just this
}

function updateOptions(thisRow)
{
	var eventMask = 0x01;
	var btnTypeIndex = btnStatus.indexOf(configData[2].Buttons[thisRow].ButtonType);
	for (var j = 0; j < 5; j++)
	{
		var t = document.getElementById("eventmask_"  + thisRow.toString() + "_" + "5_" + j.toString());
		var ttxt = document.getElementById("eventmask_"  + thisRow.toString() + "_" + "5_" + j.toString() + "_cbtxt");
		switch (j)
		{
			case 0: 
				setVisibility([1,3,4].indexOf(btnTypeIndex) >= 0, t.parentElement);
				switch (btnTypeIndex)
				{
					case 1: ttxt.innerHTML = "Btn Down"; break;
					case 3: ttxt.innerHTML = "Occupied"; break;
					case 4: ttxt.innerHTML = "Thrown"; break;
				}
				break;
			case 1: 
				setVisibility([1,3,4].indexOf(btnTypeIndex) >= 0, t.parentElement);
				switch (btnTypeIndex)
				{
					case 1: ttxt.innerHTML = "Btn Up"; break;
					case 3: ttxt.innerHTML = "Free"; break;
					case 4: ttxt.innerHTML = "Closed"; break;
				}
				break;
			case 2: 
				setVisibility([1,3,4].indexOf(btnTypeIndex) >= 0, t.parentElement);
				switch (btnTypeIndex)
				{
					case 1: ttxt.innerHTML = "Btn Click"; break;
					case 3: ttxt.innerHTML = "Inverse Logic"; break;
					case 4: ttxt.innerHTML = "Inverse Logic"; break;
				}
				break;
			case 3: //checkbox 3 hold
				setVisibility(([1].indexOf(btnTypeIndex) >= 0), t.parentElement);
				break;
			case 4: //checkbox 4 dbl click
				setVisibility(([1].indexOf(btnTypeIndex) >= 0 && (thisHatID != 6)), t.parentElement);
				break;
		}
		t.checked = ((configData[2].Buttons[thisRow].EventMask & (eventMask<<j)) > 0);
	}
}

function setButtonData(sender)
{
	var col = parseInt(sender.getAttribute("col"));
	var row = parseInt(sender.getAttribute("row"));
//	console.log(row, col);
	switch (col)
	{
		case 1: //run template
			if (templateDlg ==  null)
				templateDlg = startTemplateDialog(document.getElementById("TabHolder"), row+1);
			else
				level0Div.innerHTML = "Set channel characterstics based on settings of channel #" + (row+1).toString();
			document.getElementById("temptitle").innerHTML = "Set channel characterstics based on settings of channel #" + (row+1).toString();
			document.getElementById("startchannel").value = (row+1).toString();
			document.getElementById("endchannel").value = Math.min(16, row+6).toString();
			templateDlg.setAttribute('templchid', row+1);
			templateDlg.style.display = "block";
			break;
		case 2:
//			console.log("2: ", btnStatus[sender.selectedIndex]);
			configData[2].Buttons[row].ButtonType = btnStatus[sender.selectedIndex];
			if ([3,4].indexOf(sender.selectedIndex) >= 0)
				configData[2].Buttons[row].EventMask = 0x03;
			updateOptions(row);
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
	createDataTableLines(thisTable, [tfPos,tfLinkText,tfBtnEvtSel,tfNumeric, tfText, tfBtnEvtMask], thisData.length, "setButtonData(this)");	
	for (var i=0; i<thisData.length;i++)
	{
		var pinNr = Math.trunc(i/8) + 1; //nodeConfigData.BtnModConfig.DataPins[Math.trunc(i/16)];
		var pinPort = (i % 8) + 1;
		var e = document.getElementById(thisTable.id + "_inp_" + i.toString() + "_" + "1");
		writeTextField(e.id, "Pin " + pinPort.toString() + " Port " + String.fromCharCode(64 + pinNr)); //A = 65
		e.setAttribute("href", "javascript:void function nop(){}();");
		e.setAttribute("onclick", "setButtonData(this)");

		var e = document.getElementById(thisTable.id + "_" + i.toString() + "_" + "2");
		e.childNodes[0].selectedIndex = btnStatus.indexOf(thisData[i].ButtonType);
		var e = document.getElementById(thisTable.id + "_" + i.toString() + "_" + "3");
		e.childNodes[0].value = thisData[i].ButtonAddr;
		var e = document.getElementById(thisTable.id + "_" + i.toString() + "_" + "4");
		writeTextField(e.id, "");
		var e = document.getElementById(thisTable.id + "_" + i.toString() + "_" + "5");
		updateOptions(i);
	}
}

function setButtonStatus(portType, portNr, portAddr, portValue)
{
	var e = document.getElementById(buttonTable.id + "_" + portNr.toString() + "_" + "4");
	if (e)
		switch (portType)
		{
			case 2: //digitalAct
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
			case 4: //
				if (configData[workCfg].Buttons[portNr].EventMask & 0x04)
					portValue ^= 0x01;
				switch (portValue)
				{
					case 0: //down
						writeTextField(e.id, "Occupied");
						e.style.backgroundColor = "hsl(0, 100%, 50%)";
						break;
					case 1: //up
						writeTextField(e.id, "Free");
						e.style.backgroundColor = "hsl(116, 100%, 50%)";
						break;
				}
				break;
			case 5: //Switch Report
				if (configData[workCfg].Buttons[portNr].EventMask & 0x04)
					portValue ^= 0x01;
				switch (portValue)
				{
					case 0: //down
						writeTextField(e.id, "Thrown");
						e.style.backgroundColor = "hsl(39, 100%, 50%)";
						break;
					case 1: //up
						writeTextField(e.id, "Closed");
						e.style.backgroundColor = "hsl(116, 100%, 50%)";
						break;
				}
				break;
			case 9: //analog
				writeTextField(e.id, "Analog " + portValue.toString());
				var hue = Math.round(240 * (1 - (portValue/4096)));
//				console.log(hue);
				e.style.backgroundColor = "hsl(" + hue.toString() + ", 100%, 50%)";
				break;
		}
}

function constructPageContent(contentTab)
{
	var tempObj;
	mainScrollBox = createEmptyDiv(contentTab, "div", "pagetopicboxscroll-y", "btnconfigdiv");
		createPageTitle(mainScrollBox, "div", "tile-1", "", "h1", "Hardware Button Setup");
		createPageTitle(mainScrollBox, "div", "tile-1", "", "h2", "Basic Settings");
		var setupDiv = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
		tempObj = createEmptyDiv(setupDiv, "div", "tile-1", "");
			createTextInput(tempObj, "tile-1_4", "Hold Threshold (ms)", "n/a", "holdthreshold", "setThreshold(this)");
			setupText = createTextInput(tempObj, "tile-1_4", "Dbl Clk Threshold (ms)", "n/a", "dblclkthreshold", "setThreshold(this)");
		setupBox = createEmptyDiv(setupDiv, "div", "tile-1", "");
			createTextInput(setupBox, "tile-1_4", "Analog Refresh (ms)", "n/a", "analogrefresh", "setThreshold(this)");
			createTextInput(setupBox, "tile-1_4", "Analog Sensitivity (%)", "n/a", "analogsensitivity", "setThreshold(this)");
		tempObj = createEmptyDiv(setupDiv, "div", "tile-1", "wificb");
			createTextInput(tempObj, "tile-1_4", "Board Base Address", "n/a", "baseaddress", "setThreshold(this)");
//		setVisibility(false, setupText);
		setVisibility(false, setupBox);

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
		setVisibility(false, mqttTitle);
		setVisibility(false, mqttBox);

		createPageTitle(mainScrollBox, "div", "tile-1", "", "h2", "Button Configuration");
		tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "wificb");
			createDispText(tempObj, "tile-1_4", "# of Buttons", "n/a", "maxbuttons");

		buttonTable = createDataTable(mainScrollBox, "tile-1", ["Pos","Port #", "Input Type", "Input Address", "Input Status", "Send Messages"], "btnconfig", "");

		tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
			createButton(tempObj, "", "Save & Restart", "btnSave", "saveSettings(this)");
			createButton(tempObj, "", "Cancel", "btnCancel", "cancelSettings(this)");
		tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
			createButton(tempObj, "", "Save to File", "btnDownload", "downloadSettings(this)");
}

function loadNodeDataFields(jsonData)
{
	maxButtons = 32; //16*jsonData.BtnModConfig.DataPins.length;
	writeTextField("maxbuttons", maxButtons);
	interfaceType = jsonData.InterfaceTypeList[jsonData.InterfaceIndex].Type;
//	hatType = thisHatID; //jsonData.HatTypeList[jsonData.HatIndex].HatId;
	setVisibility(interfaceType == 3, mqttTitle);
	setVisibility(interfaceType == 3, mqttBox);
//	setVisibility(thisHatID != 6, setupText);
	setVisibility(thisHatID != 6, setupBox);
}

function loadDataFields(jsonData)
{
	setVisibility(true, document.getElementById("pgHWBtnCfg"), false);
	jsonData = upgradeJSONVersionBtn(jsonData);
	configData[2] = JSON.parse(JSON.stringify(jsonData));
	writeInputField("holdthreshold", jsonData.HoldThreshold);
	writeInputField("dblclkthreshold", jsonData.DblClickThreshold);
	writeInputField("baseaddress", jsonData.BoardBaseAddr);
	writeInputField("analogrefresh", jsonData.RefreshInterval);
	writeInputField("analogsensitivity", jsonData.Sensitivity);
	writeInputField("mqbtnask", jsonData.MQTT.Subscribe[0].Topic);
	writeCBInputField("inclbtnask", jsonData.MQTT.Subscribe[0].InclAddr);
	writeInputField("mqbtnreport", jsonData.MQTT.Publish[0].Topic);
	writeCBInputField("inclbtnreport", jsonData.MQTT.Publish[0].InclAddr);
	writeInputField("mqbtnreply", jsonData.MQTT.Publish[1].Topic);
	writeCBInputField("inclbtnreply", jsonData.MQTT.Publish[1].InclAddr);
	loadTableData(buttonTable, jsonData.Buttons);
	for (var i = 0; i < 32; i++)
	{
		var idStr = "btnconfig_inp_" + i.toString() + "_2";
		var dropDown = document.getElementById(idStr);
		if (dropDown != undefined)
			dropDown.options[2].disabled = thisHatID == 6;
	}
}

function processLocoNetInput(jsonData)
{
//	console.log(jsonData);
	setButtonStatus(jsonData[0],jsonData[1],jsonData[2],jsonData[3]); //type, portnr, addr, event
}
