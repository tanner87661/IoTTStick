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

function tfServoEditor(y, x, id, evtHandler)
{
//--main
	var divElement = document.createElement("div");
	divElement.setAttribute("class", "servosel");

//--radio
	var thisId = "seldevpanel_" + y.toString() + "_" + x.toString();
	var radioDiv = document.createElement("div");
	radioDiv.setAttribute("class", "servotile");
	radioDiv.setAttribute("id", thisId);
	divElement.append(radioDiv);
	
	var thisId = "seldevtype_" + y.toString() + "_" + x.toString();
	createRadiobox(radioDiv, "tile-1_1", "Output Type:", devOptionsList, thisId, "setOutputMode(this, id)");

//--relay
	var thisId = "relaypanel_" + y.toString() + "_" + x.toString();
	var relayDiv = document.createElement("div");
	relayDiv.setAttribute("class", "servotile");
	relayDiv.setAttribute("id", thisId);
	divElement.append(relayDiv);

//--servo main
	var thisId = "servopanel_" + y.toString() + "_" + x.toString();
	var servoDiv = document.createElement("div");
	servoDiv.setAttribute("class", "servotile");
	servoDiv.setAttribute("id", thisId);
	divElement.append(servoDiv);

//--servo simple
	var thisId = "servomovetype_" + y.toString() + "_" + x.toString();
	createRadiobox(servoDiv, "tile-1_1", "Angle:", ["90+", '90-', '180', "ind."], thisId, "setServoAngle(this, id)");

	thisId = "pwroff_" + y.toString() + "_" + x.toString();
	var cchBox = tfCheckBox(y, x, thisId, evtHandler);
	cchBox.childNodes[0].setAttribute("index", 20);
	cchBox.childNodes[1].innerHTML = "Power off";
	servoDiv.append(cchBox);

	thisId = "pwrinit_" + y.toString() + "_" + x.toString();
	var cchBox = tfCheckBox(y, x, thisId, evtHandler);
	cchBox.childNodes[0].setAttribute("index", 21);
	cchBox.childNodes[1].innerHTML = "Init Pulse";
	servoDiv.append(cchBox);

//--servo speed
	var thisId = "servospeeddiv_" + y.toString() + "_" + x.toString();
	var topDiv = document.createElement("div");
	topDiv.setAttribute("class", "servotile");
	topDiv.setAttribute("id", thisId);
	divElement.append(topDiv);

	topDiv.append(tfTab(y, x, 'Speed Up: &nbsp;',""));
	thisId = "movespeedup_" + y.toString() + "_" + x.toString();
	var speedUpBox = tfNumeric(y, x, thisId, evtHandler);
	speedUpBox.setAttribute("index", 1);
	topDiv.append(speedUpBox);

	topDiv.append(tfTab(y, x, '&nbsp; Speed Down: &nbsp;',""));
	thisId = "movespeeddown_" + y.toString() + "_" + x.toString();
	var speedDnBox = tfNumeric(y, x, thisId, evtHandler);
	speedDnBox.setAttribute("index", 2);
	topDiv.append(speedDnBox);

//--servo accel
	var thisId = "servoacceldiv_" + y.toString() + "_" + x.toString();
	var top2Div = document.createElement("div");
	top2Div.setAttribute("class", "servotile");
	top2Div.setAttribute("id", thisId);
	divElement.append(top2Div);

	top2Div.append(tfTab(y, x, 'Accel: &nbsp;',""));
	thisId = "accel_" + y.toString() + "_" + x.toString();
	var speedBox = tfNumeric(y, x, thisId, evtHandler);
	speedBox.setAttribute("index", 3);
	top2Div.append(speedBox);

	top2Div.append(tfTab(y, x, '&nbsp; Decel: &nbsp;',""));
	thisId = "decel_" + y.toString() + "_" + x.toString();
	var speedBox = tfNumeric(y, x, thisId, evtHandler);
	speedBox.setAttribute("index", 4);
	top2Div.append(speedBox);


//--servo oscillation
	var thisId = "servooscdiv_" + y.toString() + "_" + x.toString();
	var top3Div = document.createElement("div");
	top3Div.setAttribute("class", "servotile");
	top3Div.setAttribute("id", thisId);
	divElement.append(top3Div);

	top3Div.append(tfTab(y, x, 'Frequency: &nbsp;',""));
	thisId = "oscfrequ_" + y.toString() + "_" + x.toString();
	var speedBox = tfNumeric(y, x, thisId, evtHandler);
	speedBox.setAttribute("index", 5);
	top3Div.append(speedBox);

	top3Div.append(tfTab(y, x, '&nbsp; Lambda: &nbsp;',""));
	thisId = "lambda_" + y.toString() + "_" + x.toString();
	var speedBox = tfNumeric(y, x, thisId, evtHandler);
	speedBox.setAttribute("index", 6);
	top3Div.append(speedBox);

//--servo hesitation
	var thisId = "servohesidiv_" + y.toString() + "_" + x.toString();
	var top4Div = document.createElement("div");
	top4Div.setAttribute("class", "servotile");
	top4Div.setAttribute("id", thisId);
	divElement.append(top4Div);

	thisId = "hesitate_" + y.toString() + "_" + x.toString();
	var cchBox = tfCheckBox(y, x, thisId, evtHandler);
	cchBox.childNodes[0].setAttribute("index", 7);
	cchBox.childNodes[1].innerHTML = "Hesitation";
	top4Div.append(cchBox);

	top4Div.append(tfTab(y, x, '&nbsp; at &nbsp;',""));
	thisId = "hesipoint_" + y.toString() + "_" + x.toString();
	var hesiBox = tfNumeric(y, x, thisId, evtHandler);
	hesiBox.setAttribute("index", 8);
	top4Div.append(hesiBox);

	top4Div.append(tfTab(y, x, '&nbsp; Min. Speed: &nbsp;',""));
	thisId = "hesispeed_" + y.toString() + "_" + x.toString();
	var hesiBox = tfNumeric(y, x, thisId, evtHandler);
	hesiBox.setAttribute("index", 9);
	top4Div.append(hesiBox);

//--servo movement
	var thisId = "servomovediv_" + y.toString() + "_" + x.toString();
	var upperDiv = document.createElement("div");
	upperDiv.setAttribute("class", "servotile");
	upperDiv.setAttribute("id", thisId);
	upperDiv.style.backgroundColor = "#F5F5F5";
	divElement.append(upperDiv);

	thisId = "enableevent_" + y.toString() + "_" + x.toString();
	var cchBox = tfCheckBox(y, x, thisId, evtHandler);
	cchBox.childNodes[0].setAttribute("index", 10);
	cchBox.childNodes[1].innerHTML = "Enable &nbsp;";
	upperDiv.append(cchBox);

	upperDiv.append(tfTab(y, x, '&nbsp;',""));

	var sliderDiv = document.createElement("div");
	sliderDiv.setAttribute("class", "slidecontainer");
	upperDiv.append(sliderDiv);
	
	thisId = "posslider_" + y.toString() + "_" + x.toString();
	var sliderElement = document.createElement("input");
	sliderElement.setAttribute("type", "range");
	sliderElement.setAttribute("id", thisId);
	sliderElement.setAttribute("min", 150);
	sliderElement.setAttribute("max", 950);
	sliderElement.setAttribute("value", 250);
//	sliderElement.setAttribute('class', "slider");
	sliderElement.setAttribute("oninput", evtHandler);
	tfSetCoordinate(sliderElement, y, x, 12, thisId);
	upperDiv.append(sliderElement);
	upperDiv.append(tfTab(y, x, '&nbsp;',""));


	thisId = "aspectpos_" + y.toString() + "_" + x.toString();
	var posBox = tfNumeric(y, x, thisId, evtHandler);
	posBox.setAttribute("index", 13);
	upperDiv.append(posBox);

//--start/stop options
	thisId = "servostartstopdiv_" + y.toString() + "_" + x.toString();
	var lowerDiv = document.createElement("div");
	lowerDiv.setAttribute("class", "servotile");
	lowerDiv.setAttribute("id", thisId);
	lowerDiv.style.backgroundColor = "#F5F5F5";
	divElement.append(lowerDiv);


//	var lowerDiv = document.createElement("div");
//	lowerDiv.setAttribute("class", "servotile");
//	lowerDiv.style.backgroundColor = "#F5F5F5";
//	divElement.append(lowerDiv);
	
	thisId = "softaccel_" + y.toString() + "_" + x.toString();
	var cchBox = tfCheckBox(y, x, thisId, evtHandler);
	cchBox.childNodes[0].setAttribute("index", 14);
	cchBox.childNodes[1].innerHTML = "Soft Start";
	lowerDiv.append(cchBox);

	thisId = "stopmode_" + y.toString() + "_" + x.toString();
	var thisRadio = createRadiobox(lowerDiv, "", "Stop:", ["Hard","Soft","Osc.","Bounce"], thisId, evtHandler);
	var indexCtr = 15;
	for(var i = 0; i < thisRadio.children.length; i++)
	{
		if (thisRadio.children[i].getAttribute("class") == "radiobutton")
		{
			var myId = thisId + "_" + i.toString();
			thisRadio.children[i].setAttribute("row", y.toString());
			thisRadio.children[i].setAttribute("col", x.toString());
			thisRadio.children[i].setAttribute("index", indexCtr.toString());
			indexCtr++;
		}
	}

	tfSetCoordinate(divElement, y, x, 0, id);
	return divElement;
}

function tfButtonEditor(y, x, id, evtHandler)
{
	function createBtnPanel(pIx)
	{
		var panelDiv = document.createElement("div");
		panelDiv.setAttribute("class", "servosel");
		if (pIx > 0)
			panelDiv.style.backgroundColor = "#F5F5F5";
		var upperDiv = document.createElement("div");
		upperDiv.setAttribute("class", "servotile");
		upperDiv.append(tfTab(y, x, "Btn. " + (pIx+1).toString() + ": Addr.:&nbsp;",""));
		panelDiv.append(upperDiv);
		thisId = "btnaddressbox" + pIx.toString() + "_" + y.toString() + "_" + x.toString();
		var addrBox = tfNumeric(y, x, thisId, evtHandler);
		addrBox.setAttribute("index", (pIx<<8) + 10);
		upperDiv.append(addrBox);
		var thisText = tfText(y, x, id, evtHandler);
		thisText.innerHTML = "&nbsp;on:&nbsp;";
		upperDiv.append(thisText);
		thisId = "cmdlistbox" + pIx.toString() + "_" + y.toString() + "_" + x.toString();
		var selBox = tfInpTypeSel(y, x, thisId, evtHandler);
		selBox.setAttribute("index", (pIx<<8) + 11);
		upperDiv.append(selBox);

		thisId = "evtselbox" + pIx.toString() + "_" + y.toString() + "_" + x.toString();
		var cchBox = tfCheckBox(y, x, thisId, evtHandler);
		cchBox.childNodes[0].setAttribute("index", (pIx<<8) + 16);
		cchBox.childNodes[1].innerHTML = "show all";
		upperDiv.append(cchBox);

		var lowerDiv = document.createElement("div");
		lowerDiv.setAttribute("class", "servotile");
		lowerDiv.append(tfTab(y, x, "Send:&nbsp;",""));
		panelDiv.append(lowerDiv);
		thisId = "evttypebox" + pIx.toString() + "_" + y.toString() + "_" + x.toString();
		var typeBox = tfTemplateTypeSel(y, x, thisId, evtHandler);
//		createOptions(typeBox, ["Switch","Signal","Block Detector", "Local Only", "none"]);
		createOptions(typeBox, ["Switch","Switch (Ack)","Signal","Block Detector", "none"]);
		typeBox.setAttribute("index", (pIx<<8) + 12);
		lowerDiv.append(typeBox);
		thisId = "targetaddrtext" + pIx.toString() + "_" + y.toString() + "_" + x.toString();
		var thisText = tfText(y, x, thisId, evtHandler);
		thisText.innerHTML = "&nbsp;Addr:&nbsp;";
		lowerDiv.append(thisText);
		thisId = "targetaddressbox" + pIx.toString() + "_" + y.toString() + "_" + x.toString();
		var addrBox = tfNumeric(y, x, thisId, evtHandler);
		addrBox.setAttribute("index", (pIx<<8) + 13);
		lowerDiv.append(addrBox);
		thisId = "evtvaltext" + pIx.toString() + "_" + y.toString() + "_" + x.toString();
		var thisText = tfText(y, x, thisId, evtHandler);
		thisText.innerHTML = "&nbsp;Event:&nbsp;";
		lowerDiv.append(thisText);
		thisId = "evtvalbox" + pIx.toString() + "_" + y.toString() + "_" + x.toString();
		var typeBox = tfTemplateTypeSel(y, x, thisId, evtHandler);
		typeBox.setAttribute("index", (pIx<<8) + 14);
		lowerDiv.append(typeBox);
		thisId = "evtvalfield" + pIx.toString() + "_" + y.toString() + "_" + x.toString();
		var valueField = tfNumeric(y, x, thisId, evtHandler);
		valueField.setAttribute("index", (pIx<<8) + 15);
		lowerDiv.append(valueField);

		return panelDiv;
	}

	var divElement = document.createElement("div");
	divElement.setAttribute("class", "servosel");
	divElement.append(createBtnPanel(0));
	divElement.append(createBtnPanel(1));
	return divElement;
}

function tfLEDEditor(y, x, id, evtHandler)
{
	function createLEDPanel(pIx)
	{
		var panelDiv = document.createElement("div");
		panelDiv.setAttribute("class", "servoledsel");
		if (pIx > 0)
			panelDiv.style.backgroundColor = "#F5F5F5";
		var upperDiv = document.createElement("div");
		upperDiv.setAttribute("class", "servotile");
		panelDiv.append(upperDiv);


		var thisId = "ledlabel" + pIx.toString() + "_" + y.toString() + "_" + x.toString();
		var ledLabel = tfTab(y, x, "LED " + (pIx+1).toString() + " shows:&nbsp;","");
		ledLabel.setAttribute("id", thisId);
		upperDiv.append(ledLabel);
		var thisId = "srclistbox" + pIx.toString() + "_" + y.toString() + "_" + x.toString();
		var selBox = tfTemplateTypeSel(y, x, thisId, evtHandler);
		createOptions(selBox, ["Position","Input 1","Input 2"]);
		selBox.setAttribute("index", (pIx<<8) + 10);
		upperDiv.append(selBox);
		upperDiv.append(tfTab(y, x, "&nbsp;when:&nbsp;",""));
		thisId = "evttypebox" + pIx.toString() + "_" + y.toString() + "_" + x.toString();
		var typeBox = tfTemplateTypeSel(y, x, thisId, evtHandler);
		typeBox.setAttribute("index", (pIx<<8) + 11);
		upperDiv.append(typeBox);

		var centerDiv = document.createElement("div");
		centerDiv.setAttribute("class", "servotile");
		panelDiv.append(centerDiv);
		centerDiv.append(tfTab(y, x, "On-Color:&nbsp;",""));
		thisId = "oncolselbox" + pIx.toString() + "_" + y.toString() + "_" + x.toString();
		var oncolselBox = tfTemplateTypeSel(y, x, thisId, evtHandler);
		oncolselBox.setAttribute("index", (pIx<<8) + 12);
		centerDiv.append(oncolselBox);
		centerDiv.append(tfTab(y, x, "&nbsp;Off-Color:&nbsp;",""));
		thisId = "offcolselbox" + pIx.toString() + "_" + y.toString() + "_" + x.toString();
		var offcolselBox = tfTemplateTypeSel(y, x, thisId, evtHandler);
		offcolselBox.setAttribute("index", (pIx<<8) + 13);
		centerDiv.append(offcolselBox);
		
		var lowerDiv = document.createElement("div");
		lowerDiv.setAttribute("class", "servotile");
		panelDiv.append(lowerDiv);
		lowerDiv.append(tfTab(y, x, "Mode:&nbsp;",""));
		thisId = "blinkselectbox" + pIx.toString() + "_" + y.toString() + "_" + x.toString();
		var typeBox = tfTemplateTypeSel(y, x, thisId, evtHandler);
		createOptions(typeBox, ["Static", "Pos Local Blink", "Neg Local Blink", "Pos Global Blink", "Neg Global Blink","Local Rampup", "Local Rampdown","Global Rampup", "Global Rampdown"]);
		typeBox.setAttribute("index", (pIx<<8) + 14);
		lowerDiv.append(typeBox);
		lowerDiv.append(tfTab(y, x, "&nbsp;Rate:&nbsp;",""));
		thisId = "blinkratebox" + pIx.toString() + "_" + y.toString() + "_" + x.toString();
		var timeBox = tfNumeric(y, x, thisId, evtHandler);
		timeBox.setAttribute("index", (pIx<<8) + 15);
		lowerDiv.append(timeBox);
		lowerDiv.append(tfTab(y, x, "&nbsp;Trans.:&nbsp;",""));
		thisId = "transselectbox" + pIx.toString() + "_" + y.toString() + "_" + x.toString();
		var typeBox = tfTemplateTypeSel(y, x, thisId, evtHandler);
		createOptions(typeBox, ["Soft", "Direct","Merge"]);
		typeBox.setAttribute("index", (pIx<<8) + 16);
		lowerDiv.append(typeBox);
		return panelDiv;
	}
	
	var divElement = document.createElement("div");
	divElement.setAttribute("class", "servosel");
	divElement.append(createLEDPanel(0));
	divElement.append(createLEDPanel(1));
	return divElement;
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
