var mainScrollBox;
var buttonTable;

var newButtonTemplate = {"ButtonNr": 0,	"CtrlCmd": [{"EventType":"onbtndown", "BtnCondAddr": 65535, "CmdList":[]},{"EventType":"onbtnup", "CmdList": []},{"EventType":"onbtnclick", "CmdList":[]},{"EventType":"onbtnhold", "BtnCondAddr": 65535, "CmdList":[]},{"EventType":"onbtndlclick", "CmdList":[]}]};
var newCmdTemplate = {"CtrlTarget": "switch", "CtrlAddr": 0, "CtrlType":"toggle", "CtrlValue":"on", "ExecDelay":250};

var cmdOptions = ["switch", "signal", "block", "power"];
var swiCmdOptions = ["thrown","closed","toggle"];
var swiPwrOptions = ["on", "off"];
var trackPwrOptions = ["on", "off", "idle", "toggle"];

jsonFileVersion = "1.3.0";

function upgradeJSONVersion(jsonData)
{
	var thisVersion = jsonData.Version;
	console.log(thisVersion);
	if (thisVersion == jsonFileVersion)
		return;
	if ((isNaN(thisVersion)) || (thisVersion = undefined))
	{
		//upgrade from noversion to 1.3.0
		console.log("upgrade from noversion to ", jsonFileVersion);
		for (var i=0; i<jsonData.ButtonHandler.length;i++)
		{
			for (var j=0; j<jsonData.ButtonHandler[i].CtrlCmd.length;j++)
			{
				if (jsonData.ButtonHandler[i].CtrlCmd[j].EventType == "onanalog")
					jsonData.ButtonHandler[i].CtrlCmd.splice(j, 1);
			}
			for (var j=0; j<jsonData.ButtonHandler[i].CtrlCmd.length;j++)
			{
				if ((jsonData.ButtonHandler[i].CtrlCmd[j].EventType == "onbtndown") || (jsonData.ButtonHandler[i].CtrlCmd[j].EventType == "onbtnhold"))
					if (jsonData.ButtonHandler[i].CtrlCmd[j].BtnCondAddr == undefined)
						jsonData.ButtonHandler[i].CtrlCmd[j].BtnCondAddr = 0xFFFF;
			}
		}
		jsonData.Version = jsonFileVersion;
	}
}

function addFileSeq(ofObj) //object specific function to include partial files
{
	for (var j=0; j<ofObj.ButtonHandler.length; j++)
	{
		configLoadData.ButtonHandler.push(JSON.parse(JSON.stringify(ofObj.ButtonHandler[j])));
		configWorkData.ButtonHandler.push(JSON.parse(JSON.stringify(ofObj.ButtonHandler[j])));
	}
}

function prepareFileSeq() //object specific function to create partial files
{
	function addEntry()
	{
		var newEntry = {"ButtonHandler":[]}
		transferData.FileList.push(newEntry);
		return newEntry;
	}
	
	var thisEntry = addEntry();
	thisEntry.Version = jsonFileVersion;
	var thisFileLength = 0;
	
	for (var j=0; j<configWorkData.ButtonHandler.length;j++)
	{
		var thisElementStr = JSON.stringify(configWorkData.ButtonHandler[j]);
		thisFileLength += thisElementStr.length;
		thisEntry.ButtonHandler.push(JSON.parse(thisElementStr));
		if ((thisFileLength > targetSize) && (j < (configWorkData.ButtonHandler.length - 1)))
		{
			thisEntry = addEntry();
			thisFileLength = 0;
		}
	}
}

function downloadSettings(sender)
{
	downloadConfig(0x0040); //send just this
}

function setButtonData(sender)
{
	var thisRow = parseInt(sender.getAttribute("row"));
	var thisCol = parseInt(sender.getAttribute("col"));
	var thisIndex = parseInt(sender.getAttribute("index"));
	switch (thisCol)
	{
		case -1: //empty table, create first entry
			configWorkData.ButtonHandler.push(JSON.parse(JSON.stringify(newButtonTemplate)));
			loadTableData(buttonTable, configWorkData.ButtonHandler);
			break;
		case 1: //CommandSelector
			var idStr = sender.id;
			var thisElement;
			if (thisIndex == 1)
			{
				if (event.ctrlKey) //duplicate entry
					configWorkData.ButtonHandler.splice(thisRow+1, 0, JSON.parse(JSON.stringify(configWorkData.ButtonHandler[thisRow])));
				else //create new entry
					configWorkData.ButtonHandler.splice(thisRow+1, 0, JSON.parse(JSON.stringify(newButtonTemplate)));
			}
			if (thisIndex == 2)
				configWorkData.ButtonHandler.splice(thisRow, 1);
			if ((thisIndex == 3) && (thisRow > 0))
			{
				thisElement = configWorkData.ButtonHandler.splice(thisRow, 1);
				configWorkData.ButtonHandler.splice(thisRow-1,0, thisElement[0]);
			}
			if ((thisIndex == 4) && (thisRow < configWorkData.ButtonHandler.length))
			{
				thisElement = configWorkData.ButtonHandler.splice(thisRow, 1);
				configWorkData.ButtonHandler.splice(thisRow+1,0, thisElement[0]);
			}
			if (thisIndex == 5)
				configWorkData.ButtonHandler[thisRow].ButtonNr = verifyNumber(sender.value, configWorkData.ButtonHandler[thisRow].ButtonNr);
			if (thisIndex == 6)
			{
//				console.log(sender.selectedIndex);
				configWorkData.ButtonHandler[thisRow].CurrDisp = sender.selectedIndex;
//				console.log(configWorkData.ButtonHandler[thisRow]);
				//load second address field
				var e = document.getElementById("address2box_" + thisRow.toString() + "_" + "1");
				if ((configWorkData.ButtonHandler[thisRow].CurrDisp == 0) || (configWorkData.ButtonHandler[thisRow].CurrDisp == 3))
				{
					setVisibility(true, e);
					if (!isNaN(configWorkData.ButtonHandler[thisRow].CtrlCmd[configWorkData.ButtonHandler[thisRow].CurrDisp].BtnCondAddr))
						if (configWorkData.ButtonHandler[thisRow].CtrlCmd[configWorkData.ButtonHandler[thisRow].CurrDisp].BtnCondAddr == 0xFFFF)
							e.value = "";
						else
							e.value = configWorkData.ButtonHandler[thisRow].CtrlCmd[configWorkData.ButtonHandler[thisRow].CurrDisp].BtnCondAddr;
					else
						e.value = "";
				}
				else
					setVisibility(false, e);
				
				//load tfCommandEditor for thisRow
				buildCmdLines(thisRow, configWorkData.ButtonHandler[thisRow]);
			}
			if (thisIndex == 7)
			{
				configWorkData.ButtonHandler[thisRow].CtrlCmd[configWorkData.ButtonHandler[thisRow].CurrDisp].BtnCondAddr = verifyNumber(sender.value, configWorkData.ButtonHandler[thisRow].CtrlCmd[configWorkData.ButtonHandler[thisRow].CurrDisp].BtnCondAddr);
				if (isNaN(configWorkData.ButtonHandler[thisRow].CtrlCmd[configWorkData.ButtonHandler[thisRow].CurrDisp].BtnCondAddr))
					configWorkData.ButtonHandler[thisRow].CtrlCmd[configWorkData.ButtonHandler[thisRow].CurrDisp].BtnCondAddr = 0xFFFF;
			}
			if (thisIndex < 5)
				loadTableData(buttonTable, configWorkData.ButtonHandler);
			break;
		default:
//			console.log(thisRow, thisCol, thisIndex);
			break;
	}
}

function setCommandData(sender)
{
	var thisRow = parseInt(sender.getAttribute("row"));
	var thisCol = parseInt(sender.getAttribute("col"));
	var thisIndex = parseInt(sender.getAttribute("index"));
	var thisCmd = configWorkData.ButtonHandler[thisRow].CurrDisp;
	var thisCmdLine = parseInt(sender.getAttribute("cmdline"));
	if (isNaN(thisCmdLine))
		thisCmdLine = parseInt(sender.parentElement.getAttribute("cmdline"));
//	console.log(thisRow, thisCol, thisIndex, thisCmd, thisCmdLine);
	switch (thisCol)
	{
		case -1: //empty table, create first entry
			configWorkData.ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList.push(JSON.parse(JSON.stringify(newCmdTemplate)));
//			console.log(configWorkData.ButtonHandler);
			buildCmdLines(thisRow, configWorkData.ButtonHandler[thisRow]);
			break;
		case 1: //Manipulator Commands
			switch (thisIndex)
			{
				case 1: //add CmdLine
					configWorkData.ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList.splice(thisCmdLine+1, 0, JSON.parse(JSON.stringify(newCmdTemplate)));
					break;
				case 2: //delete CmdLine
					configWorkData.ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList.splice(thisCmdLine, 1);
					break;
				case 3: //up
					if (thisCmdLine > 0)
					{
						thisElement = configWorkData.ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList.splice(thisCmdLine, 1);
						configWorkData.ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList.splice(thisCmdLine-1,0, thisElement[0]);
					}
					break;
				case 4: //down
					if (thisCmdLine < configWorkData.ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList.length)
					{
						thisElement = configWorkData.ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList.splice(thisCmdLine, 1);
						configWorkData.ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList.splice(thisCmdLine+1,0, thisElement[0]);
					}
					break;

			}
			buildCmdLines(thisRow, configWorkData.ButtonHandler[thisRow]);
			break;
		case 2: //set command target
			var currTarget = configWorkData.ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList[thisCmdLine].CtrlTarget;
			configWorkData.ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList[thisCmdLine].CtrlTarget = cmdOptions[sender.selectedIndex];
			if (currTarget != configWorkData.ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList[thisCmdLine].CtrlTarget)
			{
				configWorkData.ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList[thisCmdLine].CtrlAddr = 0;
				configWorkData.ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList[thisCmdLine].ExecDelay = 250;
				switch  (sender.selectedIndex)
				{
					case 0: // switch
						configWorkData.ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList[thisCmdLine].CtrlTarget = "switch";
						configWorkData.ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList[thisCmdLine].CtrlType = "toggle";
						configWorkData.ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList[thisCmdLine].CtrlValue = "on";
						break;
					case 1: //signal
						configWorkData.ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList[thisCmdLine].CtrlTarget = "signal";
						configWorkData.ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList[thisCmdLine].CtrlType = "aspect";
						configWorkData.ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList[thisCmdLine].CtrlValue = "0";
						break;
					case 2: //block
						configWorkData.ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList[thisCmdLine].CtrlTarget = "block";
						configWorkData.ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList[thisCmdLine].CtrlType = "block";
						configWorkData.ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList[thisCmdLine].CtrlValue = "on";
						break;
					case 3: //power
						configWorkData.ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList[thisCmdLine].CtrlTarget = "power";
						configWorkData.ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList[thisCmdLine].CtrlType = "idle";
						configWorkData.ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList[thisCmdLine].CtrlValue = "0";
						break;
				}
			}
			buildCmdLines(thisRow, configWorkData.ButtonHandler[thisRow]);
			break;
		case 3: //set command target address
			configWorkData.ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList[thisCmdLine].CtrlAddr = verifyNumber(sender.value, configWorkData.ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList[thisCmdLine].CtrlAddr);
			break;
		case 4: //set delay after command
			configWorkData.ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList[thisCmdLine].ExecDelay = verifyNumber(sender.value, configWorkData.ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList[thisCmdLine].ExecDelay);
			break;
		case 5: //set CtrlType
//			console.log(configWorkData.ButtonHandler[thisRow]);
			switch (configWorkData.ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList[thisCmdLine].CtrlTarget)
			{
				case "switch": configWorkData.ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList[thisCmdLine].CtrlType = swiCmdOptions[sender.selectedIndex];
				break;
				case "block": configWorkData.ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList[thisCmdLine].CtrlType = swiPwrOptions[sender.selectedIndex];
				break;
				case "power": configWorkData.ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList[thisCmdLine].CtrlType = trackPwrOptions[sender.selectedIndex];
				break;
			}
			break;
		case 6: //set CtrlValue
			switch (configWorkData.ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList[thisCmdLine].CtrlTarget)
			{
				case "switch": configWorkData.ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList[thisCmdLine].CtrlValue = swiPwrOptions[sender.selectedIndex];
				break;
				case "block": configWorkData.ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList[thisCmdLine].CtrlValue = swiPwrOptions[sender.selectedIndex];
				break;
				case "signal": configWorkData.ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList[thisCmdLine].CtrlValue = verifyNumber(sender.value, configWorkData.ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList[thisCmdLine].CtrlValue);
				break;
			}
			break;
	}
}

function tfCmdTypeSel(y, x, id, evtHandler)
{
	var selectList = document.createElement("select");
	createOptions(selectList, ["Switch", "DCC Signal", "Block Detector", "Power Status"]);
	tfSetCoordinate(selectList, y, x, 5, id);
	selectList.setAttribute("class", "selectorbox");
	selectList.setAttribute("onchange", evtHandler);
	return selectList;
}

function tfCmdLineHeader(y, x, id, evtHandler, lineData)
{
	var thisEditor = document.createElement("div");
	var thisText = tfText(y, x, id, evtHandler);
	thisText.innerHTML = "Type:";
	thisEditor.append(thisText);
	var thisID = "cmdtypesel_" + y.toString() + "_" + x.toString();
	var thisElement = tfCmdTypeSel(y, 2, thisID, evtHandler);
	thisElement.setAttribute("index", 5);
	thisElement.setAttribute("cmdline", x);
	
	thisElement.selectedIndex = cmdOptions.indexOf(lineData.CtrlTarget);
	thisEditor.append(thisElement);
	var thisText = tfText(y, x, id, evtHandler);
	thisText.innerHTML = "Addr.:";
	thisEditor.append(thisText);
	thisID = "ctrladdr_" + y.toString() + "_" + x.toString();
	thisElement = tfNumeric(y, 3, thisID, evtHandler);
	thisElement.setAttribute("cmdline", x);
	thisElement.value = lineData.CtrlAddr;
	thisElement.setAttribute("index", 6);
	thisEditor.append(thisElement);
	thisText = tfText(y, x, id, evtHandler);
	thisText.innerHTML = "Delay [ms]:";
	thisEditor.append(thisText);
	thisID = "cmddelay_" + y.toString() + "_" + x.toString();
	thisElement = tfNumeric(y, 4, thisID, evtHandler);
	thisElement.setAttribute("cmdline", x);
	thisElement.value = lineData.ExecDelay;
	thisElement.setAttribute("index", 7);
	thisEditor.append(thisElement);
	return thisEditor;
}

function tfCmdLineEditor(y, x, index, evtHandler)
{
	var thisEditor = document.createElement("div");
	var thisText = tfText(i, evtHandler);
	thisEditor.append(thisText);
	thisEditor.append(tfNumeric(i, evtHandler));
	thisText = tfText(i, evtHandler);
	thisEditor.append(thisText);
	thisEditor.append(tfNumeric(i, evtHandler));
	return thisEditor;
}

function tfCmdSwiPosSel(y, x, id, evtHandler)
{
	var selectList = document.createElement("select");
	createOptions(selectList, ["Thrown", "Closed", "Toggle"]);
	tfSetCoordinate(selectList, y, 5, 8, id);
	selectList.setAttribute("class", "selectorbox");
	selectList.setAttribute("cmdline", x);
	selectList.setAttribute("onchange", evtHandler);
	return selectList;
}

function tfCmdPowerSel(y, x, id, evtHandler)
{
	var selectList = document.createElement("select");
	createOptions(selectList, ["On", "Off", "Idle", "Toggle"]);
	tfSetCoordinate(selectList, y, 5, 8, id);
	selectList.setAttribute("cmdline", x);
	selectList.setAttribute("class", "selectorbox");
	selectList.setAttribute("onchange", evtHandler);
	return selectList;
}

function tfCmdSwiPowerSel(y, x, id, evtHandler)
{
	var selectList = document.createElement("select");
	createOptions(selectList, ["ON", "OFF"]);
	tfSetCoordinate(selectList, y, 6, 9, id);
	selectList.setAttribute("cmdline", x);
	selectList.setAttribute("class", "selectorbox");
	selectList.setAttribute("onchange", evtHandler);
	return selectList;
}

function setSwitchEditor(lineIndex, i, thisID, evtHandler, lineData)
{
	var thisEditor = document.createElement("div");
	
	var thisSpacer = document.createElement("div");
	thisSpacer.setAttribute("class", "manipulatorbox");
	thisEditor.append(thisSpacer);
	
	var thisText = tfText(lineIndex, i, thisID, evtHandler);
	thisText.innerHTML = "Command:";
	thisEditor.append(thisText);
	var thisSel = tfCmdSwiPosSel(lineIndex, i, thisID, evtHandler);
	thisSel.selectedIndex = swiCmdOptions.indexOf(lineData.CtrlType);
	thisEditor.append(thisSel);
	thisText = tfText(lineIndex, i, thisID, evtHandler);
	thisText.innerHTML = "Coil Status:";
	thisEditor.append(thisText);
	thisSel = tfCmdSwiPowerSel(lineIndex, i, thisID, evtHandler);
	thisSel.selectedIndex = swiPwrOptions.indexOf(lineData.CtrlValue);
	thisEditor.append(thisSel);
	return thisEditor;
}

function setSignalEditor(lineIndex, i, thisID, evtHandler, lineData)
{
	var thisEditor = document.createElement("div");
	
	var thisSpacer = document.createElement("div");
	thisSpacer.setAttribute("class", "manipulatorbox");
	thisEditor.append(thisSpacer);
	
	var thisText = tfText(lineIndex, i, thisID, evtHandler);
	thisText.innerHTML = "Aspect Value:";
	thisEditor.append(thisText);

	thisElement = tfNumeric(lineIndex, 6, thisID, evtHandler);
	thisElement.setAttribute("cmdline", i);
	thisElement.value = lineData.CtrlValue;
	thisEditor.append(thisElement);
	return thisEditor;
}

function setPowerEditor(lineIndex, i, thisID, evtHandler, lineData)
{
	var thisEditor = document.createElement("div");
	
	var thisSpacer = document.createElement("div");
	thisSpacer.setAttribute("class", "manipulatorbox");
	thisEditor.append(thisSpacer);
	
	var thisText = tfText(lineIndex, i, thisID, evtHandler);
	thisText.innerHTML = "Power Status:";
	thisEditor.append(thisText);
	var thisSel = tfCmdPowerSel(lineIndex, i, thisID, evtHandler);
	thisSel.selectedIndex = trackPwrOptions.indexOf(lineData.CtrlType);
	thisEditor.append(thisSel);
	return thisEditor;
}

function setBlockDetEditor(lineIndex, i, thisID, evtHandler, lineData)
{
	var thisEditor = document.createElement("div");
	
	var thisSpacer = document.createElement("div");
	thisSpacer.setAttribute("class", "manipulatorbox");
	thisEditor.append(thisSpacer);
	
	var thisText = tfText(lineIndex, i, thisID, evtHandler);
	thisText.innerHTML = "Detector Value:";
	thisEditor.append(thisText);

	thisSel = tfCmdSwiPowerSel(lineIndex, i, thisID, evtHandler);
	thisSel.selectedIndex = swiPwrOptions.indexOf(lineData.CtrlValue);
	thisEditor.append(thisSel);
	return thisEditor;
}

function buildCmdLines(lineIndex, lineData)
{
	var thisLineBase = document.getElementById("btnconfig_inp_" + lineIndex.toString() + "_2");
	while (thisLineBase.hasChildNodes())
		thisLineBase.removeChild(thisLineBase.childNodes[0]); //delete rows
	if ((lineData.CtrlCmd.length > 0) && (lineData.CtrlCmd[lineData.CurrDisp].CmdList.length > 0))
		for (var i=0; i<lineData.CtrlCmd[lineData.CurrDisp].CmdList.length; i++)
		{
			var masterDiv = document.createElement("div");
			masterDiv.setAttribute("class", "mastertile");
			if ((i % 2) == 0) //even
				masterDiv.style.backgroundColor = "#F5F5F5";
			else
				masterDiv.style.backgroundColor = "#D3D3D3";
			tfSetCoordinate(masterDiv, i, 0, 0, thisID);
			thisLineBase.append(masterDiv);

			var thisID = "pos_" + lineIndex.toString() + "_" + i.toString();
			masterDiv.append(tfPos(i, -1, thisID, ""));
			thisID = "cmdbasedata_" + lineIndex.toString() + "_" + i.toString();
			var mainDiv = document.createElement("div");
			mainDiv.setAttribute("class", "editortile");
			tfSetCoordinate(mainDiv, i, 0, 0, thisID);
			masterDiv.append(mainDiv);

			var upperDiv = document.createElement("div");
			upperDiv.setAttribute("class", "editorpanel");
			thisID = "cmdmanipulator_" + lineIndex.toString() + "_" + i.toString();
			var thisBox = tfManipulatorBox(lineIndex, 1, thisID, "setCommandData(this)");
			thisBox.setAttribute("cmdline", i);
			upperDiv.append(thisBox);
			
			thisID = "cmdlineheader_" + lineIndex.toString() + "_" + i.toString();
			upperDiv.append(tfCmdLineHeader(lineIndex, i, thisID, "setCommandData(this)", lineData.CtrlCmd[lineData.CurrDisp].CmdList[i]));
			mainDiv.append(upperDiv);

			var lowerDiv = document.createElement("div");
			lowerDiv.setAttribute("class", "editorpanel");
			mainDiv.append(lowerDiv);

			switch (lineData.CtrlCmd[lineData.CurrDisp].CmdList[i].CtrlTarget)
			{
				case "switch":
					lowerDiv.append(setSwitchEditor(lineIndex, i, thisID, "setCommandData(this)", lineData.CtrlCmd[lineData.CurrDisp].CmdList[i]));
					break;
				case "signal":
					lowerDiv.append(setSignalEditor(lineIndex, i, thisID, "setCommandData(this)", lineData.CtrlCmd[lineData.CurrDisp].CmdList[i]));
					break;
				case "power":
					lowerDiv.append(setPowerEditor(lineIndex, i, thisID, "setCommandData(this)", lineData.CtrlCmd[lineData.CurrDisp].CmdList[i]));
					break;
				case "block":
					lowerDiv.append(setBlockDetEditor(lineIndex, i, thisID, "setCommandData(this)", lineData.CtrlCmd[lineData.CurrDisp].CmdList[i]));
					break;
			}

		}
	else
	{
		var mainDiv = document.createElement("div");
		mainDiv.setAttribute("class", "editortile");
		tfSetCoordinate(mainDiv, i, 0, 0, thisID);
		thisLineBase.append(mainDiv);
		var thisId = "cmdbasedata_initadd" + lineIndex.toString();
		var newRB = tfTableStarterBox(lineIndex, -1, thisId, "setCommandData(this)");
		mainDiv.append(newRB);
	}
}

function loadTableData(thisTable, thisData)
{
//	console.log(thisData);
	var th = document.getElementById(buttonTable.id + "_head");
	var tb = document.getElementById(buttonTable.id + "_body");
	var numCols = th.childNodes[0].children.length;

	createDataTableLines(thisTable, [tfPos, tfCommandSelector, tfCommandEditor], thisData.length, "setButtonData(this)");
//	console.log(thisTable.id);	
	for (var i=0; i<thisData.length;i++)
	{
		var e = document.getElementById("addressbox_" + i.toString() + "_" + "1");
		e.value = thisData[i].ButtonNr;
		if (!isNaN(thisData[i].CurrDisp))
		{
			var e = document.getElementById("cmdlistbox_" + i.toString() + "_" + "1");
			e.selectedIndex = thisData[i].CurrDisp;
			var e = document.getElementById("address2box_" + i.toString() + "_" + "1");
			if ((thisData[i].CurrDisp == 0) || (thisData[i].CurrDisp == 3))
			{
				setVisibility(true, e);
				if (!isNaN(thisData[i].CtrlCmd[thisData[i].CurrDisp].BtnCondAddr))
					if (thisData[i].CtrlCmd[thisData[i].CurrDisp].BtnCondAddr == 0xFFFF)
						e.value = "";
					else
						e.value = thisData[i].CtrlCmd[thisData[i].CurrDisp].BtnCondAddr;
				else
					e.value = "";
			}
			else
				setVisibility(false, e);
				
		}
		else
			thisData[i].CurrDisp = 0;
		buildCmdLines(i, thisData[i]);
	}
}


function loadNodeDataFields(jsonData)
{
}

function constructPageContent(contentTab)
{
	var tempObj;
	mainScrollBox = createEmptyDiv(contentTab, "div", "pagetopicboxscroll-y", "nodeconfigdiv");
		createPageTitle(mainScrollBox, "div", "tile-1", "BasicCfg_Title", "h1", "LocoNet Button Handler Setup");
		createPageTitle(mainScrollBox, "div", "tile-1", "BasicCfg_Title", "h2", "Button Handler Message Setup");
		buttonTable = createDataTable(mainScrollBox, "tile-1", ["Pos","Button Selector", "Button Command Sequence"], "btnconfig", "");
		tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
			createButton(tempObj, "", "Save & Restart", "btnSave", "saveSettings(this)");
			createButton(tempObj, "", "Cancel", "btnCancel", "cancelSettings(this)");
		tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
			createButton(tempObj, "", "Save to File", "btnDownload", "downloadSettings(this)");
}

function loadDataFields(jsonData)
{
	upgradeJSONVersion(jsonData);
	loadTableData(buttonTable, jsonData.ButtonHandler);
}

function processLocoNetInput(jsonData)
{
}
