var DeviceData = {"lnAddress" : 0, "manufId" : 25, "devId":25, "prodId": 0, "serNr": 0,"LNMode" : 0, "SigType": 0, "SigState": 0, "PwrUpMode": 0, "PwrState": 0, "devActuators": [], "Boosters": []};

var HWTemplate = {"Module":0,"PwrPin":0,"AnalogPin":0,"SensorFactor":0.00,"ARPin":0,"SCCurrent":0};
var ELTemplate = {"NominalCurrent":0,"FuseMode":0,"ResMode":0,"ARMode":false};
var ActTemplate = {"MemLoc": 0, "CmdType":0, "Addr":0, "Status":0};
var RTTemplate = {"OLF":0.00, "RMS":0.00,"intS": 0, "extS":0, "arS":0};

var intStatText = ["Running", "Trying AR", "Trying AR", "Short Circuit", "Overload", "Cooling", "Await Reset"];
var extStatText = ["Stop", "Running", "CS Testing"];

var hwMainTab;
var configMainTab;
var BoosterTab;
var BoosterTable;
var runtimeMainTab;
var runtimeBoosterTab;
var runtimeBoosterTable;

var mainScrollBox;

var tabHardware;
var tabConfig;
var tabRuntime;

var updateCurrent = false;
var currTrack = 0;

//var trackGauges = [];
var trackGaugeDefs = [];

var ConfigOptionPanel;
var CurrentOptionPanel;
var TrackManPanel;
var devLoconetInputPanel;

var trackTable;

var trackTableDiv;
 
function upgradeJSONVersion(jsonData)
{
	return upgradeJSONVersionSilverHat(jsonData);
}

function saveConfigFileSettings()
{
	//step 1: save shcfg.cfg
	saveJSONConfig(scriptList.Pages[currentPage].ID, scriptList.Pages[currentPage].FileName, configData[workCfg], null);
}

function sendLNCommand(msgData)
{
	ws.send("{\"Cmd\":\"LNOut\", \"Data\":" + msgData + "}");
}

function sendSVCommand(sv_cmd, sv_addr, data)
{
//	console.log(data);
	var src = 0x01;
	var dst = DeviceData.lnAddress; 

	if (sv_cmd == 0x09) //change LN Address
	{
		dst = sv_addr;
//		sv_addr = 
	}

	var svx1 = 0x10 + ((sv_addr & 0x8000) >> 12) + ((sv_addr & 0x0080) >> 5) + ((dst & 0x8000) >> 14) + ((dst & 0x0080) >> 7);
	var svx2 = 0x10 + ((data[0] & 0x80) >> 7) + ((data[1] & 0x80) >> 6) + ((data[2] & 0x80) >> 5) + ((data[3] & 0x80) >> 4);
	SVCmd = JSON.stringify([0xE5, 0x10, src, sv_cmd, 0x02, svx1, (dst & 0x007F), (dst & 0x7F00) >> 8, (sv_addr & 0x007F), (sv_addr & 0x7F00) >> 8,  svx2, data[0] & 0x7F, data[1] & 0x7F, data[2] & 0x7F, data[3] & 0x7F]);
	ws.send("{\"Cmd\":\"BST\", \"SV\":" + SVCmd + "}");
}

function downloadSettings(sender)
{
	downloadConfig(0x1000); //send just this
}

function setPanelVisibility()
{
//	console.log(DeviceData);
	setVisibility(DeviceData.LNMode > 0, devLoconetInputPanel);
	setVisibility(DeviceData.Boosters.length > 0, dataTab);
	setVisibility(DeviceData.LNMode > 0, document.getElementById("layoutstatus"));
//	setVisibility((DeviceData.LNMode > 0) && ([4,5,6].indexOf(configData[nodeCfg].InterfaceIndex) >= 0), document.getElementById("outputstatus"));
	for (var i = 0; i < DeviceData.Boosters.length; i++)
	{
		setVisibility(DeviceData.LNMode > 0, document.getElementById("boosterconfig_inp_" + i.toString() + "_2"));
		setVisibility((DeviceData.LNMode > 0) && ([4,5,6].indexOf(configData[nodeCfg].InterfaceIndex) >= 0), document.getElementById("boosterconfig_inp_" + i.toString() + "_3"));
		setVisibility(DeviceData.Boosters[i].hw.ARPin > 3, document.getElementById("panel_bstARF_" + i.toString() + "_2"));
		setVisibility(DeviceData.Boosters[i].hw.ARPin > 3, document.getElementById("panel_bstARR_" + i.toString() + "_2"));
//		console.log(DeviceData.Boosters[i].hw);
	}
}

function createHWMainTab(parentObj)
{
	createPageTitle(hwMainTab, "div", "tile-1", "HWCfg_TitleMain", "h2", "Hardware Configuration");
	dataTab = createEmptyDiv(hwMainTab, "div", "tile-1", "");
	createDispText(dataTab, "tile-1_4", "Serial Number:", "n/a", "SerNum");
	createDropdownselector(dataTab, "tile-1_4", "# Modules:", ["0", "1", "2", "3", "4", "5", "6"], "numMods", "setNumMods(this)");
	dataTab = createEmptyDiv(hwMainTab, "div", "tile-1", "");
	createTextInput(dataTab, "tile-1_4", "LN Address:", "n/a", "LNAddr", "setLNAddr(this)");
	dataTab = createEmptyDiv(hwMainTab, "div", "tile-1", "");
	createDispText(dataTab, "tile-1_4", "Input Signal:", "", "RepInpSigHW");
	dataTab = createEmptyDiv(hwMainTab, "div", "tile-1", "");
	hwTable = createDataTable(dataTab, "tile-1", ["Pos","PWR Pin","Analog Pin", "Sensor Rate", "AR Pin","Short Test", "Short Current [mA]"], "hwconfig", "");
}

function createConfigMainTab(parentObj)
{
	var dataTab = createEmptyDiv(parentObj, "div", "tile-1", "");
	createPageTitle(dataTab, "div", "tile-1", "BasicCfg_TitleMain", "h2", "Booster Device Configuration");
	var dataTab = createEmptyDiv(parentObj, "div", "tile-1", "");
	createDropdownselector(dataTab, "tile-1_4", "Valid Signal Format:", ["DCC","PWM"], "SigSel", "setSignalType(this)");
	createDropdownselector(dataTab, "tile-1_4", "Power Up State:", ["Track Power OFF","Track Power ON"], "PwrUpState", "setPowerUpState(this)");
	var dataTab = createEmptyDiv(parentObj, "div", "tile-1", "");
	createCheckbox(dataTab, "tile-1_4", "Use Command Communication", "UseLN", "setLoconetComm(this)");

	devLoconetInputPanel = createEmptyDiv(parentObj, "div", "tile-1", "");

	var dataTab = createEmptyDiv(devLoconetInputPanel, "div", "tile-1", "");
	dataTab.style.height = "40px";
	var inpPanel = createEmptyDiv(dataTab, "div", "editortile", "");
	inpPanel.append(tfActuatorLine(0,5, "bstON_", "Booster ON:", "setBoosterConfigData(this)"));

	var dataTab = createEmptyDiv(devLoconetInputPanel, "div", "tile-1", "");
	dataTab.style.height = "40px";
	var inpPanel = createEmptyDiv(dataTab, "div", "editortile", "");
	inpPanel.append(tfActuatorLine(1,5, "bstOFF_", "Booster OFF:", "setBoosterConfigData(this)"));

	var dataTab = createEmptyDiv(parentObj, "div", "tile-1", "");
	createPageTitle(dataTab, "div", "tile-1", "BasicCfg_TitleMain", "h2", "Booster Module Configuration");
	var dataTab = createEmptyDiv(parentObj, "div", "tile-1", "");
	createDispText(dataTab, "tile-1_4", "# of Booster Boards:", "0", "NumBooster");

	BoosterTab = createEmptyDiv(parentObj, "div", "tile-1", "");
	BoosterTable = createDataTable(BoosterTab, "tile-1", ["Pos","Booster Module Settings", "Actuator Settings", "Reporting Settings"], "boosterconfig", "");

	
}

function createRuntimeMainTab(parentObj)
{
	var dataTab = createEmptyDiv(parentObj, "div", "tile-1", "");
	createPageTitle(dataTab, "div", "tile-1", "BasicCfg_TitleMain", "h2", "Device Status and Control");
	var dataTab = createEmptyDiv(parentObj, "div", "tile-1", "");
	createDispText(dataTab, "tile-1_4", "Input Signal:", "", "RepInpSig");
	var dataTab = createEmptyDiv(parentObj, "div", "tile-1", "layoutPwrStatus");
	dataTab.setAttribute("id","layoutstatus"); 
	createButton(dataTab, "", "CS Power ON", "btnGlobalPowerOn", "setPowerStatus(this, 0x83)");
	createButton(dataTab, "", "CS Power OFF", "btnGlobalPowerOff", "setPowerStatus(this, 0x82)");
	createButton(dataTab, "", "CS Power Idle", "btnGlobalPowerIdle", "setPowerStatus(this, 0x85)");
	var dataTab = createEmptyDiv(parentObj, "div", "tile-1", "");
	dataTab.setAttribute("id","outputstatus"); 
	createButton(dataTab, "", "All Output On", "btnBoosterPowerOn", "setBoosterStatus(this, 1)");
	createButton(dataTab, "", "All Output Off", "btnBoosterPowerOff", "setBoosterStatus(this, 0)");

	var dataTab = createEmptyDiv(parentObj, "div", "tile-1", "");
	createPageTitle(dataTab, "div", "tile-1", "BasicCfg_TitleMain", "h2", "Booster Module Status and Control");

	runtimeBoosterTab = createEmptyDiv(parentObj, "div", "tile-1", "");
	runtimeBoosterTable = createDataTable(runtimeBoosterTab, "tile-1", ["Pos","Track Current", "Status", "Output Control"], "boosterstatus", "");
}

function tfGauge(y, x, id, evtHandler)
{
	var subGaugePanel = document.createElement("div");
	subGaugePanel.setAttribute("class","gaugebox"); 
	var trackCanvas = document.createElement("CANVAS");
	trackCanvas.setAttribute("id",id); 
	subGaugePanel.append(trackCanvas);
	var maxVal = 1000 * (Math.trunc(1.35 * DeviceData.Boosters[y].el.NominalCurrent / 1000) + 1);
	var newTicks = [];
	var tickIntv = 1000;
	var numTicks = Math.round(maxVal / tickIntv) + 1;
	while (numTicks <= 4)
	{
		tickIntv /= 2;
		numTicks = Math.round(maxVal / tickIntv) + 1;
	}
	while (numTicks > 8)
	{
		tickIntv *= 2;
		numTicks = Math.round(maxVal / tickIntv) + 1;
	}
	for (var i = 0; i < numTicks; i++)
		newTicks.push(i * tickIntv);
	var trackGauge = new RadialGauge(
		{width: 200, 
		height: 200, 
		units: 'mA', 
		renderTo: trackCanvas,
		title: "Booster " + (y+1).toString(),
		value: 0,
		minValue: 0,
		maxValue: maxVal,
		majorTicks: newTicks,
 		minorTicks: [],
 		highlights: [
			{ "from": 0, "to": 0.8 * DeviceData.Boosters[y].el.NominalCurrent, "color": "rgba(0,255,0,.3)" },
			{ "from": 0.8 * DeviceData.Boosters[y].el.NominalCurrent, "to": 1.0 * DeviceData.Boosters[y].el.NominalCurrent, "color": "rgba(255,255,0,.3)" },
			{ "from": 1.0 * DeviceData.Boosters[y].el.NominalCurrent, "to": 1.2 * DeviceData.Boosters[y].el.NominalCurrent, "color": "rgba(255,0,0,.3)" },
			{ "from": 1.2 * DeviceData.Boosters[y].el.NominalCurrent, "to": 1.35 * DeviceData.Boosters[y].el.NominalCurrent, "color": "rgba(255,0,0,.8)" },
			{ "from": 1.35 * DeviceData.Boosters[y].el.NominalCurrent, "to": maxVal, "color": "rgba(0,0,0,.5)" }
		]
	}).draw();
	if (y >= trackGaugeDefs.length)
		trackGaugeDefs.push(trackGauge);
	else
		trackGaugeDefs[y] = trackGauge;
	return subGaugePanel;
}

function tfBoosterConfig(y, x, id, evtHandler)
{
	var divElement = document.createElement("div");
	divElement.setAttribute("class", "mastertile");
	tfSetCoordinate(divElement, y, x, 0, id);
	
	var upperDiv = document.createElement("div");
	upperDiv.setAttribute("class", "editortile");
	divElement.append(upperDiv);
	
	thisId = "maxcurrtext_" + y.toString() + "_" + x.toString();
	var thisText = tfText(y, x, thisId, evtHandler);
	thisText.innerHTML = "Recommended max. [mA]:";
	upperDiv.append(tfTab(y, x, '&nbsp;',""));
	upperDiv.append(thisText);
	upperDiv.append(tfTab(y, x, '&nbsp;',""));

	thisId = "maxcurrval_" + y.toString() + "_" + x.toString();
	var thisText = tfText(y, x, thisId, evtHandler);
	thisText.innerHTML = "n/a";
	upperDiv.append(tfTab(y, x, '&nbsp;',""));
	upperDiv.append(thisText);
	upperDiv.append(tfTab(y, x, '&nbsp;',""));
	upperDiv.append(tfTab(y, x, '&nbsp;',""));

	thisId = "shortcurrtext_" + y.toString() + "_" + x.toString();
	var thisText = tfText(y, x, thisId, evtHandler);
	thisText.innerHTML = "Short Current [mA]:";
	upperDiv.append(tfTab(y, x, '&nbsp;',""));
	upperDiv.append(thisText);
	upperDiv.append(tfTab(y, x, '&nbsp;',""));

	thisId = "shortcurrval_" + y.toString() + "_" + x.toString();
	var thisText = tfText(y, x, thisId, evtHandler);
	thisText.innerHTML = "n/a";
	upperDiv.append(tfTab(y, x, '&nbsp;',""));
	upperDiv.append(thisText);
	upperDiv.append(tfTab(y, x, '&nbsp;',""));
	upperDiv.append(tfTab(y, x, '&nbsp;',""));

	var upperDiv = document.createElement("div");
	upperDiv.setAttribute("class", "editortile");
	divElement.append(upperDiv);
	
	thisId = "nomcurrtext_" + y.toString() + "_" + x.toString();
	var thisText = tfText(y, x, thisId, evtHandler);
	thisText.innerHTML = "Nominal [mA]:";
	upperDiv.append(tfTab(y, x, '&nbsp;',""));
	upperDiv.append(thisText);
	upperDiv.append(tfTab(y, x, '&nbsp;',""));

	thisId = "nomcurrval_" + y.toString() + "_" + x.toString();
	var addrBox = tfNumeric(y, x, thisId, evtHandler);
	addrBox.setAttribute("index", 0);
	upperDiv.append(addrBox);
	upperDiv.append(tfTab(y, x, '&nbsp;',""));

	var lowerDiv = document.createElement("div");
	lowerDiv.setAttribute("class", "editortile");
	divElement.append(lowerDiv);

	thisId = "armode_" + y.toString() + "_" + x.toString();
	var cchBox = tfCheckBox(y, x, thisId, evtHandler);
	cchBox.childNodes[0].setAttribute("index", 1);
	cchBox.childNodes[1].innerHTML = "Enable Autoreverse Mode";
	lowerDiv.append(cchBox);
	lowerDiv.append(tfTab(y, x, '&nbsp;',""));

	thisId = "resettext_" + y.toString() + "_" + x.toString();
	var thisText = tfText(y, x, thisId, evtHandler);
	thisText.innerHTML = "Reset Mode:";
	lowerDiv.append(tfTab(y, x, '&nbsp;',""));
	lowerDiv.append(thisText);
	lowerDiv.append(tfTab(y, x, '&nbsp;',""));
	
	thisId = "resmod_" + y.toString() + "_" + x.toString();
	var resetSel = tfResetTypeSel(y,x, thisId, evtHandler);
	resetSel.setAttribute("index", 2);
	lowerDiv.append(resetSel);

	var lowerDiv = document.createElement("div");
	lowerDiv.setAttribute("class", "editortile");
	divElement.append(lowerDiv);

	thisId = "fusetext_" + y.toString() + "_" + x.toString();
	var thisText = tfText(y, x, thisId, evtHandler);
	thisText.innerHTML = "Fuse Mode:";
	lowerDiv.append(tfTab(y, x, '&nbsp;',""));
	lowerDiv.append(thisText);
	lowerDiv.append(tfTab(y, x, '&nbsp;',""));

	thisId = "cmdfusebox_" + y.toString() + "_" + x.toString();
	var selBox = tfFuseCtrlTypeSel(y, x, thisId, evtHandler);
	selBox.setAttribute("index", 3);
	lowerDiv.append(selBox);

	thisId = "custfusetext_" + y.toString() + "_" + x.toString();
	var thisText = tfText(y, x, thisId, evtHandler);
	thisText.innerHTML = "Value:";
	lowerDiv.append(tfTab(y, x, '&nbsp;',""));
	lowerDiv.append(thisText);
	lowerDiv.append(tfTab(y, x, '&nbsp;',""));

	thisId = "fusevalbox_" + y.toString() + "_" + x.toString();
	var addrBox = tfNumeric(y, x, thisId, evtHandler);
	addrBox.setAttribute("index", 4);
	lowerDiv.append(addrBox);

//	var lowerDiv = document.createElement("div");
//	lowerDiv.setAttribute("class", "editortile");
//	divElement.append(lowerDiv);

	return divElement;
}

function tfBoosterRuntime(y, x, id, evtHandler)
{
	var divElement = document.createElement("div");
	divElement.setAttribute("class", "mastertile");
	tfSetCoordinate(divElement, y, x, 0, id);

	var upperDiv = document.createElement("div");
	upperDiv.setAttribute("class", "editortile");
	divElement.append(upperDiv);

	var thisId = "trackCanvas_" + y.toString() + "_" + x.toString();
	var gaugeBox = tfGauge(y, x, thisId, evtHandler);
	gaugeBox.setAttribute("index", 0);
	upperDiv.append(gaugeBox);

	return divElement;
}

function tfBoosterStatus(y, x, id, evtHandler)
{
	var divElement = document.createElement("div");
	divElement.setAttribute("class", "mastertile");
	tfSetCoordinate(divElement, y, x, 0, id);
//---------------
	var upperDiv = document.createElement("div");
	upperDiv.setAttribute("class", "editortile");
	divElement.append(upperDiv);

	var textDiv = document.createElement("div");
	textDiv.setAttribute("class", "manipulatorbox");
	upperDiv.append(textDiv);

	thisId = "outstatetext_" + y.toString() + "_" + x.toString();
	var thisText = tfText(y, x, thisId, evtHandler);
	thisText.innerHTML = "Track Status:";
	textDiv.append(tfTab(y, x, '&nbsp;',""));
	textDiv.append(thisText);
	textDiv.append(tfTab(y, x, '&nbsp;',""));

	thisId = "outstateval_" + y.toString() + "_" + x.toString();
	var thisText = tfText(y, x, thisId, evtHandler);
	thisText.innerHTML = "DCC ON";
	upperDiv.append(tfTab(y, x, '&nbsp;',""));
	upperDiv.append(thisText);
	upperDiv.append(tfTab(y, x, '&nbsp;',""));
//---------------
	var upperDiv = document.createElement("div");
	upperDiv.setAttribute("class", "editortile");
	divElement.append(upperDiv);

	thisId = "intstatetext_" + y.toString() + "_" + x.toString();
	var thisText = tfText(y, x, thisId, evtHandler);
	thisText.innerHTML = "Booster Status:";
	upperDiv.append(tfTab(y, x, '&nbsp;',""));
	upperDiv.append(thisText);
	upperDiv.append(tfTab(y, x, '&nbsp;',""));

	thisId = "intstateval_" + y.toString() + "_" + x.toString();
	var thisText = tfText(y, x, thisId, evtHandler);
	thisText.innerHTML = "all OK";
	upperDiv.append(tfTab(y, x, '&nbsp;',""));
	upperDiv.append(thisText);
	upperDiv.append(tfTab(y, x, '&nbsp;',""));
//---------------
	var upperDiv = document.createElement("div");
	upperDiv.setAttribute("class", "editortile");
	divElement.append(upperDiv);

	thisId = "overloadtext_" + y.toString() + "_" + x.toString();
	var thisText = tfText(y, x, thisId, evtHandler);
	thisText.innerHTML = "Load Factor [%]:";
	upperDiv.append(tfTab(y, x, '&nbsp;',""));
	upperDiv.append(thisText);
	upperDiv.append(tfTab(y, x, '&nbsp;',""));

	thisId = "overloadval_" + y.toString() + "_" + x.toString();
	var thisText = tfText(y, x, thisId, evtHandler);
	thisText.innerHTML = "";
	upperDiv.append(tfTab(y, x, '&nbsp;',""));
	upperDiv.append(thisText);
	upperDiv.append(tfTab(y, x, '&nbsp;',""));

	thisId = "ardisppanel_" + y.toString() + "_" + x.toString();
	var upperDiv = document.createElement("div");
	upperDiv.setAttribute("class", "editortile");
	upperDiv.setAttribute("id", thisId);
	divElement.append(upperDiv);

	thisId = "arstatetext_" + y.toString() + "_" + x.toString();
	var thisText = tfText(y, x, thisId, evtHandler);
	thisText.innerHTML = "AR Status:";
	upperDiv.append(tfTab(y, x, '&nbsp;',""));
	upperDiv.append(thisText);
	upperDiv.append(tfTab(y, x, '&nbsp;',""));

	thisId = "arstateval_" + y.toString() + "_" + x.toString();
	var thisText = tfText(y, x, thisId, evtHandler);
	thisText.innerHTML = "Forward";
	upperDiv.append(tfTab(y, x, '&nbsp;',""));
	upperDiv.append(thisText);
	upperDiv.append(tfTab(y, x, '&nbsp;',""));

	return divElement;
}
	
function tfBoosterControl(y, x, id, evtHandler)
{
	var divElement = document.createElement("div");
	divElement.setAttribute("class", "mastertile");
	tfSetCoordinate(divElement, y, x, 0, id);
/*
	var upperDiv = document.createElement("div");
	upperDiv.setAttribute("class", "editortile");
	divElement.append(upperDiv);

	var textDiv = document.createElement("div");
	textDiv.setAttribute("class", "manipulatorbox");
	upperDiv.append(textDiv);
*/
	thisId = "btnPower_" + y.toString() + "_" + x.toString();
//	var thisBtn = tfBtnGen(y,x,thisId,evtHandler, "Toggle Power");

//	textDiv.append(tfTab(y, x, '&nbsp;',""));
	var thisBtn = createButton(divElement, "", "Toggle Power", thisId, evtHandler);
	tfSetCoordinate(thisBtn, y, x, 0, thisId);

//	textDiv.append(thisBtn);
//	textDiv.append(tfTab(y, x, '&nbsp;',""));
/*
	var upperDiv = document.createElement("div");
	upperDiv.setAttribute("class", "editortile");
	divElement.append(upperDiv);

	var textDiv = document.createElement("div");
	textDiv.setAttribute("class", "manipulatorbox");
	upperDiv.append(textDiv);
*/
	thisId = "btnReset_" + y.toString() + "_" + x.toString();
//	var thisText = tfText(y, x, thisId, evtHandler);
//	thisText.innerHTML = "Reset Booster";
//	textDiv.append(tfTab(y, x, '&nbsp;',""));
	var thisBtn = createButton(divElement, "", "Reset Booster", thisId, evtHandler);
	tfSetCoordinate(thisBtn, y, x, 1, thisId);
//	textDiv.append(thisText);
//	textDiv.append(tfTab(y, x, '&nbsp;',""));
/*
	var upperDiv = document.createElement("div");
	upperDiv.setAttribute("class", "editortile");
	divElement.append(upperDiv);

	var textDiv = document.createElement("div");
	textDiv.setAttribute("class", "manipulatorbox");
	upperDiv.append(textDiv);
*/
	thisId = "btnAR_" + y.toString() + "_" + x.toString();
//	var thisText = tfText(y, x, thisId, evtHandler);
//	thisText.innerHTML = "Toggle AR";
//	textDiv.append(tfTab(y, x, '&nbsp;',""));
	var thisBtn = createButton(divElement, "", "Toggle AR", thisId, evtHandler);
	tfSetCoordinate(thisBtn, y, x, 2, thisId);
//	textDiv.append(thisText);
//	textDiv.append(tfTab(y, x, '&nbsp;',""));

	return divElement;
}

function tfCmdInputSel(y, x, id, evtHandler)
{
	var selectList = document.createElement("select");
	if ([4,5,6].indexOf(configData[nodeCfg].InterfaceIndex) >= 0)
		createOptions(selectList, ["None", "Switch", "DCC Signal", "Button", "Block Detector"]);
	else
		createOptions(selectList, ["None", "Switch", "DCC Signal"]);
	tfSetCoordinate(selectList, y, x, 0, id);
	selectList.setAttribute("class", "selectorbox");
	selectList.setAttribute("onchange", evtHandler);
	return selectList;
}

function tfValTypeSel(y, x, id, evtHandler)
{
	var selectList = document.createElement("select");
	createOptions(selectList, ["thrown", "closed"]);
	tfSetCoordinate(selectList, y, x, 0, id);
	selectList.setAttribute("class", "selectorbox");
	selectList.setAttribute("onchange", evtHandler);
	return selectList;
}

function tfResetTypeSel(y, x, id, evtHandler)
{
	var selectList = document.createElement("select");
	createOptions(selectList, ["Manual Reset Only","Limited Auto Reset", "Auto Reset"]);
	tfSetCoordinate(selectList, y, x, 0, id);
	selectList.setAttribute("class", "selectorbox");
	selectList.setAttribute("onchange", evtHandler);
	return selectList;
}

function tfActuatorLine(y, x, id, label, evtHandler)
{
	var thisId = id + y.toString() + "_" + x.toString();

	var divElement = document.createElement("div");
	divElement.setAttribute("class", "editorpanel");
	divElement.setAttribute("id", "panel_" + thisId);
	
	var textDiv = document.createElement("div");
	textDiv.setAttribute("class", "manipulatorbox");
	divElement.append(textDiv);

	var thisText = tfText(y, x, thisId, evtHandler);
	tfSetCoordinate(thisText, y, x, 0, "lblCmd_" + thisId);
	thisText.innerHTML = label;
	textDiv.append(tfTab(y, x, '&nbsp;',""));
	textDiv.append(thisText);
	textDiv.append(tfTab(y, x, '&nbsp;',""));

	var inpEl = tfCmdInputSel(y, x,"Cmd_" + thisId, evtHandler)
	tfSetCoordinate(inpEl, y, x, 0, "Cmd_" + thisId);
	divElement.append(inpEl);
	
	var textDiv = document.createElement("div");
	textDiv.setAttribute("class", "manipulatorbox");
	divElement.append(textDiv);

	var thisText = tfText(y, x, thisId, evtHandler);
	tfSetCoordinate(thisText, y, x, 0, "lblAddr_" + thisId);
	thisText.innerHTML = "Address:";
	textDiv.append(tfTab(y, x, '&nbsp;',""));
	textDiv.append(thisText);
	textDiv.append(tfTab(y, x, '&nbsp;',""));

	var inpEl = tfNumeric(y,x, "Addr_" + thisId, evtHandler)
	tfSetCoordinate(inpEl, y, x, 1, "Addr_" + thisId);
	divElement.append(inpEl);
	
	var textDiv = document.createElement("div");
	textDiv.setAttribute("class", "manipulatorbox");
	divElement.append(textDiv);

	var thisText = tfText(y, x, thisId, evtHandler);
	tfSetCoordinate(thisText, y, x, 0, "lblVal_" + thisId);
	thisText.innerHTML = "Value:";
	textDiv.append(tfTab(y, x, '&nbsp;',""));
	textDiv.append(thisText);
	textDiv.append(tfTab(y, x, '&nbsp;',""));

	var inpEl = tfValTypeSel(y, x,"Val_" + thisId, evtHandler)
	tfSetCoordinate(inpEl, y, x, 2, "Val_" + thisId);
	divElement.append(inpEl);

	return divElement
}

function tfActuator(y, x, id, evtHandler)
{
	var divElement = document.createElement("div");
	divElement.setAttribute("class", "mastertile");
	tfSetCoordinate(divElement, y, x, 0, id);

	var upperDiv = document.createElement("div");
	upperDiv.setAttribute("class", "editortile");
	divElement.append(upperDiv);

	var boosterOn = tfActuatorLine(y,x,"bstON_", "Booster ON:", evtHandler);
	upperDiv.append(boosterOn);
	var boosterOff = tfActuatorLine(y,x,"bstOFF_", "Booster OFF:", evtHandler);
	upperDiv.append(boosterOff);
	var boosterRes = tfActuatorLine(y,x,"bstRES_", "Reset:", evtHandler);
	upperDiv.append(boosterRes);
	var boosterARF = tfActuatorLine(y,x,"bstARF_", "AR Forward:", evtHandler);
	upperDiv.append(boosterARF);
	var boosterARR = tfActuatorLine(y,x,"bstARR_", "AR Reverse:", evtHandler);
	upperDiv.append(boosterARR);

	return divElement;
}

function tfSensor(y, x, id, evtHandler)
{
	var divElement = document.createElement("div");
	divElement.setAttribute("class", "mastertile");
	tfSetCoordinate(divElement, y, x, 0, id);

	var upperDiv = document.createElement("div");
	upperDiv.setAttribute("class", "editortile");
	divElement.append(upperDiv);

	var boosterShort = tfActuatorLine(y,x,"bstShort_", "Short Circuit:", evtHandler);
	upperDiv.append(boosterShort);
	var boosterOverload = tfActuatorLine(y,x,"bstOverload_", "Overload:", evtHandler);
	upperDiv.append(boosterOverload);

	return divElement;
}

function updateActOptions(id, y, x)
{
	var e = document.getElementById("Cmd_" + id + "_" + y.toString() + "_" +x.toString());
//	console.log("Cmd_" + id + "_" + y.toString() + "_" +x.toString());
//	console.log(e);
	if ([4,5,6].indexOf(configData[nodeCfg].InterfaceIndex) >= 0)
		createOptions(e, ["None", "Switch", "DCC Signal", "Button", "Block Detector"]);
	else
		createOptions(e, ["None", "Switch", "DCC Signal"]);
}

function updateActorDisplay(actData, id, y, x)
{
//	console.log("Update ", actData);
	var e = document.getElementById("Cmd_" + id + "_" + y.toString() + "_" +x.toString());
//	console.log(actData.CmdType, e.selectedIndex);
//	if (e.selectedIndex != actData.CmdType)
	{
		e.selectedIndex = actData.CmdType;
		var dep = document.getElementById("Val_" + id + "_" + y.toString() + "_" +x.toString());
//		console.log(dep);
		switch (actData.CmdType)
		{
			case 0:
				createOptions(dep, ["None"]);
				actData.Status = 0;
				break;
			case 1:
				createOptions(dep, ["thrown", "closed"]);
				actData.Status = Math.min(actData.Status, 1);
				break;
			case 2:
				createOptions(dep, ["0", "1","2","3","4","5","6","7","8","9","10", "11","12","13","14","15","16","17","18","19","20", "21","22","23","24","25","26","27","28","29","30", "31"]);
				break;
			case 3:
				createOptions(dep, ["Btn Down", "Btn Up", "Btn Click", "Btn Dbl Click"]);
				actData.Status = Math.min(actData.Status, 2);
				break;
			case 4:
				createOptions(dep, ["free", "occupied"]);
				actData.Status = Math.min(actData.Status, 1);
				break;
		}
	}
	var e = document.getElementById("Addr_" + id + "_" + y.toString() + "_" +x.toString());
	e.value = actData.Addr;
	var e = document.getElementById("Val_" + id + "_" + y.toString() + "_" +x.toString());
	e.selectedIndex = actData.Status;
}

function loadConfigBoosterTable(thisTable, thisData)
{
//	console.log(thisData);
	var th = document.getElementById(thisTable.id + "_head");
	var tb = document.getElementById(thisTable.id + "_body");
	var numCols = th.childNodes[0].children.length;
	createDataTableLines(thisTable, [tfPos, tfBoosterConfig, tfActuator, tfSensor], thisData.length, "setBoosterConfigData(this)");
	for (var i=0; i<thisData.length;i++)
	{
//		console.log(thisData[i].hw.SCCurrent);
		var e = document.getElementById("maxcurrval_" + i.toString() + "_1");
		e.innerHTML = (Math.round(thisData[i].hw.SCCurrent * 0.007) * 100).toString();
		var e = document.getElementById("shortcurrval_" + i.toString() + "_1");
		e.innerHTML = thisData[i].hw.SCCurrent.toString();
		var e = document.getElementById("nomcurrval_" + i.toString() + "_1");
		e.value = thisData[i].el.NominalCurrent;


		var e = document.getElementById("armode_" + i.toString() + "_1");
		e.checked = thisData[i].el.ARMode;
		setVisibility(thisData[i].hw.ARPin > 1,document.getElementById("armode_" + i.toString() + "_1").parentElement);
		var e = document.getElementById("resmod_" + i.toString() + "_1");
		e.selectedIndex = thisData[i].el.ResMode;
		var e = document.getElementById("cmdfusebox_" + i.toString() + "_1");
		var indVal = (thisData[i].el.FuseMode / 20) -1 ;
		if ([0,1,2,3].indexOf(indVal) >= 0)
			e.selectedIndex = indVal;
		else
			e.selectedIndex = 4;
		setVisibility([0,1,2,3].indexOf(indVal) < 0, document.getElementById("custfusetext_" + i.toString() + "_1"));
		setVisibility([0,1,2,3].indexOf(indVal) < 0, document.getElementById("fusevalbox_" + i.toString() + "_1"));
		var e = document.getElementById("fusevalbox_" + i.toString() + "_1");
		e.value = thisData[i].el.FuseMode;

		updateActorDisplay(thisData[i].act[0], "bstON", i, 2);
		updateActorDisplay(thisData[i].act[1], "bstOFF", i, 2);
		updateActorDisplay(thisData[i].act[2], "bstRES", i, 2);
		updateActorDisplay(thisData[i].act[3], "bstARF", i, 2);
		updateActorDisplay(thisData[i].act[4], "bstARR", i, 2);

		updateActorDisplay(thisData[i].act[5], "bstShort", i, 3);
		updateActorDisplay(thisData[i].act[6], "bstOverload", i, 3);
	}
}

function loadRuntimeBoosterTable(redrawTable, thisTable, thisData)
{
	var th = document.getElementById(thisTable.id + "_head");
	var tb = document.getElementById(thisTable.id + "_body");
	var numCols = th.childNodes[0].children.length;
	if (redrawTable)
		createDataTableLines(thisTable, [tfPos, tfBoosterRuntime, tfBoosterStatus, tfBoosterControl], thisData.length, "setBoosterRuntimeData(this)");
	for (var i=0; i<thisData.length;i++)
	{
		var e = document.getElementById("overloadval_" + i.toString() + "_" + "2");
		e.innerHTML = DeviceData.Boosters[i].rt.OLF.toString();
		var e = document.getElementById("arstateval_" + i.toString() + "_" + "2");
		e.innerHTML = DeviceData.Boosters[i].rt.arS == 0? "Forward":"Reverse";
		var e = document.getElementById("outstateval_" + i.toString() + "_" + "2");
		e.innerHTML = extStatText[DeviceData.Boosters[i].rt.extS];
		var e = document.getElementById("intstateval_" + i.toString() + "_" + "2");
		e.innerHTML = intStatText[DeviceData.Boosters[i].rt.intS];

		if (trackGaugeDefs[i].value != DeviceData.Boosters[i].rt.RMS)
			trackGaugeDefs[i].value = DeviceData.Boosters[i].rt.RMS;
		var e = document.getElementById("ardisppanel_" + i.toString() + "_" + "2");
		setVisibility(DeviceData.Boosters[i].el.ARMode, e);
		var e = document.getElementById("btnAR_" + i.toString() + "_" + "3");
		setVisibility(DeviceData.Boosters[i].el.ARMode, e);
	}
}

function dispSigState()
{
	switch (DeviceData.SigState)
	{
		case 0: 
			document.getElementById("RepInpSig").innerHTML = "No Input"; 
			document.getElementById("RepInpSigHW").innerHTML = "No Input"; 
		break;
		case 1: 
			document.getElementById("RepInpSig").innerHTML = "DCC OK"; 
			document.getElementById("RepInpSigHW").innerHTML = "DCC OK"; 
		break;
		case 2: 
			document.getElementById("RepInpSig").innerHTML = "PWM OK"; 
			document.getElementById("RepInpSigHW").innerHTML = "PWM OK"; 
		break;
		case 3: 
			document.getElementById("RepInpSig").innerHTML = "Mixed"; 
			document.getElementById("RepInpSigHW").innerHTML = "Mixed"; 
		break;
	}
}
/*
function updateRuntimeBoosterTab(nodeNr, currData)
{
	if (currData.I)
	{
	}
/*
		writeTextField("Curr_" + nodeNr.toString(), jsonData.I[3]); //refresh slot update
		writeTextField("OLF_" + nodeNr.toString(), jsonData.I[1]); //refresh slot update
		switch(jsonData.I[5])
		{
			case 0: writeTextField("Stat_" + nodeNr.toString(), "Running"); break; //
			case 1: writeTextField("Stat_" + nodeNr.toString(), "AR Test"); break; //
			case 2: writeTextField("Stat_" + nodeNr.toString(), "AR Test"); break; //
			case 3: writeTextField("Stat_" + nodeNr.toString(), "Short Circuit"); break; //
			case 4: writeTextField("Stat_" + nodeNr.toString(), "Cooling"); break; //
			case 5: writeTextField("Stat_" + nodeNr.toString(), "Cooling"); break; //
			case 6: writeTextField("Stat_" + nodeNr.toString(), "Await Reset"); break; //
		}
		document.getElementById("Rev_" + nodeNr.toString()).checked = jsonData.I[7] == 0;

}
*/
function updateAllRuntimeBoosterTabs(currData)
{
//	for (var i = 0; i < HWConfig.length; i++)
//		createRuntimeBoosterTab(i, parentObj);
}

/*

SV Commands to device
- req manufacturer, developer, product, serial # (25, 25, 1, 99)
- set/req  number of modules, LN address default 25
[- set default modes for reset, fuse mode[
- set/req power on mode, accepted signal type, LN mode
- set/req  LN Actuator for Device On/Off type, state, addr

SV Commands to Booster
- set pins PWR, Analog, AR 3 nibbles pin nr, Sensor Rate 2 bytes (*100)
- set Mode use AR 1bit, Reset Mode 2bits, Fuse Value 1byte, nominal Current 2byte
- set LN Actuator On/Off type, state, addr
- set LN Actuator Reset type, state, addr
- set LN Actuator AR Polarity type, state, addr
- request Short Test 1 byte
- request config data

other Loconet commands to Booster
- Switch, SwitchACK, button, Signal, BD Input commands 
SV replies from Booster to Web
- auto report Status (Inp Sig, AR Dir, Res Stat, Ext Stat,
- auto report Current 2byte mA, OLF (*100) 2Byte
- report on request Config pins PWR, Analog, AR
- report on request Sensor Rate 2 bytes (*100)
- report on request Mode use AR 1bit, Reset Mode 1bit, Fuse Value 1byte, nominal Current 2byte
*/

function constructPageContent(contentTab)
{
	var menueStr = ["Hardware", "Options", "Monitor"];
	var tempObj;
	mainScrollBox = createEmptyDiv(contentTab, "div", "pagetopicboxscroll-y", "btnconfigdiv");

		var menueTab = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
		for (var i = 0; i < menueStr.length; i++)
			createMenueTabElement(menueTab, "button", "tablink", "SilverHatSub", "cbsetup_" + i.toString(), menueStr[i], true, "setPageMode(this)");
		updateMenueTabs("SilverHatSub", "cbsetup_0", "grey");

		tabHardware = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
			hwMainTab = createEmptyDiv(tabHardware, "div", "tile-1", "");
			createHWMainTab(hwMainTab);

		tabConfig = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
			configMainTab = createEmptyDiv(tabConfig, "div", "tile-1", "");
			createConfigMainTab(configMainTab);


		tabRuntime = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
			runtimeMainTab = createEmptyDiv(tabRuntime, "div", "tile-1", "");
			createRuntimeMainTab(runtimeMainTab);

	tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
		createButton(tempObj, "", "Save to EEPROM", "btnSave", "saveEEPROMSettings(this)");
	setPanelVisibility();
	setVisibility(false, tabConfig);
	setVisibility(false, tabRuntime);
}

function setPowerStatus (sender, opCode) //send Loconet GPON/OFF/IDLE
{
	sendLNCommand(JSON.stringify([opCode]));
}

function setBoosterStatus (sender, newState)
{
	sendSVCommand(0x01, 0xF1, [newState]);
}

function setSignalType(sender)
{
	DeviceData.SigType = sender.selectedIndex;
	sendSVCommand(0x01, 0x04, [((DeviceData.SigType & 0x0F)<<4) + (DeviceData.PwrUpMode & 0x0F)]);
}

function setPowerUpState(sender)
{
	DeviceData.PwrUpMode = sender.selectedIndex;
	sendSVCommand(0x01, 0x04, [((DeviceData.SigType & 0x0F)<<4) + (DeviceData.PwrUpMode & 0x0F)]);
}

/*
function setSerNum(sender)
{
	DeviceData.serNr = verifyNumber(sender.value, DeviceData.serNr);
	sendSVCommand(0x01, 0x04, [((DeviceData.SigType & 0x0F)<<4) + (DeviceData.PwrUpMode & 0x0F)]);
	
}
*/
function setLNAddr(sender)
{
	DeviceData.lnAddress = verifyNumber(sender.value, DeviceData.lnAddress);
	sendSVCommand(0x09, DeviceData.lnAddress, [DeviceData.manufId, DeviceData.devId, DeviceData.prodId, DeviceData.serNr]);
}

function setHWConfig(sender)
{
//	console.log(sender);
	var thisRow = parseInt(sender.getAttribute("row"));
	var thisCol = parseInt(sender.getAttribute("col"));
	var thisIndex = parseInt(sender.getAttribute("index"));
//	console.log(thisRow, thisCol, thisIndex);
	switch (thisCol)
	{
		case 1: //PWR Pin
//			switch (thisIndex)
			{
				DeviceData.Boosters[thisRow].hw.PwrPin = Math.max(verifyNumber(sender.value, DeviceData.Boosters[thisRow].hw.PwrPin), 0);
				console.log(DeviceData.Boosters[thisRow]);
				sendSVCommand(0x05, ((thisRow+1)<<8) + 0x00, [(DeviceData.Boosters[thisRow].hw.PwrPin << 4) + (DeviceData.Boosters[thisRow].hw.AnalogPin & 0x0F), DeviceData.Boosters[thisRow].hw.ARPin & 0x0F,(DeviceData.Boosters[thisRow].hw.SensorFactor & 0xFF00)>>8,DeviceData.Boosters[thisRow].hw.SensorFactor & 0x00FF]);
			}
			break;
		case 2: //Analog Pin
//			switch (thisIndex)
			{
				DeviceData.Boosters[thisRow].hw.AnalogPin = Math.max(verifyNumber(sender.value, DeviceData.Boosters[thisRow].hw.AnalogPin), 0);
				console.log(DeviceData.Boosters[thisRow]);
				sendSVCommand(0x05, ((thisRow+1)<<8) + 0x00, [(DeviceData.Boosters[thisRow].hw.PwrPin << 4) + (DeviceData.Boosters[thisRow].hw.AnalogPin & 0x0F), DeviceData.Boosters[thisRow].hw.ARPin & 0x0F,(DeviceData.Boosters[thisRow].hw.SensorFactor & 0xFF00)>>8,DeviceData.Boosters[thisRow].hw.SensorFactor & 0x00FF]);
			}
			break;
		case 3: //Sensor rate
//			switch (thisIndex)
			{
				var inpVal = sender.value*100;
				console.log(inpVal);
				DeviceData.Boosters[thisRow].hw.SensorFactor = verifyNumber(inpVal, DeviceData.Boosters[thisRow].hw.SensorFactor);
				console.log(DeviceData.Boosters[thisRow]);
				sendSVCommand(0x05, ((thisRow+1)<<8) + 0x00, [(DeviceData.Boosters[thisRow].hw.PwrPin << 4) + (DeviceData.Boosters[thisRow].hw.AnalogPin & 0x0F), DeviceData.Boosters[thisRow].hw.ARPin & 0x0F,(DeviceData.Boosters[thisRow].hw.SensorFactor & 0xFF00)>>8,DeviceData.Boosters[thisRow].hw.SensorFactor & 0x00FF]);
			}
			break;
		case 4: //AR Pin
//			switch (thisIndex)
			{
				DeviceData.Boosters[thisRow].hw.ARPin = Math.max(verifyNumber(sender.value, DeviceData.Boosters[thisRow].hw.ARPin), 0);
				console.log(DeviceData.Boosters[thisRow]);
				sendSVCommand(0x05, ((thisRow+1)<<8) + 0x00, [(DeviceData.Boosters[thisRow].hw.PwrPin << 4) + (DeviceData.Boosters[thisRow].hw.AnalogPin & 0x0F), DeviceData.Boosters[thisRow].hw.ARPin & 0x0F,(DeviceData.Boosters[thisRow].hw.SensorFactor & 0xFF00)>>8,DeviceData.Boosters[thisRow].hw.SensorFactor & 0x00FF]);
			}
			break;
		case 5: //Short Button
			{	
				sendSVCommand(0x01, ((thisRow+1) << 8) + 0xF0, [0x00]);
				if (confirm("Bridge the rails and click OK to run short circuit test"))
				{
					sendSVCommand(0x01, ((thisRow+1) << 8) + 0xF3, [0xFF]); //trigger short circuit test
				}
			}
			break;
	}
}

function setBoosterConfigData(sender)
{
//	console.log(sender);
	var thisRow = parseInt(sender.getAttribute("row"));
	var thisCol = parseInt(sender.getAttribute("col"));
	var thisIndex = parseInt(sender.getAttribute("index"));
//	console.log(thisRow, thisCol, thisIndex);
	switch (thisCol)
	{
		case 1: //module setting
			switch (thisIndex)
			{
				case 0: //nominal current
					var newNomCurr = verifyNumber(sender.value, DeviceData.Boosters[thisRow].el.NominalCurrent);
					if (newNomCurr != DeviceData.Boosters[thisRow].el.NominalCurrent)
						sendSVCommand(0x05, ((thisRow+1)<<8) + 0x04, [(DeviceData.Boosters[thisRow].el.ARMode << 2) + (DeviceData.Boosters[thisRow].el.ResMode & 0x03), DeviceData.Boosters[thisRow].el.FuseMode,(newNomCurr & 0xFF00)>>8, newNomCurr & 0x00FF]);
					break;
				case 1: //AR mode
					DeviceData.Boosters[thisRow].el.ARMode = sender.checked;
					sendSVCommand(0x05, ((thisRow+1)<<8) + 0x04, [(DeviceData.Boosters[thisRow].el.ARMode << 2) + (DeviceData.Boosters[thisRow].el.ResMode & 0x03), DeviceData.Boosters[thisRow].el.FuseMode,(DeviceData.Boosters[thisRow].el.NominalCurrent & 0xFF00)>>8,DeviceData.Boosters[thisRow].el.NominalCurrent & 0x00FF]);
					break;
				case 2: //Reset Mode
					DeviceData.Boosters[thisRow].el.ResMode = sender.selectedIndex;
					sendSVCommand(0x05, ((thisRow+1)<<8) + 0x04, [(DeviceData.Boosters[thisRow].el.ARMode << 2) + (DeviceData.Boosters[thisRow].el.ResMode & 0x03), DeviceData.Boosters[thisRow].el.FuseMode,(DeviceData.Boosters[thisRow].el.NominalCurrent & 0xFF00)>>8,DeviceData.Boosters[thisRow].el.NominalCurrent & 0x00FF]);
					break;
				case 3: //Fuse standard
					DeviceData.Boosters[thisRow].el.FuseMode = (sender.selectedIndex+1) * 20;
					sendSVCommand(0x05, ((thisRow+1)<<8) + 0x04, [(DeviceData.Boosters[thisRow].el.ARMode << 2) + (DeviceData.Boosters[thisRow].el.ResMode & 0x03), DeviceData.Boosters[thisRow].el.FuseMode,(DeviceData.Boosters[thisRow].el.NominalCurrent & 0xFF00)>>8,DeviceData.Boosters[thisRow].el.NominalCurrent & 0x00FF]);
					setVisibility(sender.selectedIndex == 4, document.getElementById("custfusetext_" + thisRow.toString() + "_1"));
					setVisibility(sender.selectedIndex == 4, document.getElementById("fusevalbox_" + thisRow.toString() + "_1"));
					break;
				case 4: //Fuse Custom
					DeviceData.Boosters[thisRow].el.FuseMode = verifyNumber(parseInt(sender.value/10) * 10, DeviceData.Boosters[thisRow].el.FuseMode);
					sendSVCommand(0x05, ((thisRow+1)<<8) + 0x04, [(DeviceData.Boosters[thisRow].el.ARMode << 2) + (DeviceData.Boosters[thisRow].el.ResMode & 0x03), DeviceData.Boosters[thisRow].el.FuseMode,(DeviceData.Boosters[thisRow].el.NominalCurrent & 0xFF00)>>8,DeviceData.Boosters[thisRow].el.NominalCurrent & 0x00FF]);
					break;
			}
			break;
		case 2: //actuator settings
		{
			var cmdIndex = -1;
			if (sender.id.search("_bstON_") >= 0)
				cmdIndex = 0;
			else
				if (sender.id.search("_bstOFF_") >= 0)
					cmdIndex = 1;
				else
					if (sender.id.search("_bstRES_") >= 0)
						cmdIndex = 2;
					else
						if (sender.id.search("_bstARF_") >= 0)
							cmdIndex = 3;
						else
							if (sender.id.search("_bstARR_") >= 0)
								cmdIndex = 4;
			if (cmdIndex < 0) return;
			switch (thisIndex)
			{
				case 0: //cmd type select
					DeviceData.Boosters[thisRow].act[cmdIndex].CmdType = sender.selectedIndex;
					break;
				case 1: //address
					DeviceData.Boosters[thisRow].act[cmdIndex].Addr = verifyNumber(sender.value, DeviceData.Boosters[thisRow].act[cmdIndex].Addr);
					break;
				case 2: //command type select
					DeviceData.Boosters[thisRow].act[cmdIndex].Status = sender.selectedIndex;
					break;
			}
			var trigData = ((DeviceData.Boosters[thisRow].act[cmdIndex].CmdType & 0x07) << 5) + (DeviceData.Boosters[thisRow].act[cmdIndex].Status & 0x1F);
			sendSVCommand(0x05, ((thisRow+1)<<8) + 0x08 + (4 * cmdIndex), [trigData, DeviceData.Boosters[thisRow].act[cmdIndex].Addr >> 8, DeviceData.Boosters[thisRow].act[cmdIndex].Addr & 0x00FF, 0]);
			console.log(DeviceData.Boosters[thisRow].act[cmdIndex], trigData);
			break;
		}
		case 3: //report settings
			var cmdIndex = -1;
			if (sender.id.search("_bstShort_") >= 0)
				cmdIndex = 0;
			else
				if (sender.id.search("_bstOverload_") >= 0)
					cmdIndex = 1;
			if (cmdIndex < 0) return;
			cmdIndex += 5;
			switch (thisIndex)
			{
				case 0: //cmd type select
					DeviceData.Boosters[thisRow].act[cmdIndex].CmdType = sender.selectedIndex;
					break;
				case 1: //address
					DeviceData.Boosters[thisRow].act[cmdIndex].Addr = verifyNumber(sender.value, DeviceData.Boosters[thisRow].act[cmdIndex].Addr);
					break;
				case 2: //command type select
					DeviceData.Boosters[thisRow].act[cmdIndex].Status = sender.selectedIndex;
					break;
			}
			var trigData = ((DeviceData.Boosters[thisRow].act[cmdIndex].CmdType & 0x07) << 5) + (DeviceData.Boosters[thisRow].act[cmdIndex].Status & 0x1F);
			sendSVCommand(0x05, ((thisRow+1)<<8) + 0x08 + (4 * cmdIndex), [trigData, DeviceData.Boosters[thisRow].act[cmdIndex].Addr >> 8, DeviceData.Boosters[thisRow].act[cmdIndex].Addr & 0x00FF, 0]);
			console.log(DeviceData.Boosters[thisRow].act[cmdIndex], trigData);
			break;
		case 5: //device level
			switch (thisIndex)
			{
				case 0: //cmd type select
					DeviceData.devActuators[thisRow].CmdType = sender.selectedIndex;
					break;
				case 1: //address
					DeviceData.devActuators[thisRow].Addr = verifyNumber(sender.value, DeviceData.devActuators[thisRow].Addr);
					break;
				case 2: //command type select
					DeviceData.devActuators[thisRow].Status = sender.selectedIndex;
					break;
			}
			var trigData = ((DeviceData.devActuators[thisRow].CmdType & 0x07) << 5) + (DeviceData.devActuators[thisRow].Status & 0x1F);
			sendSVCommand(0x05, 0x08 + (4 * thisRow), [trigData, DeviceData.devActuators[thisRow].Addr >> 8, DeviceData.devActuators[thisRow].Addr & 0x00FF, 0]);
//			console.log(DeviceData.devActuators[thisRow], trigData);
			break;
	}
}

function setBoosterRuntimeData(sender)
{
	var thisRow = parseInt(sender.getAttribute("row"));
	var thisCol = parseInt(sender.getAttribute("col"));
	var thisIndex = parseInt(sender.getAttribute("index"));
//	console.log(thisRow, thisCol, thisIndex);
	switch (thisCol)
	{
		case 3: //the only column with input
		{
			switch (thisIndex)
			{
				case 0: sendSVCommand(0x01, ((thisRow+1) << 8) + 0xF0, [0xFF]);
				break;
				case 1: sendSVCommand(0x01, ((thisRow+1) << 8) + 0xF2, [0x00]);
				break;
				case 2: sendSVCommand(0x01, ((thisRow+1) << 8) + 0xF1, [0xFF]);
				break;
			}
		}
		break;
	}
}

/*
function requestCurrent(sender)
{
	try 
	{
		ws.send("{\"Cmd\":\"SetDCCPP\", \"SubCmd\":\"GetCurrent\",\"OpCode\": " + currTrack + "}");
	}
	catch  (error) 
	{
		console.error(error);
	}
	currTrack = Math.abs(currTrack - 1);
	if (updateCurrent)
		setTimeout(function(){requestCurrent(null)}, 500);
}
*/

function setNumMods(sender)
{
	var numMods = sender.selectedIndex;
	sendSVCommand(0x05,0x00,[numMods,DeviceData.LNMode,DeviceData.lnAddress >> 8, DeviceData.lnAddress & 0x00FF]); //Set number of booster modules. Booster will reply with full configuration settings
}

function setLoconetComm(sender)
{
	DeviceData.LNMode = sender.checked;
	sendSVCommand(0x05,0x00,[DeviceData.Boosters.length,DeviceData.LNMode,DeviceData.lnAddress >> 8, DeviceData.lnAddress & 0x00FF]); //Set Loconet/DCC communication
	setPanelVisibility();
}

function saveEEPROMSettings(sender)
{
	sendSVCommand(0x01,0xFF,[0]); //Write EEPROM
}

/*

function loadGaugeData(gaugeNr)
{
	if (gaugeNr < 0)
		gaugeNr = document.getElementById("currgaugeselector").selectedIndex;
	if (gaugeNr >= 0)
	{
		document.getElementById("showgauge").checked = configData[workCfg].CurrentTracker[gaugeNr].ShowGauge;
		document.getElementById("currgaugename").value = configData[workCfg].CurrentTracker[gaugeNr].GaugeName;
		document.getElementById("currinppin").value = configData[workCfg].CurrentTracker[gaugeNr].PinNr;
		document.getElementById("currbuffsize").value = configData[workCfg].CurrentTracker[gaugeNr].SampleSize;
		document.getElementById("currmultiplier").value = configData[workCfg].CurrentTracker[gaugeNr].Multiplier;
		document.getElementById("curroffset").value = configData[workCfg].CurrentTracker[gaugeNr].Offset;
		document.getElementById("currmaxval").value = configData[workCfg].CurrentTracker[gaugeNr].MaxVal;
		document.getElementById("majorticks").value = configData[workCfg].CurrentTracker[gaugeNr].MainTicks;
	}
}

function setCurrentOptions(sender)
{
	var activeTrack = document.getElementById("currgaugeselector").selectedIndex;
	if (activeTrack >= 0)
	{
		if (sender.id == "currgaugeselector")
			loadGaugeData(sender.selectedIndex);
		if (sender.id == "showgauge")
			configData[workCfg].CurrentTracker[activeTrack].ShowGauge = sender.checked;
		if (sender.id == "currgaugename")
			configData[workCfg].CurrentTracker[activeTrack].GaugeName = sender.value; 
		if (sender.id == "currinppin")
			configData[workCfg].CurrentTracker[activeTrack].PinNr = verifyNumber(sender.value, configData[workCfg].CurrentTracker[activeTrack].PinNr); 
		if (sender.id == "currbuffsize")
			configData[workCfg].CurrentTracker[activeTrack].SampleSize = verifyNumber(sender.value, configData[workCfg].CurrentTracker[activeTrack].SampleSize); 
		if (sender.id == "currmultiplier")
			configData[workCfg].CurrentTracker[activeTrack].Multiplier = verifyNumber(sender.value, configData[workCfg].CurrentTracker[activeTrack].Multiplier); 
		if (sender.id == "curroffset")
			configData[workCfg].CurrentTracker[activeTrack].Offset = verifyNumber(sender.value, configData[workCfg].CurrentTracker[activeTrack].Offset); 
		if (sender.id == "currmaxval")
			configData[workCfg].CurrentTracker[activeTrack].MaxVal = verifyNumber(sender.value, configData[workCfg].CurrentTracker[activeTrack].MaxVal); 
		if (sender.id == "majorticks")
		{
			var newArray = verifyNumArray(sender.value, ",");
			if (newArray.length > 0)
			{
				configData[workCfg].CurrentTracker[activeTrack].MainTicks = []; //make sure this is an array
				for (var i = 0; i < newArray.length; i++)
					configData[workCfg].CurrentTracker[activeTrack].MainTicks.push(newArray[i]);
			}
		}
	}
}
*/
function setPageMode(sender)
{
	setVisibility(false, tabHardware);
	setVisibility(false, tabConfig);
	setVisibility(false, tabRuntime);
//	setVisibility(false, tabGauges);
//	for (var i = 0; i < trackGauges.length; i++)
//		setVisibility(false, trackGauges[i]);
	updateCurrent = false;
	switch (sender.id)
	{
		case "cbsetup_0":
//			writeRBInputField("cbsetup", 0);
			setVisibility(true, tabHardware);
			break;
		case "cbsetup_1":
//			writeRBInputField("cbsetup", 0);
			setVisibility(true, tabConfig);
			break;
		case "cbsetup_2":
//			writeRBInputField("cbsetup", 0);
			setVisibility(true, tabRuntime);
			break;
/*
		case "cbsetup_2":
//			console.log(configData[workCfg].CurrentTracker);
			if (trackGauges.length > 0)
			{
				updateCurrent = true;
				setVisibility(true, tabGauges);
//				console.log(trackGauges.length);
//				for (var i = 0; i < trackGauges.length; i++)
//				{
//					setVisibility(configData[workCfg].CurrentTracker[i].ShowGauge, trackGauges[i]);
//				}
//				setTimeout(function(){requestCurrent(sender) }, 1000);
			}
			break;
*/
	}
	updateMenueTabs("SilverHatSub", sender.id, "grey");
}

function loadHWTable(thisTable, thisData, clrConf)
{
	var th = document.getElementById(thisTable.id + "_head");
	var tb = document.getElementById(thisTable.id + "_body");
	var numCols = th.childNodes[0].children.length;

	createDataTableLines(thisTable, [tfPos, tfNumeric, tfNumeric, tfNumeric, tfNumeric, tfBtnMeasure, tfText], thisData.Boosters.length, "setHWConfig(this)");	

	for (var i=0; i < thisData.Boosters.length;i++)
	{
		var e = document.getElementById(thisTable.id + "_" + i.toString() + "_" + "1");
		e.childNodes[0].value = thisData.Boosters[i].hw.PwrPin;
		var e = document.getElementById(thisTable.id + "_" + i.toString() + "_" + "2");
		e.childNodes[0].value = thisData.Boosters[i].hw.AnalogPin;
		var e = document.getElementById(thisTable.id + "_" + i.toString() + "_" + "3");
		e.childNodes[0].value = thisData.Boosters[i].hw.SensorFactor/100;
		var e = document.getElementById(thisTable.id + "_" + i.toString() + "_" + "4");
		e.childNodes[0].value = thisData.Boosters[i].hw.ARPin;
		var e = document.getElementById(thisTable.id + "_" + i.toString() + "_" + "6");
		e.childNodes[0].innerHTML = thisData.Boosters[i].hw.SCCurrent > 0 ? thisData.Boosters[i].hw.SCCurrent.toString() : "Click \"Measure\"";

	}

}

function loadNodeDataFields(jsonData)
{
//	console.log(jsonData);
	switch (jsonData.InterfaceIndex)
	{
		case 1:; //DCC
		case 2:  //DCC MQTT
//		  updateActOptions(0, "bstON", 0, 2);
//		  updateActOptions(0, "bstOFF", 0, 2);
		  updateActOptions("bstON", 0, 5);
		  updateActOptions("bstOFF", 1, 5);
		break;
		case 4:; //Loconet interface
		case 5:; //Loconet TCP
		case 6:  //Loconet MQTT
//		  updateActOptions(1, "bstON", 0, 2);
//		  updateActOptions(1, "bstOFF", 0, 2);
		  updateActOptions("bstON", 0, 5);
		  updateActOptions("bstOFF", 1, 5);
		break;
	}
}

function requestAllData()
{
	if (DeviceData.lnAddress != 0)
	{
		sendSVCommand(0x02,0x00FF,[0,0,0,0]); //request all device config data)
		sendSVCommand(0x02,0xFFFF,[0,0,0,0]); //request all boosterconfig data)
	}
	else
		setTimeout(requestAllData, 500);
}

function loadDataFields(jsonData)
{
	configData[workCfg] = upgradeJSONVersion(jsonData);
	sendSVCommand(0x07,0,[0,0,0,0]); //send 1 to addr 0 to trigger identification to get LN number	
	requestAllData();
	setPanelVisibility();
}

function processConfigDataInput(jsonData)
{
//	console.log(jsonData);
	if ((jsonData.Prm.Addr >> 8) == 0)
	{
		processConfigDataDevice(jsonData);
	}
	else
	{
		processConfigDataBooster(jsonData);
	}
	loadHWTable(hwTable, DeviceData, false);
}


//var DeviceData = {"lnAddress" : 0, "serNr": 0, "LNMode" : 0, "SigType": 0, "SigState": 0, "PwrUpMode": 0, "PwrState": 0, "devActuators": [], "Boosters": []};
//var HWTemplate = {"Module":0,"PwrPin":0,"AnalogPin":0,"SensorFactor":0.00,"ARPin":0,"SCCurrent":0};
//var ELTemplate = {"NominalCurrent":0,"FuseMode":0,"ResMode":0,"ARMode":false};
//var ActTemplate = {"MemLoc": 0, "Type":0, "Addr":0, "Status":0};
//var RTTemplate = {"OLF":0.00, "RMS":0.00,"intS": 0, "extS":0, "arS":0};

function setActuator(modNr, actPos, actType, actAddr, actState)
{
//	console.log(modNr, actPos, actType, actAddr, actState);
	if (modNr >= 0)
	{
		DeviceData.Boosters[modNr].act[actPos].CmdType = actType;
		DeviceData.Boosters[modNr].act[actPos].Addr = actAddr;
		DeviceData.Boosters[modNr].act[actPos].Status = actState;
		loadConfigBoosterTable(BoosterTable, DeviceData.Boosters);
	}
	else
	{
		DeviceData.devActuators[actPos].CmdType = actType;
		DeviceData.devActuators[actPos].Addr = actAddr;
		DeviceData.devActuators[actPos].Status = actState;
		updateActorDisplay(DeviceData.devActuators[0], "bstON", 0, 5);
		updateActorDisplay(DeviceData.devActuators[1], "bstOFF", 1, 5);
	}
}

function processConfigDataBooster(jsonData)
{
	var modNr = (jsonData.Prm.Addr >> 8) - 1;
	var memLoc = jsonData.Prm.Addr & 0x00FF;
	switch (jsonData.Prm.Opc)
	{
		case 0x46: //Report 4 bytes of data
		{
			switch (memLoc)
			{
				case 0x00: //Pins, Sensor rate
//					console.log(jsonData);
					DeviceData.Boosters[modNr].hw.PwrPin = jsonData.Prm.Vals[0] >> 4;
					DeviceData.Boosters[modNr].hw.AnalogPin = jsonData.Prm.Vals[0] & 0x0F;
					DeviceData.Boosters[modNr].hw.ARPin = jsonData.Prm.Vals[1] & 0x0F;
					DeviceData.Boosters[modNr].hw.SensorFactor = ((jsonData.Prm.Vals[2] << 8) + jsonData.Prm.Vals[3]);
					loadHWTable(hwTable, DeviceData, false);
//					console.log(Boosters[modNr]);
					break;
				case 0x04: //Reset & AR Mode, Nominal Current
					DeviceData.Boosters[modNr].el.ResMode = jsonData.Prm.Vals[0] & 0x03;
					DeviceData.Boosters[modNr].el.ARMode = (jsonData.Prm.Vals[0] & 0x0C) >> 2;
					DeviceData.Boosters[modNr].el.FuseMode = jsonData.Prm.Vals[1];
					var oldNomVal = DeviceData.Boosters[modNr].el.NominalCurrent;
					DeviceData.Boosters[modNr].el.NominalCurrent = (jsonData.Prm.Vals[2] << 8) + jsonData.Prm.Vals[3];
					loadConfigBoosterTable(BoosterTable, DeviceData.Boosters);
					if (oldNomVal != DeviceData.Boosters[modNr].el.NominalCurrent)
						loadRuntimeBoosterTable(true, runtimeBoosterTable, DeviceData.Boosters);
					break;
				case 0x08: //LN Actuator ON
					setActuator(modNr, 0, (jsonData.Prm.Vals[0] >> 5), (jsonData.Prm.Vals[1] << 8) + jsonData.Prm.Vals[2], jsonData.Prm.Vals[0] & 0x1F);
					loadConfigBoosterTable(BoosterTable, DeviceData.Boosters);
					break;
				case 0x0C: //LN Actuator OFF
					setActuator(modNr, 1, (jsonData.Prm.Vals[0] >> 5), (jsonData.Prm.Vals[1] << 8) + jsonData.Prm.Vals[2], jsonData.Prm.Vals[0] & 0x1F);
					loadConfigBoosterTable(BoosterTable, DeviceData.Boosters);
					break;
				case 0x10: //LN Actuator Reset
					setActuator(modNr, 2, (jsonData.Prm.Vals[0] >> 5), (jsonData.Prm.Vals[1] << 8) + jsonData.Prm.Vals[2], jsonData.Prm.Vals[0] & 0x1F);
					loadConfigBoosterTable(BoosterTable, DeviceData.Boosters);
					break;
				case 0x14: //LN Actuator AR Pos
					setActuator(modNr, 3, (jsonData.Prm.Vals[0] >> 5), (jsonData.Prm.Vals[1] << 8) + jsonData.Prm.Vals[2], jsonData.Prm.Vals[0] & 0x1F);
					loadConfigBoosterTable(BoosterTable, DeviceData.Boosters);
					break;
				case 0x18: //LN Actuator AR Neg
					setActuator(modNr, 4, (jsonData.Prm.Vals[0] >> 5), (jsonData.Prm.Vals[1] << 8) + jsonData.Prm.Vals[2], jsonData.Prm.Vals[0] & 0x1F);
					loadConfigBoosterTable(BoosterTable, DeviceData.Boosters);
					break;
				case 0x1C: //Short circuit report
					setActuator(modNr, 5, (jsonData.Prm.Vals[0] >> 5), (jsonData.Prm.Vals[1] << 8) + jsonData.Prm.Vals[2], jsonData.Prm.Vals[0] & 0x1F);
					loadConfigBoosterTable(BoosterTable, DeviceData.Boosters);
					break;
				case 0x20: //Overload report
					setActuator(modNr, 6, (jsonData.Prm.Vals[0] >> 5), (jsonData.Prm.Vals[1] << 8) + jsonData.Prm.Vals[2], jsonData.Prm.Vals[0] & 0x1F);
					loadConfigBoosterTable(BoosterTable, DeviceData.Boosters);
					break;
				case 0x24: //Short Circuit current report
					DeviceData.Boosters[modNr].hw.SCCurrent = ((jsonData.Prm.Vals[0] << 8) + jsonData.Prm.Vals[1]);
					loadHWTable(hwTable, DeviceData, false);
					loadConfigBoosterTable(BoosterTable, DeviceData.Boosters);
//					console.log(DeviceData.Boosters[modNr].hw.SCCurrent);
					break;
			}
		}
		break;
	}
	setPanelVisibility();
}

function processConfigDataDevice(jsonData)
{
	switch (jsonData.Prm.Opc)
	{
		case 0x42: //Report 1 byte of data
			switch (jsonData.Prm.Addr)
			{
				case 4: //power on mode, signal type mode
					DeviceData.SigType = (jsonData.Prm.Vals[0] >> 4) & 0x0F;
					DeviceData.PwrUpMode = jsonData.Prm.Vals[0] & 0x0F;
					setDropdownValue("SigSel", DeviceData.SigType);
					setDropdownValue("PwrUpState", DeviceData.PwrUpMode);
				break;
				case 0xF0: //signal State
					DeviceData.SigState = jsonData.Prm.Vals[0];
					dispSigState();
				break;
			}
			break;
		case 0x46: //Report 4 bytes of data
			switch (jsonData.Prm.Addr)
			{
				case 0: //#of modules, LN Mode, Address
					DeviceData.LNMode = jsonData.Prm.Vals[1];
					DeviceData.lnAddress = (jsonData.Prm.Vals[2] << 8) + jsonData.Prm.Vals[3];
					setDropdownValue("numMods", jsonData.Prm.Vals[0]);
					document.getElementById("NumBooster").innerHTML = jsonData.Prm.Vals[0].toString();
					writeCBInputField("UseLN", DeviceData.LNMode);
					for (var j = 0; j < 2; j++)
						if (j >= DeviceData.devActuators.length)
						{
							var newAct = JSON.parse(JSON.stringify(ActTemplate));
							newAct.MemLoc = 0x08 + (j * 4);
							DeviceData.devActuators.push(newAct);
						}
					var numBoosters = jsonData.Prm.Vals[0];
//					console.log(numBoosters, DeviceData.Boosters.length);
					while (DeviceData.Boosters.length > numBoosters)
						DeviceData.Boosters.pop();
					for (var i = 0; i < numBoosters; i++)
					{
						if (i >= DeviceData.Boosters.length)
						{
							var newBooster = {};
							newBooster.hw = JSON.parse(JSON.stringify(HWTemplate));
							newBooster.el = JSON.parse(JSON.stringify(ELTemplate));
							newBooster.rt = JSON.parse(JSON.stringify(RTTemplate));
							newBooster.act = [];
							for (var j = 0; j < 7; j++) //5 actors, 2 analog reports
							{
								var newAct = JSON.parse(JSON.stringify(ActTemplate));
								newAct.MemLoc = 0x08 + (j * 4);
								newBooster.act.push(newAct);
							}
							newBooster.hw.Module = i;
							DeviceData.Boosters.push(newBooster);
						}
						else
							DeviceData.Boosters[i].hw.Module = i;
					}
					loadConfigBoosterTable(BoosterTable, DeviceData.Boosters);
					loadRuntimeBoosterTable(true, runtimeBoosterTable, DeviceData.Boosters);
				break;
				case 0x08: //LN Actuator Global ON
					setActuator(-1, 0, (jsonData.Prm.Vals[0] >> 5), (jsonData.Prm.Vals[1] << 8) + jsonData.Prm.Vals[2], jsonData.Prm.Vals[0] & 0x1F);
//					console.log(devActuators);
					break;
				case 0x0C: //LN Actuator Global OFF
					setActuator(-1, 1, (jsonData.Prm.Vals[0] >> 5), (jsonData.Prm.Vals[1] << 8) + jsonData.Prm.Vals[2], jsonData.Prm.Vals[0] & 0x1F);
//					console.log(devActuators);
					break;
			}
			break;
		case 0x47: //Discover
			if ((jsonData.Prm.Vals[0] == 25) && (jsonData.Prm.Vals[2] == 1)) //Booster discovered, update LN Address
			{
				DeviceData.lnAddress = jsonData.Prm.Src;
				DeviceData.manufId = jsonData.Prm.Vals[0];
				DeviceData.devId = jsonData.Prm.Vals[1];
				DeviceData.prodId = jsonData.Prm.Vals[2];
				DeviceData.serNr = jsonData.Prm.Vals[3];
				writeTextField("SerNum", DeviceData.serNr); //Serial number
				writeInputField("LNAddr", DeviceData.lnAddress); //lnAddress
			}
			break;
	}
	setPanelVisibility();
}

function processTrackDataInput(jsonData)
{
//	console.log(jsonData);
	if (jsonData.I)
	{
		var nodeNr = parseInt(jsonData.I[0]);
		if (DeviceData.Boosters[nodeNr] != undefined)
		{
			DeviceData.Boosters[nodeNr].rt.OLF = jsonData.I[1];
			DeviceData.Boosters[nodeNr].rt.RMS = jsonData.I[3];
			DeviceData.Boosters[nodeNr].rt.intS = jsonData.I[5];
			DeviceData.Boosters[nodeNr].rt.extS = jsonData.I[6];
			DeviceData.Boosters[nodeNr].rt.arS = jsonData.I[7];
			loadRuntimeBoosterTable(false, runtimeBoosterTable, DeviceData.Boosters);
		}
	}
}

