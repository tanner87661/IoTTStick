const numMsgLines = 20;
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
		createDispText(tempObj, "", "Available Memory", "n/a", "heapavail");
}

function constructPageContent(contentTab)
{
	var tempObj;
	mainScrollBox = createEmptyDiv(contentTab, "div", "pagetopicboxscroll-y", "lnviewerdiv");
		createPageTitle(mainScrollBox, "div", "tile-1", "", "h1", "LocoNet Message Viewer");
		tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
			listViewer = createListViewer(tempObj, "viewerbox", "LNViewer");

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

function processStatsData(jsonData)
{
	writeTextField("sysdatetime", jsonData.systime);
	writeTextField("uptime", formatTime(Math.trunc(jsonData.uptime/1000)));
	writeTextField("IPID", jsonData.ipaddress);
	writeTextField("SigStrengthID", jsonData.sigstrength + " dBm");
	writeTextField("firmware", jsonData.version);
	writeTextField("heapavail", jsonData.freemem + " Bytes");
}

function loadDataFields(jsonData)
{
}

function getOpCodeStr(OpCodeVal)
{
	var OpCode;
    switch(OpCodeVal)
    {
        case 0x81 : OpCode = "OPC_BUSY"; break;
        case 0x82 : OpCode = "OPC_GPOFF"; break;
        case 0x83 : OpCode = "OPC_GPON"; break;
        case 0x85 : OpCode = "OPC_IDLE"; break;
        case 0xA0 : OpCode = "OPC_LOCO_SPD"; break;
        case 0xA1 : OpCode = "OPC_LOCO_DIRF"; break;
        case 0xA2 : OpCode = "OPC_LOCO_SND"; break;
        case 0xB0 : OpCode = "OPC_SW_REQ"; break;
        case 0xB1 : OpCode = "OPC_SW_REP"; break;
        case 0xB2 : OpCode = "OPC_INPUT_REP"; break;
        case 0xB3 : OpCode = "OPC_UNKNOWN"; break;
        case 0xB4 : OpCode = "OPC_LONG_ACK"; break;
        case 0xB5 : OpCode = "OPC_SLOT_STAT1"; break;
        case 0xB6 : OpCode = "OPC_CONSIST_FUNC"; break;
        case 0xB8 : OpCode = "OPC_UNLINK_SLOTS"; break;
        case 0xB9 : OpCode = "OPC_LINK_SLOTS"; break;
        case 0xBA : OpCode = "OPC_MOVE_SLOTS"; break;
        case 0xBB : OpCode = "OPC_RQ_SL_DATA"; break;
        case 0xBC : OpCode = "OPC_SW_STATE"; break;
        case 0xBD : OpCode = "OPC_SW_ACK"; break;
        case 0xBF : OpCode = "OPC_LOCO_ADR"; break;
        case 0xD0 : OpCode = "OPC_MULTI_SENSE"; break;
        case 0xD4 : OpCode = "OPC_UHLI-FUN"; break;
        case 0xD7 : OpCode = "OPC_PANEL_RESPONSE"; break;
        case 0xDF : OpCode = "OPC_PANEL_QUERY"; break;
        case 0xE4 : OpCode = "OPC_LISSY_REP"; break;
        case 0xE5 : OpCode = "OPC_PEER_XFER"; break;
        case 0xE6 : OpCode = "OPC_ALM_READ"; break;
        case 0xE7 : OpCode = "OPC_SL_RD_DATA"; break;
        case 0xED : OpCode = "OPC_IMM_PACKET"; break;
        case 0xEE : OpCode = "OPC_WR_LIM_DATA"; break;
        case 0xEF : OpCode = "OPC_WR_SL_DATA"; break;
        default: OpCode = "Unknown OpCode " + OpCodeVal.toString(16); break;
    }
    return OpCode;
}

function processLocoNetInput(jsonData)
{
	function buildDispLine(jsonData)
	{
		var dispLine = getOpCodeStr(jsonData[0]) + " \t\t";
		for (var i = 0; i< jsonData.length;i++)
		{
			if (i>0)
				dispLine += ", ";
			var newTerm = jsonData[i].toString(16).toUpperCase();
			if (newTerm.length < 2)
				dispLine += "0x0" + newTerm;
			else
				dispLine += "0x" + newTerm;
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
