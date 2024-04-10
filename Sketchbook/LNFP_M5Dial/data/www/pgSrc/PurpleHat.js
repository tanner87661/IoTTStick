var mainScrollBox;
var listViewerRot;
var listViewerSingle;

var flowActiveSingle = true;
var flowActiveRot = true;

var wayPoints = [];
var drawPoints = [];

var trackerDisp = 0x7F;
var trimReserve = 0;

var tabSetup;
var tabMeasurement;
var tabAnalysis;
var tabSpeedMatch;
var tabGPS;
var tabProgrammer;
var cvTableNative;
var cvTableJMRI;
var techSpeedDiv;
var speedTableDiv;
var progDispPane;

var colorTableSpeedDisp; 
var colorTableAngleDisp; 
var colorTableAnalysis;
var colorTableThrottle;
var colorTableProfile;
var colorTableTable;

var speedDistTable;
var elevPitchTable;

var analysisBuffer = [];
var analysisIndex = 0;

var fwTrim = 0;
var bwTrim = 0;

var trackRecFile = "";
var trackRecMode = false;

var dimOptions = ["Metric","Imperial"];
var sizeOptions = ["100%","200%","300%","400%","500%"];
var relDist = 0; //relative distance, always metric
var wheelSizeDlg = null;
var progDlg = null;
var progStatus = 1; //0: busy 1: done with success 2: done with error
var progRetCode = 0; //0: busy 1: accepted, wait for answer 0x40: accepted blind 0x7F: task not implemented
var progFailCtr = 0;
/*
var progStatus = 1; //0: busy 1: done with success 2: done with error
var progRetCode = 0; //0: busy 
* 							1: accepted, wait for answer 
* 							2: timeout error 
* 							3: success 
* 							4: Prog track found empty
* 							5: No Ack Pulse
* 							6: Value not found
* 							7: Task aborted by user
* 							0x40: accepted blind 
* 							0x7F: task not implemented
var progLastCommand;
*/
var progLastCommand;

var testDist = 0;

var testPoints = [0,0,0, 1,0,0, 0.5,0.5,0, 1,0,0, -0.5,-0.5,0, 1,0,0];
var dispSize = [2,2,2];
//var canvasSize = [screen.width, 480];

var canvasElementSpeed;
var canvasElementGrade;
var canvasElementRadius;
var canvasElementGPS;
var canvasElementAnalysis;
//var canvasElementSpeedVerify;

var dispBoundaries = [-1000, 1000, -1000, 1000, -1000, 1000]; //minX, maxX, minY, maxY, minZ, maxZ
var dispOrigin = [0,0,0];
var dispMode = false;
//var ScalingFactor = [Math.min(dispSize[0], dispSize[1], dispSize[2])/Math.max(dispBoundaries[1] - dispBoundaries[0], dispBoundaries[3] - dispBoundaries[2], dispBoundaries[5] - dispBoundaries[4]);
var ScalingFactor = [dispSize[0]/(dispBoundaries[1] - dispBoundaries[0]), dispSize[1]/(dispBoundaries[3] - dispBoundaries[2])];

var	locoAddr = -1;
var locoAddrValid = false;

jsonFileVersion = "1.0.1";


function upgradeJSONVersion(jsonData)
{
	return upgradeJSONVersionPurpleHat(jsonData);
}

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
//	console.log("Btn Stat", configData[nodeCfg].InterfaceIndex, configData[workCfg].InterfaceIndex, thisIntfID);
	setVisibility([3, 12].indexOf(thisIntfID) >= 0, document.getElementById("btnAssign"));
	setVisibility([3, 12,17,18].indexOf(thisIntfID) >= 0, document.getElementById("dccaddr").parentElement);
	setVisibility([3, 12,17,18].indexOf(thisIntfID) >= 0, document.getElementById("dccstep").parentElement);
	setVisibility([3, 12,18].indexOf(thisIntfID) >= 0, document.getElementById("btnAssignsp"));
	setVisibility([3, 12,17,18].indexOf(thisIntfID) >= 0, document.getElementById("dccaddrsp").parentElement);
	setVisibility([3, 12,17,18].indexOf(thisIntfID) >= 0, document.getElementById("dccaddrtbl").parentElement);

//	setVisibility([6,12].indexOf(configData[nodeCfg].InterfaceIndex) >= 0, document.getElementById("dccstepsp"));//.parentElement);
	setVisibility([3, 12,17,18].indexOf(thisIntfID) >= 0, document.getElementById("cbsetup_3"));
//	setVisibility([6,12].indexOf(configData[nodeCfg].InterfaceIndex) >= 0, document.getElementById("cbsetup_tx_2"));
	
	setVisibility(validLocoDef, document.getElementById("btnSaveDecoder"));
	setVisibility(validLocoDef && validTableDef, document.getElementById("btnSaveDecoderXML"));
	setVisibility(validThrottleDef, document.getElementById("btnSaveThrottle"));
	setVisibility(validTableDef && (locoAddrValid || validLocoDef) && ([3,12,18].indexOf(thisIntfID) >= 0), document.getElementById("btnProg"));

	setVisibility(!validLocoDef && [17,18, 12].indexOf(thisIntfID) >= 0, cvTableNative);
	setVisibility(validLocoDef, cvTableJMRI);
	setVisibility([3, 12,18].indexOf(thisIntfID) >= 0, tabProgrammer);
	setVisibility([3, 12,18].indexOf(thisIntfID) >= 0, progDispPane);

	setVisibility(validLocoDef || [3, 12,17,18].indexOf(thisIntfID) >= 0, techSpeedDiv);
	setVisibility(validTableDef, speedTableDiv);
	
	setVisibility(false, document.getElementById("cbsetup_4")); //do not show Layout TPS tab right now

	setVisibility(validTechSpeedDef, document.getElementById("btnSaveProfile")); 
	setVisibility(locoAddrValid, document.getElementById("cbCurrSpeedPane")); 
	setVisibility(document.getElementById("cbCurrSpeed").checked && (configData[workCfg].ProgMode == 1), document.getElementById("trimValPane")); 
	
	setVisibility(configData[workCfg].TrimMode == 1, document.getElementById("trimreservepad")); 
	
//	setVisibility((!trackRecMode && (trackRecFile != "")), document.getElementById("btnStartTrack"));
//	setVisibility(trackRecMode, document.getElementById("btnEndTrack"));
//	setVisibility(trackRecFile != "", document.getElementById("btnGetTrack"));
}

function constructPageContent(contentTab)
{
	var menueStr = ["Setup", "Track Data", "Analysis", "Speed Magic", "Layout TPS"];
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
			createButton(tempObj, "", "Measure", "btnWheelSize", "measureWheelSize(this)");
			createTextInput(tempObj, "tile-1_4", "Mag. Threshold:", "n/a", "magincr", "setMagIncr(this)");
			createCheckbox(tempObj, "tile-1_4", "Reverse Dir", "cbRevDir", "setDir(this)");
		tempObj = createEmptyDiv(tabSetup, "div", "tile-1", "");
			createDropdownselector(tempObj, "tile-1_4", "Scale:", ["1:22.5","1:29","1:87", "1:160"], "selectscale", "setScaleSettings(this)");
			createDropdownselector(tempObj, "tile-1_4", "Display:", dimOptions, "selectdim", "setDimSettings(this)");
			createDropdownselector(tempObj, "tile-1_4", "Font Size:", sizeOptions, "selectfontsize", "setFontSettings(this)");
		tempObj = createEmptyDiv(tabSetup, "div", "tile-1", "");
			createDropdownselector(tempObj, "tile-1_4", "Orientation:", ["Flat","Upright"], "mountstyle", "setMountingStyle(this)");
		tempObj = createEmptyDiv(tabSetup, "div", "tile-1", "");
			createTextInput(tempObj, "tile-1_3", "Testtrack Length [mm]:", "n/a", "ttracklen", "setTrackLength(this)");
		progDispPane = createEmptyDiv(tempObj, "div", "", "");
			createDropdownselector(progDispPane, "tile-1_4", "Default Programmer:", ["Prog Track","Main Line"], "rbdefprogmode", "setProgMode(this)");
			createDropdownselector(progDispPane, "tile-1_4", "Default Prog. Mode:", ["Direct","Paged"], "rbdefprogmethod", "setProgMethod(this)");
/*
		tempObj = createEmptyDiv(tabSetup, "div", "tile-1", "");
			createRadiobox(tempObj, "tile-1_2", "Default Programmer", ["Prog Track","Main Line"], "rbdefprogmode", "setProgMode(this)");
		tempObj = createEmptyDiv(tabSetup, "div", "tile-1", "");
			createRadiobox(tempObj, "tile-1_2", "Default Prog Mode", ["Direct","Paged"], "rbdefprogmethod", "setProgMethod(this)");
*/
		tempObj = createEmptyDiv(tabSetup, "div", "tile-1", "");
			createPageTitle(tempObj, "div", "tile-1", "", "h1", "Color Settings Track Data");
			colorTableSpeedDisp = createDataTable(tempObj, "tile-1_2", ["Color Object Speed Data", "Select Color"], "ccspeeddisp", "");
//			colorTableAngleDisp = createDataTable(tempObj, "tile-1_2", ["Color Object Track Angles", "Select Color"], "ccangledisp", "");

		tempObj = createEmptyDiv(tabSetup, "div", "tile-1", "");
			createPageTitle(tempObj, "div", "tile-1", "", "h1", "Color Settings Analysis Data");
			colorTableAnalysis = createDataTable(tempObj, "tile-1_2", ["Color Object Stability", "Select Color"], "ccspeedanalysis", "");

		tempObj = createEmptyDiv(tabSetup, "div", "tile-1", "");
			createPageTitle(tempObj, "div", "tile-1", "", "h1", "Color Settings SpeedMagic");
			colorTableThrottle = createDataTable(tempObj, "tile-1_2", ["Color Object Throttle", "Select Color"], "ccspeedthrottle", "");
			colorTableProfile = createDataTable(tempObj, "tile-1_2", ["Color Object Speed Profile", "Select Color"], "ccspeedprofile", "");
			colorTableTable = createDataTable(tempObj, "tile-1_2", ["Color Object Speed Table", "Select Color"], "ccspeedtable", "");

//		tempObj = createEmptyDiv(tabSetup, "div", "tile-1", "");
//			createPageTitle(tempObj, "div", "tile-1", "", "h1", "Sensor Calibration");

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

				speedDistTable = createDataTable(dispObj, "tile-1_2", ["Direction", "","Phys. Speed [mm/s]", "","Scale Speed","", "Abs. Distance","", "Rel. Distance"], "speeddisptable", "");

//				createDispText(dispObj, "", "Phys. Speed [mm/s]:", "n/a", "speed");
//				createDispText(dispObj, "", "Scale Speed:", "n/a", "scalespeed");
//				createDispText(dispObj, "", "Abs. Distance:", "n/a", "absdist");
//				createDispText(dispObj, "", "Rel. Distance:", "n/a", "reldist");

			var dispObj = createEmptyDiv(tabMeasurement, "div", "tile-1", "Orientation");
				createPageTitle(dispObj, "div", "tile-1", "", "h2", "Orientation");
				elevPitchTable = createDataTable(dispObj, "tile-1_2", ["Radius [mm]", "", "Heading [Deg.]", "", "Grade [%]", "", "Superelevation [%]"], "elevpitchtable", "");

//				createDispText(dispObj, "", "Radius [mm]:", "n/a", "radius");
//				createDispText(dispObj, "", "Heading [Deg]:", "n/a", "heading");
//				createDispText(dispObj, "", "Grade [%]:", "n/a", "pitch");
//				createDispText(dispObj, "", "Superelevation [%]:", "n/a", "roll");

			var dispObj = createEmptyDiv(tabMeasurement, "div", "tile-1", "SelectDisplay");
				createPageTitle(dispObj, "div", "tile-1", "", "h2", "Select Data Trackers and Colors");
				tempObj = createEmptyDiv(dispObj, "div", "tile-1", "");
				createCheckbox(tempObj, "tile-1_4", "Technical Speed", "cbTechSpeed", "setTracker(this)");
				createCheckbox(tempObj, "tile-1_4", "Scale Speed", "cbScaleSpeed", "setTracker(this)");
				createCheckbox(tempObj, "tile-1_4", "Speed Step", "cbSpeedStep", "setTracker(this)");

//				tempObj = createEmptyDiv(dispObj, "div", "tile-1", "");
//				createCheckbox(tempObj, "tile-1_4", "Track Radius", "cbRadius", "setTracker(this)");
//				createCheckbox(tempObj, "tile-1_4", "Heading", "cbHeading", "setTracker(this)");

				tempObj = createEmptyDiv(dispObj, "div", "tile-1", "");
				createCheckbox(tempObj, "tile-1_4", "Grade", "cbGrade", "setTracker(this)");
				createCheckbox(tempObj, "tile-1_4", "Superelevation", "cbSElevation", "setTracker(this)");

			tempObj = createEmptyDiv(tabMeasurement, "div", "tile-1", "");
				createPageTitle(tempObj, "div", "tile-1", "", "h2", "Measurement Data");
//				var dispObj = createEmptyDiv(tempObj, "div", "tile-1", "");
//				createRadiobox(dispObj, "tile-1_2", "Use X Axis for", ["Time","Distance"], "rbxbase", "setBaseLine(this)");
//				writeRBInputField("rbxbase", 0);
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


		tabAnalysis = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
			setVisibility(false, tabAnalysis);
			createPageTitle(tabAnalysis, "div", "tile-1", "", "h1", "Locomotive Analysis");
			createButton(tabAnalysis, "", "Start", "btnWheelStart", "startMeasuring(this)");
			createButton(tabAnalysis, "", "Assign DCC Address", "btnWheelAssign", "assignDCC(this)");
			createDispText(tabAnalysis, "tile-1_4", "DCC Address:","n/a","dccwheeladdr");
			createDispText(tabAnalysis, "tile-1_4", "Speed Step:","n/a","dccwheelstep");

			tempObj = createEmptyDiv(tabAnalysis, "div", "tile-1", "");
				createPageTitle(tempObj, "div", "tile-1", "", "h2", "Speed Stability Data");
				var dispObj = createEmptyDiv(tempObj, "div", "tile-1", "");
				canvasElementAnalysis = document.createElement("canvas");
				tempObj.append(canvasElementAnalysis);
				canvasElementAnalysis.setAttribute("id", "glCanvasAnalysis");
/*
			tempObj = createEmptyDiv(tabAnalysis, "div", "tile-1", "");
				createPageTitle(tempObj, "div", "tile-1", "", "h2", "Throttle Position to Scale Speed");
				var dispObj = createEmptyDiv(tempObj, "div", "tile-1", "");
				canvasElementSpeedVerify = document.createElement("canvas");
				tempObj.append(canvasElementSpeedVerify);
				canvasElementSpeedVerify.setAttribute("id", "glCanvasVerify");
*/
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
					createRadiobox(tempObj, "tile-1_2", "Programming on", ["Prog Track","Main Line"], "rbprogmode", "setProgMode(this)");
				tempObj = createEmptyDiv(tabProgrammer, "div", "tile-1", "");
					createRadiobox(tempObj, "tile-1_2", "Programming Mode", ["Direct","Paged"], "rbprogmethod", "setProgMethod(this)");
				tempObj = createEmptyDiv(tabProgrammer, "div", "tile-1", "");
					createDispText(tempObj, "tile-1_2", "Programmer Status:","","progstat");
				tempObj = createEmptyDiv(tabProgrammer, "div", "tile-1", "");
					createTextInput(tempObj, "tile-1_4", "Program CV:", "", "cvid", "setCV(this)");
					createTextInput(tempObj, "tile-1_4", "CV Value:", "", "cvval", "setCV(this)");
				tempObj = createEmptyDiv(tabProgrammer, "div", "tile-1", "");
					createButton(tempObj, "", "Read CV", "btnReadCV", "readCV(this)");
					createButton(tempObj, "", "Write CV", "btnWriteCV", "writeCV(this)");
				setVisibility(false, tabProgrammer);
				setVisibility(true, cvTableNative);
				setVisibility(false, cvTableJMRI);
				setVisibility(true, canvasElementAnalysis);

				setVisibility(false, canvasElementGrade);
				setVisibility(false, canvasElementRadius);


			var dispObj = createEmptyDiv(tabSpeedMatch, "div", "tile-1", "Throttle Profile");
				createPageTitle(dispObj, "div", "tile-1", "", "h2", "Throttle Profile");
				createFileDlg(dispObj, "", "Load Profile", "btnLoadThrottle", "application/json", "loadThrottle(this)");
				createButton(dispObj, "", "Save Profile", "btnSaveThrottle", "saveThrottle(this)");
				createTextInput(dispObj, "tile-1_4", "Profile Name:", "n/a", "thprofilename", "setThrottleProfile(this)");
				createDispText(dispObj, "tile-1_4", "File Name:","n/a","throttlefilename");

//				tempObj = createEmptyDiv(dispObj, "div", "tile-1", "");
				tempObj = createEmptyDiv(dispObj, "div", "tile-1", "");
				createTextInput(tempObj, "tile-1_4", "Max. Scale:", "n/a", "thscalespeed", "setThrottleProfile(this)");
				createTextInput(tempObj, "tile-1_4", "@ Throttle Step:", "n/a", "thvmax", "setThrottleProfile(this)");
				createTextInput(tempObj, "tile-1_4", "of # Steps:", "n/a", "thnumsteps", "setThrottleProfile(this)");

				tempObj = createEmptyDiv(dispObj, "div", "tile-1", "cbCurrSpeedPane");
				createCheckbox(tempObj, "tile-1_4", "Show Current Speed", "cbCurrSpeed", "setCurrSpeed(this)");
				tempObj = createEmptyDiv(dispObj, "div", "tile-1", "");
				canvasElementThrottle = document.createElement("canvas");
				tempObj.append(canvasElementThrottle);
				canvasElementThrottle.setAttribute("id", "glCanvasThrottle");
				setCanvasMouseEvents(canvasElementThrottle);

				tempObj = createEmptyDiv(dispObj, "div", "tile-1", "trimValPane");
				createTextInput(tempObj, "tile-1_4", "Forward Trim:", "n/a", "fwtrimadj", "setTrim(this)");
				createTextInput(tempObj, "tile-1_4", "Backward Trim:", "n/a", "bwtrimadj", "setTrim(this)");
				createButton(tempObj, "", "Adjust Trim Values", "btnsendtrimvals", "sendTrimValues(this)");


			var dispObj = createEmptyDiv(tabSpeedMatch, "div", "tile-1", "Technical Speed Profile");
				techSpeedDiv = dispObj;
				createPageTitle(dispObj, "div", "tile-1", "", "h2", "Technical Speed Profile");
				createFileDlg(dispObj, "", "Load Profile", "btnLoadProfile", "application/json", "loadProfile(this)");
				createButton(dispObj, "", "Save Profile", "btnSaveProfile", "saveProfile(this)");
				createTextInput(dispObj, "tile-1_4", "Profile Name:", "n/a", "techprofilename", "setTechProfile(this)");
				createDispText(dispObj, "tile-1_4", "File Name:","n/a","techprofilefilename");
				tempObj = createEmptyDiv(dispObj, "div", "tile-1", "");
				createButton(tempObj, "", "Assign DCC Address", "btnAssignsp", "assignDCC(this)");
				createDispText(tempObj, "tile-1_4", "DCC Address:","n/a","dccaddrsp");
				createDispText(tempObj, "tile-1_4", "Speed Step:","n/a","dccstepsp");
				var ssRB = createRadiobox(tempObj, "tile-1_2", "Speed Steps:", ["28","127"], "dccstepsprb", "setTestMode(this)");
				setVisibility(false, ssRB);
				tempObj = createEmptyDiv(dispObj, "div", "tile-1", "");
				createButton(tempObj, "", "Start Test", "btnstarttest", "startSpeedTest(this)");
				createButton(tempObj, "", "Abort Test", "btnaborttest", "abortSpeedTest(this)");
				createTextInput(tempObj, "tile-1_4", "Track length [mm]:", "n/a", "testtracklen", "setTrackLength(this)");
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
				createRadiobox(tempObj, "tile-1_2", "Calculation Base:", ["Average", "Lower Curve","Higher Curve"], "calcbase", "setCalcBase(this)");

				tempObj = createEmptyDiv(dispObj, "div", "tile-1", "trimreservepad");
				createTextInput(tempObj, "tile-1_2", "Trim Reserve [%]:", 0, "trimreserve", "setTrimReserve(this)");
				tempObj = createEmptyDiv(dispObj, "div", "tile-1", "");
				createRadiobox(tempObj, "tile-1_2", "Trim Range:", ["0-200%", "0-100%"], "trimmode", "setTrimMode(this)");

//				createRadiobox(tempObj, "tile-1_2", "Use", ["Speed Table","Min/Mid/Max only"], "rbtablemode", "setTableMode(this)");

				tempObj = createEmptyDiv(dispObj, "div", "tile-1", "");
				createButton(tempObj, "", "Recalculate", "btnRecalc", "calcTable(this)");
				createButton(tempObj, "", "Write CV's", "btnProg", "progTable(this)");
				createButton(tempObj, "", "Save JMRI File", "btnSaveDecoderXML", "saveJMRIDecoder(this)");
				createDispText(tempObj, "tile-1_4", "DCC Address:","n/a","dccaddrtbl");

				tempObj = createEmptyDiv(dispObj, "div", "tile-1", "");
				createTextInput(tempObj, "tile-1_4", "Forward Trim:", "n/a", "fwtrim", "setTrim(this)");
				createTextInput(tempObj, "tile-1_4", "Backward Trim:", "n/a", "bwtrim", "setTrim(this)");

		
				tempObj = createEmptyDiv(dispObj, "div", "tile-1", "");
				canvasElementSpeedTable = document.createElement("canvas");
				tempObj.append(canvasElementSpeedTable);
				canvasElementSpeedTable.setAttribute("id", "glCanvasSpeedTable");
				setCanvasMouseEvents(canvasElementSpeedTable);

				tabGPS = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
				createPageTitle(tabGPS, "div", "tile-1", "", "h1", "Layout TPS");
//				setVisibility(false, document.getElementById("btnSaveDecoderXML"));
				setVisibility(false, tabGPS);
		mainScrollBox.addEventListener('scroll', function () 
			{	
				if (!isInViewport(canvasElementThrottle))
					if (document.getElementById("cbCurrSpeed").checked)
					{
						measureSpeed(false);
						setButtonStatus();
					}
			});

		setTrackerDisplay();
}

function setColorData(sender)
{
	var thisRow = parseInt(sender.getAttribute("row"));
	var thisCol = parseInt(sender.getAttribute("col"));
	var thisIndex = parseInt(sender.getAttribute("index"));
	var thisColorStr = sender.value.replace("#", "0x");
	var thisColor = parseInt(thisColorStr);
	var colorArray = [((thisColor & 0xFF0000) >> 16), ((thisColor & 0x00FF00) >> 8), (thisColor & 0x0000FF)];
	if (sender.id.search("ccspeeddisp_") == 0)
		configData[workCfg].LEDColsSpeedDisp[thisRow].RGBVal = colorArray; //background
	if (sender.id.search("ccangledisp_") == 0)
		configData[workCfg].LEDColsAngleDisp[thisRow].RGBVal = colorArray; //background
	if (sender.id.search("ccspeedanalysis_") == 0)
		configData[workCfg].LEDColsStability[thisRow].RGBVal = colorArray; //background
	if (sender.id.search("ccspeedthrottle_") == 0)
		configData[workCfg].LEDColsThrottle[thisRow].RGBVal = colorArray; //background
	if (sender.id.search("ccspeedprofile_") == 0)
		configData[workCfg].LEDColsProfile[thisRow].RGBVal = colorArray; //background
	if (sender.id.search("ccspeedtable_") == 0)
		configData[workCfg].LEDColsTable[thisRow].RGBVal = colorArray; //background
}

function loadSpeedDispTable(thisTable, jsonData)
{
	var th = document.getElementById(thisTable.id + "_head");
	var tb = document.getElementById(thisTable.id + "_body");
	var scaleStr = configData[workCfg].ScaleList[configData[workCfg].ScaleIndex].Name;
	if (jsonData.Units)
	{
		th.childNodes[0].childNodes[4].innerHTML = scaleStr + " Scale Speed [mph]";
		th.childNodes[0].childNodes[6].innerHTML = "Abs. Distance [in]";
		th.childNodes[0].childNodes[8].innerHTML = "Rel. Distance [in]";
	}
	else
	{
		th.childNodes[0].childNodes[4].innerHTML = scaleStr + " Scale Speed [km/h]";
		th.childNodes[0].childNodes[6].innerHTML = "Abs. Distance [cm]";
		th.childNodes[0].childNodes[8].innerHTML = "Rel. Distance [cm]";
	}
	var numCols = th.childNodes[0].children.length;
	createDataTableLines(thisTable, [tfText,tfText,tfText,tfText,tfText,tfText,tfText,tfText,tfText], 1, "");	
	var fSize = configData[workCfg].SpeedDispSize.toString() + "%";
	tb.childNodes[0].style.height = 50 * (configData[workCfg].SpeedDispSize/100); // * tb.style.fontSize;
	var e = document.getElementById(thisTable.id + "_0_0");
	e.childNodes[0].style.fontSize = fSize;
	e.childNodes[0].innerHTML = jsonData.Speed > 0 ? " FW" : jsonData.Speed < 0 ? " BW" : "   ";
	var e = document.getElementById(thisTable.id + "_0_1");
	e.childNodes[0].style.fontSize = fSize;
	e.childNodes[0].innerHTML = "&nbsp";
	var currScale = configData[workCfg].ScaleList[configData[workCfg].ScaleIndex].Scale;
	var scaleSpeed = ((jsonData.Speed  * 36 * currScale) / 10000).toFixed(2); //[km/h]
	var e = document.getElementById(thisTable.id + "_0_2");
	e.childNodes[0].style.fontSize = fSize;
	e.childNodes[0].innerHTML = configData[workCfg].Units == 0 ? scaleSpeed : scaleSpeed / 1.6;
	var e = document.getElementById(thisTable.id + "_0_3");
	e.childNodes[0].style.fontSize = fSize;
	e.childNodes[0].innerHTML = "&nbsp";
	var e = document.getElementById(thisTable.id + "_0_4");
	e.childNodes[0].style.fontSize = fSize;
	e.childNodes[0].innerHTML = Math.abs(jsonData.Speed).toFixed(2);
	var e = document.getElementById(thisTable.id + "_0_5");
	e.childNodes[0].style.fontSize = fSize;
	e.childNodes[0].innerHTML = "&nbsp";
	var e = document.getElementById(thisTable.id + "_0_6");
	e.childNodes[0].style.fontSize = fSize;
	e.childNodes[0].innerHTML = configData[workCfg].Units == 0 ? (jsonData.AbsDist/10).toFixed(2) : (jsonData.AbsDist/25.4).toFixed(2);
	var e = document.getElementById(thisTable.id + "_0_7");
	e.childNodes[0].style.fontSize = fSize;
	e.childNodes[0].innerHTML = "&nbsp";
	var e = document.getElementById(thisTable.id + "_0_8");
	e.childNodes[0].style.fontSize = fSize;
	e.childNodes[0].innerHTML = configData[workCfg].Units == 0 ? (jsonData.RelDist/10).toFixed(2) : (jsonData.RelDist/25.4).toFixed(2);
}

function loadElevPitchTable(thisTable, jsonData)
{
	var th = document.getElementById(thisTable.id + "_head");
	var tb = document.getElementById(thisTable.id + "_body");
	if (jsonData.Units)
		th.childNodes[0].childNodes[0].innerHTML = "Radius [in]";
	else
		th.childNodes[0].childNodes[0].innerHTML = "Radius [mm]";
	var numCols = th.childNodes[0].children.length;

	createDataTableLines(thisTable, [tfText,tfText,tfText,tfText,tfText,tfText,tfText], 1, "");	
	var fSize = configData[workCfg].SpeedDispSize.toString() + "%";
	tb.childNodes[0].style.height = 50 * (configData[workCfg].SpeedDispSize/100); // * tb.style.fontSize;
	var e = document.getElementById(thisTable.id + "_0_0");
	e.childNodes[0].style.fontSize = fSize;
	var radiusVal = Math.min(5000, configData[workCfg].Units == 0 ? Math.abs(jsonData.Radius) : Math.abs(jsonData.Radius)/25.4);
	var radiusValGraph = radiusVal > 2000 ? 0 : radiusVal;
	var radiusSig = jsonData.Radius >= 0 ? 1 : -1; 
	var radiusStr = radiusVal.toFixed(2);
	if ((jsonData.Radius != 0) && (Math.abs(jsonData.Radius) < 5000))
		dirStr = radiusStr + ((radiusSig > 0) ? " right" : " left");
	else
		dirStr = " straight";
	e.childNodes[0].innerHTML = dirStr;
	var e = document.getElementById(thisTable.id + "_0_1");
	e.childNodes[0].style.fontSize = fSize;
	e.childNodes[0].innerHTML = "&nbsp";
	var e = document.getElementById(thisTable.id + "_0_2");
	e.childNodes[0].style.fontSize = fSize;
	e.childNodes[0].innerHTML = (180 * jsonData.EulerVect[0] / 3.1415).toFixed(2);
	var e = document.getElementById(thisTable.id + "_0_3");
	e.childNodes[0].style.fontSize = fSize;
	e.childNodes[0].innerHTML = "&nbsp";
	var e = document.getElementById(thisTable.id + "_0_4");
	e.childNodes[0].style.fontSize = fSize;
	e.childNodes[0].innerHTML = jsonData.Slope ? jsonData.Slope.toFixed(1) : "n/a";
	var e = document.getElementById(thisTable.id + "_0_5");
	e.childNodes[0].style.fontSize = fSize;
	e.childNodes[0].innerHTML = "&nbsp";
	var e = document.getElementById(thisTable.id + "_0_6");
	e.childNodes[0].style.fontSize = fSize;
	e.childNodes[0].innerHTML = jsonData.Banking ? jsonData.Banking.toFixed(1) : "n/a";
}

function loadColorTable(thisTable, thisData)
{
	function toHex(numbers) 
	{
		var r = numbers[0].toString(16), /* 1 */
			g = numbers[1].toString(16),
			b = numbers[2].toString(16);
		while (r.length < 2) {r = "0" + r};
		while (g.length < 2) {g = "0" + g};
		while (b.length < 2) {b = "0" + b};
		return "#" + r + g + b;
	}

	var th = document.getElementById(thisTable.id + "_head");
	var tb = document.getElementById(thisTable.id + "_body");
	var numCols = th.childNodes[0].children.length;

	createDataTableLines(thisTable, [tfText,tfColorPicker], thisData.length, "setColorData(this)");	

	for (var i=0; i<thisData.length;i++)
	{
		var e = document.getElementById(thisTable.id + "_" + i.toString() + "_" + "0");
		e.childNodes[0].innerHTML = thisData[i].Name;
		var e = document.getElementById(thisTable.id + "_" + i.toString() + "_" + "1");
		if (thisData[i].RGBVal == undefined)
		{
			thisData[i].RGBVal = HSVtoRGB(thisData[i].HSVVal[0], thisData[i].HSVVal[1], thisData[i].HSVVal[2]);
//			console.log(thisData[i]);
		}
		var colStr = toHex(thisData[i].RGBVal);
		e.childNodes[0].value = colStr;
	}
}

function drawLineGraphs()
{
	drawLineGraph(canvasElementSpeed, speedGraph);
	drawLineGraph(canvasElementGrade, angleGraph);
	drawLineGraph(canvasElementRadius, radiusGraph);
//	drawLineGraph(canvasElementAnalysis, stabilityGraph);
}

function drawStabilityGraphs()
{
	drawLineGraph(canvasElementAnalysis, stabilityGraph);
//	drawProfileBox(canvasElementSpeedVerify, speedVerifyProfileGraph);
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
	setVisibility(false, tabAnalysis);
	setVisibility(false, tabSpeedMatch);
	setVisibility(false, tabGPS);



	closeWheelSize(sender);
	ws.send("{\"Cmd\":\"SetSensor\", \"SubCmd\":\"RepRate\",\"Val\":0}");
	document.getElementById("btnStart").innerHTML = "Start";
	document.getElementById("btnStartLow").innerHTML = "Start";
	document.getElementById("btnWheelStart").innerHTML = "Start";
	document.getElementById("cbCurrSpeed").checked = false;

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
			setVisibility(true, tabAnalysis);
			break;
		case "cbsetup_3":
//			writeRBInputField("cbsetup", 3);
			setVisibility(true, tabSpeedMatch);
			break;
		case "cbsetup_4":
//			writeRBInputField("cbsetup", 4);
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
//	document.getElementById("cbRadius").checked = (trackerDisp & 0x08) > 0;
//	document.getElementById("cbHeading").checked = (trackerDisp & 0x10) > 0;
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
	drawStabilityGraphs();
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
//			console.log(configArray.TrackPoints.length);
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
//	console.log(jsonData);
//	console.log(thisIntfID);
	setButtonStatus();
//	document.getElementById("btnProg").innerHTML = thisIntfID == 17 ? "Save JMRI File" : "Write CV's";
//	setVisibility(false, document.getElementById("btnSaveDecoderXML"));
}


function loadDataFields(jsonData)
{
	configData[workCfg] = upgradeJSONVersion(jsonData);
//	console.log(jsonData);
//	console.log(thisIntfID);
	writeInputField("wheelsize", jsonData.WheelDia);
	writeInputField("magincr", jsonData.MagThreshold);
	writeInputField("testtracklen", jsonData.TrackLen);
	writeInputField("ttracklen", jsonData.TrackLen);
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
	setDropdownValue("selectfontsize", Math.round(jsonData.SpeedDispSize /100) - 1); //most likely HO
	setDropdownValue("mountstyle", jsonData.MountStyle); //flat or upright
	var scaleStr = jsonData.ScaleList[jsonData.ScaleIndex].Name;
	switch (jsonData.Units)
	{
		case 0: 
//			document.getElementById("scalespeed_txt").innerHTML = scaleStr + " Scale Speed [km/h]";
//			document.getElementById("absdist_txt").innerHTML = "Abs. Distance [cm]";
//			document.getElementById("reldist_txt").innerHTML = "Rel. Distance [cm]";
//			document.getElementById("radius_txt").innerHTML = "Radius [mm]";
			document.getElementById("thscalespeed_txt").innerHTML = "Max. Scale [km/h]";
			document.getElementById("testtracklen_txt").innerHTML = "Track Length [mm]"; 
			document.getElementById("ttracklen_txt").innerHTML = "Track Length [mm]"; 
			document.getElementById("testmaxspeed_txt").innerHTML = "Max Speed [km/h]"; 
			document.getElementById("jmrivmax_txt").innerHTML = "Max Speed [km/h]:"; 
			
			break;
		case 1: 
//			document.getElementById("scalespeed_txt").innerHTML = scaleStr + " Scale Speed [mph]";
//			document.getElementById("absdist_txt").innerHTML = "Abs. Distance [in]";
//			document.getElementById("reldist_txt").innerHTML = "Rel. Distance [in]";
//			document.getElementById("radius_txt").innerHTML = "Radius [in]";
			document.getElementById("thscalespeed_txt").innerHTML = "Max. Scale [mph]";
			document.getElementById("testtracklen_txt").innerHTML = "Track Length [in]";
			document.getElementById("ttracklen_txt").innerHTML = "Track Length [in]";
			document.getElementById("testmaxspeed_txt").innerHTML = "Max Speed [mph]"; 
			document.getElementById("jmrivmax_txt").innerHTML = "Max Speed [mph]:"; 
			break;
	}
	writeRBInputField("rbprogmethod", configData[workCfg].ProgMethod);
	setDropdownValue("rbdefprogmethod", configData[workCfg].ProgMethod);
	writeRBInputField("rbprogmode", configData[workCfg].ProgMode);
	setDropdownValue("rbdefprogmode", configData[workCfg].ProgMode);
	writeRBInputField("calcbase", configData[workCfg].CalcMode);
	writeRBInputField("trimmode", configData[workCfg].TrimMode);
	writeRBInputField("dccstepsprb", stepMode);

//	console.log(configData[workCfg]);
/*		
	if (configData[workCfg].SpeedDispSize)
	{
		var fSize = configData[workCfg].SpeedDispSize.toString() + "%";
		document.getElementById("speed").style.fontSize = fSize;
		document.getElementById("scalespeed").style.fontSize = fSize;
		document.getElementById("absdist").style.fontSize = fSize;
		document.getElementById("reldist").style.fontSize = fSize;
//		document.getElementById("radius").style.fontSize = fSize;
//		document.getElementById("heading").style.fontSize = fSize;
//		document.getElementById("pitch").style.fontSize = fSize;
//		document.getElementById("roll").style.fontSize = fSize;
	}
*/	
	if (configData[workCfg].LEDColsSpeedDisp)
	{
		graphColorSpeed = "rgb(" + configData[workCfg].LEDColsSpeedDisp[0].RGBVal.toString() + ")";
		graphColorTechSpeed = "rgb(" + configData[workCfg].LEDColsSpeedDisp[1].RGBVal.toString() + ")";
		graphColorScaleSpeed = "rgb(" + configData[workCfg].LEDColsSpeedDisp[2].RGBVal.toString() + ")";
		graphColorSpeedStep = "rgb(" + configData[workCfg].LEDColsSpeedDisp[3].RGBVal.toString() + ")";
		loadColorTable(colorTableSpeedDisp, configData[workCfg].LEDColsSpeedDisp);
	}
/*
	if (configData[workCfg].LEDColsAngleDisp)
	{
		graphColorGrade = "rgb(" + configData[workCfg].LEDColsAngleDisp[0].RGBVal.toString() + ")";
		graphColorSlope = "rgb(" + configData[workCfg].LEDColsAngleDisp[1].RGBVal.toString() + ")";
		graphColorSElevation = "rgb(" + configData[workCfg].LEDColsAngleDisp[2].RGBVal.toString() + ")";
		loadColorTable(colorTableAngleDisp, configData[workCfg].LEDColsAngleDisp);
	}
*/ 
	if (configData[workCfg].LEDColsStability)
	{
		graphColorStability = "rgb(" + configData[workCfg].LEDColsStability[0].RGBVal.toString() + ")";
		graphColorStab1 = "rgb(" + configData[workCfg].LEDColsStability[1].RGBVal.toString() + ")";
		graphColorStab2 = "rgb(" + configData[workCfg].LEDColsStability[2].RGBVal.toString() + ")";
		graphColorStab3 = "rgb(" + configData[workCfg].LEDColsStability[3].RGBVal.toString() + ")";
		loadColorTable(colorTableAnalysis, configData[workCfg].LEDColsStability);
	}
	if (configData[workCfg].LEDColsThrottle)
	{
		graphBoxThrottle = "rgb(" + configData[workCfg].LEDColsThrottle[0].RGBVal.toString() + ")";
		graphColorThrottle = "rgb(" + configData[workCfg].LEDColsThrottle[1].RGBVal.toString() + ")";
		graphColorSpeedDot = "rgb(" + configData[workCfg].LEDColsThrottle[2].RGBVal.toString() + ")";
		loadColorTable(colorTableThrottle, configData[workCfg].LEDColsThrottle);
	}
	if (configData[workCfg].LEDColsProfile)
	{
		graphBoxTechSpeed = "rgb(" + configData[workCfg].LEDColsProfile[0].RGBVal.toString() + ")";
		graphColorTechProfileSpeedLimit = "rgb(" + configData[workCfg].LEDColsProfile[1].RGBVal.toString() + ")";
		graphColorTechProfileSpeedFw = "rgb(" + configData[workCfg].LEDColsProfile[2].RGBVal.toString() + ")";
		graphColorTechProfileSpeedBw = "rgb(" + configData[workCfg].LEDColsProfile[3].RGBVal.toString() + ")";
		loadColorTable(colorTableProfile, configData[workCfg].LEDColsProfile);
	}
	if (configData[workCfg].LEDColsTable)
	{
		graphBoxSpeedTable = "rgb(" + configData[workCfg].LEDColsTable[0].RGBVal.toString() + ")";
		graphColorSpeedTable = "rgb(" + configData[workCfg].LEDColsTable[1].RGBVal.toString() + ")";
		loadColorTable(colorTableTable, configData[workCfg].LEDColsTable);
	}

	loadSpeedGraphData();
	loadStabilityGraphData();
	drawLineGraphs();
	drawStabilityGraphs();

	setButtonStatus();
	loadSpeedMagicGraphData();
	displayThrottleDef(throttleDef);
	displayTechSpeedDef(speedProfileDef);
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

function setFontSettings(sender)
{
	configData[workCfg].SpeedDispSize = 100 * (sender.selectedIndex + 1);
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
	if ((configData[workCfg].ProgMode > 0) && (locoAddr <= 0))
	{
		alert("Invalid Loco Address. Please assign a DCC Address.");
		return;
	}
	if (configData[workCfg].ProgMode == 0)
		if (confirm("Place locomotive on programming track and click OK") == false)
			return;
	if ((cvId >= 0) && (cvId <= 255))
	{
		writeTextField("progstat", "Read CV in progress");
		ws.send("{\"Cmd\":\"SetSensor\", \"SubCmd\":\"ReadCV\", \"Addr\":" + locoAddr.toString() + ",\"ProgMode\":" + configData[workCfg].ProgMode.toString() + ",\"ProgMethod\":" + configData[workCfg].ProgMethod.toString() + ",\"CV\":" + cvId.toString() + "}");
		progLastCommand = Date.now();
		progStatus = 0;
		progFailCtr = 0;
	}
	else
		alert("Invalid CV");
}

function writeCV(sender)
{
	if ((configData[workCfg].ProgMode > 0) && (locoAddr <= 0))
	{
		if (sender) alert("Invalid Loco Address. Please assign a DCC Address.");
		return false;
	}
	if (sender) 
		if (configData[workCfg].ProgMode == 0)
			if (confirm("Place locomotive on programming track and click OK") == false)
				return false;
	if ((cvId >= 0) && (cvVal >= 0) && (cvId <= 255) && (cvVal <= 255))
	{
		if (sender) writeTextField("progstat", "Write CV in progress");
		ws.send("{\"Cmd\":\"SetSensor\", \"SubCmd\":\"WriteCV\",\"Addr\":" + locoAddr.toString() + ", \"ProgMode\":" + configData[workCfg].ProgMode.toString() + ",\"ProgMethod\":" + configData[workCfg].ProgMethod.toString() + ",\"CV\":" + cvId.toString() + ",\"CVVal\":" + cvVal.toString() + "}");
		progLastCommand = Date.now();
		progStatus = 0;
		progFailCtr = 0;
		return true;
	}
	else
	{
		if (sender) alert("Invalid CV or CV Value");
		return false;
	}
}

function sendTrimValues(sender)
{
	initProg(false);
}

function setCurrSpeed(sender)
{
	if (sender.checked)
	{
		ws.send("{\"Cmd\":\"SetSensor\", \"SubCmd\":\"RepRate\",\"Val\":250}");
		document.getElementById("btnStart").innerHTML = "Stop";
		document.getElementById("btnStartLow").innerHTML = "Stop";
		document.getElementById("btnWheelStart").innerHTML = "Stop";
	}
	else
	{
		ws.send("{\"Cmd\":\"SetSensor\", \"SubCmd\":\"RepRate\",\"Val\":0}");
		document.getElementById("btnStart").innerHTML = "Start";
		document.getElementById("btnStartLow").innerHTML = "Start";
		document.getElementById("btnWheelStart").innerHTML = "Start";
		drawProfileBox(canvasElementThrottle, throttleProfileGraph);
	}
	setButtonStatus();
}

function startMeasuring(sender)
{
	if (sender.innerHTML == "Start")
		if (sender.id == "btnWheelStart")
			measureSpeed(250)
		else
			measureSpeed(500)
	else
		measureSpeed(0)
}
	
function measureSpeed(refRate)
{	
	if (refRate > 0)
	{
		clearAllGraphData();
		ws.send("{\"Cmd\":\"SetSensor\", \"SubCmd\":\"RepRate\",\"Val\":" + refRate.toString() + "}");
		document.getElementById("btnStart").innerHTML = "Stop";
		document.getElementById("btnStartLow").innerHTML = "Stop";
		document.getElementById("btnWheelStart").innerHTML = "Stop";
		document.getElementById("cbCurrSpeed").checked = true;
	}
	else
	{
		ws.send("{\"Cmd\":\"SetSensor\", \"SubCmd\":\"RepRate\",\"Val\":0}");
		document.getElementById("btnStart").innerHTML = "Start";
		document.getElementById("btnStartLow").innerHTML = "Start";
		document.getElementById("btnWheelStart").innerHTML = "Start";
		document.getElementById("cbCurrSpeed").checked = false;
	}
}

function assignDCC(sender)
{
	locoAddr = -1;
	writeTextField("dccaddr", "listening...");
	writeTextField("dccaddrsp", "listening...");
	writeTextField("dccwheeladdr", "listening...");
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
		if ((thisIntfID == 17) || (thisIntfID == 18)) //WiThrottle
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
			var dispText = "Verify and confirm CV settings before running the speed test.\nCV 2 expected 0 is " + cvVal_2 + "\nCV 5 expected 0 is " + cvVal_5 + "\nCV 6 expected 0 is " + cvVal_5 + "\nCV 29 Bit 4 expected 0 is " + ((cvVal_29 & 0x10)>>4) +  "\nProceed anyway?";
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
		var speedPOI = [];
//		console.log(throttleProfileGraph);
		if (throttleProfileGraph.LineGraphs[0].DataElements.length > 0)
		{
			for (var i = 0; i < throttleProfileGraph.LineGraphs[0].DataElements.length; i++)
			{
				if ((throttleProfileGraph.LineGraphs[0].DataElements[i].y > 0) && (throttleProfileGraph.LineGraphs[0].DataElements[i].y < configData[workCfg].MaxTestSpeed))
				{
					var speedPt = (1000 * throttleProfileGraph.LineGraphs[0].DataElements[i].y / 3.6) / configData[workCfg].ScaleList[configData[workCfg].ScaleIndex].Scale;
					if (configData[workCfg].Units == 1) //imperial
						speedPt *= 1.6;
					speedPOI.push(speedPt);
				}
			}
		}
		if (configData[workCfg].Units == 1) //imperial
	{
			trkLen *=25.4; //mm/inch
			techSpeed *= 1.6; //km/m
		}
//		console.log("{\"Cmd\":\"SetSensor\", \"SubCmd\":\"RunTest\",\"TrackLen\":" + trkLen.toString() + ", \"Addr\":" + locoAddr.toString()+ ", \"VMax\":" + techSpeed.toString() + ", \"POI\":[" + speedPOI.toString() + "]}");
		ws.send("{\"Cmd\":\"SetSensor\", \"SubCmd\":\"RunTest\",\"TrackLen\":" + trkLen.toString() + ", \"Addr\":" + locoAddr.toString()+ ", \"VMax\":" + techSpeed.toString() + ", \"POI\":[" + speedPOI.toString() + "]}");
	}
}

function abortSpeedTest(sender)
{
	ws.send("{\"Cmd\":\"SetSensor\", \"SubCmd\":\"StopTest\"}");
}

function setTrackLength(sender)
{
	configData[workCfg].TrackLen = verifyNumber(sender.value, configData[workCfg].TrackLen); 
	if (sender.id == "testtracklen")
		writeInputField("ttracklen", configData[workCfg].TrackLen);
	else
		writeInputField("testtracklen", configData[workCfg].TrackLen);
}

function setTestSpeed(sender)
{
	configData[workCfg].MaxTestSpeed = verifyNumber(sender.value, configData[workCfg].MaxTestSpeed); 
	var techSpeed = (1000 * configData[workCfg].MaxTestSpeed / 3.6) / configData[workCfg].ScaleList[configData[workCfg].ScaleIndex].Scale;
	if (configData[workCfg].Units == 1)
		techSpeed *= 1.6; //imperial
//	console.log(configData[workCfg].MaxTestSpeed, techSpeed, configData[workCfg].ScaleList[configData[workCfg].ScaleIndex].Scale);
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

function calcWheelSize(sender)
{
	if (Math.abs(relDist) < 20)
		alert("Distance is too short to calculate diameter. Try again");
	else
	{
		var newDist = parseFloat(readTextInputField("testdist"));
		if (newDist != NaN)
		{
//			console.log(newDist, relDist, configData[loadCfg].WheelDia);
			var newDia = Math.abs(newDist / relDist) * configData[loadCfg].WheelDia;
			configData[workCfg].WheelDia = verifyNumber(newDia.toFixed(2), configData[loadCfg].WheelDia); 
			writeInputField("wheelsize", configData[workCfg].WheelDia);
		}
		else
			alert("Not a valid distance input");
	}
	closeWheelSize(sender);
}

function startWheelSize(sender)
{
	resetDistance(sender);
	ws.send("{\"Cmd\":\"SetSensor\", \"SubCmd\":\"RepRate\",\"Val\":500}");
}

function closeWheelSize(sender)
{
	if (wheelSizeDlg)
		wheelSizeDlg.open = false;
	ws.send("{\"Cmd\":\"SetSensor\", \"SubCmd\":\"RepRate\",\"Val\":0}");
	document.getElementById("btnStart").innerHTML = "Start";
	document.getElementById("btnStartLow").innerHTML = "Start";
	document.getElementById("btnWheelStart").innerHTML = "Start";
	document.getElementById("cbCurrSpeed").checked = false;
}

function measureWheelSize(sender)
{
	if (!wheelSizeDlg)
	{
		wheelSizeDlg = document.createElement("DIALOG");
		tempObj = createEmptyDiv(wheelSizeDlg, "div", "tile-1_2", "");
		createDispText(tempObj, "tile-1", "","Enter push distance, push car, then Submit","distext");
		tempObj = createEmptyDiv(wheelSizeDlg, "div", "tile-1_2", "");
		createTextInput(tempObj, "tile-1_4", "Push Distance [mm]:", "150", "testdist", "");
//		writeInputField("testdist", 150);
	
		tempObj = createEmptyDiv(wheelSizeDlg, "div", "tile-1_2", "");
		createButton(tempObj,  "", "Start", "btnSizeStart", "startWheelSize(this)");
		createButton(tempObj,  "", "Cancel", "btnSizeCancel", "closeWheelSize(this)");
		createButton(tempObj,  "", "Submit", "btnSizeOK", "calcWheelSize(this)");

		document.body.appendChild(wheelSizeDlg);
	}
	wheelSizeDlg.open = true;

	startWheelSize(sender);
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

/*
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
*/

function addCV(cvNr, cvVal)
{
	var newVal = JSON.parse(JSON.stringify(DataElementTemplate));
	newVal.x = cvNr;
	newVal.y = cvVal;
	cvArray.push(newVal);
}

function progTable(sender)
{
	if ((configData[workCfg].ProgMode > 0) && (locoAddr <= 0))
	{
		alert("Invalid Loco Address. Please assign a DCC Address.");
		return;
	}

	if (validLocoDef)
		storeToJMRI(locoDef);

	if ((configData[workCfg].ProgMode == 0) && (thisIntfID != 17))
		if (confirm("Place locomotive on programming track and click OK") == false)
			return;

/*
	if (thisIntfID == 17)
	{
		if (validLocoDef)
			saveJMRIDecoder(null);
		else
			alert("Invalid Loco Definition. Please load JMRI file.");
	}
	else
*/
		initProg(true);
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
//			console.log(i, cv);
		}
		var attr = findXMLAttribute(varVals, "item", "Speed Table");
		if (attr)
			attr.setAttribute("value", varValStr);
	}
}

function cancelProg()
{
	cvArray = [];
	if (progDlg)
		progDlg.open = false;
}

function initProg(writeAll)
{
	if (progStatus > 0)
	{
		if (!progDlg)
		{
			progDlg = document.createElement("DIALOG");
			tempObj = createEmptyDiv(progDlg, "div", "tile-1", "");
			createDispText(tempObj, "tile-1", "Programming in Progress. Please Wait","", "dlgprogtext");
			tempObj = createEmptyDiv(progDlg, "div", "tile-1", "");
			createDispText(tempObj, "tile-1_4", "Write CV #:","00", "dlgcvnum");
			createDispText(tempObj, "tile-1_4", "with Value:","00","dlgcvval");
			tempObj = createEmptyDiv(progDlg, "div", "tile-1", "");
			createDispText(tempObj, "tile-1_4","Status:", "xx","dlgcvstat");
			createDispText(tempObj, "tile-1_4","Command:", "yy","dlgcvcmd");
	
			tempObj = createEmptyDiv(progDlg, "div", "tile-1", "");
			createButton(tempObj,  "", "Cancel", "btnSizeCancel", "cancelProg(this)");

			document.body.appendChild(progDlg);
		}
		progDlg.open = true;
		cvArray = [];
		//program CVs 29 bit4, 67-94, 66, 95
		//create CV Array
		addCV(66, fwTrim); //set table use
		addCV(95, bwTrim); //set table use
		if (writeAll == true)
		{
			for (var i = 0; i < 28; i++)
				addCV(i+67, speedTableProfileGraph.LineGraphs[0].DataElements[i+1].y);
			if (validLocoDef)
				addCV(29, findCVVal(locoDef, 29) | 0x10); //set table use
			else
				addCV(29, cvVal_29 | 0x10); //set table use
		}
		//write first CV 
		cvId = cvArray[0].x;
		cvVal = cvArray[0].y;
		if (writeCV(null))
		{
			writeTextField("dlgcvnum", cvId.toString());
			writeTextField("dlgcvval", cvVal.toString());
			writeTextField("dlgcvstat", "");
			writeTextField("dlgcvcmd", "Write CV");
			progFailCtr = 0;
			setTimeout(function(){progCVArray() }, (configData[workCfg].ProgMode > 0) ? 200 : 50);
		}
	}
	else
		alert("Programmer not available. Try again...");
}

/*
var progStatus = 1; //0: busy 1: done with success 2: done with error
var progRetCode = 0; //0: busy 
* 							1: accepted, wait for answer 
* 							2: timeout error 
* 							3: success 
* 							4: Prog track found empty
* 							5: No Ack Pulse
* 							6: Value not found
* 							7: Task aborted by user
* 							0x40: accepted blind 
* 							0x7F: task not implemented
var progLastCommand;
*/

function progCVArray()
{
//	console.log(progStatus);
	//write rest of array 
	if (configData[nodeCfg].InterfaceTypeList[configData[nodeCfg].InterfaceIndex].IntfID == 18)
		progStatus = 1;
	if (progStatus > 0)
	{
		if (progStatus == 1) 
		{
			writeTextField("dlgcvstat", "Success");
			cvArray.splice(0,1); //success, so remove first element
			if (cvArray.length > 0)
			{
				cvId = cvArray[0].x;
				cvVal = cvArray[0].y;
				progLastCommand = Date.now();
				if (!writeCV(null))
				{
					cancelProg();
					return;
				}
				writeTextField("dlgcvnum", cvId.toString());
				writeTextField("dlgcvval", cvVal.toString());
				writeTextField("dlgcvcmd", "Write CV");
				progFailCtr = 0;
//				console.log(cvId, cvVal);
			}
			else
			{
				cancelProg();
				return;
			}
		}
		else
		{
//			console.log(progRetCode);
			switch (progRetCode)
			{
				case 2: writeTextField("dlgcvstat", "Timeout error. Trying again..."); break;
				case 4: writeTextField("dlgcvstat", "Prog track found empty. Trying again..."); break;
				case 5: writeTextField("dlgcvstat", "No Ack Pulse received. Trying again..."); break;
				case 6: writeTextField("dlgcvstat", "Value not found. Trying again..."); break;
				case 7: writeTextField("dlgcvstat", "Task aborted by user. Terminating..."); cancelProg(); return;break;
			}
			progFailCtr++;
			if (progFailCtr > 3)
			{
				if (confirm("Programming failed. Keep trying?") == true)
					progFailCtr = 0;
				else
				{
//					console.log("Cancel");
					cancelProg();
					return;
				}
			}
		}
	}
	else
		if ((Date.now() - progLastCommand) > 5000)
		{
			progStatus = 2;
			progRetCode = 2;
//			writeTextField("dlgcvstat", "Timeout. Trying again...");
		}
	setTimeout(function(){progCVArray() }, (configData[workCfg].ProgMode > 0) ? 200 : 50);
}

function calcTable(sender)
{
	if (validTechSpeedDef)
	{
		var avgDevfw = 0; //avg deviation between forward and backward
		var avgDevbw = 0; //avg deviation between forward and backward
		var avgVal = [0,0,0];
		var maxVal = 0;
		speedTableProfileGraph.LineGraphs[0].DataElements = [];
		var newElement = JSON.parse(JSON.stringify(DataElementTemplate)); // = {"x":0,"y":0,"m":0};
		newElement.x = 0;
		newElement.y = 0;
		speedTableProfileGraph.LineGraphs[0].DataElements.push(newElement);
		for (var j = 1; j < 29; j++)
		{
			var targetSpeed = getSpeedForThrottleStep(j);
			var targetLevel = getMotorValueForSpeed(targetSpeed);
//			console.log(targetSpeed, techSpeedProfileGraph.LineGraphs[1].DataElements[j].y, techSpeedProfileGraph.LineGraphs[2].DataElements[j].y, targetLevel);
			avgVal[0] += targetLevel[0]; //
			avgVal[1] += targetLevel[1];
			avgVal[2] += Math.round((targetLevel[0] + targetLevel[1])/2); 
		}
		var topCurve = avgVal[0] < avgVal[1] ? 0 : 1; //higher target levels come from lower curve
		var botCurve = avgVal[0] < avgVal[1] ? 1 : 0;

//		console.log(avgVal, botCurve, topCurve);

		for (var j = 1; j < 29; j++)
		{
			var targetSpeed = getSpeedForThrottleStep(j);
			var targetLevel = getMotorValueForSpeed(targetSpeed);
			newElement = JSON.parse(JSON.stringify(DataElementTemplate)); // = {"x":0,"y":0,"m":0};
			newElement.x = j;
			switch (configData[workCfg].CalcMode)
			{
				case 0: //average of both curves
					newElement.y = Math.round((targetLevel[0] + targetLevel[1])/2); 
//					avgDevfw += (targetLevel[0] - targetLevel[1])/2; //add deviation between fw and bw for trim calculation
					avgDevfw += newElement.y - targetLevel[0]; //add deviation between fw and bw for trim calculation
					avgDevbw += newElement.y - targetLevel[1]; //add deviation between fw and bw for trim calculation
					break;
				case 1: //lower of both curves
					newElement.y = (botCurve == 0) ? targetLevel[0] : targetLevel[1];
//					avgDevfw += (targetLevel[topCurve] - targetLevel[botCurve]); //add deviation between fw and bw for trim calculation
					avgDevfw += (topCurve == 1 ? targetLevel[0] : targetLevel[1]) - newElement.y;
					avgDevbw += (topCurve == 0 ? targetLevel[0] : targetLevel[1]) - newElement.y;
					break;
				case 2: //higher of both curves
					newElement.y = (topCurve == 0) ? targetLevel[0] : targetLevel[1];
//					avgDevfw += (targetLevel[botCurve] - targetLevel[topCurve]); //add deviation between fw and bw for trim calculation
					avgDevfw += (botCurve == 0 ? targetLevel[0] : targetLevel[1]) - newElement.y;
					avgDevbw += (botCurve == 1 ? targetLevel[0] : targetLevel[1]) - newElement.y;
					break;
			}
			if (newElement.y > maxVal)
				maxVal = newElement.y;
//			console.log(targetLevel[0], targetLevel[1], newElement.y, targetLevel[0] - targetLevel[1], avgDev);
			speedTableProfileGraph.LineGraphs[0].DataElements.push(newElement);
		}
		avgDevfw = avgDevfw/28;
		avgDevbw = avgDevbw/28;
		avgVal[0] = avgVal[0]/28;
		avgVal[1] = avgVal[1]/28;
		avgVal[2] = avgVal[2]/28;

		console.log(avgDevfw, avgDevbw, avgVal);

		var trimRatio;
		switch (configData[workCfg].CalcMode)
		{
			case 0: //average of both curves
				trimRatio = ((Math.abs(avgDevfw) + avgVal[2]) / avgVal[2]);
				break;
			case 1: //lower of both curves
				trimRatio = (((botCurve == 1) ? avgDevfw : avgDevbw ) + avgVal[botCurve]) / avgVal[botCurve];
				break;
			case 2: //higher of both curves
				trimRatio = (((topCurve == 1) ? avgDevfw : avgDevbw ) + avgVal[topCurve]) / avgVal[topCurve];
				break;
		}
		
		console.log(configData[workCfg].CalcMode, configData[workCfg].TrimMode, trimReserve, trimRatio);

		var trimBase = 128;
		if (configData[workCfg].TrimMode == 1) //0-100%
		{
			if ((maxVal * trimRatio) <= 255)
				for (var j = 0; j < 29; j++)
					speedTableProfileGraph.LineGraphs[0].DataElements[j].y = Math.round(speedTableProfileGraph.LineGraphs[0].DataElements[j].y * (1 + (trimReserve/100))); //increase table values by % to make down trimmable
			trimBase = Math.round(255 /(1 + (trimReserve/100)));
		}
		else
			for (var j = 0; j < 29; j++)
				speedTableProfileGraph.LineGraphs[0].DataElements[j].y = Math.round(speedTableProfileGraph.LineGraphs[0].DataElements[j].y); //round to integer
		switch (configData[workCfg].CalcMode)
		{
			case 0: //average
				fwTrim = Math.round((topCurve == 0) ? trimBase / trimRatio : trimBase * trimRatio);
				bwTrim = Math.round((topCurve == 1) ? trimBase / trimRatio : trimBase * trimRatio);
				break;
			case 1: //lower curve
				bwTrim = Math.round((topCurve == 0) ? trimBase : trimBase * trimRatio);
				fwTrim = Math.round((topCurve == 1) ? trimBase : trimBase * trimRatio);
				break;
			case 2: //higher curve
				bwTrim = Math.round((topCurve == 0) ? trimBase * trimRatio : trimBase);
				fwTrim = Math.round((topCurve == 1) ? trimBase * trimRatio : trimBase);
				break;
		}
		fwTrim = Math.min(255, fwTrim);
		bwTrim = Math.min(255, bwTrim);
		
		writeInputField("fwtrim", fwTrim);
		writeInputField("bwtrim", bwTrim);
		writeInputField("fwtrimadj", fwTrim);
		writeInputField("bwtrimadj", bwTrim);
		drawProfileBox(canvasElementSpeedTable, speedTableProfileGraph);
		validTableDef = true;
	}
	if (validLocoDef)
		storeToJMRI(locoDef);
	setButtonStatus();
}

/*
var progStatus = 1; //0: busy 1: done with success 2: done with error
var progRetCode = 0; //0: busy 
* 							1: accepted, wait for answer 
* 							2: timeout error 
* 							3: success 
* 							4: Prog track found empty
* 							5: No Ack Pulse
* 							6: Value not found
* 							7: Task aborted by user
* 							0x40: accepted blind 
* 							0x7F: task not implemented
var progLastCommand;
*/

function processProgrammerInput(jsonData)
{
//	console.log(jsonData);
	var statMsg = "";
	switch (jsonData.Mode)
	{
		case 0x00: progStatus = 0; progRetCode = 0; statMsg = "Programmer busy"; break; //prog busy
		case 0x01: progStatus = 0; progRetCode = 1; statMsg = "Processing..."; break; //working on it
		case 0x40: progStatus = 1; progRetCode = 0x40; statMsg = "Task accepted, no reply"; break; //accepted blind
		case 0x7F: progStatus = 2; progRetCode = 0x7F; statMsg = "Function not implemented"; break; //not implemented
		case 0xFF: //Final prog status reply
		{
			switch (jsonData.Status)
			{
				case 0:
					progStatus = 1;
					progRetCode = 3;
					statMsg = "Success";
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
					if (jsonData.Status & 0x01)	statMsg += "Prog track found empty ";
					if (jsonData.Status & 0x01) progRetCode = 4;
					if (jsonData.Status & 0x02)	statMsg += "No Ack Pulse ";
					if (jsonData.Status & 0x02) progRetCode = 5;
					if (jsonData.Status & 0x04)	statMsg += "Value not found ";
					if (jsonData.Status & 0x04) progRetCode = 6;
					if (jsonData.Status & 0x08)	statMsg += "Task aborted by user";
					if (jsonData.Status & 0x08) progRetCode = 7;
					progStatus = 2;
					break;
			}
		}
	}
	writeTextField("progstat", statMsg);
}

function fillZeros(speedData)
{
	//step 1: replace zero values by data
	var lastElement = speedData.length-1;
	while (speedData[lastElement] == 0)
		lastElement--;
	var zeroPtr = lastElement;
	var curveEnd = lastElement;
	while (zeroPtr > 0)
	{
		zeroPtr--;
		if (speedData[zeroPtr] != 0)
			lastElement--;
		else
		{
			while ((zeroPtr > 0) && (speedData[zeroPtr] == 0))
				zeroPtr--;
			var dx = lastElement - zeroPtr;
			var dy = (speedData[lastElement] - speedData[zeroPtr])	/ dx;
			for (var i = 1; i < dx; i++)
				speedData[zeroPtr+ i] = speedData[zeroPtr] + (i * dy);
			lastElement = zeroPtr;
		}
	}	
	//step 2: make curve strictly monotone up to the end point
	for (var i = 1; i < speedData.length; i++)
		if (speedData[i] < speedData[i-1])
			speedData[i] = speedData[i-1];
	
	//step 3: linear smoothing
//	lastElement = speedData.length-1;
//	while (speedData[lastElement] == 0)
//		lastElement--;
	for (var i = 1; i < curveEnd; i++)
		speedData[i] = (speedData[i-1] + speedData[i+1])/2;
//	console.log(speedData);
	return speedData;
}

function processSpeedTableInput(jsonData, processCurve)
{
//	console.log(jsonData);

	if (processCurve)
	{
		jsonData.fw = fillZeros(jsonData.fw);
		jsonData.bw = fillZeros(jsonData.bw);
	}
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
	if (jsonData.final != undefined)
	{
		calcTable();
		speedProfileDef.GraphData = JSON.parse(JSON.stringify(jsonData));
	}
	else
		validTableDef = false;
	setButtonStatus();
	if ((jsonData.SlotNr > 0) && (!jsonData.final))
	{
		var speedVal = jsonData.CurrStep;
		switch (speedVal)
		{
			case 0 : speedVal = 1; break;
			case 1 : speedVal = 0; break;
		}
		writeTextField("dccstepsp", speedVal.toString() + " " + (jsonData.Dir? "FW" : "BW"));
	}
	else
		writeTextField("dccstepsp", "");
//	console.log(speedTableProfileGraph);
}

function processSensorInput(jsonData)
{
//	console.log(jsonData);
	var speedUpdate = jsonData.RR >= 500;
	
	if (!speedUpdate)
	{
		if (analysisBuffer.length < 20)
			analysisBuffer.push(jsonData.Speed);
		else
			analysisBuffer[analysisIndex] = jsonData.Speed;
		var tempIndex = analysisIndex;
		var valSum = 0;
		var val1Sec = 0;
		var val5Sec = 0;
		var val10Sec = 0;
		analysisIndex = (analysisIndex + 1) % 20; //5 seconds @ 250ms
		for (var i = 0; i < analysisBuffer.length; i++)
		{
			valSum += analysisBuffer[tempIndex];
			tempIndex = (tempIndex + 19) % 20;
			if (i == 0)
				val1Sec = valSum; //0.25sec
			if (i == 3)
				val5Sec = valSum / 4; //1sec
			if (i == 19)
				val10Sec = valSum / 20; //5sec
		}
//		console.log(analysisBuffer.length, valSum, val1Sec, val5Sec, val10Sec);
		addEntryToArray(lineGraphStab1, jsonData.TS, val1Sec, stabilityGraph.MaxXRange * 1000);
		addEntryToArray(lineGraphStab2, jsonData.TS, val5Sec, stabilityGraph.MaxXRange * 1000);
		addEntryToArray(lineGraphStab3, jsonData.TS, val10Sec, stabilityGraph.MaxXRange * 1000);
		drawStabilityGraphs();
//		console.log(throttleProfileGraph.SpeedGraph);
		var speedMultiplier = 0.0036 * configData[workCfg].ScaleList[configData[workCfg].ScaleIndex].Scale; //mm/s to km/h * scale
		if (configData[workCfg].Units == 1) //imperial
			speedMultiplier /= 1.6; //km/h to mph
		if (jsonData.SpeedStep != undefined)
		{
			if ((throttleProfileGraph.SpeedGraph.x != Math.max(0, (Math.round(jsonData.SpeedStep-1) / 128 * throttleDef.SpeedSteps))) || (throttleProfileGraph.SpeedGraph.evtCtr >= 20))
			{
				throttleProfileGraph.SpeedGraph.evtCtr = 0;
				throttleProfileGraph.SpeedGraph.Min1y = Math.abs(val1Sec * speedMultiplier);
				throttleProfileGraph.SpeedGraph.Max1y = Math.abs(val1Sec * speedMultiplier);
				throttleProfileGraph.SpeedGraph.Min2y = Math.abs(val5Sec * speedMultiplier);
				throttleProfileGraph.SpeedGraph.Max2y = Math.abs(val5Sec * speedMultiplier);
				throttleProfileGraph.SpeedGraph.Min3y = Math.abs(val10Sec * speedMultiplier);
				throttleProfileGraph.SpeedGraph.Max3y = Math.abs(val10Sec * speedMultiplier);
			}
			else
			{
				throttleProfileGraph.SpeedGraph.Min1y = Math.min(throttleProfileGraph.SpeedGraph.Min1y, Math.abs(val1Sec * speedMultiplier));
				throttleProfileGraph.SpeedGraph.Max1y = Math.max(throttleProfileGraph.SpeedGraph.Max1y, Math.abs(val1Sec * speedMultiplier));
				throttleProfileGraph.SpeedGraph.Min2y = Math.min(throttleProfileGraph.SpeedGraph.Min2y, Math.abs(val5Sec * speedMultiplier));
				throttleProfileGraph.SpeedGraph.Max2y = Math.max(throttleProfileGraph.SpeedGraph.Max2y, Math.abs(val5Sec * speedMultiplier));
				throttleProfileGraph.SpeedGraph.Min3y = Math.min(throttleProfileGraph.SpeedGraph.Min3y, Math.abs(val10Sec * speedMultiplier));
				throttleProfileGraph.SpeedGraph.Max3y = Math.max(throttleProfileGraph.SpeedGraph.Max3y, Math.abs(val10Sec * speedMultiplier));
				throttleProfileGraph.SpeedGraph.evtCtr++;
			}
			throttleProfileGraph.SpeedGraph.x = Math.max(0, (Math.round(jsonData.SpeedStep-1) / 128 * throttleDef.SpeedSteps));
			throttleProfileGraph.SpeedGraph.m = jsonData.SpeedStep > 1 ? 0 : 1;
			throttleProfileGraph.SpeedGraph.Last1y = Math.abs(val1Sec * speedMultiplier);
			throttleProfileGraph.SpeedGraph.Last2y = Math.abs(val5Sec * speedMultiplier);
			throttleProfileGraph.SpeedGraph.Last3y = Math.abs(val10Sec * speedMultiplier);
		}
		else
			throttleProfileGraph.SpeedGraph.m = 1;
//		console.log(throttleProfileGraph.SpeedGraph);
		drawProfileBox(canvasElementThrottle, throttleProfileGraph);

	}
	else
	{

		var currScale = configData[workCfg].ScaleList[configData[workCfg].ScaleIndex].Scale;
		var scaleSpeed = (jsonData.Speed  * 36 * currScale) / 10000; //[km/h]
		loadSpeedDispTable(speedDistTable, jsonData);
		loadElevPitchTable(elevPitchTable, jsonData);
/*
		var dirStr = jsonData.Speed > 0 ? " Forward" : jsonData.Speed < 0 ? " Backward" : "";
		writeTextField("speed", Math.abs(jsonData.Speed).toFixed(2) + dirStr);

		var currScale = configData[workCfg].ScaleList[configData[workCfg].ScaleIndex].Scale;
	
		var scaleSpeed = (jsonData.Speed  * 36 * currScale) / 10000; //[km/h]
		if (configData[workCfg].Units == 1) //imperial
			scaleSpeed /= 1.6; //mph
		writeTextField("scalespeed", Math.abs(scaleSpeed.toFixed(2)));
		writeTextField("absdist", configData[workCfg].Units == 0 ? (jsonData.AbsDist/10).toFixed(2) : (jsonData.AbsDist/25.4).toFixed(2));
		relDist = jsonData.RelDist;
		writeTextField("reldist", configData[workCfg].Units == 0 ? (jsonData.RelDist/10).toFixed(2) : (jsonData.RelDist/25.4).toFixed(2));

		var radiusVal = Math.min(5000, configData[workCfg].Units == 0 ? Math.abs(jsonData.Radius) : Math.abs(jsonData.Radius)/25.4);
		var radiusValGraph = radiusVal > 2000 ? 0 : radiusVal;
		var radiusSig = jsonData.Radius >= 0 ? 1 : -1; 
		var radiusStr = radiusVal.toFixed(2);
		if ((jsonData.Radius != 0) && (Math.abs(jsonData.Radius) < 5000))
			dirStr = radiusStr + ((radiusSig > 0) ? " right" : " left");
		else
			dirStr = " straight";

		writeTextField("radius", dirStr);
		writeTextField("heading", (180 * jsonData.EulerVect[0] / 3.1415).toFixed(2));
		if (jsonData.Banking)
			writeTextField("roll", jsonData.Banking.toFixed(1)); //(180 * jsonData.EulerVect[1] / 3.141).toFixed(2));
		else
			writeTextField("roll", "n/a"); //(180 * jsonData.EulerVect[1] / 3.141).toFixed(2));
		if (jsonData.Slope)
			writeTextField("pitch", jsonData.Slope.toFixed(1)); //(180 * jsonData.EulerVect[2] / 3.1415).toFixed(2));
		else
			writeTextField("pitch", "n/a"); //(180 * jsonData.EulerVect[2] / 3.1415).toFixed(2));

//		if (jsonData.EulerVect == [0, 0, 0, 0])
		{
			addEntryToArray(lineGraphGrade, jsonData.TS, jsonData.Slope, angleGraph.MaxXRange * 1000);
			addEntryToArray(lineGraphElevation, jsonData.TS, jsonData.Banking, angleGraph.MaxXRange * 1000);
		}
*/

/*
		else
		{
			addEntryToArray(lineGraphHeading, jsonData.TS , ((180 * jsonData.EulerVect[0]) / 3.1415), radiusGraph.MaxXRange * 1000);
			addEntryToArray(lineGraphGrade, jsonData.TS , ((180 * jsonData.EulerVect[2]) / 3.1415), angleGraph.MaxXRange * 1000);
			addEntryToArray(lineGraphElevation, jsonData.TS , ((180 * jsonData.EulerVect[1]) / 3.1415), angleGraph.MaxXRange * 1000);
			addEntryToArray(lineGraphRadius, jsonData.TS , radiusSig * radiusValGraph, radiusGraph.MaxXRange * 1000, radiusValGraph == 0);
		}
*/
		addEntryToArray(lineGraphTechSpeed, jsonData.TS , jsonData.Speed, speedGraph.MaxXRange * 1000);
		addEntryToArray(lineGraphScaleSpeed, jsonData.TS , scaleSpeed, speedGraph.MaxXRange * 1000);
		drawLineGraphs();
	}
	if (jsonData.DCCAddr != undefined)
	{
		writeTextField("dccaddr", jsonData.DCCAddr);
		writeTextField("dccaddrsp", jsonData.DCCAddr);
		writeTextField("dccaddrtbl", jsonData.DCCAddr);
		writeTextField("dccwheeladdr", jsonData.DCCAddr);
		
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
			writeTextField("dccwheelstep", speedStr + " " + dirStr);
//			writeTextField("dccstepsp", speedStr + " " + dirStr);
			addEntryToArray(lineGraphSpeedStep, jsonData.TS , ((jsonData.DirF & 0x20) > 0) ? jsonData.SpeedStep : -1 * jsonData.SpeedStep, speedGraph.MaxXRange * 1000);
		}
		else
		{
			writeTextField("dccstep", "n/a");
			writeTextField("dccwheelstep", "n/a");
//			writeTextField("dccstepsp", "n/a");
		}
		locoAddrValid = true;
	}
	else
	{
//		writeTextField("dccaddr", "n/a");
		writeTextField("dccstep", "n/a");
		writeTextField("dccwheelstep", "n/a");
//		writeTextField("dccstepsp", "n/a");
		locoAddr -1;
		locoAddrValid = false;
	}

	setButtonStatus();
}

