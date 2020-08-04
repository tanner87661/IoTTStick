var mainScrollBox;
var colorTable;
var ledDefTable;

var numColors = 0;

var i2cMaxNum = 525;

var ledCtrlType = ["block", "switch", "signal", "signalstat", "signaldyn", "button", "analog", "power", "constant"];
var ledModeType = ["static", "localblinkpos", "localblinkneg", "globalblinkpos", "globalblinkneg", "localrampup", "localrampdown", "globalrampup", "globalrampdown"];
var ledTransitionType = ["soft", "direct", "merge"];
var trackPwrType = ["off", "on", "idle"];
var btnType = ["onbtndown", "onbtnup", "onbtnclick", "onbtnhold", "onbtndlclick"];
var dispType = ["discrete","linear"];

var ledModeDispType = ["Static", "Pos Local Blink", "Neg Local Blink", "Pos Global Blink",  "Neg Global Blink", "Local Rampup", "Local Rampdown", "Global Rampup", "Global Rampdown"];
var ledDispType = ["discrete", "linear"];
var ledTransitionDispType = ["Soft", "Direct", "Merge"];
var blockValDispType = ["free","occupied"];
var switchValDispType = ["Thrown, Coil On", "Thrown, Coil Off", "Closed, Coil On", "Closed, Coil Off"];
var buttonValDispType = ["Btn Down", "Btn Up", "Btn Click", "Btn Hold", "Btn Dbl Click"];
var powerValDispType = ["Off", "On", "Idle"];

var newColTemplate = {"Name": "New Color","RGBVal": [255, 255, 255]};
var newLEDTemplate = {"CtrlSource": "switch","CtrlAddr": [1], "DisplayType":"discrete", "MultiColor":false, "LEDCmd": []};
var newCmdTemplate = {"Val": 0,	"ColOn": "", "Mode": "static","Rate":0,	"Transition":"soft"};

jsonFileVersion = "1.1.0";

function getMaximumLED()
{
	switch (nodeConfigData.HatTypeList[nodeConfigData.HatIndex].HatId)
	{
		case 3: return i2cMaxNum; break; //YellowHat
		default: return 0xFFFF;
	}
}

function upgradeJSONVersion(jsonData)
{
	var thisVersion = jsonData.Version;
	console.log(thisVersion);
	if (thisVersion == jsonFileVersion)
		return;
	if ((isNaN(thisVersion)) || (thisVersion = undefined))
	{
		//upgrade from noversion to 1.1.0
		console.log("upgrade from noversion to ", jsonFileVersion);
		jsonData.Version = jsonFileVersion;
	}
}

function addFileSeq(ofObj) //object specific function to include partial files
{
//	console.log(ofObj);
	for (var j=0; j<ofObj.LEDDefs.length; j++)
	{
		configLoadData.LEDDefs.push(JSON.parse(JSON.stringify(ofObj.LEDDefs[j])));
		configWorkData.LEDDefs.push(JSON.parse(JSON.stringify(ofObj.LEDDefs[j])));
	}
}

function prepareFileSeq() //object specific function to create partial files
{
	function addEntry()
	{
		var newEntry = {"LEDDefs":[]}
		transferData.FileList.push(newEntry);
		return newEntry;
	}
	
	var thisEntry = addEntry();
	thisEntry.Version = jsonFileVersion;
	thisEntry.ChainParams = JSON.parse(JSON.stringify(configWorkData.ChainParams));
	thisEntry.LEDCols = JSON.parse(JSON.stringify(configWorkData.LEDCols));
	var thisFileLength = 0;
	
	for (var j=0; j<configWorkData.LEDDefs.length;j++)
	{
		var thisElementStr = JSON.stringify(configWorkData.LEDDefs[j]);
		thisFileLength += thisElementStr.length;
		thisEntry.LEDDefs.push(JSON.parse(thisElementStr));
		if ((thisFileLength > targetSize) && (j < (configWorkData.LEDDefs.length - 1)))
		{
			thisEntry = addEntry();
			thisFileLength = 0;
		}
	}
	console.log(transferData);
}

function downloadSettings(sender)
{
	downloadConfig(0x0020); //send just this
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


function tfColorTypeSel(y, x, id, evtHandler)
{
	var selectList = document.createElement("select");
	createOptions(selectList, ["RGB", "HSV"]);
	tfSetCoordinate(selectList, y, x, 0, id);
	selectList.setAttribute("class", "selectorbox");
	selectList.setAttribute("onchange", evtHandler);
	return selectList;
}

function tfLEDCtrlTypeSel(y, x, id, evtHandler)
{
	var selectList = document.createElement("select");
	createOptions(selectList, ["Block Detector", "Switch", "DCC Signal", "Static Signal", "Dynamic Signal", "Button", "Analog Input", "Power Status", "Steady ON"]);
	tfSetCoordinate(selectList, y, x, 0, id);
	selectList.setAttribute("class", "selectorbox");
	selectList.setAttribute("onchange", evtHandler);
	return selectList;
}


function tfLEDDispTypeSel(y, x, id, evtHandler)
{
	var selectList = document.createElement("select");
	createOptions(selectList, ["discrete", "linear"]);
	tfSetCoordinate(selectList, y, x, 0, id);
	selectList.setAttribute("class", "selectorbox");
	selectList.setAttribute("onchange", evtHandler);
	return selectList;
}

function tfLEDDispSel(y, x, id, evtHandler)
{
	var selectList = document.createElement("select");
	createOptions(selectList, ["discrete", "linear"]);
	tfSetCoordinate(selectList, y, x, 0, id);
	selectList.setAttribute("class", "selectorbox");
	selectList.setAttribute("onchange", evtHandler);
	return selectList;
}

function tfLEDAddrSel(y, x, id, evtHandler, addrList)
{
	var selectList = document.createElement("select");
	var addrArray = [];
	for (var i = 0; i < addrList.length; i++)
		addrArray.push(addrList[i].toString());
	createOptions(selectList, addrArray);
	tfSetCoordinate(selectList, y, x, 0, id);
	selectList.setAttribute("class", "selectorbox");
	selectList.setAttribute("onchange", evtHandler);
	return selectList;
}

function setLEDBasics(sender)
{
//	if (sender.id == "chaintype")
//		switch (sender.selectedIndex)
//		{
//			case 0: configWorkData.ChainParams.ChainType = "WS2801"; break;
//			case 1: configWorkData.ChainParams.ChainType = "WS2812"; break;
//		}
	if (sender.id == "colorseq")
		switch (sender.selectedIndex)
		{
			case 0: configWorkData.ChainParams.ColorSeq = "RGB"; break;
			case 1: configWorkData.ChainParams.ColorSeq = "GRB"; break;
		}
//	console.log(configWorkData.ChainParams.ColorSeq);
	if (sender.id == "numleds")
	{
		var newNum = verifyNumber(sender.value, configWorkData.NumLEDs);
		if (newNum > getMaximumLED())
		{
			sender.value = i2cMaxNum;
			alert("Maximum number of LED's is limited to " + i2cMaxNum.toString() + " for this hat type!");
		}
		else
			configWorkData.ChainParams.NumLEDs = newNum;
	}
	if (sender.id == "blinkperiod")
		configWorkData.ChainParams.BlinkPeriod = verifyNumber(sender.value, configWorkData.ChainParams.BlinkPeriod);
	if (sender.id == "brightnessctrl")
		configWorkData.ChainParams.Brightness.CtrlSource = sender.value;
	if (sender.id == "brightnessaddr")
		configWorkData.ChainParams.Brightness.Addr = verifyNumber(sender.value, configWorkData.ChainParams.Brightness.Addr);
	if (sender.id == "brightnesslevel")
		configWorkData.ChainParams.Brightness.InitLevel = verifyNumber(sender.value, configWorkData.ChainParams.Brightness.InitLevel);
}

function HSVtoRGB(hue, sat, val) 
{
	var h = hue / 255; //scaling from [0,255] to [0,1] interval
	var s = sat / 255;
	var v = val / 255;

    var r, g, b, i, f, p, q, t;
    if (arguments.length === 1) {
        s = h.s, v = h.v, h = h.h;
    }
    i = Math.floor(h * 6);
    f = h * 6 - i;
    p = v * (1 - s);
    q = v * (1 - f * s);
    t = v * (1 - (1 - f) * s);
    switch (i % 6) {
        case 0: r = v, g = t, b = p; break;
        case 1: r = q, g = v, b = p; break;
        case 2: r = p, g = v, b = t; break;
        case 3: r = p, g = q, b = v; break;
        case 4: r = t, g = p, b = v; break;
        case 5: r = v, g = p, b = q; break;
    }
    return [Math.round(r * 255), Math.round(g * 255), Math.round(b * 255)];
}

function rgbToHsv(r, g, b) 
{
	r /= 255, g /= 255, b /= 255;

	var max = Math.max(r, g, b), min = Math.min(r, g, b);
	var h, s, v = max;

	var d = max - min;
	s = max == 0 ? 0 : d / max;

	if (max == min) 
	{
		h = 0; // achromatic
	} 
	else 
	{
		switch (max) 
		{
			case r: h = (g - b) / d + (g < b ? 6 : 0); break;
			case g: h = (b - r) / d + 2; break;
			case b: h = (r - g) / d + 4; break;
		}
		h /= 6;
	}
	return [Math.round(h*255), Math.round(s*255), Math.round(v*255)];
}

function findColorByName(thisArray, colName)
{
	function getCol(thisElement)
	{
//		console.log(thisElement);
		return (thisElement.Name == colName);
	}
	
	var thisIndex = thisArray.LEDCols.findIndex(getCol);
	return (thisIndex >= 0 ? thisArray.LEDCols[thisIndex] : null);
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
			configWorkData.LEDCols.push(JSON.parse(JSON.stringify(newColTemplate)));
			loadColorTable(colorTable, configWorkData.LEDCols);
			break;
		case 1: //Color Name
			configWorkData.LEDCols[thisRow].Name = sender.value;
			break;
		case 2: //Color RGB Value
			var thisColorStr = sender.value.replace("#", "0x");
			var thisColor = parseInt(thisColorStr);
			configWorkData.LEDCols[thisRow].RGBVal[0] = (thisColor & 0xFF0000) >> 16;
			configWorkData.LEDCols[thisRow].RGBVal[1] = (thisColor & 0x00FF00) >> 8;
			configWorkData.LEDCols[thisRow].RGBVal[2] = (thisColor & 0x0000FF);
			configWorkData.LEDCols[thisRow].HSVVal = rgbToHsv(configWorkData.LEDCols[thisRow].RGBVal[0],configWorkData.LEDCols[thisRow].RGBVal[1],configWorkData.LEDCols[thisRow].RGBVal[2]);
//			console.log(configWorkData.LEDCols[thisRow]);
			break;
		case 3: //Manipulator buttons
			var idStr = sender.id;
			var thisElement;
			if (thisIndex == 1)
				configWorkData.LEDCols.splice(thisRow+1, 0, JSON.parse(JSON.stringify(newColTemplate)));
			if (thisIndex == 2)
			{
				var noReference = true;
				var colName = configWorkData.LEDCols[thisRow].Name;
//				console.log("Searching for: " + colName);
				for (var i = 0; i < configWorkData.LEDDefs.length; i++)
				{
					for (var j = 0; j < configWorkData.LEDDefs[i].LEDCmd.length; j++)
					{
						if (Array.isArray(configWorkData.LEDDefs[i].LEDCmd[j].ColOn))
						{
							if (configWorkData.LEDDefs[i].LEDCmd[j].ColOn.indexOf(colName) >= 0)
							{
								noReference = false;
								break;
							}
						}
						else
							if (configWorkData.LEDDefs[i].LEDCmd[j].ColOn == colName)
							{
								noReference = false;
								break;
							}
						if (configWorkData.LEDDefs[i].LEDCmd[j].ColOff != undefined)
							if (Array.isArray(configWorkData.LEDDefs[i].LEDCmd[j].ColOn))
							{
								if (configWorkData.LEDDefs[i].LEDCmd[j].ColOn.indexOf(colName) >= 0)
								{
									noReference = false;
									break;
								}
							}
							else
								if (configWorkData.LEDDefs[i].LEDCmd[j].ColOn == colName)
								{
									noReference = false;
									break;
								}
					}
				}
				if (noReference)
					configWorkData.LEDCols.splice(thisRow, 1);
				else
					alert(colName + " is currently used in the table below. Remove any reference before deleting this color!");
			}
			if ((thisIndex == 3) && (thisRow > 0))
			{
				thisElement = configWorkData.LEDCols.splice(thisRow, 1);
				configWorkData.LEDCols.splice(thisRow-1,0, thisElement[0]);
			}
			if ((thisIndex == 4) && (thisRow < configWorkData.LEDCols.length))
			{
				thisElement = configWorkData.LEDCols.splice(thisRow, 1);
				configWorkData.LEDCols.splice(thisRow+1,0, thisElement[0]);
			}
//			console.log(configWorkData.LEDCols);
			loadColorTable(colorTable, configWorkData.LEDCols);
			break;
	}
}

function setLEDData(sender)
{
	function adjustCmdLines(currentRow)
	{
		var numCmds;
		var numAddr = 1;
		if (Array.isArray(configWorkData.LEDDefs[currentRow].CtrlAddr))
			numAddr = configWorkData.LEDDefs[currentRow].CtrlAddr.length;
		switch (ledCtrlType.indexOf(configWorkData.LEDDefs[currentRow].CtrlSource))
		{
			case 0 : numCmds = 2; break; //block
			case 1 : numCmds = 4; break; //switch
			case 2 : numCmds = 1; break; //signal
			case 3 : numCmds = Math.pow(2, numAddr); break; //signalstat
			case 4 : numCmds = 2 * numAddr; break; //signaldyn
			case 5 : numCmds = 5; break; //button
			case 6 : numCmds = 1; break; //analog
			case 7 : numCmds = 3; break; //power
			case 8 : numCmds = 1; break; //constant
		}
		while (configWorkData.LEDDefs[currentRow].LEDCmd.length < numCmds)
			configWorkData.LEDDefs[currentRow].LEDCmd.push(JSON.parse(JSON.stringify(newCmdTemplate)));
		while (configWorkData.LEDDefs[currentRow].LEDCmd.length > numCmds)
			configWorkData.LEDDefs[currentRow].LEDCmd.splice(configWorkData.LEDDefs[currentRow].LEDCmd.length-1, 1);
	}
	
	function adjustColorEntries(currentRow)
	{
		if (configWorkData.LEDDefs[currentRow].MultiColor)
		{
			var numCols = configWorkData.LEDDefs[currentRow].LEDNums.length;
			for (var i = 0; i < configWorkData.LEDDefs[currentRow].LEDCmd.length; i++)
			{	//for each command
				var oldColOn = configWorkData.LEDDefs[currentRow].LEDCmd[i].ColOn;
				if (typeof oldColOn == "object")
//				if (Array.isArray(oldColOn))
					oldColOn = oldColOn[0];
				var oldColOff = configWorkData.LEDDefs[currentRow].LEDCmd[i].ColOff;
				if (oldColOff == undefined)
					oldColOff = "";
				if (Array.isArray(oldColOff))
					oldColOff = oldColOff[0];
				var oldMode = configWorkData.LEDDefs[currentRow].LEDCmd[i].Mode;
				if (Array.isArray(oldMode))
					oldMode = oldMode[0];
				var oldRate = configWorkData.LEDDefs[currentRow].LEDCmd[i].Rate;
				if (Array.isArray(oldRate))
					oldRate = oldRate[0];
				var oldTransition = configWorkData.LEDDefs[currentRow].LEDCmd[i].Transition;
				if (Array.isArray(oldTransition))
					oldTransition = oldTransition[0];
				configWorkData.LEDDefs[currentRow].LEDCmd[i].ColOn = [];
				configWorkData.LEDDefs[currentRow].LEDCmd[i].ColOff = [];
				configWorkData.LEDDefs[currentRow].LEDCmd[i].Mode = [];
				configWorkData.LEDDefs[currentRow].LEDCmd[i].Rate = [];
				configWorkData.LEDDefs[currentRow].LEDCmd[i].Transition = [];
				for (var j = 0; j < numCols; j++)
				{
					configWorkData.LEDDefs[currentRow].LEDCmd[i].ColOn.push(oldColOn);
					configWorkData.LEDDefs[currentRow].LEDCmd[i].ColOff.push(oldColOff);
					configWorkData.LEDDefs[currentRow].LEDCmd[i].Mode.push(oldMode);
					configWorkData.LEDDefs[currentRow].LEDCmd[i].Rate.push(oldRate);
					configWorkData.LEDDefs[currentRow].LEDCmd[i].Transition.push(oldTransition);
				}
			}
		}
		else
		{
			for (var i = 0; i < configWorkData.LEDDefs[currentRow].LEDCmd.length; i++)
			{	//for each command
				var oldColOn = configWorkData.LEDDefs[currentRow].LEDCmd[i].ColOn;
//				console.log(typeof oldColOn);
				if (typeof oldColOn == "object")
					oldColOn = configWorkData.LEDDefs[currentRow].LEDCmd[i].ColOn[0];
				var oldColOff;
				try
				{
					oldColOff = configWorkData.LEDDefs[currentRow].LEDCmd[i].ColOff;
					if (typeof oldColOff == "object")
						oldColOff = configWorkData.LEDDefs[currentRow].LEDCmd[i].ColOff[0];
				}
				catch(err) 
				{
					oldColOff = "";
				}
				configWorkData.LEDDefs[currentRow].LEDCmd[i].ColOn = oldColOn;
				configWorkData.LEDDefs[currentRow].LEDCmd[i].ColOff = oldColOff;
				configWorkData.LEDDefs[currentRow].LEDCmd[i].Mode.splice(1, configWorkData.LEDDefs[currentRow].LEDCmd[i].Mode.length-1);
				configWorkData.LEDDefs[currentRow].LEDCmd[i].Rate.splice(1, configWorkData.LEDDefs[currentRow].LEDCmd[i].Rate.length-1);
				configWorkData.LEDDefs[currentRow].LEDCmd[i].Transition.splice(1, configWorkData.LEDDefs[currentRow].LEDCmd[i].Transition.length-1);
			}
		}
	}
	
	var thisRow = parseInt(sender.getAttribute("row"));
	var thisCol = parseInt(sender.getAttribute("col"));
	var thisIndex = parseInt(sender.getAttribute("index"));
//	console.log(thisRow, thisCol, thisIndex);
	switch (thisCol)
	{
		case -1: //empty table, create first entry
			configWorkData.LEDDefs.push(JSON.parse(JSON.stringify(newLEDTemplate)));
			loadLEDTable(ledDefTable, configWorkData.LEDDefs);
			break;
		case 1: //CommandSelector
			var idStr = sender.id;
			var thisElement;
			if (thisIndex == 1)
				if (event.ctrlKey) //duplicate entry
					configWorkData.LEDDefs.splice(thisRow+1, 0, JSON.parse(JSON.stringify(configWorkData.LEDDefs[thisRow])));
				else //create new entry
					configWorkData.LEDDefs.splice(thisRow+1, 0, JSON.parse(JSON.stringify(newLEDTemplate)));
			if (thisIndex == 2)
				configWorkData.LEDDefs.splice(thisRow, 1);
			if ((thisIndex == 3) && (thisRow > 0))
			{
				thisElement = configWorkData.LEDDefs.splice(thisRow, 1);
				configWorkData.LEDDefs.splice(thisRow-1,0, thisElement[0]);
			}
			if ((thisIndex == 4) && (thisRow < configWorkData.LEDDefs.length))
			{
				thisElement = configWorkData.LEDDefs.splice(thisRow, 1);
				configWorkData.LEDDefs.splice(thisRow+1,0, thisElement[0]);
			}
			if (thisIndex == 5)
			{
				var newArray = sender.value.split(',');
				if ((newArray.length > 0) && newArray.findIndex(function xNum(thisNum){return isNaN(thisNum)}) < 0)
				{
					if (newArray.length > 1)
					{
						configWorkData.LEDDefs[thisRow].LEDNums = []; //make sure this is an array
						for (var i = 0; i < newArray.length; i++)
							configWorkData.LEDDefs[thisRow].LEDNums.push(newArray[i]);
						adjustColorEntries(thisRow);
					}
					else //==1
					{
						configWorkData.LEDDefs[thisRow].LEDNums = [newArray[0]];
						configWorkData.LEDDefs[thisRow].MultiColor = false;
					}
				}
				else
					alert(sender.value + " is not a valid number or array. Please verify");
			}
			if (thisIndex == 6)
			{
				configWorkData.LEDDefs[thisRow].MultiColor = sender.checked; 
				adjustColorEntries(thisRow);
			}

			if (thisIndex == 7)
			{
				var oldSource = configWorkData.LEDDefs[thisRow].CtrlSource;
				configWorkData.LEDDefs[thisRow].CtrlSource = ledCtrlType[sender.selectedIndex];
				if (configWorkData.LEDDefs[thisRow].CtrlSource != oldSource)
					adjustCmdLines(thisRow);
			}
			if (thisIndex == 8)
			{
				var newArray = sender.value.split(',');
				if ((newArray.length > 0) && newArray.findIndex(function xNum(thisNum){return isNaN(thisNum)}) < 0)
				{
					var oldLen = Array.isArray(configWorkData.LEDDefs[thisRow].CtrlAddr) ? configWorkData.LEDDefs[thisRow].CtrlAddr.length : 1;
					if (newArray.length > 1)
					{
						configWorkData.LEDDefs[thisRow].CtrlAddr = []; //make sure this is an array
						for (var i = 0; i < newArray.length; i++)
							configWorkData.LEDDefs[thisRow].CtrlAddr.push(newArray[i]);
					}
					else
						configWorkData.LEDDefs[thisRow].CtrlAddr = newArray[0];
					if (newArray.length != oldLen)
						adjustCmdLines(thisRow);
				}
				else
					alert(sender.value + " is not a valid number or array. Please verify");
			}
			if (thisIndex == 9)
				configWorkData.LEDDefs[thisRow].DisplayType = dispType[sender.selectedIndex];
			if (thisIndex == 21)
				setLEDTestDisplay(configWorkData.LEDDefs[thisRow].LEDNums);
			if (thisIndex < 10)
				loadLEDTable(ledDefTable, configWorkData.LEDDefs);
			break;
	}
}

function tfCmdLineHeader(i, evtHandler)
{
	var thisEditor = document.createElement("div");
	thisEditor.append(tfNumericLong(i, evtHandler));
	return thisEditor;
}

function tfCmdLineEditor(i, evtHandler)
{
	var thisEditor = document.createElement("div");
	thisEditor.append(tfNumericLong(i, evtHandler));
	thisEditor.append(tfBtnAdd(i, evtHandler));
	thisEditor.append(tfBtnCancel(i, evtHandler));

	return thisEditor;
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

function setLEDCmdData(sender)
{
	var thisRow = parseInt(sender.getAttribute("row"));
	var thisCol = parseInt(sender.getAttribute("col"));
	var thisIndex = parseInt(sender.getAttribute("index"));
	var thisCmdLine = parseInt(sender.getAttribute("cmdline"));
	if (isNaN(thisCmdLine))
		thisCmdLine = parseInt(sender.parentElement.getAttribute("cmdline"));
	var thisElement;
	var thisID;
//	console.log("setLEDCmdData", thisRow, thisCol, thisIndex, thisCmdLine, sender.id, configWorkData.LEDDefs[thisRow]);
	switch (thisIndex)
	{
		case 1: //new Cmd Entry
			configWorkData.LEDDefs[thisRow].LEDCmd.splice(thisCmdLine+1, 0, JSON.parse(JSON.stringify(newCmdTemplate)));
			loadLEDTable(ledDefTable, configWorkData.LEDDefs);
			break;
		case 2: //delete Cmd Entry
			configWorkData.LEDDefs[thisRow].LEDCmd.splice(thisCmdLine, 1);
			loadLEDTable(ledDefTable, configWorkData.LEDDefs);
			break;
		case 3: //Up
			if (thisCmdLine > 0)
			{
				thisElement = configWorkData.LEDDefs[thisRow].LEDCmd.splice(thisCmdLine, 1);
				configWorkData.LEDDefs[thisRow].LEDCmd.splice(thisCmdLine-1,0, thisElement[0]);
			}
			loadLEDTable(ledDefTable, configWorkData.LEDDefs);
			break;
		case 4: //Down
			if (thisCmdLine < configWorkData.LEDDefs[thisRow].LEDCmd.length)
			{
				thisElement = configWorkData.LEDDefs[thisRow].LEDCmd.splice(thisCmdLine, 1);
				configWorkData.LEDDefs[thisRow].LEDCmd.splice(thisCmdLine+1,0, thisElement[0]);
			}
			loadLEDTable(ledDefTable, configWorkData.LEDDefs);
			break;
		case 6: //value
			configWorkData.LEDDefs[thisRow].LEDCmd[thisCol].Val = verifyNumber(sender.value, configWorkData.LEDDefs[thisRow].LEDCmd[thisCol].Val);
			break;
		case 8: //oncolor
			if (configWorkData.LEDDefs[thisRow].MultiColor)
			{
				thisID = "ledselector" + thisRow.toString() + "_" + thisCmdLine.toString();
				var e = document.getElementById(thisID);
				var currSel = e.selectedIndex;
				configWorkData.LEDDefs[thisRow].LEDCmd[thisCol].ColOn[currSel] = configWorkData.LEDCols[sender.selectedIndex].Name;
			}
			else
				configWorkData.LEDDefs[thisRow].LEDCmd[thisCol].ColOn = configWorkData.LEDCols[sender.selectedIndex].Name;
			break;
		case 9: //offcolor
			if (configWorkData.LEDDefs[thisRow].MultiColor)
			{
				thisID = "ledselector" + thisRow.toString() + "_" + thisCmdLine.toString();
				var e = document.getElementById(thisID);
				var currSel = e.selectedIndex;
				configWorkData.LEDDefs[thisRow].LEDCmd[thisCol].ColOff[currSel] = configWorkData.LEDCols[sender.selectedIndex].Name;
			}
			else
			{
				configWorkData.LEDDefs[thisRow].LEDCmd[thisCol].ColOff = configWorkData.LEDCols[sender.selectedIndex].Name;
			}
			break;
		case 10: //Mode
			if (configWorkData.LEDDefs[thisRow].MultiColor)
			{
				thisID = "ledselector" + thisRow.toString() + "_" + thisCmdLine.toString();
				var e = document.getElementById(thisID);
				var currSel = e.selectedIndex;
				configWorkData.LEDDefs[thisRow].LEDCmd[thisCol].Mode[currSel] = ledModeType[sender.selectedIndex];
			}
			else
				configWorkData.LEDDefs[thisRow].LEDCmd[thisCol].Mode[0] = ledModeType[sender.selectedIndex];
			break;
		case 11: //Rate
			if (configWorkData.LEDDefs[thisRow].MultiColor)
			{
				thisID = "ledselector" + thisRow.toString() + "_" + thisCmdLine.toString();
				var e = document.getElementById(thisID);
				var currSel = e.selectedIndex;
				configWorkData.LEDDefs[thisRow].LEDCmd[thisCol].Rate[currSel] = verifyNumber(sender.value, configWorkData.LEDDefs[thisRow].LEDCmd[thisCol].Rate[currSel]);
			}
			else
				configWorkData.LEDDefs[thisRow].LEDCmd[thisCol].Rate[0] = verifyNumber(sender.value, configWorkData.LEDDefs[thisRow].LEDCmd[thisCol].Rate[0]);
			break;
		case 12: //Transition
			if (configWorkData.LEDDefs[thisRow].MultiColor)
			{
				thisID = "ledselector" + thisRow.toString() + "_" + thisCmdLine.toString();
				var e = document.getElementById(thisID);
				var currSel = e.selectedIndex;
				configWorkData.LEDDefs[thisRow].LEDCmd[thisCol].Transition[currSel] = ledTransitionType[sender.selectedIndex];
			}
			else
				configWorkData.LEDDefs[thisRow].LEDCmd[thisCol].Transition[0] = ledTransitionType[sender.selectedIndex];
			break;
		case 15: //select individual LED
			var colorArray = [];
			for (var i = 0; i < configWorkData.LEDCols.length; i++)
				colorArray.push(configWorkData.LEDCols[i].Name);
			var thisIndex = sender.selectedIndex;
			thisID = "oncolsel_" + thisRow.toString() + "_" + thisCmdLine.toString();
			var onColSel = document.getElementById(thisID);
			thisID = "offcolsel_" + thisRow.toString() + "_" + thisCmdLine.toString();
			var offColSel = document.getElementById(thisID);
			onColSel.selectedIndex = colorArray.indexOf(configWorkData.LEDDefs[thisRow].LEDCmd[thisCol].ColOn[thisIndex]);
			offColSel.selectedIndex = colorArray.indexOf(configWorkData.LEDDefs[thisRow].LEDCmd[thisCol].ColOff[thisIndex]);
			
			thisID = "modesel_" + thisRow.toString() + "_" + thisCmdLine.toString();
			var modeSel = document.getElementById(thisID);
			modeSel.selectedIndex = ledModeType.indexOf(configWorkData.LEDDefs[thisRow].LEDCmd[thisCol].Mode[thisIndex]);
			
			thisID = "ratesel_" + thisRow.toString() + "_" + thisCmdLine.toString();
			var rateSel = document.getElementById(thisID);
			rateSel.value = configWorkData.LEDDefs[thisRow].LEDCmd[thisCol].Rate[thisIndex];

			thisID = "transitionsel_" + thisRow.toString() + "_" + thisCmdLine.toString();
			var transitionSel = document.getElementById(thisID);
			transitionSel.selectedIndex = ledTransitionType.indexOf(configWorkData.LEDDefs[thisRow].LEDCmd[thisCol].Transition[thisIndex]);
			
			break;
	}
}

function tfColorSelector(y, x, id, evtHandler, colorArray)
{
	
	var selectList = document.createElement("select");
	createOptions(selectList, colorArray);
	tfSetCoordinate(selectList, y, x, 0, id);
	selectList.setAttribute("class", "selectorbox");
	selectList.setAttribute("onchange", evtHandler);
	return selectList;
}

function tfModeSelector(y, x, id, evtHandler)
{
	
	var selectList = document.createElement("select");
	createOptions(selectList, ledModeDispType);
	tfSetCoordinate(selectList, y, x, 0, id);
	selectList.setAttribute("class", "selectorbox");
	selectList.setAttribute("onchange", evtHandler);
	return selectList;
}

function tfTransitionSelector(y, x, id, evtHandler)
{
	
	var selectList = document.createElement("select");
	createOptions(selectList, ledTransitionDispType);
	tfSetCoordinate(selectList, y, x, 0, id);
	selectList.setAttribute("class", "selectorbox");
	selectList.setAttribute("onchange", evtHandler);
	return selectList;
}

function createColorSelectField(parentObj, lineIndex, cmdIndex, cmdLineData, evtHandler)
{
	var upperDiv = document.createElement("div");
	upperDiv.setAttribute("class", "editorpanel");
	var thisID;
	if ((cmdLineData.CtrlSource == "signal") || (cmdLineData.CtrlSource == "analog"))
	{
		thisID = "cmdmanipulator_" + lineIndex.toString() + "_" + cmdIndex.toString();
		var thisBox = tfManipulatorBox(lineIndex, 1, thisID, evtHandler);
		tfSetCoordinate(thisBox, lineIndex, cmdIndex, 1, thisID);
		thisBox.setAttribute("cmdline", cmdIndex);
		upperDiv.append(thisBox);
	}
			
	var thisText = tfText(lineIndex, cmdIndex, "", evtHandler);
	thisText.innerHTML = "Value:&nbsp;";
	upperDiv.append(thisText);
	
	thisID = "ctrlvalue_" + lineIndex.toString() + "_" + cmdIndex.toString();
	var thisElement;
	if ((cmdLineData.CtrlSource == "signal") || (cmdLineData.CtrlSource == "analog"))
	{
		thisElement = tfNumeric(lineIndex, 5, thisID, evtHandler);
		tfSetCoordinate(thisElement, lineIndex, cmdIndex, 5, thisID);
		thisElement.setAttribute("cmdline", cmdIndex);
		thisElement.value = cmdLineData.LEDCmd[cmdIndex].Val;
		tfSetCoordinate(thisElement, lineIndex, cmdIndex, 6, thisID);
		thisElement.setAttribute("index", 6);
	}
	else
	{
		thisElement = tfText(lineIndex, 6, thisID, evtHandler);
		tfSetCoordinate(thisElement, lineIndex, cmdIndex, 6, thisID);
		switch (ledCtrlType.indexOf(cmdLineData.CtrlSource))
		{
			case 0: //block detector
				cmdLineData.LEDCmd[cmdIndex].Val = cmdIndex;
				thisElement.innerHTML = blockValDispType[cmdLineData.LEDCmd[cmdIndex].Val];
				break;
			case 1: //switch
				cmdLineData.LEDCmd[cmdIndex].Val = cmdIndex;
				thisElement.innerHTML = switchValDispType[cmdLineData.LEDCmd[cmdIndex].Val];
				break;
			case 3: //signalstat
				cmdLineData.LEDCmd[cmdIndex].Val = cmdIndex;
				var bitMask = 0x01;
				var dispStr = "";
				var numAddr = 1;
				var isArray = Array.isArray(cmdLineData.CtrlAddr);
				if (isArray)
					numAddr = cmdLineData.CtrlAddr.length;
				for (var i = 0; i < numAddr; i++)
				{
					if (i > 0) dispStr += ", ";
					if (isArray)
						dispStr += cmdLineData.CtrlAddr[i].toString() + ((cmdIndex & bitMask) == 0 ? "t" : "c");
					else
						dispStr += cmdLineData.CtrlAddr.toString() + ((cmdIndex & bitMask) == 0 ? "t" : "c");
					bitMask = bitMask << 1;
				}
				thisElement.innerHTML = dispStr;
				break;
			case 4: //signaldyn
				cmdLineData.LEDCmd[cmdIndex].Val = cmdIndex;
				var isArray = Array.isArray(cmdLineData.CtrlAddr);
				if (isArray)
					thisElement.innerHTML = cmdLineData.CtrlAddr[cmdIndex>>1].toString() + ((cmdIndex & 0x01) == 0 ? "t" : "c");
				else
					thisElement.innerHTML = cmdLineData.CtrlAddr.toString() + ((cmdIndex & 0x01) == 0 ? "t" : "c");
				break;
			case 5: //button
				cmdLineData.LEDCmd[cmdIndex].Val = cmdIndex; //btnType.indexOf(cmdIndex);
				thisElement.innerHTML = buttonValDispType[cmdIndex];
				break;
			case 7: //power
				cmdLineData.LEDCmd[cmdIndex].Val = cmdIndex; //trackPwrType.indexOf(cmdIndex);
				thisElement.innerHTML = powerValDispType[cmdIndex];
				break;
			case 8: //constant
				cmdLineData.LEDCmd[cmdIndex].Val = 1;
				thisElement.innerHTML = "On";
				break;
		}
	}
	upperDiv.append(thisElement);

	if (cmdLineData.MultiColor) 
	{
		thisID = "ledseltext" + lineIndex.toString() + "_" + cmdIndex.toString();
		thisText = tfText(lineIndex, cmdIndex, thisID, evtHandler);
		thisText.innerHTML = "Select LED:";
		upperDiv.append(tfTab(lineIndex, cmdIndex, '&nbsp;',""));
		upperDiv.append(thisText);
		upperDiv.append(tfTab(lineIndex, cmdIndex, '&nbsp;',""));
		thisID = "ledselector" + lineIndex.toString() + "_" + cmdIndex.toString();
		thisElement = tfLEDAddrSel(thisElement, lineIndex, thisID, evtHandler, cmdLineData.LEDNums);
		thisElement.setAttribute("cmdline", cmdIndex);
		tfSetCoordinate(thisElement, lineIndex, cmdIndex, 15, thisID);
		upperDiv.append(thisElement);
	}

	var colorArray = [];
	for (var i = 0; i < configWorkData.LEDCols.length; i++)
		colorArray.push(configWorkData.LEDCols[i].Name);

	thisID = "oncoltxt_" + lineIndex.toString() + "_" + cmdIndex.toString();
	thisText = tfText(lineIndex, cmdIndex, thisID, evtHandler);
	thisText.innerHTML = "On Color:";
	thisText.setAttribute("cmdline", cmdIndex);
	upperDiv.append(tfTab(lineIndex, cmdIndex, '&nbsp;',""));
	upperDiv.append(thisText);
	upperDiv.append(tfTab(lineIndex, cmdIndex, '&nbsp;',""));
	
	thisID = "oncolsel_" + lineIndex.toString() + "_" + cmdIndex.toString();
	thisElement = tfColorSelector(lineIndex, cmdIndex, thisID, evtHandler, colorArray);
	thisElement.setAttribute("cmdline", cmdIndex);
	if (cmdLineData.MultiColor)
		thisElement.selectedIndex = colorArray.indexOf(cmdLineData.LEDCmd[cmdIndex].ColOn[0]);
	else
		thisElement.selectedIndex = colorArray.indexOf(cmdLineData.LEDCmd[cmdIndex].ColOn);
	tfSetCoordinate(thisElement, lineIndex, cmdIndex, 8, thisID);
	upperDiv.append(thisElement);

	thisID = "offcoltxt_" + lineIndex.toString() + "_" + cmdIndex.toString();
	thisText = tfText(lineIndex, cmdIndex, thisID, evtHandler);
	thisText.innerHTML = "Off Color:";
	thisText.setAttribute("cmdline", cmdIndex);
	upperDiv.append(tfTab(lineIndex, cmdIndex, '&nbsp;',""));
	upperDiv.append(thisText);
	upperDiv.append(tfTab(lineIndex, cmdIndex, '&nbsp;',""));

//	colorArray.splice(0,0, "none");
	
	thisID = "offcolsel_" + lineIndex.toString() + "_" + cmdIndex.toString();
	thisElement = tfColorSelector(lineIndex, cmdIndex, thisID, evtHandler, colorArray);
	thisElement.setAttribute("cmdline", cmdIndex);

	if (cmdLineData.MultiColor)
		thisElement.selectedIndex = colorArray.indexOf(cmdLineData.LEDCmd[cmdIndex].ColOff[0]);
	else
		if (cmdLineData.LEDCmd[cmdIndex].ColOff == undefined)
			thisElement.selectedIndex = -1;
		else
			thisElement.selectedIndex = colorArray.indexOf(cmdLineData.LEDCmd[cmdIndex].ColOff);
	tfSetCoordinate(thisElement, lineIndex, cmdIndex, 9, thisID);
	upperDiv.append(thisElement);

	parentObj.append(upperDiv);
	
	var lowerDiv = document.createElement("div");
	lowerDiv.setAttribute("class", "editorpanel");
	if ((cmdLineData.CtrlSource == "signal") || (cmdLineData.CtrlSource == "analog"))
	{
		var thisSpacer = document.createElement("div");
		thisSpacer.setAttribute("class", "manipulatorbox");
		lowerDiv.append(thisSpacer);
	}
	thisText = tfText(lineIndex, cmdIndex, "", evtHandler);
	thisText.innerHTML = "Mode:&nbsp;";
	lowerDiv.append(thisText);

	thisID = "modesel_" + lineIndex.toString() + "_" + cmdIndex.toString();
	thisElement = tfModeSelector(lineIndex, cmdIndex, thisID, evtHandler);
	thisElement.setAttribute("cmdline", cmdIndex);
	if (!Array.isArray(cmdLineData.LEDCmd[cmdIndex].Mode))
	{
		var oldMode = cmdLineData.LEDCmd[cmdIndex].Mode;
		cmdLineData.LEDCmd[cmdIndex].Mode = [];
		cmdLineData.LEDCmd[cmdIndex].Mode.push(oldMode);
	}
	thisElement.selectedIndex = ledModeType.indexOf(cmdLineData.LEDCmd[cmdIndex].Mode[0]);
	tfSetCoordinate(thisElement, lineIndex, cmdIndex, 10, thisID);
	lowerDiv.append(thisElement);

	thisText = tfText(lineIndex, cmdIndex, "", evtHandler);
	thisText.innerHTML = "&nbsp;Rate:&nbsp;";
	lowerDiv.append(thisText);

	thisID = "ratesel_" + lineIndex.toString() + "_" + cmdIndex.toString();
	thisElement = tfNumeric(lineIndex, 5, thisID, evtHandler);
	thisElement.setAttribute("cmdline", cmdIndex);
	if (!Array.isArray(cmdLineData.LEDCmd[cmdIndex].Rate))
	{
		var oldRate = cmdLineData.LEDCmd[cmdIndex].Rate;
		cmdLineData.LEDCmd[cmdIndex].Rate = [];
		cmdLineData.LEDCmd[cmdIndex].Rate.push(oldRate);
	}
	thisElement.value = cmdLineData.LEDCmd[cmdIndex].Rate[0];
	tfSetCoordinate(thisElement, lineIndex, cmdIndex, 11, thisID);
	lowerDiv.append(thisElement);

	thisText = tfText(lineIndex, cmdIndex, "", evtHandler);
	thisText.innerHTML = "&nbsp;Transition:&nbsp;";
	lowerDiv.append(thisText);

	thisID = "transitionsel_" + lineIndex.toString() + "_" + cmdIndex.toString();
	thisElement = tfTransitionSelector(lineIndex, cmdIndex, thisID, evtHandler);
	thisElement.setAttribute("cmdline", cmdIndex);
	if (!Array.isArray(cmdLineData.LEDCmd[cmdIndex].Transition))
	{
		var oldTransition = cmdLineData.LEDCmd[cmdIndex].Transition;
		cmdLineData.LEDCmd[cmdIndex].Transition = [];
		cmdLineData.LEDCmd[cmdIndex].Transition.push(oldTransition);
	}
	thisElement.selectedIndex = ledTransitionType.indexOf(cmdLineData.LEDCmd[cmdIndex].Transition[0]);
	tfSetCoordinate(thisElement, lineIndex, cmdIndex, 12, thisID);
	lowerDiv.append(thisElement);

	parentObj.append(lowerDiv);
}

function buildCmdLines(lineIndex, lineData)
{
	var thisLineBase = document.getElementById("ledconfig_inp_" + lineIndex.toString() + "_2");
	while (thisLineBase.hasChildNodes())
		thisLineBase.removeChild(thisLineBase.childNodes[0]); //delete rows
	if (lineData.LEDCmd.length > 0)
		for (var i=0; i<lineData.LEDCmd.length; i++)
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
			
			createColorSelectField(mainDiv, lineIndex, i, lineData, "setLEDCmdData(this)");
		}
	else
	{
		var mainDiv = document.createElement("div");
		mainDiv.setAttribute("class", "editortile");
		tfSetCoordinate(mainDiv, i, 0, 0, thisID);
		thisLineBase.append(mainDiv);
		var thisId = "cmdbasedata_initadd" + lineIndex.toString();
		var newRB = tfTableStarterBox(lineIndex, -1, thisId, "setLEDCmdData(this)");
		mainDiv.append(newRB);
	}

}

function loadLEDTable(thisTable, thisData)
{
		function selByName(prmVal)
		{
			return (prmVal == thisData[i].CtrlSource);
		}

//	console.log(thisData);
	var th = document.getElementById(thisTable.id + "_head");
	var tb = document.getElementById(thisTable.id + "_body");
	var numCols = th.childNodes[0].children.length;
	createDataTableLines(thisTable, [tfPos, tfLEDSelector, tfCommandEditor], thisData.length, "setLEDData(this)");
	for (var i=0; i<thisData.length;i++)
	{
		var e = document.getElementById("lednrbox_" + i.toString() + "_" + "1");
		e.value = thisData[i].LEDNums;

		var e = document.getElementById("multicolor_" + i.toString() + "_" + "1");
		if (Array.isArray(thisData[i].LEDNums) && (thisData[i].LEDNums.length > 1))
			e.parentElement.style.visibility = "none";
		else
			e.parentElement.style.visibility = "hidden";
		e.checked = thisData[i].MultiColor;
//		console.log("Check Multi Color");
		
		var e = document.getElementById("disptypebox_" + i.toString() + "_" + "1");
		e.selectedIndex = dispType.indexOf(thisData[i].DisplayType);
		if ((thisData[i].CtrlSource == "analog") || (thisData[i].CtrlSource == "signal"))
			e.parentElement.style.visibility = "none";
		else
			e.parentElement.style.visibility = "hidden";

		var e = document.getElementById("addressbox_" + i.toString() + "_" + "1");
		e.value = thisData[i].CtrlAddr;
		
		var e = document.getElementById("cmdlistbox_" + i.toString() + "_" + "1");
		e.selectedIndex = ledCtrlType.indexOf(thisData[i].CtrlSource);
		buildCmdLines(i, thisData[i]);
	}
//	console.log();
}

function loadNodeDataFields(jsonData)
{ //get a copy of node.cfg in case it is needed (not here)
}

function constructPageContent(contentTab)
{
	var tempObj;
	mainScrollBox = createEmptyDiv(contentTab, "div", "pagetopicboxscroll-y", "nodeconfigdiv");
		createPageTitle(mainScrollBox, "div", "tile-1", "BasicCfg_Title", "h1", "LED Chain Configuration");
		createPageTitle(mainScrollBox, "div", "tile-1", "", "h2", "General Settings");
		tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
			createDropdownselector(tempObj, "tile-1_4", "Color Sequence:", ["RGB", "GRB"], "colorseq", "setLEDBasics(this)");
			createTextInput(tempObj, "tile-1_4", "# of LEDs:", "n/a", "numleds", "setLEDBasics(this)");
			createTextInput(tempObj, "tile-1_4", "System Blink Period:", "n/a", "blinkperiod", "setLEDBasics(this)");
		tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
			createTextInput(tempObj, "tile-1_4", "Brightness Ctrl:", "n/a", "brightnessctrl", "setLEDBasics(this)");
			createTextInput(tempObj, "tile-1_4", "Address:", "n/a", "brightnessaddr", "setLEDBasics(this)");
			createTextInput(tempObj, "tile-1_4", "Initial Level:", "n/a", "brightnesslevel", "setLEDBasics(this)");
		createPageTitle(mainScrollBox, "div", "tile-1", "", "h2", "Color Definitions");
		colorTable = createDataTable(mainScrollBox, "tile-1_2", ["Pos","Color Name", "Select Color", "Add/Delete/Move Color"], "colorconfig", "");

		createPageTitle(mainScrollBox, "div", "tile-1", "", "h2", "LED Settings");

		ledDefTable = createDataTable(mainScrollBox, "tile-1", ["Pos","LED Selector", "LED Command Sequence Editor"], "ledconfig", "");

		tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
			createButton(tempObj, "", "Save & Restart", "btnSave", "saveSettings(this)");
			createButton(tempObj, "", "Cancel", "btnCancel", "cancelSettings(this)");
		tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
			createButton(tempObj, "", "Save to File", "btnDownload", "downloadSettings(this)");
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
//	switch (jsonData.ChainParams.ChainType)
//	{
//		case "WS2812": setDropdownValue("chaintype", 1); break;
//		case "WS2801": setDropdownValue("chaintype", 0); break;
//	}
	switch (jsonData.ChainParams.ColorSeq)
	{
		case "RGB": setDropdownValue("colorseq", 0); break;
		case "GRB": setDropdownValue("colorseq", 1); break;
	}
	
	writeInputField("numleds", jsonData.ChainParams.NumLEDs);
	writeInputField("blinkperiod", jsonData.ChainParams.BlinkPeriod);
	writeInputField("brightnessctrl", jsonData.ChainParams.Brightness.CtrlSource);
	writeInputField("brightnesslevel", jsonData.ChainParams.Brightness.InitLevel);
	writeInputField("brightnessaddr", jsonData.ChainParams.Brightness.Addr);
	loadColorTable(colorTable, jsonData.LEDCols);
	loadLEDTable(ledDefTable, jsonData.LEDDefs);
}

function processLocoNetInput(jsonData)
{
}
