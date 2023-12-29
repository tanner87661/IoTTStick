function createMenueTabElement(parentObj, objType, className, menuName, objID, objTitle, visible, clickFct)
{
	var docElement = document.createElement(objType);
	docElement.setAttribute('class', className);	
	docElement.setAttribute("id", objID);
	docElement.setAttribute("name", menuName);
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

function createSimpleText(parentObj, divclass, dispText, dispObjID)
{
	var textDiv = document.createElement("div");
	textDiv.setAttribute('class', divclass);
	textDiv.innerHTML = dispText;
	parentObj.append(textDiv);
}
function createDispText(parentObj, divclass, labelText, dispText, dispObjID)
{
	var textDiv = document.createElement("div");
	textDiv.setAttribute('class', divclass);
	parentObj.append(textDiv);

	var textElement = document.createElement("div");
	textElement.setAttribute('class', "inputtext_tab1");
	textElement.setAttribute('id', dispObjID + "_txt");
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
		textElement.setAttribute("id", cbObjID + "_tx_" + i.toString());
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

function getOptionList(dropdown_id)
{
//	console.log(dropdown_id);
	var optList = [];
	var e = document.getElementById(dropdown_id);
	for (var i=0; i<e.options.length;i++)
		optList.push(e.options[i].text);
	return optList;
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

function setDropdownValueByText(dropdown_id, newValue)
{
	var e = document.getElementById(dropdown_id);
	for (var i=0; i<e.options.length;i++)
		if (e.options[i].text == newValue)
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
	textElement.setAttribute('id', cbObjID + "_txt");
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
	return thisButton;
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
	elementInput.setAttribute('accept', acceptfiles);
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
