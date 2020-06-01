function createMenueTabElement(parentObj, objType, className, objID, objTitle, visible, clickFct)
{
//	console.log("Create Menu Tab " + objTitle);
	var docElement = document.createElement(objType);
	docElement.setAttribute('class', className);	
	docElement.setAttribute("id", objID);
	docElement.setAttribute("name", objTitle);
	docElement.setAttribute("onclick", clickFct);
	setVisibility(visible, docElement);
	
	if (docElement.childNodes[0])
    {
        docElement.childNodes[0].nodeValue=objTitle;
    }
    else if (docElement.value)
    {
        docElement.value=objTitle;
    }
    else //if (button.innerHTML)
    {
        docElement.innerHTML=objTitle;
    }	
	parentObj.append(docElement);
	return docElement;
}

function createPageTitle(parentObj, objType, className, objID, titleGrade, objTitle)
{
	var docElement = document.createElement(objType);
	docElement.setAttribute('class', className);	
	docElement.setAttribute("id", objID);
	docElement.setAttribute("name", objTitle);
	objTitle = "<" + titleGrade + ">" + objTitle + "</" + titleGrade + ">";
	if (docElement.childNodes[0])
    {
        docElement.childNodes[0].nodeValue=objTitle;
    }
    else if (docElement.value)
    {
        docElement.value=objTitle;
    }
    else //if (button.innerHTML)
    {
        docElement.innerHTML=objTitle;
    }	
	parentObj.append(docElement);
	return docElement;
}

function createEmptyDiv(parentObj, objType, className, objID)
{
	var docElement = document.createElement(objType);
	docElement.setAttribute('class', className);	
	docElement.setAttribute("id", objID);
	parentObj.append(docElement);
	return docElement;
}

function createDispText(parentObj, divclass, labelText, dispText, dispObjID)
{
	var textDiv = document.createElement("div");
	textDiv.setAttribute('class', divclass);
	parentObj.append(textDiv);

	var textElement = document.createElement("div");
	textElement.setAttribute('class', "inputtext_tab1");
	textElement.append(document.createTextNode(labelText));
	textDiv.append(textElement);
	
	var dispElement = document.createElement("div");
	dispElement.setAttribute('class', "inputtext_tab2");
	dispElement.setAttribute('id', dispObjID);
	dispElement.append(document.createTextNode(dispText));
	textDiv.append(dispElement);
	return textDiv;
}

function createCheckbox(parentObj, divclass, labelText, cbObjID, onclick)
{
	var textDiv = document.createElement("div");
	textDiv.setAttribute('class', divclass);
	parentObj.append(textDiv);

	var cbElement = document.createElement("input");
	cbElement.setAttribute("type", "checkbox");
	cbElement.setAttribute("id", cbObjID);
	cbElement.setAttribute('class', "checkbox");
	cbElement.setAttribute("onclick", onclick);
	textDiv.append(cbElement);
	
	var textElement = document.createElement("span");
	textElement.setAttribute('class', "checkboxtext");
	textElement.append(document.createTextNode(labelText));
	textDiv.append(textElement);
	return textDiv;
}

function createRadiobox(parentObj, divclass, labelText, optionText, cbObjID, onclick)
{
	var textDiv = document.createElement("div");
	textDiv.setAttribute('class', divclass);
	parentObj.append(textDiv);

	var textElement = document.createElement("span");
	textElement.setAttribute('class', "checkboxtext");
	textElement.append(document.createTextNode(labelText));
	textDiv.append(textElement);

	for (var i=0; i<optionText.length; i++)
	{
		var cbElement = document.createElement("input");
		cbElement.setAttribute("type", "radio");
		cbElement.setAttribute("id", cbObjID + "_" + i.toString());
		cbElement.setAttribute("name", cbObjID);
		cbElement.setAttribute('class', "radiobutton");
		cbElement.setAttribute("onclick", onclick);

		var textElement = document.createElement("span");
		textElement.setAttribute('class', "checkboxtext");
		textElement.append(document.createTextNode(optionText[i]));
		textDiv.append(cbElement);
		textDiv.append(textElement);
	}
	return textDiv;
}

function createOptions(dropdownlist, optionText)
{
	while (dropdownlist.length > 0)
		dropdownlist.remove(0);
	for (var i=0; i<optionText.length;i++)
	{
		if (optionText[i] != "")
		{
			var option = document.createElement("option");
			option.value = i;
			option.text = optionText[i];
			dropdownlist.appendChild(option);
		}
	}
}

function getDropdownValue(dropdown_id)
{
	var e = document.getElementById(dropdown_id);
	if (e.selectedIndex >= 0)
		return e.options[e.selectedIndex].value;
	else
		return -1;
}

function setDropdownValue(dropdown_id, newValue)
{
	var e = document.getElementById(dropdown_id);
	for (var i=0; i<e.options.length;i++)
		if (e.options[i].value == newValue)
		{
			e.selectedIndex = i;
//	console.log(i, e.options);
			return;
		}
	e.selectedIndex = -1;
}

function createDropdownselector(parentObj, divclass, labelText, optionText, selObjID, onChange)
{

	var textDiv = document.createElement("div");
	textDiv.setAttribute('class', divclass);
	parentObj.append(textDiv);
	
	var textElement = document.createElement("div");
	textElement.setAttribute('class', "inputtext_tab1");
	textElement.append(document.createTextNode(labelText));
	textDiv.append(textElement);

	var selectList = document.createElement("select");
	selectList.setAttribute("id", selObjID);
	selectList.setAttribute("name", selObjID);
	selectList.setAttribute("onchange", onChange);
	createOptions(selectList, optionText);
	textDiv.append(tfTab(0, 0, '&nbsp;',""));
	textDiv.append(selectList);
}

function createTextInput(parentObj, divclass, labelText, inputText, cbObjID, onchange)
{
	var textDiv = document.createElement("div");
	textDiv.setAttribute('class', divclass);
	parentObj.append(textDiv);

	var textElement = document.createElement("div");
	textElement.setAttribute('class', "inputtext_tab1");
	textElement.append(document.createTextNode(labelText));
	textDiv.append(textElement);

	var inpElement = document.createElement("input");
	inpElement.setAttribute("type", "numinputshort");
	inpElement.setAttribute("id", cbObjID);
	inpElement.setAttribute("onchange", onchange);
	inpElement.value = inputText;
	textDiv.append(inpElement);
}

function createButton(parentObj, divclass, labelText, btnObjID, onclick)
{
	var textDiv = document.createElement("div");
	textDiv.setAttribute('class', divclass);
	parentObj.append(textDiv);

	var thisButton = document.createElement("button");
	thisButton.setAttribute("id", btnObjID);
	thisButton.setAttribute("onclick", onclick);
	thisButton.setAttribute('class', 'mod-button');
	textDiv.append(thisButton);
	var textElement = document.createTextNode(labelText);
	thisButton.append(textElement);
	
}

function createSmallButton(parentObj, divclass, labelText, btnObjID, onclick)
{
	var textDiv = document.createElement("div");
	textDiv.setAttribute('class', divclass);
	parentObj.append(textDiv);

	var thisButton = document.createElement("button");
	thisButton.setAttribute("id", btnObjID);
	thisButton.setAttribute("onclick", onclick);
	thisButton.setAttribute('class', 'slim-button');
	textDiv.append(thisButton);
	var textElement = document.createTextNode(labelText);
	thisButton.append(textElement);
	
}

//the trick here is to make the regular input invisible and use a regular button that can be styled
//when it is clicked, the input onclick method is called to open the file dialog
function createFileDlg(parentObj, divclass, labelText, btnObjID, acceptfiles, onclick)
{
	createButton(parentObj, divclass, labelText, btnObjID + "_button", ""); //make the regular button
	var elementButton = document.getElementById(btnObjID + "_button");
	elementButton.style.display = 'initial';
	var elementInput = document.createElement("input");
	elementInput.setAttribute("type", "file");
	elementInput.setAttribute('accept', 'acceptfiles');
	elementInput.setAttribute("id", btnObjID); //this is the ID for the file input. We keep it the same 
	elementInput.setAttribute("onchange", onclick);
	elementInput.style.display = 'none';
	parentObj.append(elementInput);
	elementButton.addEventListener('click', function (e) {e.preventDefault(); elementInput.click();});
}

function createDataTable(parentObj, divclass, colHeaders, baseObjId, onchange) //table function for button configuration
{
	var tableDiv = document.createElement("div");
	tableDiv.setAttribute('class', divclass);
//	tableDiv.setAttribute("cmdList", onchange);
	parentObj.append(tableDiv);
	
	var tt = document.createElement("table");
	tt.setAttribute("class", "table");
	tt.setAttribute("id", baseObjId);
	var th = document.createElement("thead");
	th.setAttribute("id", baseObjId + "_head");
	tt.append(th);
	var tb = document.createElement("tbody");
	tb.setAttribute("id", baseObjId + "_body");
	tt.append(tb);
	var tf = document.createElement("tfoot");
	tt.append(tf);

	var newRow = document.createElement("tr");
	newRow.setAttribute("class", "th");
	th.append(newRow);

	for (var i=0; i < colHeaders.length; i++)
	{
//		console.log(colHeaders[i]);
		var newCol = document.createElement("td");
		newCol.setAttribute("id", baseObjId + "_h_" + i.toString());
		var newRB = document.createTextNode(colHeaders[i]);
		newCol.append(newRB);
		newRow.append(newCol);
	}
	tableDiv.append(tt);
	return tt;
}

function tfSetCoordinate(element, row, col, index, id)
{
	element.setAttribute("id", id);
	element.setAttribute("row", row);
	element.setAttribute("col", col);
	element.setAttribute("index", index);
}

function tfPos(y, x, id, evtHandler)
{
	var newRB = tfText(y, x, id, evtHandler);
	tfSetCoordinate(newRB, y, x, 0, id);
	newRB.setAttribute("class", "posfield");
	newRB.innerHTML = y + 1;
	return newRB;
}

function tfText(y, x, id, evtHandler)
{
	var textDiv = document.createElement("div");
	tfSetCoordinate(textDiv, y, x, 0, id);
	textDiv.setAttribute("class", "numinputstandard");
	textDiv.append(document.createTextNode(id.toString()));
	return textDiv;
}

function tfLink(y, x, index, id, linkText, evtHandler)
{
	var textDiv = document.createElement("a");
	tfSetCoordinate(textDiv, y, x, index, id);
	textDiv.setAttribute("class", "numinputstandard");
	textDiv.setAttribute("href", "javascript:void function nop(){}();");
	textDiv.setAttribute("onclick", evtHandler);
	textDiv.innerHTML = linkText;
	return textDiv;
}

function tfTab(y, x, dispText, evtHandler)
{
	var textDiv = document.createElement("div");
	tfSetCoordinate(textDiv, y, x, 0, 0);
	textDiv.setAttribute("class", "numinputstandard");
	textDiv.innerHTML = dispText;
//	textDiv.append(document.createTextNode(dispText));
	return textDiv;
}

function tfInpTypeSel(y, x, id, evtHandler)
{
	
	var selectList = document.createElement("select");
	createOptions(selectList, ["Btn Down","Btn Up","Btn Click","Btn Hold", "Btn Dbl Click", "Analog"]);
	tfSetCoordinate(selectList, y, x, 0, id);
	selectList.setAttribute("class", "selectorbox");
	selectList.setAttribute("onchange", evtHandler);
	return selectList;
}

function tfBtnEvtSel(y, x, id, evtHandler)
{
	
	var selectList = document.createElement("select");
	createOptions(selectList, ["Off","Digital","Touch","Analog"]);
	tfSetCoordinate(selectList, y, x, 0, id);
	selectList.setAttribute("class", "selectorbox");
	selectList.setAttribute("onchange", evtHandler);
	return selectList;
}

function tfNumeric(y, x, id, evtHandler)
{
	var inpElement = document.createElement("input");
	inpElement.setAttribute("class", "numinputmedium");
	tfSetCoordinate(inpElement, y, x, 0, id);
	inpElement.setAttribute("onchange", evtHandler);
	return inpElement;
}

function tfNumericLong(y, x, id, evtHandler)
{
	var inpElement = document.createElement("input");
	inpElement.setAttribute("class", "inputtext_long");
	tfSetCoordinate(inpElement, y, x, 0, id);
	inpElement.setAttribute("onchange", evtHandler);
	return inpElement;
}

function tfBtnAdd(y, x, id, evtHandler)
{
	var thisButton = document.createElement("button");
	tfSetCoordinate(thisButton, y, x, 0, id);
	thisButton.setAttribute("onclick", evtHandler);
	thisButton.setAttribute('class', 'slim-button');
	var textElement = document.createTextNode("Add");
	thisButton.append(textElement);
	return thisButton;
}

function tfBtnCancel(y, x, id, evtHandler)
{
	var thisButton = document.createElement("button");
	tfSetCoordinate(thisButton, y, x, 0, id);
	thisButton.setAttribute("onclick", evtHandler);
	thisButton.setAttribute('class', 'slim-button');
	var textElement = document.createTextNode("Cancel");
	thisButton.append(textElement);
	return thisButton;
}

function tfColorPicker(y, x, id, evtHandler)
{
	var inpElement = document.createElement("input");
	tfSetCoordinate(inpElement, y, x, 0, id);
	inpElement.setAttribute("class", "colorpicker");
	inpElement.setAttribute("type", "color");
	inpElement.setAttribute("onchange", evtHandler);
	inpElement.value = "#ff0000";
	return inpElement;
	
}

function tfTableStarterBox(y, x, id, evtHandler)
{
	var divElement = document.createElement("div");
	divElement.setAttribute("class", "manipulatorbox");
	tfSetCoordinate(divElement, y, x, 0, id);
	var imgElement;
	imgElement = document.createElement("img");
	imgElement.setAttribute('src', 'Add.png');
	imgElement.setAttribute('alt', 'Add');
	imgElement.setAttribute('class', 'img_icon');
	imgElement.setAttribute('name', "add_" + i);
	tfSetCoordinate(imgElement, y, x, 1, id);
	imgElement.setAttribute("onclick", evtHandler);
	divElement.append(imgElement);
	return divElement;
}

function tfManipulatorBox(y, x, id, evtHandler)
{
	var divElement = document.createElement("div");
	divElement.setAttribute("class", "manipulatorbox");
	tfSetCoordinate(divElement, y, x, 0, id);
	var imgElement;
	imgElement = document.createElement("img");
	imgElement.setAttribute('src', 'Add.png');
	imgElement.setAttribute('alt', 'Add');
	imgElement.setAttribute('class', 'img_icon');
	imgElement.setAttribute('name', "add_" + i);
	tfSetCoordinate(imgElement, y, x, 1, id);
	imgElement.setAttribute("onclick", evtHandler);
	divElement.append(imgElement);
	imgElement = document.createElement("img");
	imgElement.setAttribute('src', 'Delete.png');
	imgElement.setAttribute('alt', 'Delete');
	imgElement.setAttribute('class', 'img_icon');
	imgElement.setAttribute('name', "del_" + i);
	tfSetCoordinate(imgElement, y, x, 2, id);
	imgElement.setAttribute("onclick", evtHandler);
	divElement.append(imgElement);
	imgElement = document.createElement("img");
	imgElement.setAttribute('src', 'Up.png');
	imgElement.setAttribute('alt', 'Up');
	imgElement.setAttribute('class', 'img_icon');
	imgElement.setAttribute('name', "up_" + i);
	tfSetCoordinate(imgElement, y, x, 3, id);
	imgElement.setAttribute("onclick", evtHandler);
	divElement.append(imgElement);
	imgElement = document.createElement("img");
	imgElement.setAttribute('src', 'Down.png');
	imgElement.setAttribute('alt', 'Down');
	imgElement.setAttribute('class', 'img_icon');
	imgElement.setAttribute('name', "down_" + i);
	tfSetCoordinate(imgElement, y, x, 4, id);
	imgElement.setAttribute("onclick", evtHandler);
	divElement.append(imgElement);
	return divElement;
}

function tfCommandSelector(y, x, id, evtHandler)
{
	var divElement = document.createElement("div");
	divElement.setAttribute("class", "cmdsel");
	tfSetCoordinate(divElement, y, x, 0, id);
	var thisId = "manipulatorbox_" + y.toString() + "_" + x.toString();
	var manpulatorElement = tfManipulatorBox(y, x, thisId, evtHandler);
	divElement.append(manpulatorElement);
	thisId = "addressbox_" + y.toString() + "_" + x.toString();
	var addrBox = tfNumeric(y, x, thisId, evtHandler);
	addrBox.setAttribute("index", 5);
	divElement.append(addrBox);
	thisId = "cmdlistbox_" + y.toString() + "_" + x.toString();
	var selBox = tfInpTypeSel(y, x, thisId, evtHandler);
	selBox.setAttribute("index", 6);
	divElement.append(selBox);
	return divElement;
}

function tfLEDSelector(y, x, id, evtHandler)
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

	thisId = "lednrtext_" + y.toString() + "_" + x.toString();
	var thisText = tfLink(y, x, 0, id, "LED ##:", evtHandler);
	thisText.setAttribute("index", 21); //high index not causing LED table reload
	upperDiv.append(thisText);
	upperDiv.append(tfTab(y, x, '&nbsp;',""));

	thisId = "lednrbox_" + y.toString() + "_" + x.toString();
	var ledBox = tfNumeric(y, x, thisId, evtHandler);
	ledBox.setAttribute("index", 5);
	upperDiv.append(ledBox);

	
	thisId = "multicolor_" + y.toString() + "_" + x.toString();
	var cchBox = tfCheckBox(y, x, thisId, evtHandler);
	cchBox.childNodes[0].setAttribute("index", 6);
	cchBox.childNodes[1].innerHTML = "Individual Colors";
	upperDiv.append(cchBox);
	

	var lowerDiv = document.createElement("div");
	lowerDiv.setAttribute("class", "editorpanel");
	divElement.append(lowerDiv);

	var thisSpacer = document.createElement("div");
	thisSpacer.setAttribute("class", "manipulatorbox");
	lowerDiv.append(thisSpacer);

	thisId = "cmdlistbox_" + y.toString() + "_" + x.toString();
	var selBox = tfLEDCtrlTypeSel(y, x, thisId, evtHandler);
	selBox.setAttribute("index", 7);
	lowerDiv.append(selBox);

	var thisText = tfText(y, x, id, evtHandler);
	thisText.innerHTML = "Addr.:";
	lowerDiv.append(tfTab(y, x, '&nbsp;',""));
	lowerDiv.append(thisText);
	lowerDiv.append(tfTab(y, x, '&nbsp;',""));

	thisId = "addressbox_" + y.toString() + "_" + x.toString();
	var addrBox = tfNumeric(y, x, thisId, evtHandler);
	addrBox.setAttribute("index", 8);
	lowerDiv.append(addrBox);
	
	var lowerDiv = document.createElement("div");
	lowerDiv.setAttribute("class", "editorpanel");
	divElement.append(lowerDiv);

	var thisSpacer = document.createElement("div");
	thisSpacer.setAttribute("class", "manipulatorbox");
	lowerDiv.append(thisSpacer);

	var thisText = tfText(y, x, id, evtHandler);
	thisText.innerHTML = "Color Adj.:";
	lowerDiv.append(tfTab(y, x, '&nbsp;',""));
	lowerDiv.append(thisText);
	lowerDiv.append(tfTab(y, x, '&nbsp;',""));

	thisId = "disptypebox_" + y.toString() + "_" + x.toString();
	var selBox = tfLEDDispTypeSel(y, x, thisId, evtHandler);
	selBox.setAttribute("index", 9);
	lowerDiv.append(selBox);
	
	return divElement;
}

function tfBtnCommandEditor(y, x, id, evtHandler)
{
	var divElement = document.createElement("div");
	tfSetCoordinate(divElement, y, x, 0, id);
	divElement.setAttribute("class", "cmdedit");
	divElement.append(tfPos(y, x, id, evtHandler));
	var mainDiv = document.createElement("div");
	mainDiv.setAttribute("class", "editortile");
	
	var upperDiv = document.createElement("div");
	upperDiv.setAttribute("class", "editorpanel");
	upperDiv.append(tfManipulatorBox(y, x, id, evtHandler));
	upperDiv.append(tfCmdLineHeader(y, x, id, evtHandler));
	mainDiv.append(upperDiv);
	var lowerDiv = document.createElement("div");
	lowerDiv.setAttribute("class", "editorpanel");
	var hlpDiv = tfEmptyTile(y, x, id, evtHandler);
	hlpDiv.setAttribute("class", "manipulatorbox");
	lowerDiv.append(hlpDiv);
//	lowerDiv.append(tfCmdLineEditor(x, y, id, evtHandler));
	mainDiv.append(lowerDiv);

	divElement.append(mainDiv);

	return divElement;
}

function tfCheckBox(y, x, id, evtHandler)
{
	var textDiv = document.createElement("div");

	var cbElement = document.createElement("input");
	cbElement.setAttribute("type", "checkbox");
	cbElement.setAttribute("id", id);
	cbElement.setAttribute('class', "checkbox");
	cbElement.setAttribute("onclick", evtHandler);
	tfSetCoordinate(cbElement, y, x, 0, id);
	textDiv.append(cbElement);
	
	var textElement = document.createElement("span");
	textElement.setAttribute('id', id + "_cbtxt");
	textElement.setAttribute('class', "checkboxtext");
	textElement.append(document.createTextNode(""));
	textDiv.append(textElement);
	return textDiv;
}

function tfCommandEditor(y, x, id, evtHandler)
{
	var divElement = document.createElement("div");
	divElement.setAttribute("class", "cmdedit");
	tfSetCoordinate(divElement, y, x, 0, id);
	return divElement;
}

function tfEmptyTile(y, x, id, evtHandler)
{
	var divElement = document.createElement("div");
	divElement.setAttribute("id", id);
	divElement.append(document.createTextNode("xx"));
	return divElement;
}

function createDataTableLines(tableObj, colLoaders, numLines, onchange)
{
	var th = document.getElementById(tableObj.id + "_head");
	var tb = document.getElementById(tableObj.id + "_body");
	var numCols = th.childNodes[0].children.length;
	while (tb.hasChildNodes())
		tb.removeChild(tb.childNodes[0]); //delete rows
	if (numLines > 0)
		for (var i=0; i < numLines; i++)
		{
			var newRow = document.createElement("tr");
			newRow.setAttribute("class", "th");
			tb.append(newRow);
			for (var j=0; j < numCols; j++)
			{
				var thisId = tableObj.id + "_inp_" + i.toString() + "_" + j.toString();
				var newCol = document.createElement("td");
				newCol.setAttribute("id", tableObj.id + "_" + i.toString() + "_" + j.toString());
				newRow.append(newCol);
				var newRB = colLoaders[j](i, j, thisId, onchange);
				newCol.append(newRB);
			}
		}
	else
	{
		var newRow = document.createElement("tr");
		newRow.setAttribute("class", "th");
		tb.append(newRow);
		var thisId = tableObj.id + "_initadd";
		var newCol = document.createElement("td");
		newRow.append(newCol);
		var newRB = tfTableStarterBox(-1, -1, thisId, onchange);
		newCol.append(newRB);
	}
}

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
	var thisField = document.getElementById(thisObjID + "_" + newValue.toString());
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
