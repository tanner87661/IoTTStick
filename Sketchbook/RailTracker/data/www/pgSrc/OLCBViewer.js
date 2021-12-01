var numMsgLines = 20;
var flowActive = true;
var mainScrollBox;
var listViewer;

function loadTableData(thisTable, thisData)
{
/*	while (thisData.length > maxButtons)
		thisData.pop();
	while (thisData.length < maxButtons)
	{
	}
	console.log(thisData);
	var th = document.getElementById(buttonTable.id + "_head");
	var tb = document.getElementById(buttonTable.id + "_body");
	var numCols = th.childNodes[0].children.length;

/*	createDataTableLines(thisTable, [tfPos,tfText,tfBtnEvtSel,tfNumeric, tfText], thisData.length, "setButtonData(this)");	
	for (var i=0; i<thisData.length;i++)
	{
		function selByName(prmVal)
		{
			return (prmVal == thisData[i].ButtonType);
		}

		var pinNr = nodeConfigData.BtnModConfig.DataPins[Math.trunc(i/16)];
		var pinPort = i % 16;
		var e = document.getElementById(thisTable.id + "_" + i.toString() + "_" + "1");
		writeTextField(e.id, thisData[i].PortNr.toString() + " - Port " + pinPort.toString() + " at Pin " + pinNr.toString());
		var e = document.getElementById(thisTable.id + "_" + i.toString() + "_" + "2");
		e.childNodes[0].selectedIndex = btnStatus.findIndex(selByName);
		var e = document.getElementById(thisTable.id + "_" + i.toString() + "_" + "3");
		e.childNodes[0].value = thisData[i].ButtonAddr;
		var e = document.getElementById(thisTable.id + "_" + i.toString() + "_" + "4");
		writeTextField(e.id, "");
	}
*/
}

function constructPageContent(contentTab)
{
	var tempObj;
	mainScrollBox = createEmptyDiv(contentTab, "div", "pagetopicboxscroll-y", "lnviewerdiv");
		createPageTitle(mainScrollBox, "div", "tile-1", "", "h1", "OpenLCB Message Viewer");
		tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
			listViewer = createListViewer(tempObj, "viewerbox", "OLCBViewer");

		tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
			createEmptyDiv(tempObj, "div", "tile-1_4", "");
			createButton(tempObj, "tile-1_4", "Stop", "btnStop", "stopMsgFlow(this)");
			createButton(tempObj, "tile-1_4", "Clear", "btnClear", "clearMsgList(this)");
}

function stopMsgFlow(sender)
{
	flowActive = !flowActive;
	if (flowActive)
		sender.innerHTML  = "Stop"
	else
		sender.innerHTML  = "Resume";
		
}

function clearMsgList(sender)
{
	while (listViewer.childNodes.length > 0)
	  listViewer.removeChild(listViewer.childNodes[0]);
}

function loadNodeDataFields(jsonData)
{
}

function loadDataFields(jsonData)
{
}

function processOLCBInput(jsonData)
{
	function buildDispLine(jsonData)
	{
//		console.log(jsonData);

		var dispLine = "OLCB: ";
		dispLine += "Prio: " + jsonData.priority.toString();
		dispLine += " FT: " + jsonData.frameType.toString();
		dispLine += " CFT: " + jsonData.canFrameType.toString();
		dispLine += " From Alias: " + jsonData.srcAlias.toString();
//		dispLine += " Dest Alias: " + jsonData.dstAlias.toString();
		dispLine += " MTI: " + jsonData.MTI.toString(16).toUpperCase();
		dispLine += " Data:";
        for (var i=0; i < jsonData.dlc; i++)
        {
			if (i > 0)
				dispLine += ", ";
			if ((jsonData.Data[i] & 0xF0) == 0)
				dispLine += "0x0" + jsonData.Data[i].toString(16).toUpperCase();
			else
				dispLine += "0x" + jsonData.Data[i].toString(16).toUpperCase();
		}
		return dispLine;
	}
	
	if (flowActive)
	{
		var newLine = document.createElement("li");
		newLine.innerHTML = buildDispLine(jsonData);
		listViewer.append(newLine);
		while (listViewer.childNodes.length > numMsgLines)
			listViewer.removeChild(listViewer.childNodes[0]);
	}
}
