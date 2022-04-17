var mainScrollBox;
var listViewerRot;
var listViewerSingle;

var flowActiveSingle = true;
var flowActiveRot = true;

var wayPoints = [];
var drawPoints = [];

var trackerDisp = 0x7F;

var tabSetup;
var tabMeasurement;
var tabSpeedMatch;
var tabGPS;
var tabProgrammer;
var cvTableNative;
var cvTableJMRI;
var techSpeedDiv;
var speedTableDiv;


var fwTrim = 0;
var bwTrim = 0;

var trackRecFile = "";
var trackRecMode = false;

var dimOptions = ["Metric","Imperial"];

var testPoints = [0,0,0, 1,0,0, 0.5,0.5,0, 1,0,0, -0.5,-0.5,0, 1,0,0];
var dispSize = [2,2,2];
//var canvasSize = [screen.width, 480];

var canvasElementSpeed;
var canvasElementGrade;
var canvasElementRadius;
var canvasElementGPS;

var dispBoundaries = [-1000, 1000, -1000, 1000, -1000, 1000]; //minX, maxX, minY, maxY, minZ, maxZ
var dispOrigin = [0,0,0];
var dispMode = false;
//var ScalingFactor = [Math.min(dispSize[0], dispSize[1], dispSize[2])/Math.max(dispBoundaries[1] - dispBoundaries[0], dispBoundaries[3] - dispBoundaries[2], dispBoundaries[5] - dispBoundaries[4]);
var ScalingFactor = [dispSize[0]/(dispBoundaries[1] - dispBoundaries[0]), dispSize[1]/(dispBoundaries[3] - dispBoundaries[2])];

var	locoAddr = -1;
var locoAddrValid = false;

jsonFileVersion = "1.0.0";

function saveConfigFileSettings()
{
	//step 1: save phcfg.cfg
	saveJSONConfig(scriptList.Pages[currentPage].ID, scriptList.Pages[currentPage].FileName, configData[workCfg], null);
}

function downloadSettings(sender)
{
	downloadConfig(0x2000); //send just this
}

function loadTableData(thisTable, thisData)
{
}

function setButtonStatus()
{
//	console.log(configData[nodeCfg].InterfaceIndex, configData[work].InterfaceIndex);
	setVisibility([6,12].indexOf(configData[nodeCfg].InterfaceIndex) >= 0, document.getElementById("btnAssign"));
	setVisibility([6,12].indexOf(configData[nodeCfg].InterfaceIndex) >= 0, document.getElementById("dccaddr").parentElement);
	setVisibility([6,12].indexOf(configData[nodeCfg].InterfaceIndex) >= 0, document.getElementById("dccstep").parentElement);
	setVisibility([6,12].indexOf(configData[nodeCfg].InterfaceIndex) >= 0, document.getElementById("btnAssignsp"));
	setVisibility([6,12].indexOf(configData[nodeCfg].InterfaceIndex) >= 0, document.getElementById("dccaddrsp").parentElement);
	setVisibility([6,12].indexOf(configData[nodeCfg].InterfaceIndex) >= 0, document.getElementById("dccaddrtbl").parentElement);

//	setVisibility([6,12].indexOf(configData[nodeCfg].InterfaceIndex) >= 0, document.getElementById("dccstepsp"));//.parentElement);
	setVisibility([6,12,13].indexOf(configData[nodeCfg].InterfaceIndex) >= 0, document.getElementById("cbsetup_2"));
//	setVisibility([6,12].indexOf(configData[nodeCfg].InterfaceIndex) >= 0, document.getElementById("cbsetup_tx_2"));
	
	setVisibility(validLocoDef, document.getElementById("btnSaveDecoder"));
	setVisibility(validThrottleDef, document.getElementById("btnSaveThrottle"));
	setVisibility(validTableDef, document.getElementById("btnProg"));

	setVisibility(!validLocoDef && [6,12].indexOf(configData[nodeCfg].InterfaceIndex) >= 0, cvTableNative);
	setVisibility(validLocoDef, cvTableJMRI);
	setVisibility([6,12].indexOf(configData[nodeCfg].InterfaceIndex) >= 0, tabProgrammer);

	setVisibility(validLocoDef || [6,12].indexOf(configData[nodeCfg].InterfaceIndex) >= 0, techSpeedDiv);
	setVisibility(validTechSpeedDef, speedTableDiv);
	

//	setVisibility((!trackRecMode && (trackRecFile != "")), document.getElementById("btnStartTrack"));
//	setVisibility(trackRecMode, document.getElementById("btnEndTrack"));
//	setVisibility(trackRecFile != "", document.getElementById("btnGetTrack"));
}

function constructPageContent(contentTab)
{
	var menueStr = ["Setup", "Track Data", "Speed Magic", "Layout TPS"];
	var tempObj;
	mainScrollBox = createEmptyDiv(contentTab, "div", "pagetopicboxscroll-y", "btnconfigdiv");
//		createPageTitle(mainScrollBox, "div", "tile-1", "", "h1", "Track Measuring Car Viewer");
		var menueTab = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
		for (var i = 0; i < menueStr.length; i++)
			createMenueTabElement(menueTab, "button", "tablink", "sensorSub", "cbsetup_" + i.toString(), menueStr[i], true, "setPageMode(this)");
		updateMenueTabs("sensorSub", "cbsetup_0", "grey");
		tabSetup = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
		tempObj = createEmptyDiv(tabSetup, "div", "tile-1", "");
			createPageTitle(tempObj, "div", "tile-1", "", "h1", "Setup");
			createTextInput(tempObj, "tile-1_4", "Wheel Diameter [mm]:", "n/a", "wheelsize", "setWheelSize(this)");
			createTextInput(tempObj, "tile-1_4", "Mag. Threshold:", "n/a", "magincr", "setMagIncr(this)");
			createCheckbox(tempObj, "tile-1_4", "Reverse Dir", "cbRevDir", "setDir(this)");
		tempObj = createEmptyDiv(tabSetup, "div", "tile-1", "");
			createDropdownselector(tempObj, "tile-1_4", "Scale:", ["1:22.5","1:29","1:87", "1:160"], "selectscale", "setScaleSettings(this)");
			createDropdownselector(tempObj, "tile-1_4", "Display:", dimOptions, "selectdim", "setDimSettings(this)");
		tempObj = createEmptyDiv(tabSetup, "div", "tile-1", "");
			createDropdownselector(tempObj, "tile-1_4", "Orientation:", ["Flat","Upright"], "mountstyle", "setMountingStyle(this)");

		tempObj = createEmptyDiv(tabSetup, "div", "tile-1", "");
			createButton(tempObj, "", "Save & Restart", "btnSave", "saveSettings(this)");
			createButton(tempObj, "", "Cancel", "btnCancel", "cancelSettings(this)");
		tempObj = createEmptyDiv(tabSetup, "div", "tile-1", "");
			createButton(tempObj, "", "Save to File", "btnDownload", "downloadSettings(this)");

		tabMeasurement = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
			setVisibility(false, tabMeasurement);

			createPageTitle(tabMeasurement, "div", "tile-1", "", "h1", "Track Measuring Display");
			createButton(tabMeasurement, "", "Start", "btnStart", "startMeasuring(this)");
			createButton(tabMeasurement, "", "Reset Distance", "btnResetDist", "resetDistance(this)");
			createButton(tabMeasurement, "", "Reset Orientation", "btnResetHeading", "resetHeading(this)");
			createButton(tabMeasurement, "", "Assign DCC Address", "btnAssign", "assignDCC(this)");
			createDispText(tabMeasurement, "tile-1_4", "DCC Address:","n/a","dccaddr");
			createDispText(tabMeasurement, "tile-1_4", "Speed Step:","n/a","dccstep");
			var dispObj = createEmptyDiv(tabMeasurement, "div", "tile-1", "");
				createPageTitle(dispObj, "div", "tile-1", "", "h2", "Speed and Distance");
				createDispText(dispObj, "", "Phys. Speed [mm/s]:", "n/a", "speed");
				createDispText(dispObj, "", "Scale Speed:", "n/a", "scalespeed");
				createDispText(dispObj, "", "Abs. Distance:", "n/a", "absdist");
				createDispText(dispObj, "", "Rel. Distance:", "n/a", "reldist");

			var dispObj = createEmptyDiv(tabMeasurement, "div", "tile-1", "Orientation");
				createPageTitle(dispObj, "div", "tile-1", "", "h2", "Orientation");
				createDispText(dispObj, "", "Radius [mm]:", "n/a", "radius");
				createDispText(dispObj, "", "Heading [Deg]:", "n/a", "heading");
				createDispText(dispObj, "", "Grade [%]:", "n/a", "pitch");
				createDispText(dispObj, "", "Superelevation [%]:", "n/a", "roll");

			var dispObj = createEmptyDiv(tabMeasurement, "div", "tile-1", "SelectDisplay");
				createPageTitle(dispObj, "div", "tile-1", "", "h2", "Select Data Trackers");
				tempObj = createEmptyDiv(dispObj, "div", "tile-1", "");
				createCheckbox(tempObj, "tile-1_4", "Technical Speed", "cbTechSpeed", "setTracker(this)");
				createCheckbox(tempObj, "tile-1_4", "Scale Speed", "cbScaleSpeed", "setTracker(this)");
				createCheckbox(tempObj, "tile-1_4", "Speed Step", "cbSpeedStep", "setTracker(this)");

				tempObj = createEmptyDiv(dispObj, "div", "tile-1", "");
				createCheckbox(tempObj, "tile-1_4", "Track Radius", "cbRadius", "setTracker(this)");
				createCheckbox(tempObj, "tile-1_4", "Heading", "cbHeading", "setTracker(this)");

				tempObj = createEmptyDiv(dispObj, "div", "tile-1", "");
				createCheckbox(tempObj, "tile-1_4", "Grade", "cbGrade", "setTracker(this)");
				createCheckbox(tempObj, "tile-1_4", "Superelevation", "cbSElevation", "setTracker(this)");

			tempObj = createEmptyDiv(tabMeasurement, "div", "tile-1", "");
				createPageTitle(tempObj, "div", "tile-1", "", "h2", "Measurement Data");
				var dispObj = createEmptyDiv(tempObj, "div", "tile-1", "");
				createRadiobox(dispObj, "tile-1_2", "Use X Axis for", ["Time","Distance"], "rbxbase", "setBaseLine(this)");
				writeRBInputField("rbxbase", 0);
				var dispObj = createEmptyDiv(tempObj, "div", "tile-1", "");
				createButton(dispObj, "", "Start", "btnStartLow", "startMeasuring(this)");
				canvasElementSpeed = document.createElement("canvas");
				tempObj.append(canvasElementSpeed);
				canvasElementSpeed.setAttribute("id", "glCanvasSpeed");
//				canvasElementSpeed.setAttribute("width", canvasSizeSpeed[0]);
//				canvasElementSpeed.setAttribute("height", canvasSizeSpeed[1]);

				createPageTitle(tempObj, "div", "tile-1", "", "h2", " ");
				canvasElementGrade = document.createElement("canvas");
				tempObj.append(canvasElementGrade);
				canvasElementGrade.setAttribute("id", "glCanvasGrade");
//				canvasElementGrade.setAttribute("width", canvasSizeSpeed[0]);
//				canvasElementGrade.setAttribute("height", canvasSizeSpeed[1]);

				createPageTitle(tempObj, "div", "tile-1", "", "h2", " ");
				canvasElementRadius= document.createElement("canvas");
				tempObj.append(canvasElementRadius);
				canvasElementRadius.setAttribute("id", "glCanvasRadius");
//				canvasElementRadius.setAttribute("width", canvasSizeSpeed[0]);
//				canvasElementRadius.setAttribute("height", canvasSizeSpeed[1]);

		tabSpeedMatch  = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
			setVisibility(false, tabSpeedMatch);
			createPageTitle(tabSpeedMatch, "div", "tile-1", "", "h1", "Speed Magic");
			var dispObj = createEmptyDiv(tabSpeedMatch, "div", "tile-1", "Decoder Profile");
				createPageTitle(dispObj, "div", "tile-1", "", "h2", "Locomotive Information");
				createFileDlg(dispObj, "", "Load JMRI File", "btnLoadDecoder", "application/xml", "loadJMRIDecoder(this)");
				createButton(dispObj, "", "Save JMRI File", "btnSaveDecoder", "saveJMRIDecoder(this)");
				createDispText(dispObj, "tile-1_4", "File Name:","n/a","jmrifilename");

				cvTableJMRI = createEmptyDiv(dispObj, "div", "tile-1", "");

					tempObj = createEmptyDiv(cvTableJMRI, "div", "tile-1", "");
					createDispText(tempObj, "tile-1_4", "Locomotive:","n/a","dccloco");
					createDispText(tempObj, "tile-1_4", "DCC Address:","n/a","jmriaddr");
					createDispText(tempObj, "tile-1_4", "Vmax []:","n/a","jmrivmax");

					tempObj = createEmptyDiv(cvTableJMRI, "div", "tile-1", "");
					createDispText(tempObj, "tile-1_4", "Decoder Brand:","n/a","decbrand");
					createDispText(tempObj, "tile-1_4", "Decoder Family:","n/a","decfamily");
					createDispText(tempObj, "tile-1_4", "Model:","n/a","decmodel");

					tempObj = createEmptyDiv(cvTableJMRI, "div", "tile-1", "");
					createDispText(tempObj, "tile-1_4", "CV 2:","n/a","cv02");
					createDispText(tempObj, "tile-1_4", "CV 5:","n/a","cv05");
					createDispText(tempObj, "tile-1_4", "CV 6:","n/a","cv06");
					tempObj = createEmptyDiv(cvTableJMRI, "div", "tile-1", "");
					createDispText(tempObj, "tile-1_4", "CV 29:","n/a","cv29");


				cvTableNative = createEmptyDiv(dispObj, "div", "tile-1", "");
					tempObj = createEmptyDiv(cvTableNative, "div", "tile-1", "");
					createTextInput(tempObj, "tile-1_4", "CV 2:", "0", "cvn02", "setCVVal(this)");
					createTextInput(tempObj, "tile-1_4", "CV 5:", "0", "cvn05", "setCVVal(this)");
					createTextInput(tempObj, "tile-1_4", "CV 6:", "0", "cvn06", "setCVVal(this)");
					tempObj = createEmptyDiv(cvTableNative, "div", "tile-1", "");
					createTextInput(tempObj, "tile-1_4", "CV 29:", "0", "cvn29", "setCVVal(this)");

				tabProgrammer = createEmptyDiv(dispObj, "div", "tile-1", "");
				tempObj = createEmptyDiv(tabProgrammer, "div", "tile-1", "");
					createRadiobox(tempObj, "tile-1_2", "Programming Method", ["Prog Track","Main Line"], "rbprogmethod", "setProgMode(this)");
					createDispText(tempObj, "tile-1_4", "Status:","","progstat");
				tempObj = createEmptyDiv(tabProgrammer, "div", "tile-1", "");
					createTextInput(tempObj, "tile-1_4", "Program CV:", "", "cvid", "setCV(this)");
					createTextInput(tempObj, "tile-1_4", "CV Value:", "", "cvval", "setCV(this)");
					createButton(tempObj, "", "Read CV", "btnReadCV", "readCV(this)");
					createButton(tempObj, "", "Write CV", "btnWriteCV", "writeCV(this)");
				setVisibility(false, tabProgrammer);
				setVisibility(true, cvTableNative);
				setVisibility(false, cvTableJMRI);

			var dispObj = createEmptyDiv(tabSpeedMatch, "div", "tile-1", "Throttle Profile");
				createPageTitle(dispObj, "div", "tile-1", "", "h2", "Throttle Profile");
				createFileDlg(dispObj, "", "Load Profile", "btnLoadThrottle", "application/json", "loadThrottle(this)");
				createButton(dispObj, "", "Save Profile", "btnSaveThrottle", "saveThrottle(this)");
				createDispText(dispObj, "tile-1_4", "File Name:","n/a","throttlefilename");

				tempObj = createEmptyDiv(dispObj, "div", "tile-1", "");
				createTextInput(tempObj, "tile-1_4", "Profile Name:", "n/a", "thprofilename", "setThrottleProfile(this)");
				tempObj = createEmptyDiv(dispObj, "div", "tile-1", "");
				createTextInput(tempObj, "tile-1_4", "Max. Scale:", "n/a", "thscalespeed", "setThrottleProfile(this)");
				createTextInput(tempObj, "tile-1_4", "@ Throttle Step:", "n/a", "thvmax", "setThrottleProfile(this)");
				createTextInput(tempObj, "tile-1_4", "of # Steps:", "n/a", "thnumsteps", "setThrottleProfile(this)");

				tempObj = createEmptyDiv(dispObj, "div", "tile-1", "");
				canvasElementThrottle = document.createElement("canvas");
				tempObj.append(canvasElementThrottle);
				canvasElementThrottle.setAttribute("id", "glCanvasThrottle");
				setCanvasMouseEvents(canvasElementThrottle);

			var dispObj = createEmptyDiv(tabSpeedMatch, "div", "tile-1", "Technical Speed Profile");
				techSpeedDiv = dispObj;
				createPageTitle(dispObj, "div", "tile-1", "", "h2", "Technical Speed Profile");
				tempObj = createEmptyDiv(dispObj, "div", "tile-1", "");
				createButton(tempObj, "", "Assign DCC Address", "btnAssignsp", "assignDCC(this)");
				createDispText(tempObj, "tile-1_4", "DCC Address:","n/a","dccaddrsp");
//				createDispText(tempObj, "tile-1_4", "Speed Step:","n/a","dccstepsp");
				createRadiobox(tempObj, "tile-1_2", "Speed Steps:", ["28","128"], "dccstepsp", "setTestMode(this)");
				tempObj = createEmptyDiv(dispObj, "div", "tile-1", "");
				createButton(tempObj, "", "Start Test", "btnTrackLen", "startSpeedTest(this)");
				createButton(tempObj, "", "Abort Test", "btnTrackLen", "abortSpeedTest(this)");
				createTextInput(tempObj, "tile-1_4", "Track length [cm]:", "n/a", "testtracklen", "setTrackLength(this)");
				createTextInput(tempObj, "tile-1_4", "max. Speed [km/h]:", "n/a", "testmaxspeed", "setTestSpeed(this)");

				tempObj = createEmptyDiv(dispObj, "div", "tile-1", "");
				canvasElementTechSpeed = document.createElement("canvas");
				tempObj.append(canvasElementTechSpeed);
				canvasElementTechSpeed.setAttribute("id", "glCanvasTechSpeed");
				setCanvasMouseEvents(canvasElementTechSpeed);

			var dispObj = createEmptyDiv(tabSpeedMatch, "div", "tile-1", "Speed Table Settings");
				speedTableDiv = dispObj;
				createPageTitle(dispObj, "div", "tile-1", "", "h2", "Speed Table Settings");
				tempObj = createEmptyDiv(dispObj, "div", "tile-1", "");
				createButton(tempObj, "", "Recalculate", "btnRecalc", "calcTable(this)");
				createButton(tempObj, "", "Write CV's", "btnProg", "progTable(this)");
				createDispText(tempObj, "tile-1_4", "DCC Address:","n/a","dccaddrtbl");
				tempObj = createEmptyDiv(dispObj, "div", "tile-1", "");
				createRadiobox(tempObj, "tile-1_2", "Trim Range", ["0 - 200%", "0 - 100%"], "trimmode", "setTrimMode(this)");
//				createRadiobox(tempObj, "tile-1_2", "Use", ["Speed Table","Min/Mid/Max only"], "rbtablemode", "setTableMode(this)");
				tempObj = createEmptyDiv(dispObj, "div", "tile-1", "");
				createTextInput(tempObj, "tile-1_4", "Forward Trim:", "n/a", "fwtrim", "setTrim(this)");
				createTextInput(tempObj, "tile-1_4", "Backward Trim:", "n/a", "bwtrim", "setTrim(this)");
		
				tempObj = createEmptyDiv(dispObj, "div", "tile-1", "");
				canvasElementSpeedTable = document.createElement("canvas");
				tempObj.append(canvasElementSpeedTable);
				canvasElementSpeedTable.setAttribute("id", "glCanvasSpeedTable");
				setCanvasMouseEvents(canvasElementSpeedTable);

		tabGPS = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
			setVisibility(false, tabGPS);
			createPageTitle(tabGPS, "div", "tile-1", "", "h1", "Layout TPS");
/*
			var dispObj = createEmptyDiv(tempObj, "div", "tile-1", "Layout Position");
				createPageTitle(dispObj, "div", "tile-1", "", "h2", "Layout Position");
				createDispText(dispObj, "", "Pos x [mm]:", "n/a", "locx");
				createDispText(dispObj, "", "Pos y [mm]:", "n/a", "locy");
				createDispText(dispObj, "", "Pos z [mm]:", "n/a", "locz");
				createDispText(dispObj, "", "Altitude [m]:", "n/a", "altitude");
*/
//		var thisObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
//			createButton(thisObj, "tile-1_4", "Reset Distance", "btnClearDist", "clearDistCtr(this)");
//			createButton(thisObj, "tile-1_4", "Reset Position", "btnClearPos", "clearPositions(this)");
//			createButton(thisObj, "tile-1_4", "Reset Ofsets", "btnClearOfs", "clearOfsets(this)");
/*
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

*/
//		tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");

		setTrackerDisplay();
//		drawLineGraphs();
//		graphicMain();
}

function drawLineGraphs()
{
	drawLineGraph(canvasElementSpeed, speedGraph);
	drawLineGraph(canvasElementGrade, radiusGraph);
	drawLineGraph(canvasElementRadius, angleGraph);
}

function drawSpeedMagicGraphs()
{
	drawProfileBox(canvasElementThrottle, throttleProfileGraph);
	drawProfileBox(canvasElementTechSpeed, techSpeedProfileGraph);
	drawProfileBox(canvasElementSpeedTable, speedTableProfileGraph);
}

function setBaseLine(sender)
{
//	console.log(sender);
	if (sender.id == "rbxbase_0")
		setGraphBaseLine(0);
	else
		setGraphBaseLine(1);
	drawLineGraphs();
}

function setPageMode(sender)
{
	setVisibility(false, tabSetup);
	setVisibility(false, tabMeasurement);
	setVisibility(false, tabSpeedMatch);
	setVisibility(false, tabGPS);

	switch (sender.id)
	{
		case "cbsetup_0":
//			writeRBInputField("cbsetup", 0);
			setVisibility(true, tabSetup);
			break;
		case "cbsetup_1":
//			writeRBInputField("cbsetup", 1);
			setVisibility(true, tabMeasurement);
			break;
		case "cbsetup_2":
//			writeRBInputField("cbsetup", 2);
			setVisibility(true, tabSpeedMatch);
			break;
		case "cbsetup_3":
//			writeRBInputField("cbsetup", 3);
			setVisibility(true, tabGPS);
			break;
	}
	updateMenueTabs("sensorSub", sender.id, "grey");
}

function setTrackerDisplay()
{
	document.getElementById("cbSpeedStep").checked = (trackerDisp & 0x01) > 0;
	document.getElementById("cbTechSpeed").checked = (trackerDisp & 0x02) > 0;
	document.getElementById("cbScaleSpeed").checked = (trackerDisp & 0x04) > 0;
	document.getElementById("cbRadius").checked = (trackerDisp & 0x08) > 0;
	document.getElementById("cbHeading").checked = (trackerDisp & 0x10) > 0;
	document.getElementById("cbGrade").checked = (trackerDisp & 0x20) > 0;
	document.getElementById("cbSElevation").checked = (trackerDisp & 0x40) > 0;
	
}

function setTracker(sender)
{
	switch (sender.id)
	{
		case "cbSpeedStep":
			trackerDisp = sender.checked ? trackerDisp | 0x01 : trackerDisp & ~0x01;
			speedGraph.LineGraphs[2].Show = sender.checked;
			
			break;
		case "cbTechSpeed":
			trackerDisp = sender.checked ? trackerDisp | 0x02 : trackerDisp & ~0x02;
			speedGraph.LineGraphs[0].Show = sender.checked;
			break;
		case "cbScaleSpeed":
			trackerDisp = sender.checked ? trackerDisp | 0x04 : trackerDisp & ~0x04;
			speedGraph.LineGraphs[1].Show = sender.checked;
			break;
		case "cbRadius":
			trackerDisp = sender.checked ? trackerDisp | 0x08 : trackerDisp & ~0x08;
			radiusGraph.LineGraphs[0].Show = sender.checked;
			break;
		case "cbHeading":
			trackerDisp = sender.checked ? trackerDisp | 0x10 : trackerDisp & ~0x10;
			radiusGraph.LineGraphs[1].Show = sender.checked;
			break;
		case "cbGrade":
			trackerDisp = sender.checked ? trackerDisp | 0x20 : trackerDisp & ~0x20;
			angleGraph.LineGraphs[0].Show = sender.checked;
			break;
		case "cbSElevation":
			trackerDisp = sender.checked ? trackerDisp | 0x40 : trackerDisp & ~0x40;
			angleGraph.LineGraphs[1].Show = sender.checked;
			break;
	}
	drawLineGraphs();
}

function loadSettings(sender)
{
	var fileIDName = "";
	function findIDName(pageEntry)
	{
		return pageEntry.ID == fileIDName;
	}
	
	var fileName = document.getElementById("btnLoad").files[0];
//	console.log("Load file ", fileName);
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
	document.getElementById("btnProg").innerHTML = configData[nodeCfg].InterfaceIndex == 13 ? "Save JMRI File" : "Write CV's";
}

function loadDataFields(jsonData)
{
//	console.log(jsonData);
	writeInputField("wheelsize", jsonData.WheelDia);
	writeInputField("magincr", jsonData.MagThreshold);
	writeInputField("testtracklen", jsonData.TrackLen);
	if (jsonData.ScaleList != undefined)
	{
		//load scale list
		var scaleOptions = [];
		for (var i = 0; i < jsonData.ScaleList.length; i++)
		{
			var optStr = jsonData.ScaleList[i].Name + "  1:" + jsonData.ScaleList[i].Scale.toString();
			scaleOptions.push(optStr);
		}
		createOptions(document.getElementById("selectscale"), scaleOptions); 
		
		if (jsonData.ScaleIndex != undefined)
			setDropdownValue("selectscale", jsonData.ScaleIndex);
		else
			setDropdownValue("selectscale", 3); //most likely HO
	}
	else
		setDropdownValue("selectscale", 0);
	setDropdownValue("selectdim", jsonData.Units); //most likely HO
	setDropdownValue("mountstyle", jsonData.MountStyle); //flat or upright
	var scaleStr = jsonData.ScaleList[jsonData.ScaleIndex].Name;
	switch (jsonData.Units)
	{
		case 0: 
			document.getElementById("scalespeed_txt").innerHTML = scaleStr + " Scale Speed [km/h]";
			document.getElementById("absdist_txt").innerHTML = "Abs. Distance [cm]";
			document.getElementById("reldist_txt").innerHTML = "Rel. Distance [cm]";
			document.getElementById("radius_txt").innerHTML = "Radius [mm]";
			document.getElementById("thscalespeed_txt").innerHTML = "Max. Scale [km/h]";
			document.getElementById("testtracklen_txt").innerHTML = "Track Length [cm]"; 
			document.getElementById("testmaxspeed_txt").innerHTML = "Max Speed [km/h]"; 
			document.getElementById("jmrivmax_txt").innerHTML = "Max Speed [km/h]:"; 
			
			break;
		case 1: 
			document.getElementById("scalespeed_txt").innerHTML = scaleStr + " Scale Speed [mph]";
			document.getElementById("absdist_txt").innerHTML = "Abs. Distance [in]";
			document.getElementById("reldist_txt").innerHTML = "Rel. Distance [in]";
			document.getElementById("radius_txt").innerHTML = "Radius [in]";
			document.getElementById("thscalespeed_txt").innerHTML = "Max. Scale [mph]";
			document.getElementById("testtracklen_txt").innerHTML = "Track Length [in]";
			document.getElementById("testmaxspeed_txt").innerHTML = "Max Speed [mph]"; 
			document.getElementById("jmrivmax_txt").innerHTML = "Max Speed [mph]:"; 
			break;
	}
	writeRBInputField("rbprogmethod", progMode);
//	writeRBInputField("rbtablemode", tableMode);
	writeRBInputField("trimmode", trimMode);
	writeRBInputField("dccstepsp", stepMode);

	

	loadSpeedGraphData();
	drawLineGraphs();
	

	setButtonStatus();
	
	loadSpeedMagicGraphData();
	displayThrottleDef(throttleDef);
	drawSpeedMagicGraphs();
}

function setWheelSize(sender)
{
	configData[workCfg].WheelDia = verifyNumber(sender.value, configData[workCfg].WheelDia); 
}

function setScaleSettings(sender)
{
	configData[workCfg].ScaleIndex = sender.selectedIndex;
}

function setDimSettings(sender)
{
	configData[workCfg].Units = sender.selectedIndex;
}

function setMountingStyle(sender)
{
	configData[workCfg].MountStyle = sender.selectedIndex;
}

function setDir(sender)
{
	configData[workCfg].ReverseDir = sender.checked ? true : false;
}

function setFileName(sender)
{
	trackRecFile = sender.value;
	setButtonStatus();
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

function readCV(sender)
{
	if ((progMode > 0) && (locoAddr <= 0))
	{
		alert("Invalid Loco Address. Please assign a DCC Address.");
		return;
	}
	if (progMode == 0)
		if (confirm("Place locomotive on programming track and click OK") == false)
			return;
	if ((cvId >= 0) && (cvId <= 255))
	{
		ws.send("{\"Cmd\":\"SetSensor\", \"SubCmd\":\"ReadCV\", \"Addr\":" + locoAddr.toString() + ",\"ProgMode\":" + progMode.toString() + ",\"CV\":" + cvId.toString() + "}");
		writeTextField("progstat", "Read CV in progress");
	}
	else
		alert("Invalid CV");
}

function writeCV(sender)
{
	if ((progMode > 0) && (locoAddr <= 0))
	{
		alert("Invalid Loco Address. Please assign a DCC Address.");
		return;
	}
	if (progMode == 0)
		if (confirm("Place locomotive on programming track and click OK") == false)
			return;
	if ((cvId >= 0) && (cvVal >= 0) && (cvId <= 255) && (cvVal <= 255))
	{
		ws.send("{\"Cmd\":\"SetSensor\", \"SubCmd\":\"WriteCV\",\"Addr\":" + locoAddr.toString() + ", \"ProgMode\":" + progMode.toString() + ",\"CV\":" + cvId.toString() + ",\"CVVal\":" + cvVal.toString() + "}");
		writeTextField("progstat", "Write CV in progress");
	}
	else
		alert("Invalid CV or Value");
}

function startMeasuring(sender)
{
	if (sender.innerHTML == "Start")
	{
		clearAllGraphData();
		ws.send("{\"Cmd\":\"SetSensor\", \"SubCmd\":\"RepRate\",\"Val\":500}");
		document.getElementById("btnStart").innerHTML = "Stop";
		document.getElementById("btnStartLow").innerHTML = "Stop";
	}
	else
	{
		ws.send("{\"Cmd\":\"SetSensor\", \"SubCmd\":\"RepRate\",\"Val\":0}");
		document.getElementById("btnStart").innerHTML = "Start";
		document.getElementById("btnStartLow").innerHTML = "Start";
	}
}

function assignDCC(sender)
{
	locoAddr = -1;
	writeTextField("dccaddr", "listening...");
	writeTextField("dccaddrsp", "listening...");
	reqDCCAssign(locoAddr);
}

function reqDCCAssign(forAddress)
{
	if (forAddress >= 0)
		ws.send("{\"Cmd\":\"SetSensor\", \"SubCmd\":\"SetDCC\"," + " \"Addr\":" + forAddress.toString()+ " }");
	else
		ws.send("{\"Cmd\":\"SetSensor\", \"SubCmd\":\"SetDCC\"}");
}

function startSpeedTest(sender)
{
	if (locoAddr < 0)
		if (configData[nodeCfg].InterfaceIndex == 13) //WiThrottle
		{
			alert("No DCC Address assigned! Verify connection to WiThrottle Server and reload JMRI file");
			document.getElementById("btnLoadDecoder").files = [];
		}
		else
			alert("No DCC Address assigned! Verify LocoNet communication.");
	else
	{

		if ((cvVal_2 != 0) || (cvVal_5 != 0) || (cvVal_6 != 0) || ((cvVal_29 & 0x10) != 0))
		{
			var dispText = "Verify and confirm CV settings before running the speed test.\nCV 2 expected 0 is " + cvVal_2 + "\nCV 5 expected 0 is " + cvVal_5 + "\nCV 6 expected 0 is " + cvVal_5 + "\nCV 29 Bit 6 expected 0 is " + ((cvVal_29 & 0x10)>>4) +  "\nProceed anyway?";
			if (confirm(dispText) == false)
				return;
		}


		var trkLen = configData[workCfg].TrackLen;
		if (isNaN(trkLen))
		{
			alert("No Test Track length specified");
			return;
		}
		var techSpeed = (1000 * configData[workCfg].MaxTestSpeed / 3.6) / configData[workCfg].ScaleList[configData[workCfg].ScaleIndex].Scale;
		if (isNaN(techSpeed))
		{
			alert("No maximum speed specified");
			return;
		}
		if (configData[workCfg].Units == 1) //imperial
	{
			trkLen *=2.54;
			techSpeed *= 1.6; 
		}
		ws.send("{\"Cmd\":\"SetSensor\", \"SubCmd\":\"RunTest\",\"TrackLen\":" + trkLen.toString() + ", \"Addr\":" + locoAddr.toString()+ ", \"VMax\":" + techSpeed.toString() + ", \"Mode\":" + stepMode.toString() + "}");
	}
}

function abortSpeedTest(sender)
{
	ws.send("{\"Cmd\":\"SetSensor\", \"SubCmd\":\"StopTest\"}");
}

function setTrackLength(sender)
{
	configData[workCfg].TrackLen = verifyNumber(sender.value, configData[workCfg].TrackLen); 
}

function setTestSpeed(sender)
{
	configData[workCfg].MaxTestSpeed = verifyNumber(sender.value, configData[workCfg].MaxTestSpeed); 
	var techSpeed = (1000 * configData[workCfg].MaxTestSpeed / 3.6) / configData[workCfg].ScaleList[configData[workCfg].ScaleIndex].Scale;
	console.log(configData[workCfg].MaxTestSpeed, techSpeed, configData[workCfg].ScaleList[configData[workCfg].ScaleIndex].Scale);
	if (!isNaN(techSpeed))
	{
		techSpeedProfileGraph.LineGraphs[0].DataElements[0].y = techSpeed;
		techSpeedProfileGraph.LineGraphs[0].DataElements[1].y = techSpeed;
		var yVal = 100 * (Math.trunc(1.2*techSpeed/100) + 1);
		techSpeedProfileGraph.LineGraphs[0].ValsY[1] = yVal;
		techSpeedProfileGraph.LineGraphs[1].ValsY[1] = yVal;
		techSpeedProfileGraph.LineGraphs[2].ValsY[1] = yVal;
		drawProfileBox(canvasElementTechSpeed, techSpeedProfileGraph);
	}
}

function resetDistance(sender)
{
	ws.send("{\"Cmd\":\"SetSensor\", \"SubCmd\":\"ClearDist\"}");
	writeTextField("absdist", (0).toFixed(2));
	writeTextField("reldist", (0).toFixed(2));
}

function resetHeading(sender)
{
	ws.send("{\"Cmd\":\"SetSensor\", \"SubCmd\":\"ClearHeading\"}");
	writeTextField("heading", (0).toFixed(2));
	writeTextField("roll", (0).toFixed(2));
	writeTextField("pitch", (0).toFixed(2));
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

function addEntryToArray(thisGraph, xData, yData, maxXSpan, hideData)
{
	var newElement = JSON.parse(JSON.stringify(DataElementTemplate)); // = {"x":0,"y":0};
	newElement.x = xData;
	newElement.y = yData;
	if (hideData)
		newElement.h = 1;
	thisGraph.DataElements.push(newElement);
	while (thisGraph.DataElements[0].x < (xData - maxXSpan)) //old elements in array
		thisGraph.DataElements.splice(0,1);
}


var testObj =
{
    "SlotNr": 3,
    "NumSteps": 27,
    "fw": [
        0,
        16.39818,
        51.77051,
        79.79408,
        106.53,
        128.8849,
        164.6054,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0
    ],
    "bw": [
        0,
        12.62883,
        50.51368,
        82.94406,
        112.7042,
        131.0111,
        154.6672,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0
    ]
}

function addCV(cvNr, cvVal)
{
	var newVal = JSON.parse(JSON.stringify(DataElementTemplate));
	newVal.x = cvNr;
	newVal.y = cvVal;
	cvArray.push(newVal);
}

function progTable(sender)
{
	if ((progMode > 0) && (locoAddr <= 0))
	{
		alert("Invalid Loco Address. Please assign a DCC Address.");
		return;
	}

	if (validLocoDef)
		storeToJMRI(locoDef);

	if ((progMode == 0) && (configData[nodeCfg].InterfaceIndex != 13))
		if (confirm("Place locomotive on programming track and click OK") == false)
			return;

	if (configData[nodeCfg].InterfaceIndex == 13)
	{
		if (validLocoDef)
			saveJMRIDecoder(null);
		else
			alert("Invalid Loco Definition. Please load JMRI file.");
	}
	else
		initProg();
}

function storeToJMRI(xmlNode)
{
	var decDef = xmlNode.getElementsByTagName("decoderDef");
	var varVals = xmlNode.getElementsByTagName("varValue");
	var attr = findXMLAttribute(varVals, "item", "Speed Table Definition");
	if (attr)
		attr.setAttribute("value", "1");
	var attr = findXMLAttribute(varVals, "item", "Forward Trim");
	if (attr)
		attr.setAttribute("value", fwTrim.toString());
	var attr = findXMLAttribute(varVals, "item", "Reverse Trim");
	if (attr)
		attr.setAttribute("value", bwTrim.toString());
	
	var hasSpeedTable = -1;
	var	cvVal = xmlNode.getElementsByTagName("CVvalue");
	{
		var varValStr = "";
		var cv = findXMLAttribute(cvVal, "name", (66).toString());
		if (cv)
			cv.setAttribute("value", fwTrim);
		var cv = findXMLAttribute(cvVal, "name", (95).toString());
		if (cv)
			cv.setAttribute("value", bwTrim);
		
		for (var i = 1; i <= 28; i++) //28 table entries by definition
		{
			cv = findXMLAttribute(cvVal, "name", (i+66).toString());
			if (cv)
			{
				cv.setAttribute("value", speedTableProfileGraph.LineGraphs[0].DataElements[i].y);
			}
			if (i > 1)
				varValStr += ", ";
			varValStr += speedTableProfileGraph.LineGraphs[0].DataElements[i].y.toString();
			console.log(i, cv);
		}
		var attr = findXMLAttribute(varVals, "item", "Speed Table");
		if (attr)
			attr.setAttribute("value", varValStr);
	}
}

function initProg()
{
	cvArray = [];
	//program CVs 29 bit4, 67-94, 66, 95
	//create CV Array
	addCV(66, fwTrim); //set table use
	addCV(95, bwTrim); //set table use
	for (var i = 0; i < 28; i++)
		addCV(i+67, speedTableProfileGraph.LineGraphs[0].DataElements[i].y);
	addCV(29, findCVVal(locoDef, 29) | 0x10); //set table use
	//write first CV 
	cvId = cvArray[0].x;
	cvVal = cvArray[0].y;
	writeCV(null);
	setTimeout(function(){progCVArray() }, 1000);
}

function progCVArray()
{
	//write rest of array 
	cvArray.splice(0,1); //remove first element
	if (cvArray.length > 0)
	{
		cvId = cvArray[0].x;
		cvVal = cvArray[0].y;
		writeCV(null);
		console.log("write ", cvId, cvVal);
		setTimeout(function(){progCVArray() }, 500);
	}
}

function calcTable(sender)
{
	if (validTechSpeedDef)
	{
		var avgDev = 0; //avg deviation between forward and backward
		var avgVal = 0;
		var maxVal = 0;
		speedTableProfileGraph.LineGraphs[0].DataElements = [];
		var newElement = JSON.parse(JSON.stringify(DataElementTemplate)); // = {"x":0,"y":0,"m":0};
		newElement.x = 0;
		newElement.y = 0;
		speedTableProfileGraph.LineGraphs[0].DataElements.push(newElement);
		for (var j = 0; j < 28; j++)
		{
			var targetSpeed = getSpeedForThrottleStep(j);
			var targetLevel = getMotorValueForSpeed(targetSpeed);
//			console.log(targetLevel);
			newElement = JSON.parse(JSON.stringify(DataElementTemplate)); // = {"x":0,"y":0,"m":0};
			newElement.x = j+1;
			newElement.y = Math.round((targetLevel[0] + targetLevel[1])/2);
			avgVal += newElement.y;
			if (newElement.y > maxVal)
				maxVal = newElement.y;
			avgDev += targetLevel[0] - targetLevel[1]; //add deviation between fw and bw for trim calculation
//			console.log(targetLevel[0], targetLevel[1], newElement.y, targetLevel[0] - targetLevel[1], avgDev);
			speedTableProfileGraph.LineGraphs[0].DataElements.push(newElement);
		}
		avgDev = Math.round(avgDev/28);
		avgVal = Math.round(avgVal/28);
		var trimRatio = (Math.abs(avgDev) + avgVal) / avgVal;
		if (trimMode == 0) //0-200%
		{
			fwTrim = avgDev > 0 ? Math.round(128 * trimRatio) : 128;
			bwTrim = avgDev < 0 ? Math.round(128 * trimRatio) : 128;
		}
		else //0-100%, disable Trim Values
		{
			if ((maxVal * trimRatio) <= 255)
			{
				for (var j = 0; j < 28; j++)
					speedTableProfileGraph.LineGraphs[0].DataElements[j].y = Math.round(speedTableProfileGraph.LineGraphs[0].DataElements[j].y * trimRatio); //increase table values
				fwTrim = avgDev > 0 ? 255 : Math.round(255 / trimRatio);
				bwTrim = avgDev < 0 ? 255 : Math.round(255 / trimRatio);
			}
			else //don't change anything, spoeed scale is maxed out
			{
				fwTrim = 0;
				bwTrim = 0;
			}
		}
		writeInputField("fwtrim", fwTrim);
		writeInputField("bwtrim", bwTrim);
		drawProfileBox(canvasElementSpeedTable, speedTableProfileGraph);
		validTableDef = true;
	}
	setButtonStatus();
}

function processProgrammerInput(jsonData)
{
	switch (jsonData.Status)
	{
		case 0:
			writeTextField("progstat", "Success");
			writeInputField("cvid", jsonData.CVNr);
			writeInputField("cvval", jsonData.CVVal);
			switch (jsonData.CVNr)
			{
				case 2:
					writeInputField("cvn02", jsonData.CVVal);
					writeTextField("cv02", jsonData.CVVal);
					break;
				case 5:
					writeInputField("cvn05", jsonData.CVVal);
					writeTextField("cv05", jsonData.CVVal);
					break;
				case 6:
					writeInputField("cvn06", jsonData.CVVal);
					writeTextField("cv06", jsonData.CVVal);
					break;
				case 29:
					writeInputField("cvn29", jsonData.CVVal);
					writeTextField("cv29", jsonData.CVVal);
					break;
			}
			break;
		default:
			var msgStr;
			if (jsonData.Status & 0x01)	msgStr += "Prog track empty. ";
			if (jsonData.Status & 0x02)	msgStr += "No Ack. ";
			if (jsonData.Status & 0x04)	msgStr += "Value not found. ";
			if (jsonData.Status & 0x08)	msgStr += "User aborted.";
			writeTextField("progstat", msgStr);
			break;
	}
}

function processSpeedTableInput(jsonData)
{
//	console.log(jsonData);
	for (var j = 1; j < techSpeedProfileGraph.LineGraphs.length; j++)
	{
		techSpeedProfileGraph.LineGraphs[j].DataElements = [];
		for (var i = 0; i < jsonData.bw.length; i++)
		{
			var newElement = JSON.parse(JSON.stringify(DataElementTemplate)); // = {"x":0,"y":0};
			newElement.x = i;
			newElement.y = j==1 ? jsonData.fw[i] : jsonData.bw[i];
			techSpeedProfileGraph.LineGraphs[j].DataElements.push(newElement);
		}
	}
	validTechSpeedDef = true;
	drawProfileBox(canvasElementTechSpeed, techSpeedProfileGraph);
	calcTable();
//	console.log(speedTableProfileGraph);
}

function processSensorInput(jsonData)
{
//	console.log(jsonData.Radius);

	var dirStr = jsonData.Speed > 0 ? " Forward" : jsonData.Speed < 0 ? " Backward" : "";
	writeTextField("speed", Math.abs(jsonData.Speed).toFixed(2) + dirStr);

	var currScale = configData[workCfg].ScaleList[configData[workCfg].ScaleIndex].Scale;
	
    var scaleSpeed = (jsonData.Speed  * 36 * currScale) / 10000; //[km/h]
    if (configData[workCfg].Units == 1) //imperial
		scaleSpeed /= 1.6;
	writeTextField("scalespeed", Math.abs(scaleSpeed.toFixed(2)));
	writeTextField("absdist", configData[workCfg].Units == 0 ? (jsonData.AbsDist/10).toFixed(2) : (jsonData.AbsDist/25.4).toFixed(2));
	writeTextField("reldist", configData[workCfg].Units == 0 ? (jsonData.RelDist/10).toFixed(2) : (jsonData.RelDist/25.4).toFixed(2));

	var radiusVal = Math.min(5000, configData[workCfg].Units == 0 ? Math.abs(jsonData.Radius) : Math.abs(jsonData.Radius)/25.4);
	var radiusValGraph = radiusVal > 2000 ? 0 : radiusVal;
	var radiusSig = jsonData.Radius >= 0 ? 1 : -1; 
	var radiusStr = radiusVal.toFixed(2);
	if ((jsonData.Radius != 0) && (Math.abs(jsonData.Radius) < 5000))
		dirStr = radiusStr + ((radiusSig > 0) ? " right" : " left");
	else
		dirStr = " straight";

	if (jsonData.EulerVect != undefined)
	{
		writeTextField("radius", dirStr);
		writeTextField("heading", (180 * jsonData.EulerVect[0] / 3.1415).toFixed(2));
		writeTextField("roll", (180 * jsonData.EulerVect[1] / 3.141).toFixed(2));
		writeTextField("pitch", (180 * jsonData.EulerVect[2] / 3.1415).toFixed(2));
		addEntryToArray(lineGraphHeading, jsonData.TS , ((180 * jsonData.EulerVect[0]) / 3.1415), speedGraph.MaxXRange * 1000);
		addEntryToArray(lineGraphGrade, jsonData.TS , ((180 * jsonData.EulerVect[2]) / 3.1415), speedGraph.MaxXRange * 1000);
		addEntryToArray(lineGraphElevation, jsonData.TS , ((180 * jsonData.EulerVect[1]) / 3.1415), speedGraph.MaxXRange * 1000);
		addEntryToArray(lineGraphRadius, jsonData.TS , radiusSig * radiusValGraph, speedGraph.MaxXRange * 1000, radiusValGraph == 0);
	}
	addEntryToArray(lineGraphTechSpeed, jsonData.TS , jsonData.Speed, speedGraph.MaxXRange * 1000);

	addEntryToArray(lineGraphScaleSpeed, jsonData.TS , scaleSpeed, speedGraph.MaxXRange * 1000);
	
	if (jsonData.DCCAddr != undefined)
	{
		writeTextField("dccaddr", jsonData.DCCAddr);
		writeTextField("dccaddrsp", jsonData.DCCAddr);
		writeTextField("dccaddrtbl", jsonData.DCCAddr);
		
		locoAddr = jsonData.DCCAddr;
		if ((jsonData.DCCAddr != undefined) && (jsonData.DirF != undefined) && locoAddrValid)
		{
			if (locoAddr > 127)
				cvVal_29 |= 0x20;
			else
				cvVal_29 &= ~0x20;
			writeTextField("cv29", cvVal_29);
			writeInputField("cvn29", cvVal_29);
			var speedStr = jsonData.SpeedStep.toString();
			var dirStr =((jsonData.DirF & 0x20) > 0) ? " forward" : " backward";
			writeTextField("dccstep", speedStr + " " + dirStr);
//			writeTextField("dccstepsp", speedStr + " " + dirStr);
			addEntryToArray(lineGraphSpeedStep, jsonData.TS , ((jsonData.DirF & 0x20) > 0) ? jsonData.SpeedStep : -1 * jsonData.SpeedStep, speedGraph.MaxXRange * 1000);
		}
		else
		{
			writeTextField("dccstep", "n/a");
//			writeTextField("dccstepsp", "n/a");
		}
		locoAddrValid = true;
	}
	else
	{
//		writeTextField("dccaddr", "n/a");
		writeTextField("dccstep", "n/a");
//		writeTextField("dccstepsp", "n/a");
		locoAddr -1;
		locoAddrValid = false;
	}

	drawLineGraphs();
	setButtonStatus();
}

