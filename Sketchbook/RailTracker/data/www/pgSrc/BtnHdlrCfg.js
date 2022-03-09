var mainScrollBox;
var buttonTable;

var sourceOptionArray = ["switch","dynsignal","dccsignal", "button","analogvalue", "blockdetector", "transponder", "power", "analogscaler"];
var enableOptionArray = ["on", "off", "button", "switch", "block"];

var newEventTemplate = {"BtnCondAddr": [], "CmdList":[]};
var newButtonTemplate = {"EnableSource": "on", "EnableAddr": 0, "EnableState": 0, "EventSource":"button", "CondData": [], "ButtonNr": 0,	"CtrlCmd": []};
var newCmdTemplate = {"CtrlTarget": "switch", "CtrlAddr": 0, "CtrlType":"toggle", "CtrlValue":"on", "ExecDelay":250};
//var newEnableTemplate = {"EnableSource": "alwayson", "EnableAddr": 0, "EnableState": 0};

var cmdOptions = ["switch", "switchack", "signal", "button", "analog", "block", "power"];
var swiCmdOptions = ["thrown","closed","toggle"];
var swiStatOptions = ["thrown","closed"];
var btnStatOptions = ["Btn Down","Btn Up"];
var blockStatOptions = ["free","occupied"];
var swiPwrOptions = ["on", "off"];
var trackPwrOptions = ["on", "off", "idle", "toggle"];
var buttonOptions = ["btndown", "btnup", "btnclick", "btnhold", "btndblclick"];
var buttonValDispType = ["Btn Down", "Btn Up", "Btn Click", "Btn Hold", "Btn Dbl Click"];
var powerValDispType = ["Off", "On", "Idle"];
var transponderValDispType = ["Enter", "Leave"];

jsonFileVersion = "1.3.2";

function saveConfigFileSettings()
{
	//step 1: save greenhat.cfg
	saveJSONConfig(scriptList.Pages[currentPage].ID, scriptList.Pages[currentPage].FileName, configData[workCfg], prepareFileSeqBtnEvt);
}

function upgradeJSONVersion(jsonData)
{
	return upgradeJSONVersionBtnHdlr(jsonData);
}

function addFileSeq(ofObj, cfgData) //object specific function to include partial files
{
	addFileSeqBtnHdlr(ofObj, cfgData);
}


function clearDisplay()
{
	var tb = document.getElementById(buttonTable.id + "_body");
	while (tb.hasChildNodes())
		tb.removeChild(tb.childNodes[0]); //delete rows
}

function downloadSettings(sender)
{
	downloadConfig(0x0040); //send just this
}

function setAddr2Disp(sourceMode, eventMode, thisRow, thisCol)
{
//	console.log(sourceMode, eventMode, thisRow, thisCol);
	var addrBox = document.getElementById("parambox_" + thisRow.toString() + "_1");
	var addrField = document.getElementById("address2box_" + thisRow.toString() + "_1");
	var addrText = 	document.getElementById("paramtext_" + thisRow.toString() + "_1");
	var btnAdd = document.getElementById("btn_add_" + thisRow.toString() + "_1");
	var btnCancel = document.getElementById("btn_cancel_" + thisRow.toString() + "_1");
	var srcMode = sourceOptionArray.indexOf(configData[workCfg].ButtonHandler[thisRow].EventSource);
	var dispField = false;
	setVisibility(false, btnAdd);
	setVisibility(false, btnCancel);
	switch (sourceMode)
	{
		case 0: //Switch
			break; 
		case 1: //dyn signal
			break;
		case 2: //DCC signal
			dispField = true;  
			setVisibility(true, btnAdd);
			setVisibility(true, btnCancel);
			addrText.innerHTML = "Value:&nbsp;" 
			break; //Signal
		case 3: //Button
			dispField = [0,3].indexOf(eventMode) >= 0; 
			addrText.innerHTML = "2 Btn. Hold Addr.:&nbsp;" 
			break;
		case 4: 
			dispField = [1,2].indexOf(eventMode) >= 0; 
			addrText.innerHTML = "Value:&nbsp;";
			break; //Analog
		case 5: 
			break; //Block Detector
		case 6: //Transponder 
			dispField = true;  
			addrText.innerHTML = "Locos:&nbsp;" 
			break;
		case 7: dispField = false;  break; //Power State
		case 8: dispField = false;  break; //Analog Scaler
	}
	if (dispField)
	{
		setVisibility(true, addrBox);
		if ([3,4,6].indexOf(srcMode) >= 0) //analog, transponder
			if (!Array.isArray(configData[workCfg].ButtonHandler[thisRow].CondData))
				addrField.value = "";
			else
				addrField.value = configData[workCfg].ButtonHandler[thisRow].CondData;
		else //dcc signal
			if (isNaN(configData[workCfg].ButtonHandler[thisRow].CtrlCmd[configData[workCfg].ButtonHandler[thisRow].CurrDisp].BtnCondAddr))
				addrField.value = "";
			else
				if (configData[workCfg].ButtonHandler[thisRow].CtrlCmd[configData[workCfg].ButtonHandler[thisRow].CurrDisp].BtnCondAddr == 0xFFFF)
					addrField.value = "";
				else
					addrField.value = configData[workCfg].ButtonHandler[thisRow].CtrlCmd[configData[workCfg].ButtonHandler[thisRow].CurrDisp].BtnCondAddr;
	}
	else
		setVisibility(false, addrBox);
}

function setEnableDisp(thisRow)
{
	var dispDiv = document.getElementById("enablediv_" + thisRow.toString() + "_1");
	var eventList = document.getElementById("enabletypebox_" + thisRow.toString() + "_1");
	var addrBox = document.getElementById("enableaddressbox_" + thisRow.toString() + "_1");
	var statusList = document.getElementById("enablecmdlistbox_" + thisRow.toString() + "_1");
	var currOption = enableOptionArray.indexOf(configData[workCfg].ButtonHandler[thisRow].EnableSource);
	eventList.selectedIndex = currOption;
	switch (currOption)
	{
		case 2: createOptions(statusList, btnStatOptions); break; //button
		case 3: createOptions(statusList, swiStatOptions); break; //switch
		case 4: createOptions(statusList, blockStatOptions); break; //block
	}
	addrBox.value = configData[workCfg].ButtonHandler[thisRow].EnableAddr;
	statusList.selectedIndex = configData[workCfg].ButtonHandler[thisRow].EnableState;
	setVisibility(currOption > 1, dispDiv);
}

function adjustEventList(ofButton, newLength)
{
	while (ofButton.CtrlCmd.length > newLength) 
		ofButton.CtrlCmd.splice(ofButton.CtrlCmd.length-1, 1); //remove last element
	while (ofButton.CtrlCmd.length < newLength) 
		ofButton.CtrlCmd.push(JSON.parse(JSON.stringify(newEventTemplate)));
}

function adjustSourceSelector(thisHandlerData, thisRow, thisCol)
{
	var evtSrcBox = document.getElementById("evttypebox_" + thisRow.toString() + "_" + thisCol.toString());
	var evtListBox = document.getElementById("evtcmdlistbox_" + thisRow.toString() + "_" + thisCol.toString());
	evtSrcBox.selectedIndex = sourceOptionArray.indexOf(thisHandlerData[thisRow].EventSource);
	var numAddr = thisHandlerData[thisRow].ButtonNr.length;
	var optArray = [];
	var eventMode = thisHandlerData[thisRow].CurrDisp; //evtListBox.selectedIndex; //get old settings
	//set option strings
	switch (evtSrcBox.selectedIndex)
	{
		case 0: //static switch
			for (var i = 0; i < Math.pow(2, numAddr); i ++)
			{
				var resStr = "";
				for (var p = 0; p < numAddr; p++)
				{
					if (((0x01 << p) & i) > 0)
						resStr += thisHandlerData[thisRow].ButtonNr[p].toString() + " Cl ";
					else
						resStr += thisHandlerData[thisRow].ButtonNr[p].toString() + " Th ";
				}
				optArray.push(resStr);
			}
			createOptions(evtListBox, optArray); 
			break;
		case 1: //signaldyn
			for (var i = 0; i < numAddr; i ++)
			{
				optArray.push(thisHandlerData[thisRow].ButtonNr[i].toString() + " Th ");
				optArray.push(thisHandlerData[thisRow].ButtonNr[i].toString() + " Cl ");
			}
			createOptions(evtListBox, optArray); 
			break;
		case 2: //DCC Signal
			var aspCnt = thisHandlerData[thisRow].CtrlCmd.length;
			if (aspCnt > 0)
				for (var i = 0; i < aspCnt; i++)
				{	
					if (thisHandlerData[thisRow].CtrlCmd[i].BtnCondAddr != undefined)
						if (thisHandlerData[thisRow].CtrlCmd[i].BtnCondAddr != 0xFFFF)
							optArray.push("Aspect " + thisHandlerData[thisRow].CtrlCmd[i].BtnCondAddr.toString());
						else
							optArray.push("Aspect #" + i.toString());
					else
						optArray.push("Aspect #" + i.toString());
				}
			else
				optArray.push("Aspect");
			createOptions(evtListBox, optArray); 
			break; 
		case 3: createOptions(evtListBox, buttonValDispType); break; //Button
		case 4: createOptions(evtListBox, [ "== 0", "<", ">=", "== max"]); break; //Analog
		case 5:  //Block detector
			for (var i = 0; i < Math.pow(2, numAddr); i ++)
			{
				var resStr = "";
				for (var p = 0; p < numAddr; p++)
				{
					if (((0x01 << p) & i) > 0)
						resStr += thisHandlerData[thisRow].ButtonNr[p].toString() + " Oc ";
					else
						resStr += thisHandlerData[thisRow].ButtonNr[p].toString() + " Fr ";
				}
				optArray.push(resStr);
			}
			createOptions(evtListBox, optArray); 
			break;
		case 6: createOptions(evtListBox, transponderValDispType); break; //Transponder
		case 7: createOptions(evtListBox, powerValDispType); break; //power options
		case 8: createOptions(evtListBox, [ "== 0", "== max"]); break; //analog scaler options
	}
	if (eventMode >= evtListBox.options.length)
		eventMode = 0;
	//adjust length of event list array to new event list
	adjustEventList(thisHandlerData[thisRow], evtListBox.options.length);
	//adjust visibility of address box 2
	evtListBox.selectedIndex = eventMode;
	thisHandlerData[thisRow].CurrDisp = eventMode;
	setAddr2Disp(evtSrcBox.selectedIndex, eventMode, thisRow, thisCol);
}

function setButtonData(sender)
{
	var thisRow = parseInt(sender.getAttribute("row"));
	var thisCol = parseInt(sender.getAttribute("col"));
	var thisIndex = parseInt(sender.getAttribute("index"));
	var newLength;
	switch (thisCol)
	{
		case -1: //empty table, create first entry
			newLength = configData[2].ButtonHandler.push(JSON.parse(JSON.stringify(newButtonTemplate)));
			adjustEventList(configData[2].ButtonHandler[newLength-1], 5);
			loadTableData(buttonTable, configData[2].ButtonHandler);
			break;
		case 1: //CommandSelector
			var idStr = sender.id;
			var thisElement;
			switch (thisIndex)
			{
				case 1:
					if (event.ctrlKey) //duplicate entry
						configData[2].ButtonHandler.splice(thisRow+1, 0, JSON.parse(JSON.stringify(configData[2].ButtonHandler[thisRow])));
					else //create new entry
						configData[2].ButtonHandler.splice(thisRow+1, 0, JSON.parse(JSON.stringify(newButtonTemplate)));
					break;
				case 2:
					configData[2].ButtonHandler.splice(thisRow, 1);
					break;
				case 3:
					if (thisRow > 0)
					{
						thisElement = configData[2].ButtonHandler.splice(thisRow, 1);
						configData[2].ButtonHandler.splice(thisRow-1,0, thisElement[0]);
					}
					break;
				case 4:
					if (thisRow < configData[2].ButtonHandler.length)
					{
						thisElement = configData[2].ButtonHandler.splice(thisRow, 1);
						configData[2].ButtonHandler.splice(thisRow+1,0, thisElement[0]);
					}
					break;
				case 11: //Event Type
					{
						configData[workCfg].ButtonHandler[thisRow].EventSource = sourceOptionArray[sender.selectedIndex];
						adjustSourceSelector(configData[workCfg].ButtonHandler, thisRow, thisCol);
						buildCmdLines(thisRow, configData[workCfg].ButtonHandler[thisRow]);
					}
					break;
				case 12: //Button Addr, add support for comma separated format
					var newRes = verifyNumArray(sender.value, ",");
					var srcMode = sourceOptionArray.indexOf(configData[workCfg].ButtonHandler[thisRow].EventSource);
					if ([0,1,5].indexOf(srcMode) >= 0) //limit the length to 3 for stat and dyn signals
						while (newRes.length > 3)
							newRes.pop();
					else
						while (newRes.length > 1) //limit to one for everything else
							newRes.pop();
					configData[workCfg].ButtonHandler[thisRow].ButtonNr = newRes; 
					sender.value = newRes;
					adjustSourceSelector(configData[workCfg].ButtonHandler, thisRow, thisCol);
					break;
				case 13: //select event
					configData[workCfg].ButtonHandler[thisRow].CurrDisp = sender.selectedIndex;
					buildCmdLines(thisRow, configData[2].ButtonHandler[thisRow]);
					setAddr2Disp(sourceOptionArray.indexOf(configData[workCfg].ButtonHandler[thisRow].EventSource), sender.selectedIndex, thisRow, thisCol);
					break;
				case 14: //2nd addr dcc signal, analog, button, transponder
					var srcMode = sourceOptionArray.indexOf(configData[workCfg].ButtonHandler[thisRow].EventSource);
					var newRes = verifyNumArray(sender.value, ",");
					if ([2,3,4].indexOf(srcMode) >= 0) //limit the length to 1 for all except transponders
						while (newRes.length > 1) 
							newRes.pop();
					switch (srcMode)
					{
						case 2: //dcc signal
							if (isNaN(newRes))
								configData[workCfg].ButtonHandler[thisRow].CtrlCmd[configData[workCfg].ButtonHandler[thisRow].CurrDisp].BtnCondAddr = [];
							else
								configData[workCfg].ButtonHandler[thisRow].CtrlCmd[configData[workCfg].ButtonHandler[thisRow].CurrDisp].BtnCondAddr = newRes;
							var sourceList= document.getElementById("evtcmdlistbox_" + thisRow.toString() + "_" + thisCol.toString());
							sourceList.options[sourceList.selectedIndex].text = "Aspect " + configData[workCfg].ButtonHandler[thisRow].CtrlCmd[configData[workCfg].ButtonHandler[thisRow].CurrDisp].BtnCondAddr.toString();
							break;
						case 3: ;//button
						case 4: ;//analog
						case 6:  //transponder
							configData[workCfg].ButtonHandler[thisRow].CondData = newRes;
							break;
					}
					sender.value = newRes;
					break;
				case 15: //New event, only for signal aspects
					configData[workCfg].ButtonHandler[thisRow].CtrlCmd.splice(configData[workCfg].ButtonHandler[thisRow].CurrDisp, 0, JSON.parse(JSON.stringify(newEventTemplate)));
//					console.log(configData[workCfg].ButtonHandler[thisRow]);
					loadTableData(buttonTable, configData[workCfg].ButtonHandler);
					break;
				case 16: //delete event
					if (configData[workCfg].ButtonHandler[thisRow].CtrlCmd.length > 0) //can't delete the last event
					{
						configData[workCfg].ButtonHandler[thisRow].CtrlCmd.splice(configData[workCfg].ButtonHandler[thisRow].CurrDisp, 1);
						if (configData[workCfg].ButtonHandler[thisRow].CurrDisp > configData[workCfg].ButtonHandler[thisRow].CtrlCmd.length - 1)
						{
							configData[workCfg].ButtonHandler[thisRow].CurrDisp = (configData[workCfg].ButtonHandler[thisRow].CtrlCmd.length - 1);
							var evtListBox = document.getElementById("evtcmdlistbox_" + thisRow.toString() + "_" + thisCol.toString());
							evtListBox.selectedIndex -= 1;
						}
						loadTableData(buttonTable, configData[workCfg].ButtonHandler);
					}
					break;
				case 21: // Enable Source
					configData[workCfg].ButtonHandler[thisRow].EnableSource = enableOptionArray[sender.selectedIndex];
					setEnableDisp(thisRow);
					break;
				case 23: // Enable Address
					configData[workCfg].ButtonHandler[thisRow].EnableAddr = verifyNumber(sender.value, configData[2].ButtonHandler[thisRow].EnableAddr);
					break;
				case 24: // Enable Status
					configData[workCfg].ButtonHandler[thisRow].EnableState = verifyNumber(sender.value, configData[2].ButtonHandler[thisRow].EnableState);
					break;
					
			}
			if (thisIndex < 5)
				loadTableData(buttonTable, configData[workCfg].ButtonHandler);
			break;
		default:
			break;
	}
}

function setCommandData(sender)
{
	var thisRow = parseInt(sender.getAttribute("row"));
	var thisCol = parseInt(sender.getAttribute("col"));
	var thisIndex = parseInt(sender.getAttribute("index"));
	var thisCmd = configData[2].ButtonHandler[thisRow].CurrDisp;
	var thisCmdLine = parseInt(sender.getAttribute("cmdline"));
	if (isNaN(thisCmdLine))
		thisCmdLine = parseInt(sender.parentElement.getAttribute("cmdline"));
	switch (thisCol)
	{
		case -1: //empty table, create first entry
			configData[2].ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList.push(JSON.parse(JSON.stringify(newCmdTemplate)));
			buildCmdLines(thisRow, configData[2].ButtonHandler[thisRow]);
			break;
		case 1: //Manipulator Commands
			switch (thisIndex)
			{
				case 1: //add CmdLine
					if (event.ctrlKey) //duplicate entry
						configData[2].ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList.splice(thisCmdLine+1, 0, JSON.parse(JSON.stringify(configData[2].ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList[thisCmdLine])));
					else //create new entry
						configData[2].ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList.splice(thisCmdLine+1, 0, JSON.parse(JSON.stringify(newCmdTemplate)));
					break;
				case 2: //delete CmdLine
					configData[2].ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList.splice(thisCmdLine, 1);
					break;
				case 3: //up
					if (thisCmdLine > 0)
					{
						thisElement = configData[2].ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList.splice(thisCmdLine, 1);
						configData[2].ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList.splice(thisCmdLine-1,0, thisElement[0]);
					}
					break;
				case 4: //down
					if (thisCmdLine < configData[2].ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList.length)
					{
						thisElement = configData[2].ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList.splice(thisCmdLine, 1);
						configData[2].ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList.splice(thisCmdLine+1,0, thisElement[0]);
					}
					break;

			}
			buildCmdLines(thisRow, configData[2].ButtonHandler[thisRow]);
			break;
		case 2: //set command target
			var currTarget = configData[2].ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList[thisCmdLine].CtrlTarget;
			configData[2].ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList[thisCmdLine].CtrlTarget = cmdOptions[sender.selectedIndex];
			
//			console.log(currTarget, configData[2].ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList[thisCmdLine].CtrlTarget);
			
			if (currTarget != configData[2].ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList[thisCmdLine].CtrlTarget)
			{
				configData[2].ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList[thisCmdLine].CtrlAddr = 0;
				configData[2].ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList[thisCmdLine].ExecDelay = 250;
				switch  (sender.selectedIndex)
				{
					case 0: ;// switch
						configData[2].ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList[thisCmdLine].CtrlTarget = "switch";
						configData[2].ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList[thisCmdLine].CtrlType = "toggle";
						configData[2].ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList[thisCmdLine].CtrlValue = "on";
						break;
					case 1: //switch ack
						configData[2].ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList[thisCmdLine].CtrlTarget = "switchack";
						configData[2].ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList[thisCmdLine].CtrlType = "toggle";
						configData[2].ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList[thisCmdLine].CtrlValue = "on";
						break;
					case 2: //signal
						configData[2].ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList[thisCmdLine].CtrlTarget = "signal";
						configData[2].ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList[thisCmdLine].CtrlType = "aspect";
						configData[2].ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList[thisCmdLine].CtrlValue = "0";
						break;
					case 3: //button
						configData[2].ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList[thisCmdLine].CtrlTarget = "button";
						configData[2].ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList[thisCmdLine].CtrlType = "btndown";
						configData[2].ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList[thisCmdLine].CtrlValue = "0";
						break;
					case 4: //analog
						configData[2].ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList[thisCmdLine].CtrlTarget = "analog";
						configData[2].ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList[thisCmdLine].CtrlType = "value";
						configData[2].ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList[thisCmdLine].CtrlValue = "0";
						break;
					case 5: //block
						configData[2].ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList[thisCmdLine].CtrlTarget = "block";
						configData[2].ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList[thisCmdLine].CtrlType = "block";
						configData[2].ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList[thisCmdLine].CtrlValue = "on";
						break;
					case 6: //power
						configData[2].ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList[thisCmdLine].CtrlTarget = "power";
						configData[2].ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList[thisCmdLine].CtrlType = "idle";
						configData[2].ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList[thisCmdLine].CtrlValue = "0";
						break;
				}
			}
			buildCmdLines(thisRow, configData[2].ButtonHandler[thisRow]);
			break;
		case 3: //set command target address
			configData[2].ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList[thisCmdLine].CtrlAddr = verifyNumber(sender.value, configData[2].ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList[thisCmdLine].CtrlAddr);
			break;
		case 4: //set delay after command
			configData[2].ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList[thisCmdLine].ExecDelay = verifyNumber(sender.value, configData[2].ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList[thisCmdLine].ExecDelay);
			break;
		case 5: //set CtrlType
			switch (configData[2].ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList[thisCmdLine].CtrlTarget)
			{
				case "switch": ;
				case "switchack": configData[2].ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList[thisCmdLine].CtrlType = swiCmdOptions[sender.selectedIndex];
				break;
				case "block": configData[2].ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList[thisCmdLine].CtrlType = swiPwrOptions[sender.selectedIndex];
				break;
				case "power": configData[2].ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList[thisCmdLine].CtrlType = trackPwrOptions[sender.selectedIndex];
				break;
				case "button": configData[2].ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList[thisCmdLine].CtrlType = buttonOptions[sender.selectedIndex];
				break;
			}
			break;
		case 6: //set CtrlValue
			switch (configData[2].ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList[thisCmdLine].CtrlTarget)
			{
				case "switch": ;
				case "switchack": configData[2].ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList[thisCmdLine].CtrlValue = swiPwrOptions[sender.selectedIndex];
				break;
				case "block": configData[2].ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList[thisCmdLine].CtrlValue = swiPwrOptions[sender.selectedIndex];
				break;
//				case "button": configData[2].ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList[thisCmdLine].CtrlValue = buttonOptions[sender.selectedIndex];
//				break;
				case "signal": configData[2].ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList[thisCmdLine].CtrlValue = verifyNumber(sender.value, configData[2].ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList[thisCmdLine].CtrlValue);
				break;
				case "analog": 
					var oldVal = (parseInt(configData[2].ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList[thisCmdLine].CtrlType) << 8) + parseInt(configData[2].ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList[thisCmdLine].CtrlValue);
					var newVal = verifyNumber(sender.value, oldVal) & 0x3FFF;
					configData[2].ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList[thisCmdLine].CtrlType = ((newVal & 0x3F00) >> 8).toString();
					configData[2].ButtonHandler[thisRow].CtrlCmd[thisCmd].CmdList[thisCmdLine].CtrlValue = (newVal & 0x00FF).toString();
				break;
			}
			break;
	}
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
			upperDiv.append(tfCmdEvtLineHeader(lineIndex, i, thisID, "setCommandData(this)", lineData.CtrlCmd[lineData.CurrDisp].CmdList[i]));
			mainDiv.append(upperDiv);

			var lowerDiv = document.createElement("div");
			lowerDiv.setAttribute("class", "editorpanel");
			mainDiv.append(lowerDiv);

			switch (lineData.CtrlCmd[lineData.CurrDisp].CmdList[i].CtrlTarget)
			{
				case "switch":;
				case "switchack":
					lowerDiv.append(setSwitchEditor(lineIndex, i, thisID, "setCommandData(this)", lineData.CtrlCmd[lineData.CurrDisp].CmdList[i]));
					break;
				case "signal":
					lowerDiv.append(setSignalEditor(lineIndex, i, thisID, "setCommandData(this)", lineData.CtrlCmd[lineData.CurrDisp].CmdList[i]));
					break;
				case "button":
					lowerDiv.append(setButtonEditor(lineIndex, i, thisID, "setCommandData(this)", lineData.CtrlCmd[lineData.CurrDisp].CmdList[i]));
					break;
				case "analog":
					lowerDiv.append(setAnalogEditor(lineIndex, i, thisID, "setCommandData(this)", lineData.CtrlCmd[lineData.CurrDisp].CmdList[i]));
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
	var th = document.getElementById(thisTable.id + "_head");
	var tb = document.getElementById(thisTable.id + "_body");
	var numCols = th.childNodes[0].children.length;
	createDataTableLines(thisTable, [tfPos, tfCommandEvtSelector, tfCommandEditor], thisData.length, "setButtonData(this)");
	for (var i=0; i<thisData.length;i++)
	{
		var e = document.getElementById("evtaddressbox_" + i.toString() + "_" + "1");
		e.value = thisData[i].ButtonNr;
		if (isNaN(thisData[i].CurrDisp))
			thisData[i].CurrDisp = 0;
		var evtSrcBox = document.getElementById("evttypebox_" + i.toString() + "_" + "1");
		evtSrcBox.selectedIndex = -1;
		for (var j = 0; j < evtSrcBox.options.length; j++) 
		{
			if (evtSrcBox.options[j].text == thisData[i].EventSource) 
			{ 
				evtSrcBox.selectedIndex = j;
				break; 
			} 
		}
		var evtOptBox = document.getElementById("evtcmdlistbox_" + i.toString() + "_" + "1");
		adjustSourceSelector(configData[workCfg].ButtonHandler, i, 1);
		evtOptBox.selectedIndex = thisData[i].CurrDisp;
		buildCmdLines(i, thisData[i]);
		setEnableDisp(i);
	}
}

function loadNodeDataFields(jsonData)
{
}

function constructPageContent(contentTab)
{
	var tempObj;
	mainScrollBox = createEmptyDiv(contentTab, "div", "pagetopicboxscroll-y", "nodeconfigdiv");
		createPageTitle(mainScrollBox, "div", "tile-1", "BasicCfg_Title", "h1", "LocoNet Event Handler Setup");
//		createPageTitle(mainScrollBox, "div", "tile-1", "BasicCfg_Title", "h2", "Event Handler Message Setup");
		buttonTable = createDataTable(mainScrollBox, "tile-1", ["Pos","IF THIS: (Input Event Selector)", "THEN THAT: (Event Command Sequence)"], "btnconfig", "");
		tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
			createButton(tempObj, "", "Save & Restart", "btnSave", "saveSettings(this)");
			createButton(tempObj, "", "Cancel", "btnCancel", "cancelSettings(this)");
		tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
			createButton(tempObj, "", "Save to File", "btnDownload", "downloadSettings(this)");
}

function loadDataFields(jsonData)
{
	configData[workCfg] = upgradeJSONVersion(jsonData);
	loadTableData(buttonTable, configData[workCfg].ButtonHandler);
}

function processLocoNetInput(jsonData)
{
}
