var mainScrollBox;
var pingDelay;
var dynSpeedTable;
var staticSpeedTable;
var aspectGenTable;
var secelModelTable;

var newSpeedListTemplate = {"ModelName": "New Model", "SpeedOptions" : []};
var newSpeedLineTemplate = {"SpeedName" : "New Speed Name",	"SpeedVal" : 0};
var newAspectListTemplate = {"AspectSetName" : "New Aspect Group", "DynSpeedModel": "", "StaticSpeedModel": "","Aspects" : []};
var newAspectLineTemplate = {"AspectLabel" : "Aspect Name",	"AspectValue" : 0, "StaticSpeed" : "", "DynamicSpeed" : "", "DispColHSV" : [0,255,255], "DispBlink" : false};

jsonFileVersion = "1.1.0";

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
		//upgrade from noversion to 1.1.0
		console.log("upgrade from noversion to ", jsonFileVersion);
		jsonData.Version = jsonFileVersion;
	}
}

function addFileSeq(ofObj) //object specific function to include partial files
{
	console.log(ofObj);
	for (var j=0; j<ofObj.SecurityElementSections.length; j++)
	{
		configData[1].SecurityElementSections.push(JSON.parse(JSON.stringify(ofObj.SecurityElementSections[j])));
		configData[2].SecurityElementSections.push(JSON.parse(JSON.stringify(ofObj.SecurityElementSections[j])));
	}
}

function prepareFileSeq() //object specific function to create partial files
{
	function addEntry()
	{
		var newEntry = {"SecurityElementSections":[]}
		transferData.FileList.push(newEntry);
		return newEntry;
	}
	
	var thisEntry = addEntry();
	thisEntry.Version = jsonFileVersion;
	thisEntry.DynSpeedModelList = JSON.parse(JSON.stringify(configData[2].DynSpeedModelList));
	thisEntry.StaticSpeedModelList = JSON.parse(JSON.stringify(configData[2].StaticSpeedModelList));
	thisEntry.AspectGeneratorList = JSON.parse(JSON.stringify(configData[2].AspectGeneratorList));
	var thisFileLength = JSON.stringify(thisEntry).length;
	
	for (var j=0; j<configData[2].SecurityElementSections.length;j++)
	{
		var thisElementStr = JSON.stringify(configData[2].SecurityElementSections[j]);
		thisFileLength += thisElementStr.length;
		thisEntry.SecurityElementSections.push(JSON.parse(thisElementStr));
		if ((thisFileLength > targetSize) && (j < (configData[2].SecurityElementSections.length - 1)))
		{
			thisEntry = addEntry();
			thisFileLength = 0;
		}
	}
	console.log(transferData);
}

function downloadSettings(sender)
{
	downloadConfig(0x0080); //send just this
}

function getSpeedNames(ofModel)
{
	var speedArray = [];
//	console.log(ofModel);
	for (var i = 0; i < ofModel.length; i++)
	{
		speedArray.push(ofModel[i].ModelName);
	}
//	console.log(speedArray);
	return speedArray;
}

function getSpeedListNames(ofModel)
{
	var speedArray = [];
	for (var i = 0; i < ofModel.SpeedOptions.length; i++)
	{
		speedArray.push(ofModel.SpeedOptions[i].SpeedName);
	}
	console.log(speedArray);
	return speedArray;
}

function setSpeedData(sender)
{
	var modelBase;
	var thisTable;
	if (sender.id.indexOf("dynspeedconfig") >= 0)
	{
		modelBase = configData[2].DynSpeedModelList;
		thisTable = dynSpeedTable;
	}
	else
	{
		modelBase = configData[2].StaticSpeedModelList;
		thisTable = staticSpeedTable;
	}
//	console.log(modelBase);
	var thisRow = parseInt(sender.getAttribute("row"));
	var thisCol = parseInt(sender.getAttribute("col"));
	var thisIndex = parseInt(sender.getAttribute("index"));
	var thisCmdLine = parseInt(sender.getAttribute("cmdline"));
	if (isNaN(thisCmdLine))
		thisCmdLine = parseInt(sender.parentElement.getAttribute("cmdline"));
	console.log(thisRow, thisCmdLine, thisCol, thisIndex, sender);
	var thisElement;

	switch (thisCol)
	{
		case -1:
			switch (thisRow)
			{
				case -1:
					modelBase.push(JSON.parse(JSON.stringify(newSpeedListTemplate)));
					break;
				default:
					console.log(modelBase);
					modelBase[thisRow].SpeedOptions.push(JSON.parse(JSON.stringify(newSpeedLineTemplate)));
					break;
			}
			loadSpeedTable(thisTable, modelBase);
			break;
		case 1:
			switch (thisIndex)
			{
				case 1: //new Cmd Entry
					modelBase.splice(thisRow+1, 0, JSON.parse(JSON.stringify(newSpeedListTemplate)));
					break;
				case 2: //delete Cmd Entry
					modelBase.splice(thisRow, 1);
					break;
				case 3: //Up
					if (thisRow > 0)
					{
						thisElement = modelBase.splice(thisRow, 1);
						console.log(thisElement);
						modelBase.splice(thisRow-1,0, thisElement[0]);
					}
					break;
				case 4: //Down
					if (thisRow < modelBase.length)
					{
						thisElement = modelBase.splice(thisRow, 1);
						console.log(thisElement);
						modelBase.splice(thisRow+1,0, thisElement[0]);
					}
					break;
			}
			console.log(modelBase);
			loadSpeedTable(thisTable, modelBase);
			break;
		case 2:
			modelBase[thisRow].ModelName = sender.value;
			break;
		case 3:
			switch (thisIndex)
			{
				case 1: //new Cmd Entry
					modelBase[thisRow].SpeedOptions.splice(thisCmdLine+1, 0, JSON.parse(JSON.stringify(newSpeedLineTemplate)));
					break;
				case 2: //delete Cmd Entry
					modelBase[thisRow].SpeedOptions.splice(thisCmdLine, 1);
					break;
				case 3: //Up
					if (thisCmdLine > 0)
					{
						thisElement = modelBase[thisRow].SpeedOptions.splice(thisCmdLine, 1);
						modelBase[thisRow].SpeedOptions.splice(thisCmdLine-1,0, thisElement[0]);
					}
					break;
				case 4: //Down
					if (thisCmdLine < modelBase[thisRow].SpeedOptions.length)
					{
						thisElement = modelBase[thisRow].SpeedOptions.splice(thisCmdLine, 1);
						modelBase[thisRow].SpeedOptions.splice(thisCmdLine+1,0, thisElement[0]);
					}
					break;
				case 5: //Speed Name
					modelBase[thisRow].SpeedOptions[thisCmdLine].SpeedName = sender.value;
					break;
				case 6: //Speed value
					modelBase[thisRow].SpeedOptions[thisCmdLine].SpeedVal = verifyNumber(sender.value, modelBase[thisRow].SpeedOptions[thisCmdLine].SpeedVal);
					break;
			}		
			loadSpeedTable(thisTable, modelBase);
			break;
	}
	
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

function setAspectData(sender)
{
	var thisRow = parseInt(sender.getAttribute("row"));
	var thisCol = parseInt(sender.getAttribute("col"));
	var thisIndex = parseInt(sender.getAttribute("index"));
	var thisCmdLine = parseInt(sender.getAttribute("cmdline"));
	if (isNaN(thisCmdLine))
		thisCmdLine = parseInt(sender.parentElement.getAttribute("cmdline"));
	console.log(thisRow, thisCmdLine, thisCol, thisIndex, sender);
	var thisElement;
	switch (thisCol)
	{
		case -1:
			switch (thisRow)
			{
				case -1:
					configData[2].AspectGeneratorList.push(JSON.parse(JSON.stringify(newAspectListTemplate)));
					break;
				default:
//					console.log(configData[2].AspectGeneratorList);
					configData[2].AspectGeneratorList[thisRow].Aspects.push(JSON.parse(JSON.stringify(newAspectLineTemplate)));
					break;
			}
			loadAspectTable(aspectGenTable, configData[2].AspectGeneratorList);
			break;
		case 1:
			switch (thisIndex)
			{
				case 1: //new Cmd Entry
					configData[2].AspectGeneratorList.splice(thisRow+1, 0, JSON.parse(JSON.stringify(newAspectListTemplate)));
					break;
				case 2: //delete Cmd Entry
					configData[2].AspectGeneratorList.splice(thisRow, 1);
					break;
				case 3: //Up
					if (thisRow > 0)
					{
						thisElement = configData[2].AspectGeneratorList.splice(thisRow, 1);
						console.log(thisElement);
						configData[2].AspectGeneratorList.splice(thisRow-1,0, thisElement[0]);
					}
					break;
				case 4: //Down
					if (thisRow < configData[2].AspectGeneratorList.length)
					{
						thisElement = configData[2].AspectGeneratorList.splice(thisRow, 1);
						console.log(thisElement);
						configData[2].AspectGeneratorList.splice(thisRow+1,0, thisElement[0]);
					}
					break;
			}
			console.log(configData[2].AspectGeneratorList);
			loadAspectTable(aspectGenTable, configData[2].AspectGeneratorList);
			break;
		case 2:
			switch (thisIndex)
			{
				case 5: 
					configData[2].AspectGeneratorList[thisRow].AspectSetName = sender.value;
					break;
				case 6:
					configData[2].AspectGeneratorList[thisRow].DynSpeedModel = sender.options[sender.selectedIndex].text;
					break;
				case 7:
					configData[2].AspectGeneratorList[thisRow].StaticSpeedModel = sender.options[sender.selectedIndex].text;
					break;
			}
			break;
		case 3:
			switch (thisIndex)
			{
				case 1: //new Cmd Entry
					configData[2].AspectGeneratorList[thisRow].Aspects.splice(thisCmdLine+1, 0, JSON.parse(JSON.stringify(newAspectLineTemplate)));
					break;
				case 2: //delete Cmd Entry
					configData[2].AspectGeneratorList[thisRow].Aspects.splice(thisCmdLine, 1);
					break;
				case 3: //Up
					if (thisCmdLine > 0)
					{
						thisElement = configData[2].AspectGeneratorList[thisRow].Aspects.splice(thisCmdLine, 1);
						configData[2].AspectGeneratorList[thisRow].Aspects.splice(thisCmdLine-1,0, thisElement[0]);
					}
					break;
				case 4: //Down
					if (thisCmdLine < configData[2].AspectGeneratorList[thisRow].Aspects.length)
					{
						thisElement = configData[2].AspectGeneratorList[thisRow].Aspects.splice(thisCmdLine, 1);
						configData[2].AspectGeneratorList[thisRow].Aspects.splice(thisCmdLine+1,0, thisElement[0]);
					}
					break;
				case 5: //Aspect Name
					configData[2].AspectGeneratorList[thisRow].Aspects[thisCmdLine].AspectLabel = sender.value;
					break;
				case 6: //Speed value
					configData[2].AspectGeneratorList[thisRow].Aspects[thisCmdLine].AspectValue = verifyNumber(sender.value, configData[2].AspectGeneratorList[thisRow].Aspects[thisCmdLine].AspectValue);
					break;
				case 7:
					configData[2].AspectGeneratorList[thisRow].Aspects[thisCmdLine].DynamicSpeed = sender.options[sender.selectedIndex].text;
					break;
				case 8:
					configData[2].AspectGeneratorList[thisRow].Aspects[thisCmdLine].StaticSpeed = sender.options[sender.selectedIndex].text;
					break;
				case 9: //Color
						var thisColorStr = sender.value.replace("#", "0x");
						var thisColor = parseInt(thisColorStr);
						var RGBVal = [0,0,0];
						RGBVal[0] = (thisColor & 0xFF0000) >> 16;
						RGBVal[1] = (thisColor & 0x00FF00) >> 8;
						RGBVal[2] = (thisColor & 0x0000FF);
						configData[2].AspectGeneratorList[thisRow].Aspects[thisCmdLine].DispColHSV = rgbToHsv(RGBVal[0],RGBVal[1],RGBVal[2]);
						console.log(configData[2].AspectGeneratorList[thisRow].Aspects[thisCmdLine].DispColHSV);
					break;
				case 10: //Blink
					configData[2].AspectGeneratorList[thisRow].Aspects[thisCmdLine].DispBlink = sender.checked;
					console.log(configData[2].AspectGeneratorList[thisRow].Aspects[thisCmdLine].DispBlink);
					break;
			}		
			loadAspectTable(aspectGenTable, configData[2].AspectGeneratorList);
			break;
	}
}

function tfAspectSelector(y, x, id, evtHandler)
{
	var divElement = document.createElement("div");
	divElement.setAttribute("class", "cmdsel");
	tfSetCoordinate(divElement, y, x, 0, id);
		
	var upperDiv = document.createElement("div");
	upperDiv.setAttribute("class", "editorpanel");
	divElement.append(upperDiv);

	var thisId = "manipulatorbox_" + y.toString() + "_" + x.toString();
	var manpulatorElement = tfManipulatorBox(y, x, thisId, evtHandler);
	upperDiv.append(manpulatorElement);

	var thisText = tfText(y, x, id, evtHandler);
	thisText.innerHTML = "Name:&nbsp;";
	upperDiv.append(thisText);

	thisId = "aspectnamebox_" + y.toString() + "_" + x.toString();
	var addrBox = tfNumericLong(y, 2, thisId, evtHandler);
	addrBox.setAttribute("index", 5);
	upperDiv.append(addrBox);
		
	var lowerDiv = document.createElement("div");
	lowerDiv.setAttribute("class", "editorpanel");
	divElement.append(lowerDiv);
		
	var thisText = tfText(y, x, "", evtHandler);
	thisText.innerHTML = "Dyn. Speed:&nbsp;";
	lowerDiv.append(thisText);

	thisID = "dynspeedselbox_" + y.toString() + "_" + x.toString();
	console.log(configData[2].DynSpeedModelList);
	thisElement = tfSpeedSelector(y, 2, thisID, evtHandler, getSpeedNames(configData[2].DynSpeedModelList));
	thisElement.setAttribute("cmdline", x);
	tfSetCoordinate(thisElement, y, 2, 6, thisID);
	lowerDiv.append(thisElement);

	
	var thisText = tfText(y, x, "", evtHandler);
	thisText.innerHTML = "&nbsp;Tech. Speed:&nbsp;";
	lowerDiv.append(thisText);
	
	thisID = "statspeedselbox_" + y.toString() + "_" + x.toString();
	thisElement = tfSpeedSelector(y, 7, thisID, evtHandler, getSpeedNames(configData[2].StaticSpeedModelList));
	thisElement.setAttribute("cmdline", x);
	tfSetCoordinate(thisElement, y, 2, 7, thisID);
	lowerDiv.append(thisElement);
		
	return divElement;
}

function tfModelSelector(y, x, id, evtHandler)
{
	var divElement = document.createElement("div");
	divElement.setAttribute("class", "cmdsel");
	tfSetCoordinate(divElement, y, x, 0, id);
		
	var upperDiv = document.createElement("div");
	upperDiv.setAttribute("class", "editorpanel");
	divElement.append(upperDiv);

	var thisId = "manipulatorbox_" + y.toString() + "_" + x.toString();
	var manpulatorElement = tfManipulatorBox(y, x, thisId, evtHandler);
	upperDiv.append(manpulatorElement);

	var thisText = tfText(y, x, id, evtHandler);
	thisText.innerHTML = "Name:&nbsp;";
	upperDiv.append(thisText);

	thisId = "modnamebox_" + y.toString() + "_" + x.toString();
	var addrBox = tfNumericLong(y, x, thisId, evtHandler);
	addrBox.setAttribute("index", 8);
	upperDiv.append(addrBox);
		
	var centerDiv = document.createElement("div");
	centerDiv.setAttribute("class", "editorpanel");
	divElement.append(centerDiv);

	var thisText = tfText(y, x, id, evtHandler);
	thisText.innerHTML = "Ctrl Mode:&nbsp;";
	centerDiv.append(thisText);
	
	var modeArray = ["none","ABSS","ABSD","APB","CTC"];

	thisID = "modesel_" + y.toString() + "_" + x.toString();
	thisElement = tfSpeedSelector(y, 9, thisID, evtHandler, modeArray);
	thisElement.setAttribute("cmdline", 9);
	thisElement.selectedIndex = 0;
	tfSetCoordinate(thisElement, y, x, 11, thisID);
	centerDiv.append(thisElement);
	
	thisID = "modelactive_" + y.toString() + "_" + x.toString();
	thisElement = tfCheckBox(y, 9, thisID, evtHandler);
	thisElement.childNodes[0].setAttribute("index", 9);
	thisElement.childNodes[1].innerHTML = "Active &nbsp;";
	tfSetCoordinate(thisElement, y, x, 12, thisID);

	centerDiv.append(thisElement);

	var lowerDiv = document.createElement("div");
	lowerDiv.setAttribute("class", "editorpanel");
	divElement.append(lowerDiv);
		
	var thisText = tfText(y, x, "", evtHandler);
	thisText.innerHTML = "Dyn. Speed:&nbsp;";
	lowerDiv.append(thisText);

	thisID = "dynspeedselmod_" + y.toString() + "_" + x.toString();
	thisElement = tfSpeedSelector(y, 6, thisID, evtHandler, getSpeedNames(configData[2].DynSpeedModelList));
	thisElement.setAttribute("cmdline", x);
//	thisElement.selectedIndex = ;
	tfSetCoordinate(thisElement, y, x, 11, thisID);
	lowerDiv.append(thisElement);

	
	var thisText = tfText(y, x, "", evtHandler);
	thisText.innerHTML = "&nbsp;Tech. Speed:&nbsp;";
	lowerDiv.append(thisText);
	
	thisID = "statspeedselmod_" + y.toString() + "_" + x.toString();
	thisElement = tfSpeedSelector(y, 7, thisID, evtHandler, getSpeedNames(configData[2].StaticSpeedModelList));
	thisElement.setAttribute("cmdline", x);
//	thisElement.selectedIndex = ;
	tfSetCoordinate(thisElement, y, x, 12, thisID);
	lowerDiv.append(thisElement);
		
	return divElement;
}

function tfSpeedSelector(y, x, id, evtHandler, speedArray)
{
	
	var selectList = document.createElement("select");
	createOptions(selectList, speedArray);
	tfSetCoordinate(selectList, y, x, 0, id);
	selectList.setAttribute("class", "selectorbox");
	selectList.setAttribute("onchange", evtHandler);
	return selectList;
}


function createSpeedField(parentObj, tableId, lineIndex, cmdIndex, cmdLineData, evtHandler)
{
	var upperDiv = document.createElement("div");
	upperDiv.setAttribute("class", "cmdedit");
	var thisID;

	thisID = tableId + "cmdmanipulator_" + lineIndex.toString() + "_" + cmdIndex.toString();
	var thisBox = tfManipulatorBox(lineIndex, 3, thisID, evtHandler);
	tfSetCoordinate(thisBox, lineIndex, cmdIndex, 3, thisID);
	thisBox.setAttribute("cmdline", cmdIndex);
	upperDiv.append(thisBox);
			
	var thisText = tfText(lineIndex, cmdIndex, "", evtHandler);
	thisText.innerHTML = "Speed Name:&nbsp;";
	upperDiv.append(thisText);

	thisID = tableId + "namesel_" + lineIndex.toString() + "_" + cmdIndex.toString();
	thisElement = tfNumericLong(lineIndex, 3, thisID, evtHandler);
	thisElement.setAttribute("cmdline", cmdIndex);
	thisElement.value = cmdLineData.SpeedOptions[cmdIndex].SpeedName;
	tfSetCoordinate(thisElement, lineIndex, 3, 5, thisID);
	upperDiv.append(thisElement);

	
	var thisText = tfText(lineIndex, cmdIndex, "", evtHandler);
	thisText.innerHTML = "&nbsp;Speed Value:&nbsp;";
	upperDiv.append(thisText);
	
	thisID = tableId + "valsel_" + lineIndex.toString() + "_" + cmdIndex.toString();
	thisElement = tfNumeric(lineIndex, 3, thisID, evtHandler);
	thisElement.setAttribute("cmdline", cmdIndex);
	thisElement.value = cmdLineData.SpeedOptions[cmdIndex].SpeedVal;
	tfSetCoordinate(thisElement, lineIndex, 3, 6, thisID);
	upperDiv.append(thisElement);


	parentObj.append(upperDiv);
}

function buildSpeedLines(lineIndex, tableId, lineData)
{
	console.log(tableId + "_inp_" + lineIndex.toString() + "_3");
	var thisLineBase = document.getElementById(tableId + "_inp_" + lineIndex.toString() + "_3");
	while (thisLineBase.hasChildNodes())
		thisLineBase.removeChild(thisLineBase.childNodes[0]); //delete rows
	if (lineData.SpeedOptions.length > 0)
		for (var i=0; i<lineData.SpeedOptions.length; i++)
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
//			thisID = tableId + "_cmdbasedata_" + lineIndex.toString() + "_" + i.toString();
			
			createSpeedField(masterDiv, tableId, lineIndex, i, lineData, "setSpeedData(this)");
		}
	else
	{
		var mainDiv = document.createElement("div");
		mainDiv.setAttribute("class", "editortile");
		tfSetCoordinate(mainDiv, i, 0, 0, thisID);
		thisLineBase.append(mainDiv);
		var thisId = tableId + "_cmdbasedata_initadd" + lineIndex.toString();
		var newRB = tfTableStarterBox(lineIndex, -1, thisId, "setSpeedData(this)");
		mainDiv.append(newRB);
	}
}

function loadSpeedTable(thisTable, thisData)
{
	var th = document.getElementById(thisTable.id + "_head");
	var tb = document.getElementById(thisTable.id + "_body");
	var numCols = th.childNodes[0].children.length;
	createDataTableLines(thisTable, [tfPos, tfManipulatorBox, tfNumericLong, tfEmptyTile], thisData.length, "setSpeedData(this)");
	for (var i=0; i<thisData.length;i++)
	{
//		console.log(thisTable.id + "_inp_" + i.toString() + "_" + "2");
		var e = document.getElementById(thisTable.id + "_inp_" + i.toString() + "_" + "2");
		e.value = thisData[i].ModelName;
		buildSpeedLines(i, thisTable.id, thisData[i]);
	}
}

function createAspectField(parentObj, lineIndex, cmdIndex, cmdLineData, evtHandler)
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

	var upperDiv = document.createElement("div");
	upperDiv.setAttribute("class", "editorpanel3");
	var thisID;

	thisID = "aspectmanipulator_" + lineIndex.toString() + "_" + cmdIndex.toString();
	var thisBox = tfManipulatorBox(lineIndex, 3, thisID, evtHandler);
	tfSetCoordinate(thisBox, lineIndex, cmdIndex, 3, thisID);
	thisBox.setAttribute("cmdline", cmdIndex);
	upperDiv.append(thisBox);
			
	var thisText = tfText(lineIndex, cmdIndex, "", evtHandler);
	thisText.innerHTML = "Aspect Label:&nbsp;";
	upperDiv.append(thisText);

	thisID = "namesel_" + lineIndex.toString() + "_" + cmdIndex.toString();
	thisElement = tfNumericLong(lineIndex, 3, thisID, evtHandler);
	thisElement.setAttribute("cmdline", cmdIndex);
	thisElement.value = cmdLineData.Aspects[cmdIndex].AspectLabel;
	tfSetCoordinate(thisElement, lineIndex, 3, 5, thisID);
	upperDiv.append(thisElement);

	
	var thisText = tfText(lineIndex, cmdIndex, "", evtHandler);
	thisText.innerHTML = "&nbsp;Aspect Value:&nbsp;";
	upperDiv.append(thisText);
	
	thisID = "valsel_" + lineIndex.toString() + "_" + cmdIndex.toString();
	thisElement = tfNumeric(lineIndex, 3, thisID, evtHandler);
	thisElement.setAttribute("cmdline", cmdIndex);
	thisElement.value = cmdLineData.Aspects[cmdIndex].AspectValue;
	tfSetCoordinate(thisElement, lineIndex, 3, 6, thisID);
	upperDiv.append(thisElement);

	parentObj.append(upperDiv);
	
	var centerDiv = document.createElement("div");
	centerDiv.setAttribute("class", "editorpanel3");
	var thisSpacer = document.createElement("div");
	thisSpacer.setAttribute("class", "manipulatorbox");
	centerDiv.append(thisSpacer);
	
	var thisText = tfText(lineIndex, cmdIndex, "", evtHandler);
	thisText.innerHTML = "Dynamic Speed:&nbsp;";
	centerDiv.append(thisText);

	thisID = "dynspeedsel_" + lineIndex.toString() + "_" + cmdIndex.toString();
	var dynSpeedModel = getSpeedNames(configData[2].DynSpeedModelList).indexOf(cmdLineData.DynSpeedModel);
	var dynSpeedArray = getSpeedListNames(configData[2].DynSpeedModelList[dynSpeedModel]);
	thisElement = tfSpeedSelector(lineIndex, 3, thisID, evtHandler, dynSpeedArray);
	thisElement.setAttribute("cmdline", cmdIndex);
	thisElement.selectedIndex = dynSpeedArray.indexOf(cmdLineData.Aspects[cmdIndex].DynamicSpeed);
	tfSetCoordinate(thisElement, lineIndex, 3, 7, thisID);
	centerDiv.append(thisElement);

	
	var thisText = tfText(lineIndex, cmdIndex, "", evtHandler);
	thisText.innerHTML = "&nbsp;Technical Speed:&nbsp;";
	centerDiv.append(thisText);
	
	thisID = "statspeedsel_" + lineIndex.toString() + "_" + cmdIndex.toString();
	var staticSpeedModel = getSpeedNames(configData[2].StaticSpeedModelList).indexOf(cmdLineData.StaticSpeedModel);
	var staticSpeedArray = getSpeedListNames(configData[2].StaticSpeedModelList[staticSpeedModel]);
	thisElement = tfSpeedSelector(lineIndex, 3, thisID, evtHandler, staticSpeedArray);
	thisElement.setAttribute("cmdline", cmdIndex);
	thisElement.selectedIndex = staticSpeedArray.indexOf(cmdLineData.Aspects[cmdIndex].StaticSpeed);
	tfSetCoordinate(thisElement, lineIndex, 3, 8, thisID);
	centerDiv.append(thisElement);

	parentObj.append(centerDiv);

	var lowerDiv = document.createElement("div");
	lowerDiv.setAttribute("class", "editorpanel3");
	var thisSpacer = document.createElement("div");
	thisSpacer.setAttribute("class", "manipulatorbox");
	lowerDiv.append(thisSpacer);
	
	var thisText = tfText(lineIndex, cmdIndex, "", evtHandler);
	thisText.innerHTML = "Display Color:&nbsp;";
	lowerDiv.append(thisText);

	thisID = "dispcolsel_" + lineIndex.toString() + "_" + cmdIndex.toString();
	thisElement = tfColorPicker(lineIndex, 3, thisID, evtHandler);
	thisElement.setAttribute("cmdline", cmdIndex);

	var RGBVal = HSVtoRGB(cmdLineData.Aspects[cmdIndex].DispColHSV[0], cmdLineData.Aspects[cmdIndex].DispColHSV[1], cmdLineData.Aspects[cmdIndex].DispColHSV[2]);
//			console.log(thisData[i]);
	var colStr = toHex(RGBVal);
	thisElement.value = colStr;



//	thisElement.value = cmdLineData.Aspects[cmdIndex].AspectLabel;
	tfSetCoordinate(thisElement, lineIndex, 3, 9, thisID);
	lowerDiv.append(thisElement);

	thisID = "blinktypesel_" + lineIndex.toString() + "_" + cmdIndex.toString();
	thisElement = tfCheckBox(lineIndex, 3, thisID, evtHandler);
	thisElement.childNodes[0].setAttribute("index", 10);
	thisElement.childNodes[1].innerHTML = "Blinking &nbsp;";
	thisElement.setAttribute("cmdline", cmdIndex);
	tfSetCoordinate(thisElement, lineIndex, 3, 10, thisID);
	thisElement.childNodes[0].checked = cmdLineData.Aspects[cmdIndex].DispBlink;

	lowerDiv.append(thisElement);

	parentObj.append(lowerDiv);
	
}

function buildAspectLines(lineIndex, lineData)
{
	var thisLineBase = document.getElementById("aspectconfig_inp_" + lineIndex.toString() + "_2");
	while (thisLineBase.hasChildNodes())
		thisLineBase.removeChild(thisLineBase.childNodes[0]); //delete rows
	if (lineData.Aspects.length > 0)
		for (var i=0; i<lineData.Aspects.length; i++)
		{
			var masterDiv = document.createElement("div");
			masterDiv.setAttribute("class", "mastertile3L");
			if ((i % 2) == 0) //even
				masterDiv.style.backgroundColor = "#F5F5F5";
			else
				masterDiv.style.backgroundColor = "#D3D3D3";
			var thisID = "master_" + lineIndex.toString() + "_" + i.toString();
			tfSetCoordinate(masterDiv, i, 0, 0, thisID);
			thisLineBase.append(masterDiv);

			thisID = "pos_" + lineIndex.toString() + "_" + i.toString();
			masterDiv.append(tfPos(i, -1, thisID, ""));

			thisID = "aspectbasedata_" + lineIndex.toString() + "_" + i.toString();
			var mainDiv = document.createElement("div");
			mainDiv.setAttribute("class", "editortile");
			tfSetCoordinate(mainDiv, i, 0, 0, thisID);
			masterDiv.append(mainDiv);
			
			createAspectField(mainDiv, lineIndex, i, lineData, "setAspectData(this)");
		}
	else
	{
		var mainDiv = document.createElement("div");
		mainDiv.setAttribute("class", "editortile");
		tfSetCoordinate(mainDiv, i, 0, 0, thisID);
		thisLineBase.append(mainDiv);
		var thisId = "cmdbasedata_initadd" + lineIndex.toString();
		var newRB = tfTableStarterBox(lineIndex, -1, thisId, "setAspectData(this)");
		mainDiv.append(newRB);
	}
}

function loadAspectTable(thisTable, thisData)
{
	var th = document.getElementById(thisTable.id + "_head");
	var tb = document.getElementById(thisTable.id + "_body");
	var numCols = th.childNodes[0].children.length;
	createDataTableLines(thisTable, [tfPos, tfAspectSelector, tfCommandEditor], thisData.length, "setAspectData(this)");
	for (var i=0; i<thisData.length;i++)
	{
//		console.log(thisTable.id + "_inp_" + i.toString() + "_" + "2");
		var e = document.getElementById("aspectnamebox_" + i.toString() + "_" + "1");
		e.value = thisData[i].AspectSetName;

		var e = document.getElementById("dynspeedselbox_" + i.toString() + "_" + "1");
		e.selectedIndex = getSpeedNames(configData[2].DynSpeedModelList).indexOf(thisData[i].DynSpeedModel);
		var e = document.getElementById("statspeedselbox_" + i.toString() + "_" + "1");
		e.selectedIndex = getSpeedNames(configData[2].StaticSpeedModelList).indexOf(thisData[i].StaticSpeedModel);

		buildAspectLines(i, thisData[i]);
	}
}

function loadSecElModelTable(thisTable, thisData)
{
	var th = document.getElementById(thisTable.id + "_head");
	var tb = document.getElementById(thisTable.id + "_body");
	var numCols = th.childNodes[0].children.length;
	createDataTableLines(thisTable, [tfPos, tfModelSelector, tfCommandEditor], thisData.length, "setModelData(this)");
	for (var i=0; i<thisData.length;i++)
	{
//		console.log(thisTable.id + "_inp_" + i.toString() + "_" + "2");
		var e = document.getElementById("modnamebox_" + i.toString() + "_" + "1");
		e.value = thisData[i].ModelName;

		var e = document.getElementById("dynspeedselmod_" + i.toString() + "_" + "1");
		e.selectedIndex = getSpeedNames(configData[2].DynSpeedModelList).indexOf(thisData[i].DynSpeedModel);
		var e = document.getElementById("statspeedselmod_" + i.toString() + "_" + "1");
		e.selectedIndex = getSpeedNames(configData[2].StaticSpeedModelList).indexOf(thisData[i].StaticSpeedModel);


//		buildModelLines(i, thisData[i]);
	}
}

function constructPageContent(contentTab)
{
	var tempObj;
	mainScrollBox = createEmptyDiv(contentTab, "div", "pagetopicboxscroll-y", "nodeconfigdiv");
		createPageTitle(mainScrollBox, "div", "tile-1", "BasicCfg_Title", "h1", "Security Element Configuration");
		createPageTitle(mainScrollBox, "div", "tile-1", "", "h2", "Dynamic Speed Models");
		dynSpeedTable = createDataTable(mainScrollBox, "tile-1_2", ["Pos","Add/Delete/Up/Down", "Model Name", "Speed Options"], "dynspeedconfig", "");

		createPageTitle(mainScrollBox, "div", "tile-1", "", "h2", "Static Speed Limits");
		staticSpeedTable = createDataTable(mainScrollBox, "tile-1_2", ["Pos","Add/Delete/Up/Down","Model Name", "Speed Options"], "statspeedconfig", "");


		createPageTitle(mainScrollBox, "div", "tile-1", "", "h2", "Signal Aspect Generators");
		aspectGenTable = createDataTable(mainScrollBox, "tile-1_2", ["Pos","Aspect Set Configuration","Aspects"], "aspectconfig", "");


		createPageTitle(mainScrollBox, "div", "tile-1", "", "h2", "Security Element Models");
		secelModelTable = createDataTable(mainScrollBox, "tile-1_2", ["Pos","Model Configuration", "Security Element Editor"], "secelconfig", "");


		tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
			createButton(tempObj, "", "Save & Restart", "btnSave", "saveSettings(this)");
			createButton(tempObj, "", "Cancel", "btnCancel", "cancelSettings(this)");
		tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
			createButton(tempObj, "", "Save to File", "btnDownload", "downloadSettings(this)");
}

function loadNodeDataFields(jsonData)
{
	//get a copy of node.cfg in case it is needed (not here)
}

function loadDataFields(jsonData)
{
	console.log(jsonData);
	loadSpeedTable(dynSpeedTable, jsonData.DynSpeedModelList);
	loadSpeedTable(staticSpeedTable, jsonData.StaticSpeedModelList);
	loadAspectTable(aspectGenTable, jsonData.AspectGeneratorList);
	loadSecElModelTable(secelModelTable, jsonData.SecurityElementSections);
}
