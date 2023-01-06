var mainScrollBox;
var colorTable;
var ledDefTable;
var mqttTitle;
var mqttBox;
var generalBox1;
var generalBox2;
var colorTableDiv;

var numColors = 0;

var i2cMaxNum = 525;

var ledCtrlType = ["switch", "signaldyn", "signal", "button", "analog", "block", "transponder", "power", "constant", "signalstat"];
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
var transponderValDispType = ["Enter", "Leave"];

var newColTemplate = {"Name": "New Color","RGBVal": [255, 255, 255]};
var newLEDTemplate = {"CtrlSource": "block","CtrlAddr": [1], "DisplayType":"discrete", "MultiColor":false, "LEDCmd": [{"Val": 0,"ColOn": "","Mode":"static","Rate":0,"Transition":"soft"},{"Val": 0,"ColOn": "","Mode":"static","Rate":0,"Transition":"soft"}]};
var newCmdTemplate = {"Val": 0,	"ColOn": "", "Mode": "static","Rate":0,	"Transition":"soft"};

function getMaximumLED()
{
	switch (configData[0].HatTypeList[configData[0].HatIndex].HatId)
	{
		case 3: return i2cMaxNum; break; //YellowHat
		default: return 0xFFFF;
	}
}

function saveConfigFileSettings()
{
	//step 1: save greenhat.cfg
	saveJSONConfig(scriptList.Pages[currentPage].ID, scriptList.Pages[currentPage].FileName, configData[workCfg], prepareFileSeqLED);
}

function upgradeJSONVersion(jsonData)
{
	return upgradeJSONVersionLED(jsonData);
}

function addFileSeq(ofObj, loadData, workData) //object specific function to include partial files
{
//	console.log(ofObj);
	addFileSeqLED(ofObj, loadData, workData)
}

function downloadSettings(sender)
{
	downloadConfig(0x0020); //send just this
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
function setLEDBasics(sender)
{
	if (sender.id == "colorseq")
		switch (sender.selectedIndex)
		{
			case 0: configData[2].ChainParams.ColorSeq = "RGB"; break;
			case 1: configData[2].ChainParams.ColorSeq = "GRB"; break;
		}
	if (sender.id == "numleds")
	{
		var newNum = verifyNumber(sender.value, configData[2].NumLEDs);
		if (newNum > getMaximumLED())
		{
			sender.value = i2cMaxNum;
			alert("Maximum number of LED's is limited to " + i2cMaxNum.toString() + " for this hat type!");
		}
		else
			configData[2].ChainParams.NumLEDs = newNum;
	}
	if (sender.id == "blinkperiod")
		configData[2].ChainParams.BlinkPeriod = verifyNumber(sender.value, configData[2].ChainParams.BlinkPeriod);
	if (sender.id == "brightnessctrl")
		configData[2].ChainParams.Brightness.CtrlSource = sender.value;
	if (sender.id == "brightnessaddr")
		configData[2].ChainParams.Brightness.Addr = verifyNumber(sender.value, configData[2].ChainParams.Brightness.Addr);
	if (sender.id == "brightnesslevel")
		configData[2].ChainParams.Brightness.InitLevel = verifyNumber(sender.value, configData[2].ChainParams.Brightness.InitLevel);
	if (sender.id == "mqtopledset")
		configData[2].MQTT.Subscribe[0].Topic = sender.value;
	if (sender.id == "incladdrset")
		configData[2].MQTT.Subscribe[0].InclAddr = sender.checked;
	if (sender.id == "mqtopledask")
		configData[2].MQTT.Subscribe[1].Topic = sender.value;
	if (sender.id == "incladdrask")
		configData[2].MQTT.Subscribe[1].InclAddr = sender.checked;
	if (sender.id == "mqtopledreply")
		configData[2].MQTT.Publish[0].Topic = sender.value;
	if (sender.id == "incladdrreply")
		configData[2].MQTT.Publish[0].InclAddr = sender.checked;
//	console.log(configData[2]);
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
			configData[2].LEDCols.push(JSON.parse(JSON.stringify(newColTemplate)));
			loadColorTable(colorTable, configData[2].LEDCols);
			break;
		case 1: //Color Name
			configData[2].LEDCols[thisRow].Name = sender.value;
			break;
		case 2: //Color RGB Value
			var thisColorStr = sender.value.replace("#", "0x");
			var thisColor = parseInt(thisColorStr);
			configData[2].LEDCols[thisRow].RGBVal[0] = (thisColor & 0xFF0000) >> 16;
			configData[2].LEDCols[thisRow].RGBVal[1] = (thisColor & 0x00FF00) >> 8;
			configData[2].LEDCols[thisRow].RGBVal[2] = (thisColor & 0x0000FF);
			configData[2].LEDCols[thisRow].HSVVal = rgbToHsv(configData[2].LEDCols[thisRow].RGBVal[0],configData[2].LEDCols[thisRow].RGBVal[1],configData[2].LEDCols[thisRow].RGBVal[2]);
//			console.log(configData[2].LEDCols[thisRow]);
			break;
		case 3: //Manipulator buttons
			var idStr = sender.id;
			var thisElement;
			if (thisIndex == 1)
				configData[2].LEDCols.splice(thisRow+1, 0, JSON.parse(JSON.stringify(newColTemplate)));
			if (thisIndex == 2)
			{
				var noReference = true;
				var colName = configData[2].LEDCols[thisRow].Name;
//				console.log("Searching for: " + colName);
				for (var i = 0; i < configData[2].LEDDefs.length; i++)
				{
					for (var j = 0; j < configData[2].LEDDefs[i].LEDCmd.length; j++)
					{
						if (Array.isArray(configData[2].LEDDefs[i].LEDCmd[j].ColOn))
						{
							if (configData[2].LEDDefs[i].LEDCmd[j].ColOn.indexOf(colName) >= 0)
							{
								noReference = false;
								break;
							}
						}
						else
							if (configData[2].LEDDefs[i].LEDCmd[j].ColOn == colName)
							{
								noReference = false;
								break;
							}
						if (configData[2].LEDDefs[i].LEDCmd[j].ColOff != undefined)
							if (Array.isArray(configData[2].LEDDefs[i].LEDCmd[j].ColOn))
							{
								if (configData[2].LEDDefs[i].LEDCmd[j].ColOn.indexOf(colName) >= 0)
								{
									noReference = false;
									break;
								}
							}
							else
								if (configData[2].LEDDefs[i].LEDCmd[j].ColOn == colName)
								{
									noReference = false;
									break;
								}
					}
				}
				if (noReference)
					configData[2].LEDCols.splice(thisRow, 1);
				else
					alert(colName + " is currently used in the table below. Remove any reference before deleting this color!");
			}
			if ((thisIndex == 3) && (thisRow > 0))
			{
				thisElement = configData[2].LEDCols.splice(thisRow, 1);
				configData[2].LEDCols.splice(thisRow-1,0, thisElement[0]);
			}
			if ((thisIndex == 4) && (thisRow < configData[2].LEDCols.length))
			{
				thisElement = configData[2].LEDCols.splice(thisRow, 1);
				configData[2].LEDCols.splice(thisRow+1,0, thisElement[0]);
			}
//			console.log(configData[2].LEDCols);
			loadColorTable(colorTable, configData[2].LEDCols);
			break;
	}
}

function changeLEDNr(chgVal)
{
	var startLED = parseInt(document.getElementById("ledAdjNr").value);
	if (isNaN(startLED))
		alert("Please enter a valid LED number to start with");
	else
		 if (chgVal + startLED >= 0)
		{
			var isChanged = false;
			for (var i = 0; i < configData[2].LEDDefs.length; i++)
			{
				for (var j = 0; j < configData[2].LEDDefs[i].LEDNums.length; j++)
					if (configData[2].LEDDefs[i].LEDNums[j] >= startLED)
					{
						configData[2].LEDDefs[i].LEDNums[j] = (parseInt(configData[2].LEDDefs[i].LEDNums[j]) + chgVal).toString();
						isChanged = true;
					}
			}
			if (isChanged)
				loadLEDTable(ledDefTable, configData[2].LEDDefs);
		}
		else
			alert("Starting LED can't be zero!");
}

	function adjustColorEntries(currentRow)
	{
		function verifyArray(thisArray, newLen, initVal)
		{
			var resArray = [];
			var oldLen = 0;
			var setVal = initVal;
			if (Array.isArray(thisArray))
			{
				oldLen = Math.min(thisArray.length, newLen);
				setVal = thisArray[oldLen - 1];
				for (var i = 0; i < oldLen; i++)
					resArray.push(thisArray[i]);
			}
			else
				if (thisArray != undefined)
					setVal = thisArray;
			while (resArray.length < newLen)
				resArray.push(setVal);
			return resArray;
		}
		
		if (configData[2].LEDDefs[currentRow].MultiColor)
		{
			var numCols = configData[2].LEDDefs[currentRow].LEDNums.length;
			for (var i = 0; i < configData[2].LEDDefs[currentRow].LEDCmd.length; i++)
			{	//for each command
				configData[2].LEDDefs[currentRow].LEDCmd[i].ColOn = verifyArray(configData[2].LEDDefs[currentRow].LEDCmd[i].ColOn, numCols, "");
				configData[2].LEDDefs[currentRow].LEDCmd[i].ColOff = verifyArray(configData[2].LEDDefs[currentRow].LEDCmd[i].ColOff, numCols, "");
				configData[2].LEDDefs[currentRow].LEDCmd[i].Mode = verifyArray(configData[2].LEDDefs[currentRow].LEDCmd[i].Mode, numCols, "static");
				configData[2].LEDDefs[currentRow].LEDCmd[i].Rate = verifyArray(configData[2].LEDDefs[currentRow].LEDCmd[i].Rate, numCols, 0);
				configData[2].LEDDefs[currentRow].LEDCmd[i].Transition = verifyArray(configData[2].LEDDefs[currentRow].LEDCmd[i].Transition, numCols, "soft");
			}
		}
		else
		{
			for (var i = 0; i < configData[2].LEDDefs[currentRow].LEDCmd.length; i++)
			{	//for each command
				var oldColOn = configData[2].LEDDefs[currentRow].LEDCmd[i].ColOn;
//				console.log(typeof oldColOn);
				if (typeof oldColOn == "object")
					oldColOn = configData[2].LEDDefs[currentRow].LEDCmd[i].ColOn[0];
				var oldColOff;
				try
				{
					oldColOff = configData[2].LEDDefs[currentRow].LEDCmd[i].ColOff;
					if (typeof oldColOff == "object")
						oldColOff = configData[2].LEDDefs[currentRow].LEDCmd[i].ColOff[0];
				}
				catch(err) 
				{
					oldColOff = "";
				}
				configData[2].LEDDefs[currentRow].LEDCmd[i].ColOn = oldColOn;
				configData[2].LEDDefs[currentRow].LEDCmd[i].ColOff = oldColOff;
				configData[2].LEDDefs[currentRow].LEDCmd[i].Mode.splice(1, configData[2].LEDDefs[currentRow].LEDCmd[i].Mode.length-1);
				configData[2].LEDDefs[currentRow].LEDCmd[i].Rate.splice(1, configData[2].LEDDefs[currentRow].LEDCmd[i].Rate.length-1);
				configData[2].LEDDefs[currentRow].LEDCmd[i].Transition.splice(1, configData[2].LEDDefs[currentRow].LEDCmd[i].Transition.length-1);
			}
		}
	}

function setLEDData(sender)
{
	function adjustCmdLines(currentRow)
	{
		var numCmds;
		var numAddr = 1;
		if (Array.isArray(configData[2].LEDDefs[currentRow].CtrlAddr))
			numAddr = configData[2].LEDDefs[currentRow].CtrlAddr.length;
		switch (ledCtrlType.indexOf(configData[2].LEDDefs[currentRow].CtrlSource))
		{
//			case 9 : ; //used to be static signal, now switch
			case 0 : numCmds = Math.pow(2, numAddr); break; //switch
			case 1 : numCmds = 2 * numAddr; break; //signaldyn
			case 2 : numCmds = 1; break; //signal
			case 3 : numCmds = 5; break; //button
			case 4 : numCmds = 1; break; //analog
			case 5 : numCmds = Math.pow(2, numAddr); break; //block
			case 6 : numCmds = 2; break; //transponder
			case 7 : numCmds = 3; break; //power
			case 8 : numCmds = 1; break; //constant
		}
		while (configData[2].LEDDefs[currentRow].LEDCmd.length < numCmds)
			configData[2].LEDDefs[currentRow].LEDCmd.push(JSON.parse(JSON.stringify(newCmdTemplate)));
		while (configData[2].LEDDefs[currentRow].LEDCmd.length > numCmds)
			configData[2].LEDDefs[currentRow].LEDCmd.splice(configData[2].LEDDefs[currentRow].LEDCmd.length-1, 1);
		adjustColorEntries(currentRow);
	}
	
	
	var thisRow = parseInt(sender.getAttribute("row"));
	var thisCol = parseInt(sender.getAttribute("col"));
	var thisIndex = parseInt(sender.getAttribute("index"));
	switch (thisCol)
	{
		case -1: //empty table, create first entry
			configData[2].LEDDefs.push(JSON.parse(JSON.stringify(newLEDTemplate)));
			loadLEDTable(ledDefTable, configData[2].LEDDefs);
			break;
		case 1: //CommandSelector
			var idStr = sender.id;
			var thisElement;
			switch (thisIndex)
			{
				case 1:
					if (event.ctrlKey) //duplicate entry
						configData[2].LEDDefs.splice(thisRow+1, 0, JSON.parse(JSON.stringify(configData[2].LEDDefs[thisRow])));
					else //create new entry
						configData[2].LEDDefs.splice(thisRow+1, 0, JSON.parse(JSON.stringify(newLEDTemplate)));
					break;
				case 2:
					configData[2].LEDDefs.splice(thisRow, 1);
					break;
				case 3:
					if (thisRow > 0)
					{
						thisElement = configData[2].LEDDefs.splice(thisRow, 1);
						configData[2].LEDDefs.splice(thisRow-1,0, thisElement[0]);
					}
					break;
				case 4:
					if (thisRow < configData[2].LEDDefs.length)
					{
						thisElement = configData[2].LEDDefs.splice(thisRow, 1);
						configData[2].LEDDefs.splice(thisRow+1,0, thisElement[0]);
					}
					break;
				case 5:
					{
						var newArray = verifyNumArray(sender.value, ",");
						if (newArray.length > 0)
						{
							if (newArray.length > 1)
							{
								configData[2].LEDDefs[thisRow].LEDNums = []; //make sure this is an array
								for (var i = 0; i < newArray.length; i++)
									configData[2].LEDDefs[thisRow].LEDNums.push(newArray[i]);
								adjustColorEntries(thisRow);
							}
							else //==1
							{
								configData[2].LEDDefs[thisRow].LEDNums = [newArray[0]];
								configData[2].LEDDefs[thisRow].MultiColor = false;
							}
						}
						else
							alert(sender.value + " is not a valid number or array. Please verify");
					}
					break;
				case 6: //ind. col. checkbox
					{
						configData[2].LEDDefs[thisRow].MultiColor = sender.checked; 
						adjustColorEntries(thisRow);
					}
					break;
				case 7:
					{
						var oldSource = configData[workCfg].LEDDefs[thisRow].CtrlSource;
						configData[workCfg].LEDDefs[thisRow].CtrlSource = ledCtrlType[sender.selectedIndex];
						if (configData[workCfg].LEDDefs[thisRow].CtrlSource != oldSource)
							adjustCmdLines(thisRow);
					}
					break;
				case 8: //Addr
					{
						var newArray = verifyNumArray(sender.value, ",");
						if (newArray.length > 0)
						{
							var oldLen = Array.isArray(configData[2].LEDDefs[thisRow].CtrlAddr) ? configData[2].LEDDefs[thisRow].CtrlAddr.length : 1;
							if (newArray.length > 1)
								configData[2].LEDDefs[thisRow].CtrlAddr = newArray; //make sure this is an array
							else
								configData[2].LEDDefs[thisRow].CtrlAddr = newArray[0];
							//shorten array length.switch and block detector max 3, others max 1
							if ([0,1,5].indexOf(ledCtrlType.indexOf(configData[2].LEDDefs[thisRow].CtrlSource)) >= 0)
								while (newArray.length > 3)
									newArray.pop();
							else
								while (newArray.length > 1)
									newArray.pop();
							sender.value = newArray;
							if (newArray.length != oldLen)
								adjustCmdLines(thisRow);
//							console.log(sender.value);
						}
						else
							alert(sender.value + " is not a valid number or array. Please verify");
					}
					break;
				case 9:
					configData[2].LEDDefs[thisRow].DisplayType = dispType[sender.selectedIndex];
					break;
				case 10:
					{
						var newArray = verifyNumArray(sender.value, ",");
						if (newArray.length > 0)
							configData[2].LEDDefs[thisRow].CondAddr = newArray;
						else
							configData[2].LEDDefs[thisRow].CondAddr = [];
					}
					break;
				case 21:
					setLEDTestDisplay(configData[2].LEDDefs[thisRow].LEDNums);
					break;
			} //thisIndex
			loadLEDTable(ledDefTable, configData[2].LEDDefs);
	} //thisCol
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
//	console.log("setLEDCmdData", thisRow, thisCol, thisIndex, thisCmdLine, sender.id, configData[2].LEDDefs[thisRow]);
	switch (thisIndex)
	{
		case 1: //new Cmd Entry
			configData[2].LEDDefs[thisRow].LEDCmd.splice(thisCmdLine+1, 0, JSON.parse(JSON.stringify(newCmdTemplate)));
			adjustColorEntries(thisRow);
			loadLEDTable(ledDefTable, configData[2].LEDDefs);
			break;
		case 2: //delete Cmd Entry
			configData[2].LEDDefs[thisRow].LEDCmd.splice(thisCmdLine, 1);
			loadLEDTable(ledDefTable, configData[2].LEDDefs);
			break;
		case 3: //Up
			if (thisCmdLine > 0)
			{
				thisElement = configData[2].LEDDefs[thisRow].LEDCmd.splice(thisCmdLine, 1);
				configData[2].LEDDefs[thisRow].LEDCmd.splice(thisCmdLine-1,0, thisElement[0]);
			}
			loadLEDTable(ledDefTable, configData[2].LEDDefs);
			break;
		case 4: //Down
			if (thisCmdLine < configData[2].LEDDefs[thisRow].LEDCmd.length)
			{
				thisElement = configData[2].LEDDefs[thisRow].LEDCmd.splice(thisCmdLine, 1);
				configData[2].LEDDefs[thisRow].LEDCmd.splice(thisCmdLine+1,0, thisElement[0]);
			}
			loadLEDTable(ledDefTable, configData[2].LEDDefs);
			break;
		case 6: //value
			configData[2].LEDDefs[thisRow].LEDCmd[thisCol].Val = verifyNumber(sender.value, configData[2].LEDDefs[thisRow].LEDCmd[thisCol].Val);
			break;
		case 8: //oncolor
			if (configData[2].LEDDefs[thisRow].MultiColor)
			{
				thisID = "ledselector" + thisRow.toString() + "_" + thisCmdLine.toString();
				var e = document.getElementById(thisID);
				var currSel = e.selectedIndex;
				configData[2].LEDDefs[thisRow].LEDCmd[thisCol].ColOn[currSel] = configData[2].LEDCols[sender.selectedIndex].Name;
			}
			else
				configData[2].LEDDefs[thisRow].LEDCmd[thisCol].ColOn = configData[2].LEDCols[sender.selectedIndex].Name;
			break;
		case 9: //offcolor
			if (configData[2].LEDDefs[thisRow].MultiColor)
			{
				thisID = "ledselector" + thisRow.toString() + "_" + thisCmdLine.toString();
				var e = document.getElementById(thisID);
				var currSel = e.selectedIndex;
				configData[2].LEDDefs[thisRow].LEDCmd[thisCol].ColOff[currSel] = configData[2].LEDCols[sender.selectedIndex].Name;
			}
			else
			{
				configData[2].LEDDefs[thisRow].LEDCmd[thisCol].ColOff = configData[2].LEDCols[sender.selectedIndex].Name;
			}
			break;
		case 10: //Mode
			if (configData[2].LEDDefs[thisRow].MultiColor)
			{
				thisID = "ledselector" + thisRow.toString() + "_" + thisCmdLine.toString();
				var e = document.getElementById(thisID);
				var currSel = e.selectedIndex;
				configData[2].LEDDefs[thisRow].LEDCmd[thisCol].Mode[currSel] = ledModeType[sender.selectedIndex];
			}
			else
				configData[2].LEDDefs[thisRow].LEDCmd[thisCol].Mode[0] = ledModeType[sender.selectedIndex];
			break;
		case 11: //Rate
			if (configData[2].LEDDefs[thisRow].MultiColor)
			{
				thisID = "ledselector" + thisRow.toString() + "_" + thisCmdLine.toString();
				var e = document.getElementById(thisID);
				var currSel = e.selectedIndex;
				configData[2].LEDDefs[thisRow].LEDCmd[thisCol].Rate[currSel] = verifyNumber(sender.value, configData[2].LEDDefs[thisRow].LEDCmd[thisCol].Rate[currSel]);
			}
			else
				configData[2].LEDDefs[thisRow].LEDCmd[thisCol].Rate[0] = verifyNumber(sender.value, configData[2].LEDDefs[thisRow].LEDCmd[thisCol].Rate[0]);
			break;
		case 12: //Transition
			if (configData[2].LEDDefs[thisRow].MultiColor)
			{
				thisID = "ledselector" + thisRow.toString() + "_" + thisCmdLine.toString();
				var e = document.getElementById(thisID);
				var currSel = e.selectedIndex;
				configData[2].LEDDefs[thisRow].LEDCmd[thisCol].Transition[currSel] = ledTransitionType[sender.selectedIndex];
			}
			else
				configData[2].LEDDefs[thisRow].LEDCmd[thisCol].Transition[0] = ledTransitionType[sender.selectedIndex];
			break;
		case 15: //select individual LED
			var colorArray = [];
			for (var i = 0; i < configData[2].LEDCols.length; i++)
				colorArray.push(configData[2].LEDCols[i].Name);
			var thisIndex = sender.selectedIndex;
			thisID = "oncolsel_" + thisRow.toString() + "_" + thisCmdLine.toString();
			var onColSel = document.getElementById(thisID);
			thisID = "offcolsel_" + thisRow.toString() + "_" + thisCmdLine.toString();
			var offColSel = document.getElementById(thisID);
			onColSel.selectedIndex = colorArray.indexOf(configData[2].LEDDefs[thisRow].LEDCmd[thisCol].ColOn[thisIndex]);
			offColSel.selectedIndex = colorArray.indexOf(configData[2].LEDDefs[thisRow].LEDCmd[thisCol].ColOff[thisIndex]);
			
			thisID = "modesel_" + thisRow.toString() + "_" + thisCmdLine.toString();
			var modeSel = document.getElementById(thisID);
			modeSel.selectedIndex = ledModeType.indexOf(configData[2].LEDDefs[thisRow].LEDCmd[thisCol].Mode[thisIndex]);
			
			thisID = "ratesel_" + thisRow.toString() + "_" + thisCmdLine.toString();
			var rateSel = document.getElementById(thisID);
			rateSel.value = configData[2].LEDDefs[thisRow].LEDCmd[thisCol].Rate[thisIndex];

			thisID = "transitionsel_" + thisRow.toString() + "_" + thisCmdLine.toString();
			var transitionSel = document.getElementById(thisID);
			transitionSel.selectedIndex = ledTransitionType.indexOf(configData[2].LEDDefs[thisRow].LEDCmd[thisCol].Transition[thisIndex]);
			
			break;
	}
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
//add code for	if (cmdLineData.CtrlSource == "transponder")
	{
		thisElement = tfText(lineIndex, 6, thisID, evtHandler);
		tfSetCoordinate(thisElement, lineIndex, cmdIndex, 6, thisID);
		switch (ledCtrlType.indexOf(cmdLineData.CtrlSource))
		{
			case 0: //switch
			case 9: //signalstat
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
			case 1: //signaldyn
				cmdLineData.LEDCmd[cmdIndex].Val = cmdIndex;
				var isArray = Array.isArray(cmdLineData.CtrlAddr);
				if (isArray)
					thisElement.innerHTML = cmdLineData.CtrlAddr[cmdIndex>>1].toString() + ((cmdIndex & 0x01) == 0 ? "t" : "c");
				else
					thisElement.innerHTML = cmdLineData.CtrlAddr.toString() + ((cmdIndex & 0x01) == 0 ? "t" : "c");
				break;
			case 3: //button
				cmdLineData.LEDCmd[cmdIndex].Val = cmdIndex; //btnType.indexOf(cmdIndex);
				thisElement.innerHTML = buttonValDispType[cmdIndex];
				break;
			case 5: //block detector
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
						dispStr += cmdLineData.CtrlAddr[i].toString() + ((cmdIndex & bitMask) == 0 ? " fr" : " oc");
					else
						dispStr += cmdLineData.CtrlAddr.toString() + ((cmdIndex & bitMask) == 0 ? " fr" : " oc");
					bitMask = bitMask << 1;
				}
				thisElement.innerHTML = dispStr;
				break;
			case 6: //transponder
				cmdLineData.LEDCmd[cmdIndex].Val = cmdIndex; //trackPwrType.indexOf(cmdIndex);
				thisElement.innerHTML = transponderValDispType[cmdIndex];
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
	for (var i = 0; i < configData[2].LEDCols.length; i++)
		colorArray.push(configData[2].LEDCols[i].Name);

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
			var thisID = "master_" + lineIndex.toString() + "_" + i.toString();
			tfSetCoordinate(masterDiv, i, 0, 0, thisID);
			thisLineBase.append(masterDiv);

			thisID = "pos_" + lineIndex.toString() + "_" + i.toString();
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
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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
		if ([2,4].indexOf(ledCtrlType.indexOf(thisData[i].CtrlSource)) >= 0)
//		if ((thisData[i].CtrlSource == "analog") || (thisData[i].CtrlSource == "signal"))
			e.parentElement.style.visibility = "none";
		else
			e.parentElement.style.visibility = "hidden";

		var e = document.getElementById("transpselbox_" + i.toString() + "_" + "1");
		if ([6].indexOf(ledCtrlType.indexOf(thisData[i].CtrlSource)) >= 0)
		{
			e.value = thisData[i].CondAddr;
			e.parentElement.style.visibility = "none";
		}
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
{
	//get a copy of node.cfg in case it is needed
	var interfaceType = jsonData.InterfaceTypeList[jsonData.InterfaceIndex].Type;

	setVisibility(interfaceType == 3, mqttTitle);
	setVisibility(interfaceType == 3, mqttBox);
	setVisibility(interfaceType != 3, generalBox1);
	setVisibility(interfaceType != 3, generalBox2);
	setVisibility(interfaceType != 3, colorTableDiv);
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
		generalBox1 = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
			createTextInput(generalBox1, "tile-1_4", "System Blink Period:", "n/a", "blinkperiod", "setLEDBasics(this)");
		generalBox2 = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
			createTextInput(generalBox2, "tile-1_4", "Brightness Ctrl:", "n/a", "brightnessctrl", "setLEDBasics(this)");
			createTextInput(generalBox2, "tile-1_4", "Address:", "n/a", "brightnessaddr", "setLEDBasics(this)");
			createTextInput(generalBox2, "tile-1_4", "Initial Level:", "n/a", "brightnesslevel", "setLEDBasics(this)");

		mqttTitle = createPageTitle(mainScrollBox, "div", "tile-1", "", "h2", "MQTT Settings");
		mqttBox = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
			tempObj = createEmptyDiv(mqttBox, "div", "tile-1", "");
			createTextInput(tempObj, "tile-1_4", "(S) LED Cmd. Topic:", "n/a", "mqtopledset", "setLEDBasics(this)");
			createCheckbox(tempObj, "tile-1_4", "Include LED #", "incladdrset", "setLEDBasics(this)");
			tempObj = createEmptyDiv(mqttBox, "div", "tile-1", "");
			createTextInput(tempObj, "tile-1_4", "(S) LED Query Topic:", "n/a", "mqtopledask", "setLEDBasics(this)");
			createCheckbox(tempObj, "tile-1_4", "Include LED #", "incladdrask", "setLEDBasics(this)");
			tempObj = createEmptyDiv(mqttBox, "div", "tile-1", "");
			createTextInput(tempObj, "tile-1_4", "(P) LED Reply Topic:", "n/a", "mqtopledreply", "setLEDBasics(this)");
			createCheckbox(tempObj, "tile-1_4", "Include LED #", "incladdrreply", "setLEDBasics(this)");

		colorTableDiv = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
		createPageTitle(colorTableDiv, "div", "tile-1", "", "h2", "Color Definitions");
		colorTable = createDataTable(colorTableDiv, "tile-1_2", ["Pos","Color Name", "Select Color", "Add/Delete/Move Color"], "colorconfig", "");

		createPageTitle(colorTableDiv, "div", "tile-1", "", "h2", "Chain Adjustments");
		tempObj = createEmptyDiv(colorTableDiv, "div", "tile-1", "");
			createButton(tempObj, "", "LED++", "btnIncr", "changeLEDNr(1)");
			createButton(tempObj, "", "LED--", "btnDecr", "changeLEDNr(-1)");
			createTextInput(tempObj, "tile-1_4", "Starting with LED:", "n/a", "ledAdjNr", "");
		
		createPageTitle(colorTableDiv, "div", "tile-1", "", "h2", "LED Settings");

		ledDefTable = createDataTable(colorTableDiv, "tile-1", ["Pos","IF THIS: (LED/Input Selector)", "THEN THAT: (LED Command Sequence Editor)"], "ledconfig", "");

		tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
			createButton(tempObj, "", "Save & Restart", "btnSave", "saveSettings(this)");
			createButton(tempObj, "", "Cancel", "btnCancel", "cancelSettings(this)");
		tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
			createButton(tempObj, "", "Save to File", "btnDownload", "downloadSettings(this)");
}

function loadDataFields(jsonData)
{
	setVisibility(true, document.getElementById("pgLEDCfg"), false);
	configData[workCfg] = upgradeJSONVersion(jsonData);
//	console.log(configData[workCfg]);
	switch (configData[workCfg].ChainParams.ColorSeq)
	{
		case "RGB": setDropdownValue("colorseq", 0); break;
		case "GRB": setDropdownValue("colorseq", 1); break;
	}
	
	writeInputField("numleds", configData[workCfg].ChainParams.NumLEDs);
	writeInputField("blinkperiod", configData[workCfg].ChainParams.BlinkPeriod);
	writeInputField("brightnessctrl", configData[workCfg].ChainParams.Brightness.CtrlSource);
	writeInputField("brightnesslevel", configData[workCfg].ChainParams.Brightness.InitLevel);
	writeInputField("brightnessaddr", configData[workCfg].ChainParams.Brightness.Addr);
	writeInputField("mqtopledset", configData[workCfg].MQTT.Subscribe[0].Topic);
	writeCBInputField("incladdrset", configData[workCfg].MQTT.Subscribe[0].InclAddr);
	writeInputField("mqtopledask", configData[workCfg].MQTT.Subscribe[1].Topic);
	writeCBInputField("incladdrask", configData[workCfg].MQTT.Subscribe[1].InclAddr);
	writeInputField("mqtopledreply", configData[workCfg].MQTT.Publish[0].Topic);
	writeCBInputField("incladdrreply", configData[workCfg].MQTT.Publish[0].InclAddr);
	loadColorTable(colorTable, configData[workCfg].LEDCols);
	loadLEDTable(ledDefTable, configData[workCfg].LEDDefs);
}

function processLocoNetInput(jsonData)
{
}
