var mainScrollBox;
var listViewerRot;
var listViewerSingle;

var flowActiveSingle = true;
var flowActiveRot = true;

var numMsgLines = 20;

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

		var pinNr = configData[0].BtnModConfig.DataPins[Math.trunc(i/16)];
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
	mainScrollBox = createEmptyDiv(contentTab, "div", "pagetopicboxscroll-y", "btnconfigdiv");
		createPageTitle(mainScrollBox, "div", "tile-1", "", "h1", "DCC Track Signal Viewer");
		tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
			var leftDiv = createEmptyDiv(tempObj, "div", "tile-1_2", "leftside");
				createPageTitle(leftDiv, "div", "tile-1", "", "h2", "Cyclical Updates");
				listViewerRot = createListViewer(leftDiv, "viewerbox", "LNViewerCyclic");
				var thisObj = createEmptyDiv(leftDiv, "div", "tile-1", "");
					createButton(thisObj, "tile-1_4", "Stop", "btnStop", "stopRotFlow(this)");
					createButton(thisObj, "tile-1_4", "Clear", "btnClear", "clearRotList(this)");

			var rightDiv = createEmptyDiv(tempObj, "div", "tile-1_2", "rightside");
				createPageTitle(rightDiv, "div", "tile-1", "", "h2", "One-time messages");
				listViewerSingle = createListViewer(rightDiv, "viewerbox", "LNViewerSingleShot");
				thisObj = createEmptyDiv(rightDiv, "div", "tile-1", "");
					createButton(thisObj, "tile-1_4", "Stop", "btnStop", "stopSingleFlow(this)");
					createButton(thisObj, "tile-1_4", "Clear", "btnClear", "clearSingleList(this)");

		tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
}

function loadNodeDataFields(jsonData)
{
}

function loadDataFields(jsonData)
{
}

function stopRotFlow(sender)
{
	flowActiveRot = !flowActiveRot;
	if (flowActiveRot)
		sender.innerHTML  = "Stop"
	else
		sender.innerHTML  = "Resume";
		
}

function clearRotList(sender)
{
	while (listViewerRot.childNodes.length > 0)
	  listViewerRot.removeChild(listViewerRot.childNodes[0]);
}

function stopSingleFlow(sender)
{
	flowActiveSingle = !flowActiveSingle;
	if (flowActiveSingle)
		sender.innerHTML  = "Stop"
	else
		sender.innerHTML  = "Resume";
		
}

function clearSingleList(sender)
{
	while (listViewerSingle.childNodes.length > 0)
	  listViewerSingle.removeChild(listViewerSingle.childNodes[0]);
}

function processDCCInput(jsonData)
{
	var newLine;
	if (jsonData.oneTime)
		if (flowActiveSingle)
		{
			newLine = document.createElement("li");
			newLine.innerHTML = jsonData.dispStr;
			listViewerSingle.append(newLine);
		}
		else
		{}
	else
		if (flowActiveRot)
		{
			while (listViewerRot.childNodes.length > 0)
				listViewerRot.removeChild(listViewerRot.childNodes[0]);
			var dispArray = jsonData.dispStr.split('\r');
			for (var i=0; i < dispArray.length; i++)
			{
				newLine = document.createElement("li");
				newLine.innerHTML = dispArray[i];
				listViewerRot.append(newLine);
			}
		}
	while (listViewerSingle.childNodes.length > numMsgLines)
		listViewerSingle.removeChild(listViewerSingle.childNodes[0]);

}

/*
function processDCCInput(jsonData)
{
	function buildDispLine(jsonData)
	{
		var dispLine = "Sync Bits: " + jsonData[0].toString() + " DCC Packet: ";
		var dccBaseAddr = jsonData[1];
		var decAddr=0;
		var decInstr=0xFF;
		if (dccBaseAddr === 0)
		{
			if (jsonData.length == 4) //MF7
				dispLine = "MF7 BC ";
			else
				dispLine = "MF14 BC ";
			decAddr = 0;
		}
		else
			if (dccBaseAddr < 128)
			{
				dispLine += "MF7 ";
				decAddr = dccBaseAddr;
			}
			else 
				if (dccBaseAddr < 192)
				{
					var BoardAddress = (((~jsonData[2]) & 0b01110000) << 2) | (jsonData[1] & 0b00111111);
					var TurnoutPairIndex = (jsonData[2] & 0b00000110) >> 1;
					decAddr = (((BoardAddress - 1) << 2 ) | TurnoutPairIndex) + 1 ; //decoder output addresses start with 1, packet address range starts with 0
					if (jsonData.length == 4) //Base Acc Packet
					{
						dispLine += "Acc. ";
						
//						decAddr = (((((jsonData[2] & 0x70) << 2) + ((jsonData[1] & 0x3F)))-1)<<2) + ((jsonData[2] & 0x06) >> 1)+1;
						decInstr = (jsonData[2] & 0x01);
					}
					else
					{
						dispLine += "Ext. Acc. ";
//						decAddr = (((((jsonData[2] & 0x70) << 2) + ((jsonData[1] & 0x3F)))-1)<<2) + ((jsonData[2] & 0x06) >> 1);
						decInstr = (jsonData[3] & 0x1F);
					}
				}
				else
					if (dccBaseAddr < 232)
					{
						dispLine += "MF14 ";
						if (((jsonData[1] & 0xC0) == 0xC0) && ((jsonData[1] & 0x3F) != 0x3F)) //extended Address
						{
							decAddr = ((jsonData[1] & 0x3F) << 8) + jsonData[2];
							decInstr =  (jsonData[3] & 0xE0) >> 5;
						}
						else
						{
							decAddr = jsonData[1];
							decInstr =  (jsonData[2] & 0xE0) >> 5;
						}
					}
					else
						if (dccBaseAddr < 255)
						{
							dispLine += "Res. "
						}
						else
						{
							dispLine += "Idle "
						}
						
		dispLine += "Addr: " + decAddr.toString();
		if (decInstr != 0xFF)
			dispLine += " Instr.: " + decInstr.toString() + " DCC Msg: [";
		else
			dispLine += " DCC Msg: [";
		
		for (var i = 1; i < jsonData.length-1;i++)
		{
			if (i>1)
				dispLine += ", ";
			var newTerm = jsonData[i].toString(16).toUpperCase();
			if (newTerm.length < 2)
				dispLine += "0x0" + newTerm;
			else
				dispLine += "0x" + newTerm;
		}
		dispLine += "]";
		return dispLine;
	}

	var newLine;
	if ((jsonData[1] >= 128) && (jsonData[1] < 192))
		if (flowActiveSingle)
		{
			newLine = document.createElement("li");
			newLine.innerHTML = buildDispLine(jsonData);
			listViewerSingle.append(newLine);
		}
		else
		{}
	else
		if (flowActiveRot)
		{
			newLine = document.createElement("li");
			newLine.innerHTML = buildDispLine(jsonData);
			listViewerRot.append(newLine);
		}
	while (listViewerRot.childNodes.length > numMsgLines)
		listViewerRot.removeChild(listViewerRot.childNodes[0]);
	while (listViewerSingle.childNodes.length > numMsgLines)
		listViewerSingle.removeChild(listViewerSingle.childNodes[0]);
}
*/
