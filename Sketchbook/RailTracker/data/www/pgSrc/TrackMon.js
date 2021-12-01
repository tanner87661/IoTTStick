var mainScrollBox;
var listViewerRot;
var listViewerSingle;

var flowActiveSingle = true;
var flowActiveRot = true;

var wayPoints = [];
var drawPoints = [];

var trackRecFile = "";
var trackRecMode = false;

var testPoints = [0,0,0, 1,0,0, 0.5,0.5,0, 1,0,0, -0.5,-0.5,0, 1,0,0];
var dispSize = [2,2,2];
var canvasSize = [1024, 680];
var dispBoundaries = [-1000, 1000, -1000, 1000, -1000, 1000]; //minX, maxX, minY, maxY, minZ, maxZ
var dispOrigin = [0,0,0];
var dispMode = false;
//var ScalingFactor = [Math.min(dispSize[0], dispSize[1], dispSize[2])/Math.max(dispBoundaries[1] - dispBoundaries[0], dispBoundaries[3] - dispBoundaries[2], dispBoundaries[5] - dispBoundaries[4]);
var ScalingFactor = [dispSize[0]/(dispBoundaries[1] - dispBoundaries[0]), dispSize[1]/(dispBoundaries[3] - dispBoundaries[2])];

jsonFileVersion = "1.0.0";

function saveConfigFileSettings()
{
	//step 1: save trmon.cfg
	saveJSONConfig(scriptList.Pages[currentPage].ID, scriptList.Pages[currentPage].FileName, configData[workCfg], null);
}

function downloadSettings(sender)
{
	downloadConfig(0x0080); //send just this
}

function loadTableData(thisTable, thisData)
{
}

function setButtonStatus()
{
	setVisibility((!trackRecMode && (trackRecFile != "")), document.getElementById("btnStartTrack"));
	setVisibility(trackRecMode, document.getElementById("btnEndTrack"));
	setVisibility(trackRecFile != "", document.getElementById("btnGetTrack"));
}

function constructPageContent(contentTab)
{
	var tempObj;
	mainScrollBox = createEmptyDiv(contentTab, "div", "pagetopicboxscroll-y", "btnconfigdiv");
		createPageTitle(mainScrollBox, "div", "tile-1", "", "h1", "Track Measuring Car Viewer");
		tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
			createPageTitle(tempObj, "div", "tile-1", "", "h1", "Setup");
			createDropdownselector(tempObj, "tile-1_4", "Scale:", ["1:22.5","1:29","1:87", "1:160"], "selectscale", "setScaleSettings(this)");
			createTextInput(tempObj, "tile-1_4", "Wheel Diameter [mm/]:", "n/a", "wheelsize", "setWheelSize(this)");
			createTextInput(tempObj, "tile-1_4", "Sealevel Pressure [mbar]:", "n/a", "sealevel", "setSeaLevel(this)");
			createTextInput(tempObj, "tile-1_4", "Mag. Threshold:", "n/a", "magincr", "setMagIncr(this)");
//		tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
//			createButton(tempObj, "", "Calibrate", "btnCalib", "calibHall(this)");
		tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
			createButton(tempObj, "", "Save & Restart", "btnSave", "saveSettings(this)");
			createButton(tempObj, "", "Cancel", "btnCancel", "cancelSettings(this)");
		tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
			createButton(tempObj, "", "Save to File", "btnDownload", "downloadSettings(this)");

		tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
			createPageTitle(tempObj, "div", "tile-1", "", "h1", "Measuring Results");
			var dispObj = createEmptyDiv(tempObj, "div", "tile-1", "");
				createPageTitle(dispObj, "div", "tile-1", "", "h2", "Speed and Distance");
				createDispText(dispObj, "", "Speed [mm/s]:", "n/a", "speed");
				createDispText(dispObj, "", "Scale Speed [km/h]:", "n/a", "scalespeed");
				createDispText(dispObj, "", "Abs. Distance [mm]:", "n/a", "absdist");
				createDispText(dispObj, "", "Rel. Distance [mm]:", "n/a", "reldist");

			var dispObj = createEmptyDiv(tempObj, "div", "tile-1", "Orientation");
				createPageTitle(dispObj, "div", "tile-1", "", "h2", "Orientation");
				createDispText(dispObj, "", "Heading [Deg]:", "n/a", "heading");
				createDispText(dispObj, "", "Radius [mm]:", "n/a", "radius");
				createDispText(dispObj, "", "Grade [Deg]:", "n/a", "pitch");
				createDispText(dispObj, "", "Superelevation [Deg]:", "n/a", "roll");

			var dispObj = createEmptyDiv(tempObj, "div", "tile-1", "Layout Position");
				createPageTitle(dispObj, "div", "tile-1", "", "h2", "Layout Position");
				createDispText(dispObj, "", "Pos x [mm]:", "n/a", "locx");
				createDispText(dispObj, "", "Pos y [mm]:", "n/a", "locy");
				createDispText(dispObj, "", "Pos z [mm]:", "n/a", "locz");
				createDispText(dispObj, "", "Altitude [m]:", "n/a", "altitude");

		var thisObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
			createButton(thisObj, "tile-1_4", "Reset Distance", "btnClearDist", "clearDistCtr(this)");
			createButton(thisObj, "tile-1_4", "Reset Position", "btnClearPos", "clearPositions(this)");
			createButton(thisObj, "tile-1_4", "Reset Ofsets", "btnClearOfs", "clearOfsets(this)");

		tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
			createPageTitle(tempObj, "div", "tile-1", "", "h1", "Track Map");
			var canvasElement = document.createElement("canvas");
			tempObj.append(canvasElement);
			canvasElement.setAttribute("id", "glCanvas");
			canvasElement.setAttribute("width", canvasSize[0]);
			canvasElement.setAttribute("height", canvasSize[1]);

		var thisObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
			createButton(thisObj, "tile-1_4", "Start Recording", "btnStartTrack", "startRecording(this)");
			createTextInput(tempObj, "tile-1_4", "File:", "", "txtFileName", "setFileName(this)");
			createButton(thisObj, "tile-1_4", "End Recording", "btnEndTrack", "endRecording(this)");
			createButton(thisObj, "tile-1_4", "Download Recording", "btnGetTrack", "downloadTrack(this)");
		var thisObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
			createFileDlg(thisObj, "", "Load from File", "btnLoad", ".json", "loadSettings(this)");

		graphicMain();

//		tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
}

function loadSettings(sender)
{
	var fileIDName = "";
	function findIDName(pageEntry)
	{
		return pageEntry.ID == fileIDName;
	}
	
	var fileName = document.getElementById("btnLoad").files[0];
	console.log("Load file ", fileName);
	var reader = new FileReader();
    reader.onload = function()
    {
        try 
        {
			var configArray = JSON.parse(reader.result);
			console.log(configArray.TrackPoints.length);
		}
		catch(err) 
		{
			alert("Configuration data not valid");
			return;
		}
		resetDisplay();
		dispMode = true;
		for (var i = 0; i < configArray.TrackPoints.length; i++)
			addWayPoint(configArray.TrackPoints[i].Coord);
		graphicMain();
    };
    reader.readAsText(fileName);	
}

function loadNodeDataFields(jsonData)
{
	setButtonStatus();
}

function loadDataFields(jsonData)
{
	console.log(jsonData);
	writeInputField("wheelsize", jsonData.WheelSize);
	writeInputField("sealevel", jsonData.SeaLevel);
	writeInputField("magincr", jsonData.MagThreshold);
	if (jsonData.DispScale != undefined)
		setDropdownValueByText("selectscale", jsonData.DispScale.toString());
	else
		setDropdownValue("selectscale", 0);
	setButtonStatus();
}

function setWheelSize(sender)
{
	configData[workCfg].WheelSize = verifyNumber(sender.value, configData[workCfg].WheelSize); 
}

function setScaleSettings(sender)
{
	configData[workCfg].DispScale = sender.options[sender.selectedIndex].text;
}

function setFileName(sender)
{
	trackRecFile = sender.value;
	setButtonStatus();
}

function setSeaLevel(sender)
{
	configData[workCfg].SeaLevel = verifyNumber(sender.value, configData[workCfg].SeaLevel); 
}

function setMagIncr(sender)
{
	configData[workCfg].MagThreshold = verifyNumber(sender.value, configData[workCfg].MagThreshold); 
}

function calibHall(sender)
{
}

function downloadTrack(sender)
{
	window.open("trackdata/" + trackRecFile, '_blank').focus();
}

function clearDistCtr(sender)
{
	resetDistCounter();
}

function clearOfsets(sender)
{
	resetIMUOfset();
}

function clearPositions(sender)
{
	resetPosOfset();
	resetDisplay();
	graphicMain();
}

function startRecording(sender)
{
//	var TrackFileName = document.getElementById("txtFileName").text;
//	console.log(trackRecFile);
	if (trackRecFile)
	{
		startTracking(trackRecFile);
		trackRecMode = true;
	}
	else
		alert("Please enter a FileName first!");
	setButtonStatus();
}

function endRecording(sender)
{
	stopTracking();
	trackRecMode = false;
	setButtonStatus();
}

function processSensorInput(jsonData)
{
	console.log(jsonData);
	if ((Math.abs(jsonData.Radius) > 0) && (Math.abs(jsonData.Radius) < 5000))
		writeTextField("radius", jsonData.Radius.toFixed(2));
	else
		writeTextField("radius", "straight");
	writeTextField("speed", jsonData.Speed.toFixed(2));
	switch (parseInt(getDropdownValue("selectscale")))
	{
		case 0: writeTextField("scalespeed", (jsonData.Speed * 22.5 * 0.0036).toFixed(2));
			break;
		case 1: writeTextField("scalespeed", (jsonData.Speed * 29 * 0.0036).toFixed(2));
			break;
		case 2: writeTextField("scalespeed", (jsonData.Speed * 87 * 0.0036).toFixed(2));
			break;
		case 3: writeTextField("scalespeed", (jsonData.Speed * 160 * 0.0036).toFixed(2));
			break;
		default: writeTextField("scalespeed", "n/a");
			break;
	}
	writeTextField("absdist", jsonData.AbsDist.toFixed(2));
	writeTextField("reldist", jsonData.RelDist.toFixed(2));
	writeTextField("heading", jsonData.Euler[0].toFixed(2));
	writeTextField("roll", jsonData.Euler[1].toFixed(2));
	writeTextField("pitch", jsonData.Euler[2].toFixed(2));
	writeTextField("locx", jsonData.Pos[0].toFixed(2));
	writeTextField("locy", jsonData.Pos[1].toFixed(2));
	writeTextField("locz", jsonData.Pos[2].toFixed(2));
	writeTextField("altitude", jsonData.Alti.toFixed(2));
	if (!dispMode)
	{
		addWayPoint(jsonData.Pos);
		graphicMain();
	}
	setButtonStatus();
//	console.log(ScalingFactor, jsonData.Pos[0], ScalingFactor * (jsonData.Pos[0] - dispOrigin[0]), dispOrigin[0], dispBoundaries[0], dispBoundaries[1]);
	
}

