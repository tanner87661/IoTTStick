function enableInput(senderstatus, target)
{
	document.getElementById(target).disabled = !senderstatus;
}

function setVisibility(senderstatus, target, reverse)
{
	if (senderstatus ^ reverse)
		target.style.display = "block";
	else
		target.style.display = "none";
}

function readNumInputField(thisObjID)
{
	var thisField = document.getElementById(thisObjID);
	return verifyNumber(thisField.value, 0);
}

function verifyNumber(inpValue, defValue)
{
	var numVal;
	if (isNaN(inpValue))
	{
		alert(inpValue + " is not a valid number. Please verify");
		return defValue;	
	}
	else
		return parseFloat(inpValue);
}

function verifyNumArray(inpValue, sepChar)
{
	var newInp = inpValue.split(sepChar);
	var newRes = [];
	for (var i=0; i < newInp.length; i++)
	{
		var hlpRes = parseInt(newInp[i]);
		if (!isNaN(hlpRes))
			newRes.push(hlpRes);
	}
	return newRes;
}

function writeInputField(thisObjID, newValue)
{
//	console.log(typeof(newValue), newValue);
	var thisField = document.getElementById(thisObjID);
	if (typeof(newValue) == "number")
		thisField.value = newValue.toString();
	else
		thisField.value = newValue;
}

function readTextInputToArray(thisObjID, minLen, maxLen)
{
	var readVal = readTextInputField(thisObjID);
	var newVal = readVal.split(',');
	var intVal = [];
	for (var i=0; i<newVal.length; i++)
	  if ((newVal[i] != "") && !isNaN(newVal[i]))
		intVal.push(parseInt(newVal[i]));
	if (((intVal.length >= minLen) && (intVal.length <= maxLen)) && (intVal.length == newVal.length))
		return intVal;
	else
	{
		alert(readVal + " is not a correct comma separated array. Please verify");
		return -1;
	}
}

function readTextInputField(thisObjID)
{
	var thisField = document.getElementById(thisObjID);
	return thisField.value;
}

function readCBInputField(thisObjID)
{
	var thisField = document.getElementById(thisObjID);
	return thisField.checked;
}

function writeCBInputField(thisObjID, newValue)
{
	var thisField = document.getElementById(thisObjID);
	thisField.checked = newValue;
}

function writeRBInputField(thisObjID, newValue)
{
	var thisField = document.getElementById(thisObjID + "_" + parseInt(newValue).toString());
	if (thisField != null)
		thisField.checked = true;
}

function writeTextField(thisObjID, newValue)
{
	var thisField = document.getElementById(thisObjID);
	if (thisField != null)
		thisField.innerHTML = newValue;
}

function formatTime(seconds) {
    return [
        parseInt(seconds / 60 / 60),
        parseInt(seconds / 60 % 60),
        parseInt(seconds % 60)
    ]
        .join(":")
        .replace(/\b(\d)\b/g, "0$1")
}

function createListViewer(parentObj, className, dispListID)
{
	var docElement = document.createElement("ul");
	docElement.setAttribute('class', className);	
	docElement.setAttribute("id", dispListID);
	parentObj.append(docElement);
	return docElement;
}

function tfCmdTypeSel(y, x, id, evtHandler)
{
	var selectList = document.createElement("select");
	createOptions(selectList, ["Switch", "Switch (Ack)", "DCC Signal", "Button", "Analog Value", "Block Detector", "Power Status"]);
	tfSetCoordinate(selectList, y, x, 5, id);
	selectList.setAttribute("class", "selectorbox");
	selectList.setAttribute("onchange", evtHandler);
	return selectList;
}

function tfCmdEvtLineHeader(y, x, id, evtHandler, lineData)
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

function tfCmdBtnPosSel(y, x, id, evtHandler)
{
	var selectList = document.createElement("select");
	createOptions(selectList, ["Btn Down", "Btn Up", "Btn Click", "Btn Hold", "Btn Dbl Click"]);
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

function setButtonEditor(lineIndex, i, thisID, evtHandler, lineData)
{
	var thisEditor = document.createElement("div");
	
	var thisSpacer = document.createElement("div");
	thisSpacer.setAttribute("class", "manipulatorbox");
	thisEditor.append(thisSpacer);
	
	var thisText = tfText(lineIndex, i, thisID, evtHandler);
	thisText.innerHTML = "Button Command:";
	thisEditor.append(thisText);
	var thisSel = tfCmdBtnPosSel(lineIndex, i, thisID, evtHandler);
	thisSel.selectedIndex = buttonOptions.indexOf(lineData.CtrlType);
	thisEditor.append(thisSel);
	return thisEditor;
}

function setAnalogEditor(lineIndex, i, thisID, evtHandler, lineData)
{
	var thisEditor = document.createElement("div");
	
	var thisSpacer = document.createElement("div");
	thisSpacer.setAttribute("class", "manipulatorbox");
	thisEditor.append(thisSpacer);
	
	var thisText = tfText(lineIndex, i, thisID, evtHandler);
	thisText.innerHTML = "Analog Value:";
	thisEditor.append(thisText);

	thisElement = tfNumeric(lineIndex, 6, thisID, evtHandler);
	thisElement.setAttribute("cmdline", i);
	thisElement.value = (parseInt(lineData.CtrlType) << 8) + parseInt(lineData.CtrlValue);
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

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
function tfColorTypeSel(y, x, id, evtHandler)
{
	var selectList = document.createElement("select");
	createOptions(selectList, ["RGB", "HSV"]);
	tfSetCoordinate(selectList, y, x, 0, id);
	selectList.setAttribute("class", "selectorbox");
	selectList.setAttribute("onchange", evtHandler);
	return selectList;
}

function tfFuseCtrlTypeSel(y, x, id, evtHandler)
{
	var selectList = document.createElement("select");
	createOptions(selectList, ["Very Fast", "Fast", "Slow", "Very Slow", "User defined"]);
	tfSetCoordinate(selectList, y, x, 0, id);
	selectList.setAttribute("class", "selectorbox");
	selectList.setAttribute("onchange", evtHandler);
	return selectList;
}

function tfLEDCtrlTypeSel(y, x, id, evtHandler)
{
	var selectList = document.createElement("select");
	createOptions(selectList, ["Switch", "Dyn. Signal", "DCC Signal", "Button", "Analog Value", "Block Detector", "Transponder", "Power Status", "Steady ON"]);
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
