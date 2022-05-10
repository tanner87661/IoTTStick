var numMsgLines = 20;
var flowActive = true;
var mainScrollBox;
var listViewer;
var showPlain = true;
var lastLACKMsg = [];


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
		createPageTitle(mainScrollBox, "div", "tile-1", "", "h1", "LocoNet Message Viewer");
		tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
			createCheckbox(tempObj, "tile-1_4", "Show Plain English Message", "cbPlainMsg", "setMsgStyle(this)");
			document.getElementById("cbPlainMsg").checked = true;
		tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
			listViewer = createListViewer(tempObj, "viewerbox", "LNViewer");

		tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
			createEmptyDiv(tempObj, "div", "tile-1_4", "");
			createButton(tempObj, "tile-1_4", "Stop", "btnStop", "stopMsgFlow(this)");
			createButton(tempObj, "tile-1_4", "Clear", "btnClear", "clearMsgList(this)");
}

function setMsgStyle(sender)
{
	showPlain = sender.checked;
	console.log(sender);
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
        case 0xE0 : OpCode = "OPC_MULTI_SENSE_LONG"; break;
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

function passFilter(lnMsg)
{
	return true;
    if (filterList.indexOf(lnMsg[0]) >= 0) return true;
    if (((filterConfig & opcTrackBit) > 0) && ([0x82, 0x83, 0x85].indexOf(lnMsg[0]) >= 0)) return true;
    if (((filterConfig & opcFastClockBit) > 0) && ([0xE7, 0xEF].indexOf(lnMsg[0]) >= 0) && (lnMsg[2] == 0x7B)) return true;
    if (((filterConfig & opcFastClockBit) > 0) && ([0xBB].indexOf(lnMsg[0]) >= 0) && (lnMsg[1] == 0x7B)) return true;
    if (((filterConfig & opcProgBit) > 0) && ([0xE7, 0xEF].indexOf(lnMsg[0]) >= 0) && (lnMsg[2] == 0x7C)) return true;
    if (((filterConfig & opcProgBit) > 0) && ([0xBB].indexOf(lnMsg[0]) >= 0) && (lnMsg[1] == 0x7C)) return true;
    if (((filterConfig & opcSysBit) > 0) && ([0x81,0xB4].indexOf(lnMsg[0]) >= 0)) return true;
    if (((filterConfig & opcImmBit) > 0) && ([0xED].indexOf(lnMsg[0]) >= 0)) return true;
    if (((filterConfig & opcSVBit) > 0) && (lnMsg[0] == 0xE5)  && (lnMsg[1] == 0x10)) return true;
    if (((filterConfig & opcSwiBit) > 0) && ([0xB0, 0xB1, 0xBC, 0xBD].indexOf(lnMsg[0]) >= 0)) return true;
    if (((filterConfig & opcSigBit) > 0) && (lnMsg[0] == 0xED) && (lnMsg[1] == 0x0B) && (lnMsg[2] == 0x7F) && ((lnMsg[3] & 0x70) == 0x30)) //extended acc command
    {
        var accAddr = lnMsg[5] + ((lnMsg[4] & 0x01) << 7);
        if ((accAddr >= 128 ) && (accAddr < 192 ))  return true;
    }
    if (((filterConfig & opcLocoBit) > 0) && ([0xA0, 0xA1, 0xA2, 0xB6].indexOf(lnMsg[0]) >= 0)) return true;
    if (((filterConfig & opcSlotBit) > 0) && ([0xE7, 0xEF].indexOf(lnMsg[0]) >= 0) && (lnMsg[2] < 0x70)) return true;
    if (((filterConfig & opcSlotBit) > 0) && ([0xB5, 0xB8, 0xB9, 0xBA].indexOf(lnMsg[0]) >= 0) && (lnMsg[1] < 0x70)) return true;
    if (((filterConfig & opcSensorBit) > 0) && ([0xB2].indexOf(lnMsg[0]) >= 0)) return true;
    if (((filterConfig & opcSensorBit) > 0) && (lnMsg[0] == 0xD0) && ((lnMsg[1] & 0x40) === 0)) return true; //multi_sense BDL16
    if (((filterConfig & opcPwrBit) > 0) && (lnMsg[0] == 0xD0) && ((lnMsg[1] & 0x60) == 0x60)) return true; //multi_sense PowerMgr
    if (((filterConfig & opcBtnBit) > 0) && (lnMsg[0] == 0xE5) && (lnMsg[1] == 0x10) && (lnMsg[3] == 0x71) && (lnMsg[11] == 0x01))  return true;
    if (((filterConfig & opcAnalogBit) > 0) && ([0xE5].indexOf(lnMsg[0]) >= 0) && (lnMsg[3] == 0x71) && (lnMsg[11] == 0x00)) return true;
    if ((filterConfig & opcMiscBit) > 0) return true;
    return false;
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
	
//	console.log(jsonData);
	if (flowActive)
	{
        var passOK = false;
        if (jsonData[0] == 0xB4) //LACK
            passOK = passFilter(lastLACKMsg);
        else
            passOK = passFilter(jsonData)

/*
        if (passOK)
        {
        
            var msgStr = "";
            if (dispConfig === 0)
                msgStr = "New Loconet message, no display options set";
            if (dispConfig & dispTimeStmpBit)
                msgStr = "@" + Math.round(jsonData.ReqRecTime/1000).toString() + "&nbsp;";
            if (dispConfig & dispOpCodeBit)
                msgStr += getOpCodeText(jsonData[0]) + "&nbsp;";
            if (dispConfig & (dispTimeStmpBit || dispOpCodeBit))
                msgStr += "<br>";
            if (dispConfig & dispRawDataBit)
                msgStr += getHexString(jsonData) + "<br>";
            if (dispConfig & dispPlainEnglishBit)
                msgStr += getPlainMsgText(jsonData) + "<br>";          
            lnData.unshift(msgStr);
            while (lnData.length > lnDispLen)
                lnData.pop();
            updateDisplay();
        }
*/

		var newLine = document.createElement("li");
		var dispLine = buildDispLine(jsonData);
		if (showPlain && passOK)
			dispLine += (" -- " + getPlainMsgText(jsonData));
		newLine.innerHTML = dispLine;
		listViewer.append(newLine);
		while (listViewer.childNodes.length > numMsgLines)
			listViewer.removeChild(listViewer.childNodes[0]);
		if ((jsonData[0] & 0x08) > 0)
			lastLACKMsg = jsonData; //keep a copy of the message if a reply is expected. This is for decoding LACK messages if meaning depends on data bytes
		else
			lastLACKMsg = []; 
	}
}
