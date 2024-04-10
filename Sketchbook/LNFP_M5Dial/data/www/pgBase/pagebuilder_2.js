var sourceArray = ["Switch", "Dyn. Signal", "DCC Signal", "Button","Analog Value", "Block Detector", "Transponder", "Power State", "Analog Scaler"];
var enableArray = ["Always On", "Always Off", "Button", "Switch", "Block Detector"];
var btnArray = ["Btn Down","Btn Up","Btn Click","Btn Hold", "Btn Dbl Click"];
var btnArray0x03 = ["Btn Down","Btn Up"];
var truefalseArray = ["false","true"];
var posnegArray = ["positive","negative"];
var msgTypeArray = ["Input Report", "Switch Report", "Button Down/Up"];
var turnoutTypeArray = ["ZPIN (!)", "DCC", "Servo", "VPIN"];
var startupArray = ["restore", "set active", "set inactive"];
var trackArrayUNO = ["NONE", "MAIN", "PROG"];
var trackArrayMEGA = ["NONE", "MAIN", "PROG", "DC", "DCX"];

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

function tfLinkText(y, x, id, evtHandler)
{
	return tfLink(y, x, 99, id, "Run as template", evtHandler);
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


function tfTemplateTypeSel(y, x, id, evtHandler)
{
	var selectList = document.createElement("select");
	tfSetCoordinate(selectList, y, x, 0, id);
	selectList.setAttribute("class", "selectorbox");
	selectList.setAttribute("onchange", evtHandler);
	return selectList;
}

function tfInpTypeSel(y, x, id, evtHandler)
{
	var selectList = tfTemplateTypeSel(y, x, id, evtHandler);
	createOptions(selectList, btnArray);
	return selectList;
}

function tfEvtTypeSrcSel(y, x, id, evtHandler)
{
	var selectList = tfTemplateTypeSel(y, x, id, evtHandler);
	createOptions(selectList, sourceArray);
	return selectList;
}

function tfStartupSel(y, x, id, evtHandler)
{
	var selectList = tfTemplateTypeSel(y, x, id, evtHandler);
	createOptions(selectList, startupArray);
	return selectList;
}

function tfEnableTypeSel(y, x, id, evtHandler)
{
	var selectList = tfTemplateTypeSel(y, x, id, evtHandler);
	createOptions(selectList, enableArray);
	return selectList;
}

function tfEnableTrueFalseSel(y, x, id, evtHandler)
{
	var selectList = tfTemplateTypeSel(y, x, id, evtHandler);
	createOptions(selectList, truefalseArray);
	return selectList;
}

function tfEnablePosNegSel(y, x, id, evtHandler)
{
	var selectList = tfTemplateTypeSel(y, x, id, evtHandler);
	createOptions(selectList, posnegArray);
	return selectList;
}

function tfTurnoutTypeSel(y, x, id, evtHandler)
{
	var selectList = tfTemplateTypeSel(y, x, id, evtHandler);
	createOptions(selectList, turnoutTypeArray);
	return selectList;
}

function tfMessageTypeSel(y, x, id, evtHandler)
{
	var selectList = tfTemplateTypeSel(y, x, id, evtHandler);
	createOptions(selectList, msgTypeArray);
	return selectList;
}

function tfTrackSelUNO(y, x, id, evtHandler)
{
	var selectList = tfTemplateTypeSel(y, x, id, evtHandler);
	createOptions(selectList, trackArrayUNO);
	return selectList;
}

function tfTrackSelMEGA(y, x, id, evtHandler)
{
	var selectList = tfTemplateTypeSel(y, x, id, evtHandler);
	createOptions(selectList, trackArrayMEGA);
	return selectList;
}

function tfBtnEvtSel(y, x, id, evtHandler)
{
	var selectList = tfTemplateTypeSel(y, x, id, evtHandler);
	createOptions(selectList, ["Off","Digital","Analog","Detector","Switch Report"]);
	return selectList;
}

function tfBtnEvtMask(y, x, id, evtHandler)
{
	function createCB(appenddiv, index, disptext)
	{
		var thisId = "eventmask_" + y.toString() + "_" + x.toString() + "_" + index.toString();
		var cchBox = tfCheckBox(y, x, thisId, evtHandler);
		cchBox.childNodes[0].setAttribute("index", index);
		cchBox.childNodes[1].innerHTML = disptext;
		appenddiv.append(cchBox);
		return cchBox;
	}
		
	var evtMaskDiv = document.createElement("div");
	evtMaskDiv.setAttribute("class", "eventmask");
	tfSetCoordinate(evtMaskDiv, y, x, 0, id);
	createCB(evtMaskDiv, 0, "Btn Down");
	createCB(evtMaskDiv, 1, "Btn Up");
	createCB(evtMaskDiv, 2, "Btn Click");
	createCB(evtMaskDiv, 4, "Btn Dbl Click");
	createCB(evtMaskDiv, 3, "Btn Hold");
	
	return evtMaskDiv;
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

function tfBtnGen(y, x, id, evtHandler, label)
{
	var thisButton = document.createElement("button");
	tfSetCoordinate(thisButton, y, x, 0, id);
	thisButton.setAttribute("onclick", evtHandler);
	thisButton.setAttribute('class', 'slim-button');
	var textElement = document.createTextNode(label);
	thisButton.append(textElement);
	return thisButton;
}

function tfBtnAdd(y, x, id, evtHandler)
{
	return tfBtnGen(y,x,id,evtHandler, "Add");
}

function tfBtnNew(y, x, id, evtHandler)
{
	return tfBtnGen(y,x,id,evtHandler, "New");
}

function tfBtnUpdate(y, x, id, evtHandler)
{
	return tfBtnGen(y,x,id,evtHandler, "Update");
}

function tfBtnCancel(y, x, id, evtHandler)
{
	return tfBtnGen(y,x,id,evtHandler, "Cancel");
}

function tfBtnDelete(y, x, id, evtHandler)
{
	return tfBtnGen(y,x,id,evtHandler, "Delete");
}

function tfBtnMeasure(y, x, id, evtHandler)
{
	return tfBtnGen(y,x,id,evtHandler, "Measure");
}

function tfBtnTogglePwr(y, x, id, evtHandler)
{
	return tfBtnGen(y,x,id,evtHandler, "Toggle Power");
}

function tfBtnReset(y, x, id, evtHandler)
{
	return tfBtnGen(y,x,id,evtHandler, "Reset");
}

function tfBtnToggleAR(y, x, id, evtHandler)
{
	return tfBtnGen(y,x,id,evtHandler, "Toggle AR");
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

function tfCommandEvtSelector(y, x, id, evtHandler)
{
	var divElement = document.createElement("div");
	divElement.setAttribute("class", "sourcesel");
	tfSetCoordinate(divElement, y, x, 0, id);
	var topDiv = document.createElement("div");
	topDiv.setAttribute("class", "editorpanel");
	divElement.append(topDiv);

	var thisId = "manipulatorbox_" + y.toString() + "_" + x.toString();
	var manpulatorElement = tfManipulatorBox(y, x, thisId, evtHandler);
	topDiv.append(manpulatorElement);
	createEvtElements(divElement, y, x, id, evtHandler, true);
	return divElement;

}

function tfCommandSwiSelector(y, x, id, evtHandler)
{
	var divElement = document.createElement("div");
	divElement.setAttribute("class", "sourcesel");
	tfSetCoordinate(divElement, y, x, 0, id);
	createEvtElements(divElement, y, x, id, evtHandler, false);
	var upperDiv = document.createElement("div");
	upperDiv.setAttribute("class", "editorpanel");
	divElement.append(upperDiv);

	var thisId = "templatelink" + y.toString() + "_" + x.toString();
	var templateLink = tfLink(y, x, 25, thisId, "Run as template", evtHandler);
	upperDiv.append(templateLink);
	return divElement;
}

function createEvtElements(divElement, y, x, id, evtHandler, showEnable)
{
	var upperDiv = document.createElement("div");
	upperDiv.setAttribute("class", "editorpanel");
	divElement.append(upperDiv);
	
	var thisText = tfText(y, x, id, evtHandler);
	thisText.innerHTML = "Event Type:&nbsp;";
	upperDiv.append(thisText);

	thisId = "evttypebox_" + y.toString() + "_" + x.toString();
	var typeBox = tfEvtTypeSrcSel(y, x, thisId, evtHandler);
	typeBox.setAttribute("index", 11);
	upperDiv.append(typeBox);

	var lowerDiv = document.createElement("div");
	lowerDiv.setAttribute("class", "editorpanel");
	divElement.append(lowerDiv);
	
	var thisText = tfText(y, x, id, evtHandler);
	thisText.innerHTML = "Addr:&nbsp;";
	lowerDiv.append(thisText);

	thisId = "evtaddressbox_" + y.toString() + "_" + x.toString();
	var addrBox = tfNumeric(y, x, thisId, evtHandler);
	addrBox.setAttribute("index", 12);
	lowerDiv.append(addrBox);
	var thisText = tfText(y, x, id, evtHandler);
	thisText.innerHTML = "&nbsp;Event:&nbsp;";
	lowerDiv.append(thisText);
	thisId = "evtcmdlistbox_" + y.toString() + "_" + x.toString();
	var selBox = tfInpTypeSel(y, x, thisId, evtHandler);
	selBox.setAttribute("index", 13);
	lowerDiv.append(selBox);

	var bottomDiv = document.createElement("div");
	bottomDiv.setAttribute("class", "editorpanel");
	thisId = "parambox_" + y.toString() + "_" + x.toString();
	bottomDiv.setAttribute("id", thisId);
	divElement.append(bottomDiv);
	
	var thisText = tfText(y, x, id, evtHandler);
	thisId = "paramtext_" + y.toString() + "_" + x.toString();
	thisText.setAttribute("id", thisId);
	thisText.innerHTML = "Param:&nbsp;";
	bottomDiv.append(thisText);
	thisId = "address2box_" + y.toString() + "_" + x.toString();
	var addrBox = tfNumeric(y, x, thisId, evtHandler);
	addrBox.setAttribute("index", 14);
	bottomDiv.append(addrBox);

	bottomDiv.append(tfTab(y, x, '&nbsp;',""));
	thisId = "btn_add_" + y.toString() + "_" + x.toString();
	var btnNew = tfBtnNew(y, x, thisId, evtHandler);
	btnNew.setAttribute("index", 15);
	bottomDiv.append(btnNew);
	bottomDiv.append(tfTab(y, x, '&nbsp;',""));
	thisId = "btn_cancel_" + y.toString() + "_" + x.toString();
	var btnDelete = tfBtnDelete(y, x, thisId, evtHandler);
	btnDelete.setAttribute("index", 16);
	bottomDiv.append(btnDelete);

	if (showEnable)
	{
		var topDiv = document.createElement("div");
		topDiv.setAttribute("class", "editorpanel");
		divElement.append(topDiv);

		var thisText = tfText(y, x, id, evtHandler);
		thisText.innerHTML = "Enabled by:&nbsp;";
		topDiv.append(thisText);

		thisId = "enabletypebox_" + y.toString() + "_" + x.toString();
		var typeBox = tfEnableTypeSel(y, x, thisId, evtHandler);
		typeBox.setAttribute("index", 21);
		topDiv.append(typeBox);

		thisId = "enablediv_" + y.toString() + "_" + x.toString();
		var secDiv = document.createElement("div");
		secDiv.setAttribute("class", "editorpanel");
		secDiv.setAttribute("id", thisId);
		divElement.append(secDiv);

		var thisText = tfText(y, x, id, evtHandler);
		thisText.innerHTML = "Addr:&nbsp;";
		secDiv.append(thisText);

		thisId = "enableaddressbox_" + y.toString() + "_" + x.toString();
		var addrBox = tfNumeric(y, x, thisId, evtHandler);
		addrBox.setAttribute("index", 23);
		secDiv.append(addrBox);

		var thisText = tfText(y, x, id, evtHandler);
		thisText.innerHTML = "&nbsp;Event:&nbsp;";
		secDiv.append(thisText);

		thisId = "enablecmdlistbox_" + y.toString() + "_" + x.toString();
		var selBox = tfInpTypeSel(y, x, thisId, evtHandler);
		selBox.setAttribute("index", 24);
		secDiv.append(selBox);
	}
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

	thisId = "addrtext_" + y.toString() + "_" + x.toString();
	var thisText = tfText(y, x, thisId, evtHandler);
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

	var bottomDiv = document.createElement("div");
	bottomDiv.setAttribute("class", "editorpanel");
	divElement.append(bottomDiv);

	var thisSpacer = document.createElement("div");
	thisSpacer.setAttribute("class", "manipulatorbox");
	bottomDiv.append(thisSpacer);

	var thisText = tfText(y, x, id, evtHandler);
	thisText.innerHTML = "Locos:";
	bottomDiv.append(tfTab(y, x, '&nbsp;',""));
	bottomDiv.append(thisText);
	bottomDiv.append(tfTab(y, x, '&nbsp;',""));

	thisId = "transpselbox_" + y.toString() + "_" + x.toString();
	var selBox = tfNumeric(y, x, thisId, evtHandler);
	selBox.setAttribute("index", 10);
	bottomDiv.append(selBox);
	return divElement;
}
