var mainScrollBox;
var listViewerRot;
var listViewerSingle;

var flowActivePublish = true;
var flowActiveSubscribe = true;

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
	mainScrollBox = createEmptyDiv(contentTab, "div", "pagetopicboxscroll-y", "btnconfigdiv");
		createPageTitle(mainScrollBox, "div", "tile-1", "", "h1", "MQTT Traffic Viewer");
		tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
			var leftDiv = createEmptyDiv(tempObj, "div", "tile-1_2", "leftside");
				createPageTitle(leftDiv, "div", "tile-1", "", "h2", "Received on Subscriptions");
				listViewerRot = createListViewer(leftDiv, "viewerbox", "MQTTViewerSubscribe");
				var thisObj = createEmptyDiv(leftDiv, "div", "tile-1", "");
					createButton(thisObj, "tile-1_4", "Stop", "btnStop", "stopRotFlow(this)");
					createButton(thisObj, "tile-1_4", "Clear", "btnClear", "clearRotList(this)");

			var rightDiv = createEmptyDiv(tempObj, "div", "tile-1_2", "rightside");
				createPageTitle(rightDiv, "div", "tile-1", "", "h2", "Published to Broker");
				listViewerSingle = createListViewer(rightDiv, "viewerbox", "MQTTViewerPublish");
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
	flowActiveSubscribe = !flowActiveSubscribe;
	if (flowActiveSubscribe)
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
	flowActivePublish = !flowActivePublish;
	if (flowActivePublish)
		sender.innerHTML  = "Stop"
	else
		sender.innerHTML  = "Resume";
		
}

function clearSingleList(sender)
{
	while (listViewerSingle.childNodes.length > 0)
	  listViewerSingle.removeChild(listViewerSingle.childNodes[0]);
}

function processMQTTInput(jsonData)
{
//	console.log(jsonData);
	var newLine;
	if (jsonData.CmdType == "MQTTOut")
		if (flowActivePublish)
		{
			newLine = document.createElement("li");
			newLine.innerHTML = jsonData.Topic + " " + jsonData.Payload;
			listViewerSingle.append(newLine);
		}
		else
		{}
	else
		if (jsonData.CmdType == "MQTTIn")
			if (flowActiveSubscribe)
			{
				newLine = document.createElement("li");
				newLine.innerHTML = jsonData.Topic + " " + jsonData.Payload;
				listViewerRot.append(newLine);
			}
	while (listViewerSingle.childNodes.length > numMsgLines)
		listViewerSingle.removeChild(listViewerSingle.childNodes[0]);
	while (listViewerRot.childNodes.length > numMsgLines)
		listViewerRot.removeChild(listViewerRot.childNodes[0]);

}
