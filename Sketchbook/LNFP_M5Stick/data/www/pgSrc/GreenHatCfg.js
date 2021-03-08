var mainScrollBox;
var switchTable;
var mqttTitle;
var mqttBox;

var swiCfgData = [{},{},{}];
var btnCfgData = [{},{},{}];
var evtHdlrCfgData = [{},{},{}];
var ledData = [{},{},{}];
var colOptions = [];

var subFileIndex = 0;
var updateServoPos = false;

var currDispPage = 0;

var maxButtons = 0;
var numChannels = 16;

var btnStatus = ["off", "digital", "analog"];

var ledOptionArray = ["switch", "signaldyn", "signal", "button", "analog", "block", "transponder", "power", "constant", "signalstat"];
var sourceOptionArray = ["switch","dynsignal","dccsignal", "button","analogvalue", "blockdetector", "transponder"];
var newButtonTemplate = {"PortNr": 0, "ButtonType": "digital", "EventMask": 3, "ButtonAddr": 0};
var newEventTemplate = {"Used": 1, "AspVal": 65535, "PosPt": 270, "MoveCfg": 0};
var newEventHdlrTemplate = {"ButtonNr":[0],"CondData":[],"CtrlCmd": []};
var newCmdTemplate = {"BtnCondAddr": [], "CmdList": [{"CtrlTarget": "switch", "CtrlAddr": 0, "CtrlType":"toggle", "CtrlValue":"on", "ExecDelay":250}]};
var newColTemplate = {"Name": "New Color","RGBVal": [255, 255, 255]};
var newLEDTemplate = {"LEDNums":[],"CtrlSource": "","CtrlAddr": [],"DisplayType":"discrete","LEDCmd": []};
var newLEDCmdTemplate = {"Val": 0,"ColOn": "", "ColOff": "", "Mode": "static", "Rate":0, "Transition":"soft"};
var cmdOptions = ["switch", "signal", "block", "local", "none"];
var swiCmdOptions = ["thrown","closed","toggle"];
var blockCmdOptions = ["occupied","free"];
//var blockCmdOptions = ["occupied","free"];

var dispOptions = ["static", "localblinkpos", "localblinkneg", "globalblinkpos", "globalblinkneg","localrampup", "localrampdown","globalrampup", "globalrampdown"]
var transOptions = ["soft", "direct","merge"];

var templateDlg = null;
var level0Div;
var level1Div;
var level2Div;
var level3Div;
var level4Div;

function downloadSettings(sender)
{
	downloadConfig(0x0200); //send just this
}

function saveConfigFileSettings()
{
	//step 1: save greenhat.cfg
	saveJSONConfig(scriptList.Pages[currentPage].ID, scriptList.Pages[currentPage].FileName, configData[workCfg], null);
	//then save the other files
	for (var i = 0; i < configData[workCfg].Modules.length; i++)
	{
		saveJSONConfig(configData[workCfg].Modules[i].CfgFiles[0].ID, configData[workCfg].Modules[i].CfgFiles[0].FileName, swiCfgData[workCfg], null);
		saveJSONConfig(configData[workCfg].Modules[i].CfgFiles[0].ID, configData[workCfg].Modules[i].CfgFiles[1].FileName, btnCfgData[workCfg], null);
		saveJSONConfig(configData[workCfg].Modules[i].CfgFiles[0].ID, configData[workCfg].Modules[i].CfgFiles[2].FileName, evtHdlrCfgData[workCfg], null);
		saveJSONConfig(configData[workCfg].Modules[i].CfgFiles[0].ID, configData[workCfg].Modules[i].CfgFiles[3].FileName, ledData[workCfg], null);
	}
}

function addFileSeq(ofObj) //object specific function to include partial files
{
//	console.log(ofObj);
}

function prepareFileSeq() //object specific function to create partial files
{
	var objCopy = JSON.parse(JSON.stringify(configData[workCfg]));
	transferData[0].FileList.push(objCopy);
	
/*	
	function addEntry()
	{
		var newEntry = {"ButtonHandler":[]}
		transferData[0].FileList.push(newEntry);
		return newEntry;
	}
	
	var thisEntry = addEntry();
	thisEntry.Version = jsonFileVersion;
	var thisFileLength = 0;
	
	for (var j=0; j<configData[workCfg].ButtonHandler.length;j++)
	{
		var thisElementStr = JSON.stringify(configData[workCfg].ButtonHandler[j]);
		thisFileLength += thisElementStr.length;
		thisEntry.ButtonHandler.push(JSON.parse(thisElementStr));
		if ((thisFileLength > targetSize) && (j < (configData[workCfg].ButtonHandler.length - 1)))
		{
			thisEntry = addEntry();
			thisFileLength = 0;
		}
	}
*/
}

function addDataFile(ofObj)
{
	switch (ofObj.Type)
	{
		case "pgSwitchCfg":
			swiCfgData[loadCfg] = JSON.parse(JSON.stringify(ofObj.Data));
			swiCfgData[workCfg] = upgradeJSONVersionSwitch(JSON.parse(JSON.stringify(swiCfgData[loadCfg])));
//			console.log(swiCfgData[workCfg]);
			break;
		case "pgHWBtnCfg":
			btnCfgData[loadCfg] = JSON.parse(JSON.stringify(ofObj.Data));
			btnCfgData[workCfg] = upgradeJSONVersionBtn(JSON.parse(JSON.stringify(btnCfgData[loadCfg])));
			while (btnCfgData[workCfg].Buttons.length > 32)
				btnCfgData[workCfg].Buttons.pop();
			while (btnCfgData[workCfg].Buttons.length < 32)
				btnCfgData[workCfg].Buttons.push(JSON.parse(JSON.stringify(newButtonTemplate)));
			console.log(btnCfgData[workCfg]);
			break;
		case "pgBtnHdlrCfg":
//			if (evtHdlrCfgData[loadCfg].ButtonHandler == undefined)
				evtHdlrCfgData[loadCfg] = JSON.parse(JSON.stringify(ofObj.Data));
//			else
//				addFileSeqBtnHdlr(ofObj, evtHdlrCfgData);
			evtHdlrCfgData[workCfg] = upgradeJSONVersionBtnHdlr(JSON.parse(JSON.stringify(evtHdlrCfgData[loadCfg])));
			while (evtHdlrCfgData[workCfg].ButtonHandler.length > 32)
				evtHdlrCfgData[workCfg].ButtonHandler.pop();
			while (evtHdlrCfgData[workCfg].ButtonHandler.length < 32)
				evtHdlrCfgData[workCfg].ButtonHandler.push(JSON.parse(JSON.stringify(newEventHdlrTemplate)));
			for (var i = 0; i < 32; i++)
			{
				evtHdlrCfgData[workCfg].ButtonHandler[i].ButtonNr = btnCfgData[workCfg].Buttons[i].ButtonAddr;
				adjustHdlrEventList(evtHdlrCfgData[workCfg].ButtonHandler[i], btnCfgData[workCfg].Buttons[i].EventMask == 0x03 ? 2 : 5);
			}
			console.log(evtHdlrCfgData[workCfg]);
			break;
		case "pgLEDCfg":
//			if (ledData[loadCfg].LEDDefs == undefined)
				ledData[loadCfg] = JSON.parse(JSON.stringify(ofObj.Data));
//			else
//				addFileSeqLED(ofObj, ledData);
			ledData[workCfg] = upgradeJSONVersionLED(JSON.parse(JSON.stringify(ledData[loadCfg])));
			while (ledData[workCfg].LEDDefs.length > 33)
				ledData[workCfg].LEDDefs.pop();
			while (ledData[workCfg].LEDDefs.length < 33)
				ledData[workCfg].LEDDefs.push(JSON.parse(JSON.stringify(newLEDTemplate)));
//			console.log(ledData[workCfg]);
			break;
	}


	if (ledData[workCfg].LEDCols != undefined) //LEDCols
		loadColorTable(colorTable, ledData[workCfg].LEDCols);
	updateServoPos = false;
	loadTableData(switchTable, swiCfgData[workCfg].Drivers, btnCfgData[workCfg].Buttons, evtHdlrCfgData[workCfg].ButtonHandler, ledData[workCfg].LEDDefs);
	updateServoPos = true;
}

function setColorData(sender)
{
	var thisRow = parseInt(sender.getAttribute("row"));
	var thisCol = parseInt(sender.getAttribute("col"));
	var thisIndex = parseInt(sender.getAttribute("index"));
//	console.log(thisRow, thisCol, thisIndex);
	switch (thisCol)
	{
		case -1: //empty table, create first entry
			ledData[workCfg].LEDCols.push(JSON.parse(JSON.stringify(newColTemplate)));
			loadColorTable(colorTable, ledData[workCfg].LEDCols);
			break;
		case 1: //Color Name
			ledData[workCfg].LEDCols[thisRow].Name = sender.value;
			break;
		case 2: //Color RGB Value
			var thisColorStr = sender.value.replace("#", "0x");
			var thisColor = parseInt(thisColorStr);
			ledData[workCfg].LEDCols[thisRow].RGBVal[0] = (thisColor & 0xFF0000) >> 16;
			ledData[workCfg].LEDCols[thisRow].RGBVal[1] = (thisColor & 0x00FF00) >> 8;
			ledData[workCfg].LEDCols[thisRow].RGBVal[2] = (thisColor & 0x0000FF);
			ledData[workCfg].LEDCols[thisRow].HSVVal = rgbToHsv(ledData[workCfg].LEDCols[thisRow].RGBVal[0],ledData[workCfg].LEDCols[thisRow].RGBVal[1],ledData[workCfg].LEDCols[thisRow].RGBVal[2]);
//			console.log(configData[2].LEDCols[thisRow]);
			break;
		case 3: //Manipulator buttons
			var idStr = sender.id;
			var thisElement;
			if (thisIndex == 1)
				ledData[workCfg].LEDCols.splice(thisRow+1, 0, JSON.parse(JSON.stringify(newColTemplate)));
			if (thisIndex == 2)
			{
				var noReference = true;
				var colName = ledData[workCfg].LEDCols[thisRow].Name;
//				console.log("Searching for: " + colName);
				for (var i = 0; i < ledData[workCfg].LEDDefs.length; i++)
				{
					for (var j = 0; j < ledData[workCfg].LEDDefs[i].LEDCmd.length; j++)
					{
						if (Array.isArray(ledData[workCfg].LEDDefs[i].LEDCmd[j].ColOn))
						{
							if (ledData[workCfg].LEDDefs[i].LEDCmd[j].ColOn.indexOf(colName) >= 0)
							{
								noReference = false;
								break;
							}
						}
						else
							if (ledData[workCfg].LEDDefs[i].LEDCmd[j].ColOn == colName)
							{
								noReference = false;
								break;
							}
						if (ledData[workCfg].LEDDefs[i].LEDCmd[j].ColOff != undefined)
							if (Array.isArray(ledData[workCfg].LEDDefs[i].LEDCmd[j].ColOn))
							{
								if (ledData[workCfg].LEDDefs[i].LEDCmd[j].ColOn.indexOf(colName) >= 0)
								{
									noReference = false;
									break;
								}
							}
							else
								if (ledData[workCfg].LEDDefs[i].LEDCmd[j].ColOn == colName)
								{
									noReference = false;
									break;
								}
					}
				}
				if (noReference)
					ledData[workCfg].LEDCols.splice(thisRow, 1);
				else
					alert(colName + " is currently used in the table below. Remove any reference before deleting this color!");
			}
			if ((thisIndex == 3) && (thisRow > 0))
			{
				thisElement = ledData[workCfg].LEDCols.splice(thisRow, 1);
				ledData[workCfg].LEDCols.splice(thisRow-1,0, thisElement[0]);
			}
			if ((thisIndex == 4) && (thisRow < ledData[workCfg].LEDCols.length))
			{
				thisElement = ledData[workCfg].LEDCols.splice(thisRow, 1);
				ledData[workCfg].LEDCols.splice(thisRow+1,0, thisElement[0]);
			}
//			console.log(configData[2].LEDCols);
			loadColorTable(colorTable, ledData[workCfg].LEDCols);
			break;
	}
}

function setLEDBasics(sender)
{
	if (sender.id == "colorseq")
		switch (sender.selectedIndex)
		{
			case 0: ledData[workCfg].ChainParams.ColorSeq = "RGB"; break;
			case 1: ledData[workCfg].ChainParams.ColorSeq = "GRB"; break;
		}
	if (sender.id == "blinkperiod")
		ledData[workCfg].ChainParams.BlinkPeriod = verifyNumber(sender.value, ledData[workCfg].ChainParams.BlinkPeriod);
	if (sender.id == "brightnessaddr")
		ledData[workCfg].ChainParams.Brightness.Addr = verifyNumber(sender.value, ledData[workCfg].ChainParams.Brightness.Addr);
	if (sender.id == "brightnesslevel")
		ledData[workCfg].ChainParams.Brightness.InitLevel = verifyNumber(sender.value/100, ledData[workCfg].ChainParams.Brightness.InitLevel);
	if (sender.id == "ledblock")
	{
		if (configData[workCfg].Modules[0].LEDPattern != sender.selectedIndex)
		{
			configData[workCfg].Modules[0].LEDPattern = sender.selectedIndex;
			var newLEDDefs = [];
			newLEDDefs.push(JSON.parse(JSON.stringify(ledData[workCfg].LEDDefs[0])));
			if (sender.selectedIndex == 0) //banks to continuous
				for (var i = 0; i < 16; i++)
				{
					newLEDDefs.push(JSON.parse(JSON.stringify(ledData[workCfg].LEDDefs[i+1])));
					newLEDDefs.push(JSON.parse(JSON.stringify(ledData[workCfg].LEDDefs[i+17])));
				}
			else //continuous to banks
			{
				for (var i = 0; i < 16; i++)
					newLEDDefs.push(JSON.parse(JSON.stringify(ledData[workCfg].LEDDefs[(2*i)+1])));
				for (var i = 0; i < 16; i++)
					newLEDDefs.push(JSON.parse(JSON.stringify(ledData[workCfg].LEDDefs[(2*i)+2])));
			}
			for (var i = 0; i < 33; i++)
				newLEDDefs[i].LEDNums = [i];
			console.log(ledData[workCfg].LEDDefs);
			console.log(newLEDDefs);
			
			ledData[workCfg].LEDDefs = JSON.parse(JSON.stringify(newLEDDefs));
			loadTableData(switchTable, swiCfgData[workCfg].Drivers, btnCfgData[workCfg].Buttons, evtHdlrCfgData[workCfg].ButtonHandler, ledData[workCfg].LEDDefs);
		}
	}
}

function adjustLEDAspectList(ledIndex, numAspects)
{
	while (ledData[workCfg].LEDDefs[ledIndex].LEDCmd.length > numAspects)
		ledData[workCfg].LEDDefs[ledIndex].LEDCmd.pop();
	while (ledData[workCfg].LEDDefs[ledIndex].LEDCmd.length < numAspects)
		ledData[workCfg].LEDDefs[ledIndex].LEDCmd.push(JSON.parse(JSON.stringify(newLEDCmdTemplate)));
	for (var i = 0; i < numAspects; i++)
		ledData[workCfg].LEDDefs[ledIndex].LEDCmd[i].Val = i;
}

function startTemplateDialog(parentObj, templateChannel)
{
	var mainDlg = document.createElement("div");
	mainDlg.setAttribute('class', "modal");
	
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
				level0Div.innerHTML = "Set channel characterstics based on settings of channel #" + templateChannel.toString();
				dlgTextDispArea.append(level0Div);

				level1Div = document.createElement("div"); //global var
				level1Div.setAttribute('style', "height: 50px; width:100%"); 
				dlgTextDispArea.append(level1Div);
				var tempField = createTextInput(level1Div, "tile-1_2", "Run from Channel:", templateChannel.toString(), "startchannel", "setTemplateParams(this)");
				tempField = createTextInput(level1Div, "tile-1_2", "to Channel:", Math.min(16, templateChannel+5).toString(), "endchannel", "setTemplateParams(this)");

				level2Div = document.createElement("div"); //global var
				level2Div.setAttribute('style', "height: 50px; width:100%"); 
				dlgTextDispArea.append(level2Div);
				createTextInput(level2Div, "tile-1_2", "Increment Servo Control Address by:", "1", "mainaddrincr", "setTemplateParams(this)");

				level3Div = document.createElement("div"); //global var
				level3Div.setAttribute('style', "height: 50px; width:100%"); 
				dlgTextDispArea.append(level3Div);
				createTextInput(level3Div, "tile-1_2", "Increment Button Address by:", "2", "btnaddrincr", "setTemplateParams(this)");
				createTextInput(level3Div, "tile-1_2", "Increment Button Event Address by:", "2", "evtaddrincr", "setTemplateParams(this)");

/*
				level4Div = document.createElement("div"); //global var
				level4Div.setAttribute('style', "height: 50px; width:100%"); 
				dlgTextDispArea.append(level4Div);
				createButton(level4Div, "", "Run Template", "btnRun", "runTemplate(this)");
				createButton(level4Div, "", "Cancel", "btnCancel", "cancelSettings(this)");
*/

			dlgSubDiv.append(dlgTextDispArea);
			var footerDiv = document.createElement("div");
			footerDiv.setAttribute('class', "modal-footer");
			dlgDiv.append(footerDiv);
			var dlgHeader = document.createElement("div");
				dlgHeader.setAttribute('style', "height: 70px; width:100%"); 
				dlgHeader.innerHTML = "&nbsp;";
				createButton(dlgHeader, "", "Run Template", "btnRun", "runTemplate(this)");
				createButton(dlgHeader, "", "Cancel", "btnCancel", "cancelSettings(this)");
			footerDiv.append(dlgHeader);

//				dlgHeader.innerHTML = "Thank you for using the IoTT Stick!";
	
	parentObj.append(mainDlg);
	var span = document.getElementsByClassName("close"); //get array of close elements, should only be 1
	span[0].onclick = function() {templateDlg.style.display = "none";}
	return mainDlg;
}

function setSwitchData(sender)
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
				case 11: //Event Type
					swiCfgData[workCfg].Drivers[thisRow].CmdSource = sourceOptionArray[sender.selectedIndex];
					adjustSourceSelector(swiCfgData[workCfg].Drivers, thisRow, thisCol);
					break;
				case 12: //Address
					var newRes = verifyNumArray(sender.value, ",");
					var srcMode = sourceOptionArray.indexOf(swiCfgData[workCfg].Drivers[thisRow].CmdSource);
					if ([0,1,5].indexOf(srcMode) >= 0) //limit the length to 3 for stat and dyn signals
						while (newRes.length > 3)
							newRes.pop();
					else
						while (newRes.length > 1) //limit to one for everything else
							newRes.pop();
					swiCfgData[workCfg].Drivers[thisRow].Addr = newRes; //verifyNumber(sender.value, swiCfgData[workCfg].Drivers[thisRow].Addr[0]);
					sender.value = newRes;
					adjustSourceSelector(swiCfgData[workCfg].Drivers, thisRow, thisCol);
					break;
				case 13: //select event
					swiCfgData[workCfg].Drivers[thisRow].CurrDisp = sender.selectedIndex;
					dispSwitchData(swiCfgData[workCfg].Drivers, thisRow);
					setAddr2Disp(sourceOptionArray.indexOf(swiCfgData[workCfg].Drivers[thisRow].CmdSource), sender.selectedIndex, thisRow, thisCol);
					break;
				case 14: //2nd addr
					var srcMode = sourceOptionArray.indexOf(swiCfgData[workCfg].Drivers[thisRow].CmdSource);
					if ([2,4,6].indexOf(srcMode) >= 0) //signal, analog, transponding
					{
						var newRes = verifyNumArray(sender.value, ",");
						if ([2,4].indexOf(srcMode) >= 0) //limit the length to 1 for analog and signal aspects
							while (newRes.length > 1)
								newRes.pop();
						if ([2].indexOf(srcMode) >= 0) //for signals, this is the AspVal
							swiCfgData[workCfg].Drivers[thisRow].Positions[swiCfgData[workCfg].Drivers[thisRow].CurrDisp].AspVal = newRes;
						else
							swiCfgData[workCfg].Drivers[thisRow].CondData = newRes;
						sender.value = newRes;
					}
					if (srcMode == 2) //signal
					{
						var sourceList= document.getElementById("cmdlistbox_" + thisRow.toString() + "_" + thisCol.toString());
						if (swiCfgData[workCfg].Drivers[thisRow].Positions[swiCfgData[workCfg].Drivers[thisRow].CurrDisp].AspVal != undefined)
							if (swiCfgData[workCfg].Drivers[thisRow].Positions[swiCfgData[workCfg].Drivers[thisRow].CurrDisp].AspVal != 0xFFFF)
								sourceList.options[sourceList.selectedIndex].text = "Aspect " + swiCfgData[workCfg].Drivers[thisRow].Positions[swiCfgData[workCfg].Drivers[thisRow].CurrDisp].AspVal.toString();
							else
								sourceList.options[sourceList.selectedIndex].text = "Aspect #" + swiCfgData[workCfg].Drivers[thisRow].CurrDisp.toString();
						else
							sourceList.options[sourceList.selectedIndex].text = "Aspect #" + swiCfgData[workCfg].Drivers[thisRow].CurrDisp.toString();
					}
					dispSwitchData(swiCfgData[workCfg].Drivers, thisRow);
					break;
				case 15: //New event
					swiCfgData[workCfg].Drivers[thisRow].Positions.splice(swiCfgData[workCfg].Drivers[thisRow].CurrDisp, 0, JSON.parse(JSON.stringify(newEventTemplate)));
					dispSwitchData(swiCfgData[workCfg].Drivers, thisRow);
					break;
				case 16: //delete event
					if (swiCfgData[workCfg].Drivers[thisRow].Positions.length > 0) //can't delete the last event
					{
						swiCfgData[workCfg].Drivers[thisRow].Positions.splice(swiCfgData[workCfg].Drivers[thisRow].CurrDisp, 1);
						if (swiCfgData[workCfg].Drivers[thisRow].CurrDisp > swiCfgData[workCfg].Drivers[thisRow].Positions.length - 1)
						{
							swiCfgData[workCfg].Drivers[thisRow].CurrDisp = (swiCfgData[workCfg].Drivers[thisRow].Positions.length - 1);
							var evtListBox = document.getElementById("cmdlistbox_" + thisRow.toString() + "_" + thisCol.toString());
							evtListBox.selectedIndex -= 1;
						}
						dispSwitchData(swiCfgData[workCfg].Drivers, thisRow);
//						console.log(swiCfgData[workCfg].Drivers[thisRow]);
					}
					break;
				case 25: //run template
					if (templateDlg ==  null)
						templateDlg = startTemplateDialog(document.getElementById("TabHolder"), thisRow+1);
					else
						level0Div.innerHTML = "Set channel characterstics based on settings of channel #" + (thisRow+1).toString();
					document.getElementById("temptitle").innerHTML = "Set channel characterstics based on settings of channel #" + (thisRow+1).toString();
					document.getElementById("startchannel").value = (thisRow+1).toString();
					document.getElementById("endchannel").value = Math.min(16, thisRow+6).toString();
					templateDlg.style.display = "block";
					break;
			}
			break;
		case 2:
			switch (thisIndex)
			{
				case 1: //Up Speed
					swiCfgData[workCfg].Drivers[thisRow].UpSpeed = verifyNumber(sender.value, swiCfgData[workCfg].Drivers[thisRow].UpSpeed);
					break;
				case 2: //Down Speed
					swiCfgData[workCfg].Drivers[thisRow].DownSpeed = verifyNumber(sender.value, swiCfgData[workCfg].Drivers[thisRow].DownSpeed);
					break;
				case 3: //Accel Rate
					swiCfgData[workCfg].Drivers[thisRow].AccelRate = verifyNumber(sender.value, swiCfgData[workCfg].Drivers[thisRow].AccelRate);
					break;
				case 4: //Decel rate
					swiCfgData[workCfg].Drivers[thisRow].DecelRate = verifyNumber(sender.value, swiCfgData[workCfg].Drivers[thisRow].DecelRate);
					break;
				case 5: //Frequency
					swiCfgData[workCfg].Drivers[thisRow].Frequency = verifyNumber(sender.value, swiCfgData[workCfg].Drivers[thisRow].Frequency);
					break;
				case 6: //Lambda
					swiCfgData[workCfg].Drivers[thisRow].Lambda = verifyNumber(sender.value, swiCfgData[workCfg].Drivers[thisRow].Lambda);
					break;
				case 7: //use Hesitation
					swiCfgData[workCfg].Drivers[thisRow].UseHesi = sender.checked;
					break;
				case 8: //Hes Point
					swiCfgData[workCfg].Drivers[thisRow].HesPoint = verifyNumber(sender.value, swiCfgData[workCfg].Drivers[thisRow].HesPoint);
					break;
				case 9: //Hes Speed
					swiCfgData[workCfg].Drivers[thisRow].HesSpeed = verifyNumber(sender.value, swiCfgData[workCfg].Drivers[thisRow].HesSpeed);
					break;
				case 10: //use this Aspect
					swiCfgData[workCfg].Drivers[thisRow].Positions[swiCfgData[workCfg].Drivers[thisRow].CurrDisp].Used = sender.checked;
					dispSwitchData(swiCfgData[workCfg].Drivers, thisRow);
					break;
				case 12: //Speed slider
					swiCfgData[workCfg].Drivers[thisRow].Positions[swiCfgData[workCfg].Drivers[thisRow].CurrDisp].PosPt = sender.value;
					dispSwitchData(swiCfgData[workCfg].Drivers, thisRow);
					break;
				case 13: //Speed input
					swiCfgData[workCfg].Drivers[thisRow].Positions[swiCfgData[workCfg].Drivers[thisRow].CurrDisp].PosPt = verifyNumber(sender.value, swiCfgData[workCfg].Drivers[thisRow].Positions[swiCfgData[workCfg].Drivers[thisRow].CurrDisp].PosPt);
					dispSwitchData(swiCfgData[workCfg].Drivers, thisRow);
					break;
				case 14: //Soft Start
					if (sender.checked)
						swiCfgData[workCfg].Drivers[thisRow].Positions[swiCfgData[workCfg].Drivers[thisRow].CurrDisp].MoveCfg |= 0x04;
					else
						swiCfgData[workCfg].Drivers[thisRow].Positions[swiCfgData[workCfg].Drivers[thisRow].CurrDisp].MoveCfg &= (~0x04);
					break;
				case 15: //Hard Stop
					swiCfgData[workCfg].Drivers[thisRow].Positions[swiCfgData[workCfg].Drivers[thisRow].CurrDisp].MoveCfg &= (~0x03);
					break;
				case 16: //Soft Stop
					swiCfgData[workCfg].Drivers[thisRow].Positions[swiCfgData[workCfg].Drivers[thisRow].CurrDisp].MoveCfg &= (~0x03);
					swiCfgData[workCfg].Drivers[thisRow].Positions[swiCfgData[workCfg].Drivers[thisRow].CurrDisp].MoveCfg |= 0x01;
					break;
				case 17: //Oscillate
					swiCfgData[workCfg].Drivers[thisRow].Positions[swiCfgData[workCfg].Drivers[thisRow].CurrDisp].MoveCfg &= (~0x03);
					swiCfgData[workCfg].Drivers[thisRow].Positions[swiCfgData[workCfg].Drivers[thisRow].CurrDisp].MoveCfg |= 0x02;
					break;
				case 18: //Bounce Back
					swiCfgData[workCfg].Drivers[thisRow].Positions[swiCfgData[workCfg].Drivers[thisRow].CurrDisp].MoveCfg |= 0x03;
					break;
			}
			break;
		case 3: 
			console.log("Set Input data", thisRow, thisCol, thisIndex);
			var dataRow = (2 * thisRow) + ((thisIndex & 0x100) >> 8);
			var maskIndex = (thisIndex & 0x00FF);
			switch (maskIndex)
			{
				case 10:
					btnCfgData[workCfg].Buttons[dataRow].ButtonAddr = verifyNumber(sender.value, btnCfgData[workCfg].Buttons[dataRow].ButtonAddr);
					evtHdlrCfgData[workCfg].ButtonHandler[dataRow].ButtonNr = btnCfgData[workCfg].Buttons[dataRow].ButtonAddr;
					break;
				case 11:
					btnCfgData[workCfg].Buttons[dataRow].currDisp = sender.selectedIndex;
					setButtonDisplay(btnCfgData[workCfg].Buttons[dataRow], evtHdlrCfgData[workCfg].ButtonHandler, thisRow, ((thisIndex & 0x100) >> 8));
					break;
				case 12:
					evtHdlrCfgData[workCfg].ButtonHandler[dataRow].CtrlCmd[btnCfgData[workCfg].Buttons[dataRow].currDisp].CmdList[0].CtrlTarget = cmdOptions[sender.selectedIndex];
					setButtonDisplay(btnCfgData[workCfg].Buttons[dataRow], evtHdlrCfgData[workCfg].ButtonHandler, thisRow, ((thisIndex & 0x100) >> 8));
					break;
				case 13:
				    var oldValue = evtHdlrCfgData[workCfg].ButtonHandler[dataRow].CtrlCmd[btnCfgData[workCfg].Buttons[dataRow].currDisp].CmdList[0].CtrlAddr;
					evtHdlrCfgData[workCfg].ButtonHandler[dataRow].CtrlCmd[btnCfgData[workCfg].Buttons[dataRow].currDisp].CmdList[0].CtrlAddr = verifyNumber(sender.value, oldValue); 
					break;
				case 14:
					evtHdlrCfgData[workCfg].ButtonHandler[dataRow].CtrlCmd[btnCfgData[workCfg].Buttons[dataRow].currDisp].CmdList[0].CtrlType = sender.options[sender.selectedIndex].text;
					if (evtHdlrCfgData[workCfg].ButtonHandler[dataRow].CtrlCmd[btnCfgData[workCfg].Buttons[dataRow].currDisp].CmdList[0].CtrlTarget == "block")
						evtHdlrCfgData[workCfg].ButtonHandler[dataRow].CtrlCmd[btnCfgData[workCfg].Buttons[dataRow].currDisp].CmdList[0].CtrlValue = Math.abs(sender.selectedIndex - 1);
					break;
				case 15:
					var oldValue = evtHdlrCfgData[workCfg].ButtonHandler[dataRow].CtrlCmd[btnCfgData[workCfg].Buttons[dataRow].currDisp].CmdList[0].CtrlValue;
					evtHdlrCfgData[workCfg].ButtonHandler[dataRow].CtrlCmd[btnCfgData[workCfg].Buttons[dataRow].currDisp].CmdList[0].CtrlValue = verifyNumber(sender.value, oldValue); 
					break;
				case 16:
					if (sender.checked)
						btnCfgData[workCfg].Buttons[dataRow].EventMask = 0x1F;
					else
						btnCfgData[workCfg].Buttons[dataRow].EventMask = 0x03;
					adjustHdlrEventList(evtHdlrCfgData[workCfg].ButtonHandler[dataRow], btnCfgData[workCfg].Buttons[dataRow].EventMask == 0x03 ? 2 : 5);
					setButtonDisplay(btnCfgData[workCfg].Buttons[dataRow], evtHdlrCfgData[workCfg].ButtonHandler, thisRow, ((thisIndex & 0x100) >> 8));
					break;
			}
			break;
		case 4: 
			console.log("Set LED data", thisRow, thisCol, thisIndex);
			var dataRow;
			if (configData[workCfg].Modules[0].LEDPattern == 0) //continuous
				dataRow = (2 * thisRow) + ((thisIndex & 0x100) >> 8) +1; //LED 0 not used for switches
			else //banked
				dataRow = thisRow + (16 * ((thisIndex & 0x100) >> 8)) +1; //LED 0 not used for switches
			var maskIndex = (thisIndex & 0x00FF);
			switch (maskIndex)
			{
				case 10:
					switch (sender.selectedIndex)
					{
						case 0: //Position
							//set source and address
							var swiSource = sourceOptionArray.indexOf(swiCfgData[workCfg].Drivers[thisRow].CmdSource);
							var swiAddr = swiCfgData[workCfg].Drivers[thisRow].Addr;
							ledData[workCfg].LEDDefs[dataRow].CtrlSource = ledOptionArray[swiSource];
							ledData[workCfg].LEDDefs[dataRow].CtrlAddr = swiAddr;
							//adjust length of aspects
							adjustLEDAspectList(dataRow, getOptionList("cmdlistbox_" + thisRow.toString() + "_1").length);
							//display LED
							break;
						case 1:
							var btn1Addr = btnCfgData[workCfg].Buttons[2*thisRow].ButtonAddr;
							ledData[workCfg].LEDDefs[dataRow].CtrlSource = ledOptionArray[3];
							ledData[workCfg].LEDDefs[dataRow].CtrlAddr = btn1Addr;
							adjustLEDAspectList(dataRow, getOptionList("cmdlistbox0_" + thisRow.toString() + "_3").length);
							break;
						case 2:
							var btn2Addr = btnCfgData[workCfg].Buttons[2*thisRow+1].ButtonAddr;
							ledData[workCfg].LEDDefs[dataRow].CtrlSource = ledOptionArray[3];
							ledData[workCfg].LEDDefs[dataRow].CtrlAddr = btn2Addr;
							adjustLEDAspectList(dataRow, getOptionList("cmdlistbox1_" + thisRow.toString() + "_3").length);
							break;
					}
					setLEDDisplay(ledData[workCfg].LEDDefs, thisRow, ((thisIndex & 0x100) >> 8));
					break;
				case 11:
					ledData[workCfg].LEDDefs[dataRow].currDisp = sender.selectedIndex;
					setLEDDisplay(ledData[workCfg].LEDDefs, thisRow, ((thisIndex & 0x100) >> 8));
					break;
				case 12:
					ledData[workCfg].LEDDefs[dataRow].LEDCmd[ledData[workCfg].LEDDefs[dataRow].currDisp].ColOn = sender.options[sender.selectedIndex].text;
					break;
				case 13:
					ledData[workCfg].LEDDefs[dataRow].LEDCmd[ledData[workCfg].LEDDefs[dataRow].currDisp].ColOff = sender.options[sender.selectedIndex].text;
					break;
				case 14:
					ledData[workCfg].LEDDefs[dataRow].LEDCmd[ledData[workCfg].LEDDefs[dataRow].currDisp].Mode = dispOptions[sender.selectedIndex];
					break;
				case 15:
					ledData[workCfg].LEDDefs[dataRow].LEDCmd[ledData[workCfg].LEDDefs[dataRow].currDisp].Rate = sender.value;
					break;
				case 16:
					ledData[workCfg].LEDDefs[dataRow].LEDCmd[ledData[workCfg].LEDDefs[dataRow].currDisp].Transition = transOptions[sender.selectedIndex];
					break;
			}
			break;
	}

}

function dispSwitchData(swiData, thisRow)
{
	var evtSrcBox = document.getElementById("evttypebox_" + thisRow.toString() + "_" + "1");
	evtSrcBox.selectedIndex = sourceOptionArray.indexOf(swiData[thisRow].CmdSource);
	var addrBox = document.getElementById("addressbox_" + thisRow.toString() + "_1");
	addrBox.value = swiData[thisRow].Addr;
//	console.log(swiData[thisRow].CurrDisp);
	adjustSourceSelector(swiData, thisRow, 1);
	document.getElementById("movespeedup_" + thisRow.toString() + "_" + "2").value = swiData[thisRow].UpSpeed;
	document.getElementById("movespeeddown_" + thisRow.toString() + "_" + "2").value = swiData[thisRow].DownSpeed;
	document.getElementById("accel_" + thisRow.toString() + "_" + "2").value = swiData[thisRow].AccelRate;
	document.getElementById("decel_" + thisRow.toString() + "_" + "2").value = swiData[thisRow].DecelRate;
	document.getElementById("lambda_" + thisRow.toString() + "_" + "2").value = swiData[thisRow].Lambda;
	document.getElementById("oscfrequ_" + thisRow.toString() + "_" + "2").value = swiData[thisRow].Frequency;
	document.getElementById("hesitate_" + thisRow.toString() + "_" + "2").checked = (swiData[thisRow].UseHesi > 0);
	document.getElementById("hesipoint_" + thisRow.toString() + "_" + "2").value = swiData[thisRow].HesPoint;
	document.getElementById("hesispeed_" + thisRow.toString() + "_" + "2").value = swiData[thisRow].HesSpeed;

	document.getElementById("enableevent_" + thisRow.toString() + "_" + "2").checked = swiData[thisRow].Positions[swiData[thisRow].CurrDisp].Used;
	document.getElementById("aspectpos_" + thisRow.toString() + "_" + "2").value = swiData[thisRow].Positions[swiData[thisRow].CurrDisp].PosPt;
	document.getElementById("posslider_" + thisRow.toString() + "_" + "2").value = swiData[thisRow].Positions[swiData[thisRow].CurrDisp].PosPt;
	writeCBInputField("softaccel_" + thisRow.toString()+ "_2", swiData[thisRow].Positions[swiData[thisRow].CurrDisp].MoveCfg & 0x04);
	writeRBInputField("stopmode_" + thisRow.toString() + "_2", swiData[thisRow].Positions[swiData[thisRow].CurrDisp].MoveCfg & 0x03);
	if (updateServoPos)
		if (swiData[thisRow].Positions[swiData[thisRow].CurrDisp].Used)
			setServoPos(thisRow, swiData[thisRow].Positions[swiData[thisRow].CurrDisp].PosPt);
	var enBounce = ((swiData[thisRow].CurrDisp == 0) || (swiData[thisRow].CurrDisp == (swiData[thisRow].Positions.length - 1)));
	setVisibility(enBounce, document.getElementById("stopmode_" + thisRow.toString() + "_" + "2_3"));
	setVisibility(enBounce, document.getElementById("stopmode_" + thisRow.toString() + "_" + "2_tx_3"));
}

function setButtonDisplay(btnData, btnEvtArray, thisRow, thisIndex)
{
	function evtByAddr(hdlrElement)
	{
		return (hdlrElement.ButtonNr == btnData.ButtonAddr);
	}
			
	function loadTargEvtOptions(CtrlCmdData)
	{
		var targetSel = document.getElementById("evttypebox" + thisIndex.toString() + "_" + thisRow.toString() + "_3");
		var targetAddrText = document.getElementById("targetaddrtext" + thisIndex.toString() + "_" + thisRow.toString() + "_3");
		var targAddrField = document.getElementById("targetaddressbox" + thisIndex.toString() + "_" + thisRow.toString() + "_3");
		var targEvtSel = document.getElementById("evtvalbox" + thisIndex.toString() + "_" + thisRow.toString() + "_3");
		var targEvtVal = document.getElementById("evtvalfield" + thisIndex.toString() + "_" + thisRow.toString() + "_3");
		var tatgetEvtText = document.getElementById("evtvaltext" + thisIndex.toString() + "_" + thisRow.toString() + "_3");
		
		if (CtrlCmdData.CmdList.length > 0)
		{
			targetSel.selectedIndex = cmdOptions.indexOf(CtrlCmdData.CmdList[0].CtrlTarget);
			targAddrField.value = CtrlCmdData.CmdList[0].CtrlAddr;
			if ([1].indexOf(cmdOptions.indexOf(CtrlCmdData.CmdList[0].CtrlTarget)) >= 0)
				tatgetEvtText.innerHTML = "&nbsp;Aspect:&nbsp";
			else
				tatgetEvtText.innerHTML = "&nbsp;Event:&nbsp";
			if ([0,1,2].indexOf(cmdOptions.indexOf(CtrlCmdData.CmdList[0].CtrlTarget)) >= 0)
			{
				setVisibility(true, targAddrField);
				setVisibility(true, targetAddrText);
			}
			else
			{
				setVisibility(false, targAddrField);
				setVisibility(false, targetAddrText);
			}
			if ([0,1,2,3].indexOf(cmdOptions.indexOf(CtrlCmdData.CmdList[0].CtrlTarget)) >= 0)
			{
				setVisibility(true, tatgetEvtText);
			}
			else
			{
				setVisibility(false, tatgetEvtText);
			}
			if ([0,2,3].indexOf(cmdOptions.indexOf(CtrlCmdData.CmdList[0].CtrlTarget)) >= 0)
				setVisibility(true, targEvtSel);
			else
				setVisibility(false, targEvtSel);
			if ([1].indexOf(cmdOptions.indexOf(CtrlCmdData.CmdList[0].CtrlTarget)) >= 0)
			{
				setVisibility(true, targEvtVal);
				targEvtVal.value = CtrlCmdData.CmdList[0].CtrlValue;
			}
			else
				setVisibility(false, targEvtVal);
		}
		else
		{
			targetSel.selectedIndex = -1;
			setVisibility(false, targetAddrText);
			setVisibility(false, targAddrField);
			setVisibility(false, targEvtSel);
			setVisibility(false, targEvtVal);
			setVisibility(false, tatgetEvtText);
		}
		var currCmdOptions = [];
		if (CtrlCmdData.CmdList.length > 0)
		{
			switch (cmdOptions.indexOf(CtrlCmdData.CmdList[0].CtrlTarget))
			{
				case 0: currCmdOptions = swiCmdOptions; break;
				case 1: break; //Signal, display aspect field
				case 2: currCmdOptions = blockCmdOptions; break;
				case 3: //local, display options selector
					currCmdOptions = getOptionList("cmdlistbox_" + thisRow.toString() + "_1");
					break;
				case 4: break; //none. Hide everything
			}
			createOptions(targEvtSel, currCmdOptions); 
			if (currCmdOptions != [])
				targEvtSel.selectedIndex = currCmdOptions.indexOf(CtrlCmdData.CmdList[0].CtrlType);
			else
				targEvtSel.selectedIndex = -1;
		}
	}
	
	if (btnData)
	{
		if (btnData.currDisp == undefined)
			btnData.currDisp = 0;
		var addrField = document.getElementById("btnaddressbox" + thisIndex.toString() + "_" + thisRow.toString() + "_3");
		addrField.value = btnData.ButtonAddr;
		var targetCmdSel = document.getElementById("cmdlistbox" + thisIndex.toString() + "_" + thisRow.toString() + "_3");
		var evtSelCB = document.getElementById("evtselbox" + thisIndex.toString() + "_" + thisRow.toString() + "_3");
		if (btnData.EventMask == 0x03)
		{
			createOptions(targetCmdSel, btnArray0x03);
			evtSelCB.checked = false;
		}
		else
		{
			btnData.EventMask == 0x1F;
			createOptions(targetCmdSel, btnArray); 
			evtSelCB.checked = true;
		}
		targetCmdSel.selectedIndex = btnData.currDisp;

		if (btnEvtArray)
		{
			var btnHdlrData = btnEvtArray[(2*thisRow) + thisIndex]; //.find(evtByAddr);
			if (btnHdlrData)
				loadTargEvtOptions(btnHdlrData.CtrlCmd[btnData.currDisp]);
		}
	}
}

function setLEDDisplay(ledArray, thisRow, thisIndex)
{
	function ledByPos(ledElement)
	{
		return (ledElement.LEDNums[0] == ledNr) && (ledElement.LEDNums.length == 1);
	}

	var ledNr;
	if (configData[workCfg].Modules[0].LEDPattern == 0)
		ledNr = (2*thisRow) + thisIndex + 1; //calculate continous LEDNr starting with 1
	else
		ledNr = thisRow + (16 * thisIndex) + 1; //calculate banked LEDNr starting with 1
	
	if (ledNr < ledArray.length)
	{
		var thisLED = ledArray[ledNr]; //.find(ledByPos);
		if (thisLED)
		{
			var swiSource = sourceOptionArray.indexOf(swiCfgData[workCfg].Drivers[thisRow].CmdSource);
			var swiAddr = swiCfgData[workCfg].Drivers[thisRow].Addr;
			var ledSource = ledOptionArray.indexOf(thisLED.CtrlSource);
			var ledAddr = thisLED.CtrlAddr;
			var btn1Addr = btnCfgData[workCfg].Buttons[2*thisRow].ButtonAddr;
			var btn2Addr = btnCfgData[workCfg].Buttons[2*thisRow+1].ButtonAddr;
//			console.log(swiSource, swiAddr, ledSource, ledAddr, btn1Addr, btn2Addr, thisIndex, thisRow);

			var evtSrcBox = document.getElementById("srclistbox" + thisIndex.toString() + "_" + thisRow.toString() + "_4");
			var evtDispBox = document.getElementById("evttypebox" + thisIndex.toString() + "_" + thisRow.toString() + "_4");
			var onColList = document.getElementById("oncolselbox" + thisIndex.toString() + "_" + thisRow.toString() + "_4");
			var offColList = document.getElementById("offcolselbox" + thisIndex.toString() + "_" + thisRow.toString() + "_4");
			var modeList = document.getElementById("blinkselectbox" + thisIndex.toString() + "_" + thisRow.toString() + "_4");
			var rateField = document.getElementById("blinkratebox" + thisIndex.toString() + "_" + thisRow.toString() + "_4");
			var transList = document.getElementById("transselectbox" + thisIndex.toString() + "_" + thisRow.toString() + "_4");
			var ledLabel = document.getElementById("ledlabel" + thisIndex.toString() + "_" + thisRow.toString() + "_4");
			ledLabel.innerHTML = "LED " + ledNr.toString() + " shows:&nbsp;";
			
			if ((ledSource == swiSource) && (ledAddr == swiAddr))
			{
				evtSrcBox.selectedIndex = 0;
				createOptions(evtDispBox, getOptionList("cmdlistbox_" + thisRow.toString() + "_1"));
			}
			else
				if ((ledSource == 3) && (ledAddr == btn1Addr))
				{
					evtSrcBox.selectedIndex = 1;
					createOptions(evtDispBox, getOptionList("cmdlistbox0_" + thisRow.toString() + "_3"));
				}
				else
					if ((ledSource == 3) && (ledAddr == btn2Addr))
					{
						evtSrcBox.selectedIndex = 2;
						createOptions(evtDispBox, getOptionList("cmdlistbox1_" + thisRow.toString() + "_3"));
					}
					else
					{
						evtSrcBox.selectedIndex = 0;
						ledData[workCfg].LEDDefs[ledNr].CtrlSource = ledOptionArray[swiSource];
						ledData[workCfg].LEDDefs[ledNr].CtrlAddr = swiAddr;
						createOptions(evtDispBox, getOptionList("cmdlistbox_" + thisRow.toString() + "_1"));
					}
		
			if (thisLED.currDisp == undefined)
				thisLED.currDisp = 0;

			if (thisLED.LEDCmd.length > thisLED.currDisp)
			{
				evtDispBox.selectedIndex = thisLED.currDisp;
				onColList.selectedIndex = colOptions.indexOf(thisLED.LEDCmd[thisLED.currDisp].ColOn);
				offColList.selectedIndex = colOptions.indexOf(thisLED.LEDCmd[thisLED.currDisp].ColOff);
				modeList.selectedIndex = dispOptions.indexOf(thisLED.LEDCmd[thisLED.currDisp].Mode);
				rateField.value = thisLED.LEDCmd[thisLED.currDisp].Rate;
				transList.selectedIndex = transOptions.indexOf(thisLED.LEDCmd[thisLED.currDisp].Transition);
			}
		}
	}
}

function loadColorTable(thisTable, thisData)
{
	function toHex(numbers) 
	{
		var r = numbers[0].toString(16), /* 1 */
			g = numbers[1].toString(16),
			b = numbers[2].toString(16);
		while (r.length < 2) {r = "0" + r};
		while (g.length < 2) {g = "0" + g};
		while (b.length < 2) {b = "0" + b};
		return "#" + r + g + b;
	}

	ledData[workCfg].ChainParams.NumLEDs = 33;
	ledData[workCfg].ChainParams.CtrlSource = "analog";
	switch (ledData[workCfg].ChainParams.ColorSeq)
	{
		case "RGB": setDropdownValue("colorseq", 0); break;
		case "GRB": setDropdownValue("colorseq", 1); break;
		default: setDropdownValue("colorseq", -1); break;
	}
	writeInputField("blinkperiod", ledData[workCfg].ChainParams.BlinkPeriod);
	writeInputField("brightnesslevel", 100 * ledData[workCfg].ChainParams.Brightness.InitLevel);
	writeInputField("brightnessaddr", ledData[workCfg].ChainParams.Brightness.Addr);
	setDropdownValue("ledblock", configData[workCfg].Modules[0].LEDPattern);
	
	var th = document.getElementById(thisTable.id + "_head");
	var tb = document.getElementById(thisTable.id + "_body");
	var numCols = th.childNodes[0].children.length;

	createDataTableLines(thisTable, [tfPos,tfNumericLong,tfColorPicker, tfManipulatorBox], thisData.length, "setColorData(this)");	

	for (var i=0; i<thisData.length;i++)
	{
		var e = document.getElementById(thisTable.id + "_" + i.toString() + "_" + "1");
		e.childNodes[0].value = thisData[i].Name;
		var e = document.getElementById(thisTable.id + "_" + i.toString() + "_" + "2");
		if (thisData[i].RGBVal == undefined)
		{
			thisData[i].RGBVal = HSVtoRGB(thisData[i].HSVVal[0], thisData[i].HSVVal[1], thisData[i].HSVVal[2]);
//			console.log(thisData[i]);
		}
		var colStr = toHex(thisData[i].RGBVal);
		e.childNodes[0].value = colStr;
	}
}

function loadTableData(thisTable, thisSwiData, thisBtnData, thisBtnEvtData, thisLEDData)
{
	var th = document.getElementById(thisTable.id + "_head");
	var tb = document.getElementById(thisTable.id + "_body");
	var numCols = th.childNodes[0].children.length;

	colOptions = [];
	if (thisLEDData)
		for (var j=0; j<ledData[workCfg].LEDCols.length;j++)
			colOptions.push(ledData[workCfg].LEDCols[j].Name);

	createDataTableLines(thisTable, [tfPos, tfCommandSwiSelector, tfServoEditor, tfButtonEditor, tfLEDEditor], thisSwiData.length, "setSwitchData(this)");
	for (var thisRow=0; thisRow < numChannels; thisRow++)
	{
		if ((thisSwiData != undefined) && (thisSwiData.length > thisRow))
		{
			if (isNaN(thisSwiData[thisRow].CurrDisp))
				thisSwiData[thisRow].CurrDisp = 0;
			dispSwitchData(thisSwiData, thisRow);
		}
		if ((thisBtnData != undefined) && (thisBtnData.length > (2*thisRow)))
		{
			for (var i=0; i < 2; i++)
				setButtonDisplay(thisBtnData[2*thisRow + i], thisBtnEvtData, thisRow, i);
		}
		if ((thisLEDData != undefined) && (thisLEDData.length > (2*thisRow))) //LEDDefs
		{
			for (var i=0; i < 2;i++)
			{
				var onColList = document.getElementById("oncolselbox" + i.toString() + "_" + thisRow.toString() + "_4");
				var offColList = document.getElementById("offcolselbox" + i.toString() + "_" + thisRow.toString() + "_4");
				createOptions(onColList, colOptions);
				createOptions(offColList, colOptions);
				setLEDDisplay(thisLEDData, thisRow, i);
			}
		}
	}
}

function setAddr2Disp(sourceMode, eventMode, thisRow, thisCol)
{
	var addrBox = document.getElementById("parambox_" + thisRow.toString() + "_1");
	var addrField = document.getElementById("address2box_" + thisRow.toString() + "_1");
	var addrText = 	document.getElementById("paramtext_" + thisRow.toString() + "_1");
	var btnAdd = document.getElementById("btn_add_" + thisRow.toString() + "_1");
	var btnCancel = document.getElementById("btn_cancel_" + thisRow.toString() + "_1");
	var srcMode = sourceOptionArray.indexOf(swiCfgData[workCfg].Drivers[thisRow].CmdSource);
	var dispField = false;
	setVisibility(false, btnAdd);
	setVisibility(false, btnCancel);
	switch (sourceMode) //event type
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
		case 3: //button
			break;
		case 4:  //analog
			dispField = [1,2].indexOf(eventMode) >= 0; 
			addrText.innerHTML = "Value:&nbsp;";
			break; 
		case 5: //block detector
			break; 
		case 6: //transponder
			dispField = true;  
			addrText.innerHTML = "Locos:&nbsp;"  
			break;
	}
	setVisibility(dispField, addrBox);
	if (dispField)
	{
		if ([4,6].indexOf(srcMode) >= 0) //analog, transponder
			if (!Array.isArray(swiCfgData[workCfg].Drivers[thisRow].CondData))
				addrField.value = "";
			else
				addrField.value = swiCfgData[workCfg].Drivers[thisRow].CondData;
		else //dcc signal
			if (isNaN(swiCfgData[workCfg].Drivers[thisRow].Positions[swiCfgData[workCfg].Drivers[thisRow].CurrDisp].AspVal))
				addrField.value = "";
			else
				if (swiCfgData[workCfg].Drivers[thisRow].Positions[swiCfgData[workCfg].Drivers[thisRow].CurrDisp].AspVal == 0xFFFF)
					addrField.value = "";
				else
					addrField.value = swiCfgData[workCfg].Drivers[thisRow].Positions[swiCfgData[workCfg].Drivers[thisRow].CurrDisp].AspVal;
	}
}

function adjustHdlrEventList(ofHandler, newLength)
{
	while (ofHandler.CtrlCmd.length > newLength) 
		ofHandler.CtrlCmd.splice(ofHandler.CtrlCmd.length-1, 1); //remove last element
	while (ofHandler.CtrlCmd.length < newLength) 
		ofHandler.CtrlCmd.push(JSON.parse(JSON.stringify(newCmdTemplate)));
}

function adjustEventList(ofSwitch, newLength)
{
	while (ofSwitch.Positions.length > newLength) 
		ofSwitch.Positions.splice(ofSwitch.Positions.length-1, 1); //remove last element
	while (ofSwitch.Positions.length < newLength) 
		ofSwitch.Positions.push(JSON.parse(JSON.stringify(newEventTemplate)));
}

function adjustSourceSelector(thisSwiData, thisRow, thisCol)
{
	var evtSrcBox = document.getElementById("evttypebox_" + thisRow.toString() + "_" + thisCol.toString());
	var evtListBox = document.getElementById("cmdlistbox_" + thisRow.toString() + "_" + thisCol.toString());
	evtSrcBox.selectedIndex = sourceOptionArray.indexOf(thisSwiData[thisRow].CmdSource);
	var numAddr = swiCfgData[workCfg].Drivers[thisRow].Addr.length;
	var optArray = [];
	var eventMode = evtListBox.selectedIndex; //get old settings
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
						resStr += swiCfgData[workCfg].Drivers[thisRow].Addr[p].toString() + " Cl ";
					else
						resStr += swiCfgData[workCfg].Drivers[thisRow].Addr[p].toString() + " Th ";
				}
				optArray.push(resStr);
			}
			createOptions(evtListBox, optArray); 
			break;
		case 1: //signaldyn
			for (var i = 0; i < numAddr; i ++)
			{
				optArray.push(swiCfgData[workCfg].Drivers[thisRow].Addr[i].toString() + " Th");
				optArray.push(swiCfgData[workCfg].Drivers[thisRow].Addr[i].toString() + " Cl");
			}
			createOptions(evtListBox, optArray); 
			break;
		case 2: //DCC Signal
			var aspCnt = thisSwiData[thisRow].Positions.length;
			if (aspCnt > 0)
				for (var i = 0; i < aspCnt; i++)
				{	
					if (thisSwiData[thisRow].Positions[thisSwiData[thisRow].CurrDisp] != undefined)
						if (thisSwiData[thisRow].Positions[i].AspVal != 0xFFFF)
							optArray.push("Aspect " + thisSwiData[thisRow].Positions[i].AspVal.toString());
						else
							optArray.push("Aspect #" + i.toString());
					else
						optArray.push("Aspect #" + i.toString());
				}
			else
				optArray.push("Aspect");
			createOptions(evtListBox, optArray); 
			break; 
		case 3: createOptions(evtListBox, ["Btn Down","Btn Up","Btn Click","Btn Hold", "Btn Dbl Click"]); break; //Button
		case 4: createOptions(evtListBox, [ "== 0", "<", ">=", "== max"]); break; //Analog
		case 5:  //Block detector
			for (var i = 0; i < Math.pow(2, numAddr); i ++)
			{
				var resStr = "";
				for (var p = 0; p < numAddr; p++)
				{
					if (((0x01 << p) & i) > 0)
						resStr += swiCfgData[workCfg].Drivers[thisRow].Addr[p].toString() + " Oc ";
					else
						resStr += swiCfgData[workCfg].Drivers[thisRow].Addr[p].toString() + " Fr ";
				}
				optArray.push(resStr);
			}
			createOptions(evtListBox, optArray); 
			break;
		case 6: createOptions(evtListBox, ["Enter", "Leave"]); break; //Transponder
		case 7: createOptions(evtListBox, ["On", "Off", "Idle"]); break; //power options??
	}
	if (eventMode >= evtListBox.options.length)
		eventMode = 0;
	//adjust length of event list array to new event list
	adjustEventList(thisSwiData[thisRow], evtListBox.options.length);
	//adjust visibility of address box 2
	evtListBox.selectedIndex = eventMode;
	thisSwiData[thisRow].CurrDisp = eventMode;
	setAddr2Disp(evtSrcBox.selectedIndex, eventMode, thisRow, thisCol);
}

function requestDataFiles()
{
	var moduleData = configData[workCfg].Modules[currDispPage];
	ws.send("{\"Cmd\":\"CfgData\", \"Type\":\"" + moduleData.CfgFiles[subFileIndex].ID + "\", \"FileName\":\"" + moduleData.CfgFiles[subFileIndex].FileName+ "\"}");
	subFileIndex++;
	if (subFileIndex < moduleData.CfgFiles.length)
		setTimeout(requestDataFiles, 1500);
}

function constructPageContent(contentTab)
{
	var tempObj;
	mainScrollBox = createEmptyDiv(contentTab, "div", "pagetopicboxscroll-y", "btnconfigdiv");
		createPageTitle(mainScrollBox, "div", "tile-1", "", "h1", "GreenHat Setup");
		createPageTitle(mainScrollBox, "div", "tile-1", "", "h2", "Module Settings");
		//LED Assignment pair vs group
		tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
			createPageTitle(tempObj, "div", "tile-1", "", "h3", "Board Settings");
			createDropdownselector(tempObj, "tile-1_4", "Color Sequence:", ["RGB", "GRB"], "colorseq", "setLEDBasics(this)");
		generalBox1 = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
			createTextInput(generalBox1, "tile-1_4", "System Blink Period:", "n/a", "blinkperiod", "setLEDBasics(this)");
			createDropdownselector(tempObj, "tile-1_4", "LED Assignment:", ["Continuous", "Blocks"], "ledblock", "setLEDBasics(this)");
		generalBox2 = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
			createTextInput(generalBox2, "tile-1_4", "Brightness Ctrl. Addr.:", "n/a", "brightnessaddr", "setLEDBasics(this)");
			createTextInput(generalBox2, "tile-1_4", "Initial Level [0-100%]:", "n/a", "brightnesslevel", "setLEDBasics(this)");
		//LED Colors
		colorTableDiv = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
		createPageTitle(colorTableDiv, "div", "tile-1", "", "h3", "LED Color Definitions");
		colorTable = createDataTable(colorTableDiv, "tile-1_2", ["Pos","Color Name", "Select Color", "Add/Delete/Move Color"], "colorconfig", "");
		createPageTitle(mainScrollBox, "div", "tile-1", "", "h2", "Channel Settings");

		switchTable = createDataTable(mainScrollBox, "tile-1", ["Pos","Input Event Selector", "Servo Movement", "Input Setup", "LED Setup"], "swiconfig", "");

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
	configData[workCfg] = upgradeJSONVersionGH(jsonData);
	subFileIndex = 0;
	swiCfgData = [{},{},{}];
	btnCfgData = [{},{},{}];
	evtHdlrCfgData = [{},{},{}];
	ledData = [{},{},{}];
	requestDataFiles();
}

function processLocoNetInput(jsonData)
{
}
