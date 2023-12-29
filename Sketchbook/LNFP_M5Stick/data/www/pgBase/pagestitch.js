function upgradeJSONVersionNode(jsonData)
{
//	var InterfaceList = [{"Name":"none","IntfId":0,"Type":0,"ReqSTA":0},{"Name":"DCC Interface","IntfId":1,"Type":0,"ReqSTA":0},{"Name":"DCC to MQTT","IntfId":9,"Type":0,"ReqSTA":1},{"Name":"DCC from MQTT","IntfId":10,"Type":0,"ReqSTA":1},{"Name":"LocoNet Interface","IntfId":2,"Type":1,"ReqSTA":0},{"Name":"LocoNet Loopback","IntfId":16,"Type":1,"ReqSTA":0},{"Name":"LocoNet over MQTT","IntfId":3,"Type":1,"ReqSTA":1},{"Name":"LocoNet MQTT Gateway","IntfId":4,"Type":2,"ReqSTA":1},{"Name":"LocoNet lbServer","IntfId":11,"Type":2,"ReqSTA":0},{"Name":"LocoNet lbServer / MQTT Gateway","IntfId":13,"Type":2,"ReqSTA":1},{"Name":"LocoNet Loopback lbServer","IntfId":14,"Type":2,"ReqSTA":0},{"Name":"LocoNet Loopback lbServer / MQTT Gateway","IntfId":15,"Type":2,"ReqSTA":1},{"Name":"LocoNet lbServer Client","IntfId":12,"Type":2,"ReqSTA":1},{"Name":"MQTT with Topics","IntfId":8,"Type":3,"ReqSTA":1}];
//	var HatList = [{"Name":"none","HatId":0,"Type":0,"InterfaceList":[0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16]},{"Name":"Blue Hat","HatId":1,"Type":0,"InterfaceList":[0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16]},{"Name":"Brown Hat","HatId":2,"Type":0,"InterfaceList":[0,2,3,4]},{"Name":"Yellow Hat","HatId":3,"Type":0,"InterfaceList":[0,1,2,3,4,5,6,7,8,11,12,13,14,15,16]},{"Name":"Green Hat","HatId":4,"Type":0,"InterfaceList":[0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16]},{"Name": "Red Hat++","HatId": 6,"Type": 0,"InterfaceList": [2, 3, 4, 11, 13]},{"Name":"Black Hat","HatId":5,"Type":0,"InterfaceList":[0,2,3,4,5,6,7,8]}];

//	var InterfaceList = [{"Name":"none","IntfId":0,"Type":0,"ReqSTA":0},{"Name":"DCC Interface","IntfId":1,"Type":0,"ReqSTA":0},{"Name":"DCC to MQTT","IntfId":9,"Type":0,"ReqSTA":1},{"Name":"DCC from MQTT","IntfId":10,"Type":0,"ReqSTA":1},{"Name":"LocoNet Interface","IntfId":2,"Type":1,"ReqSTA":0},{"Name":"LocoNet Loopback","IntfId":16,"Type":1,"ReqSTA":0},{"Name":"LocoNet over MQTT","IntfId":3,"Type":1,"ReqSTA":1},{"Name":"LocoNet MQTT Gateway","IntfId":4,"Type":2,"ReqSTA":1},{"Name":"LocoNet lbServer","IntfId":11,"Type":2,"ReqSTA":0},{"Name":"LocoNet lbServer / MQTT Gateway","IntfId":13,"Type":2,"ReqSTA":1},{"Name":"LocoNet Loopback lbServer","IntfId":14,"Type":2,"ReqSTA":0},{"Name":"LocoNet Loopback lbServer / MQTT Gateway","IntfId":15,"Type":2,"ReqSTA":1},{"Name":"LocoNet lbServer Client","IntfId":12,"Type":2,"ReqSTA":1},{"Name":"WiThrottle Client","IntfId":17,"Type":2,"ReqSTA":1},{"Name":"MQTT with Topics","IntfId":8,"Type":3,"ReqSTA":1}];
//	var HatList = [{"Name":"none","HatId":0,"Type":0,"InterfaceList":[0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16]},{"Name":"Blue Hat","HatId":1,"Type":0,"InterfaceList":[0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16]},{"Name":"Brown Hat","HatId":2,"Type":0,"InterfaceList":[0,2,3,4]},{"Name":"Yellow Hat","HatId":3,"Type":0,"InterfaceList":[0,1,2,3,4,5,6,7,8,11,12,13,14,15,16]},{"Name":"Green Hat","HatId":4,"Type":0,"InterfaceList":[0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16]},{"Name":"Red Hat++ CS","HatId":6,"Type":0,"InterfaceList":[2,4,11,13]},{"Name":"Red Hat++ B","HatId":8,"Type":0,"InterfaceList":[2,4,11,13]},{"Name":"Black Hat","HatId":5,"Type":0,"InterfaceList":[0,2,3,4,5,6,7,8]},{"Name":"Purple Hat","HatId":7,"Type":0,"InterfaceList":[0,3,12,16,17]}]

var InterfaceList = [{"Name": "none",	"IntfId": 0,"Type": 0,"ReqSTA": 0}, {"Name": "DCC Interface","IntfId": 1,"Type": 0,"ReqSTA": 0}, {"Name": "DCC from MQTT","IntfId": 10,"Type": 0,"ReqSTA": 1}, {"Name": "LocoNet Loopback","IntfId": 16,"Type": 1,"ReqSTA": 0}, {"Name": "LocoNet Interface","IntfId": 2,"Type": 1,"ReqSTA": 0}, {"Name": "LocoNet lbServer Client","IntfId": 12,"Type": 2,"ReqSTA": 1}, {"Name": "LocoNet from MQTT","IntfId": 3,"Type": 1,"ReqSTA": 1}, {"Name": "WiThrottle Client","IntfId": 17,"Type": 2,"ReqSTA": 1}, {"Name": "WiThrottle Client (DCC EX)", "IntfId": 18, "Type": 2, "ReqSTA": 1}, {"Name": "MQTT with Topics","IntfId": 8,"Type": 3,"ReqSTA": 1}];
//1.6.5 var HatList = [{"Name": "none","HatId": 0,"Type": 0,"InterfaceList": [0, 1, 2, 3, 8, 10, 12, 16, 17, 18]}, {"Name": "Blue Hat","HatId": 1,"Type": 0,"InterfaceList": [0, 1, 2, 3, 8, 10, 12, 16]}, {"Name": "Brown Hat","HatId": 2,"Type": 0,"InterfaceList": [0, 2, 3, 12]}, {"Name": "Yellow Hat","HatId": 3,"Type": 0,"InterfaceList": [0, 1, 2, 3, 8, 10, 12, 16]}, {"Name": "Green Hat","HatId": 4,"Type": 0,"InterfaceList": [0, 1, 2, 3, 8, 10, 12, 16]}, {"Name": "Red Hat Shield","HatId": 6,"Type": 0,"InterfaceList": [2, 16]}, {"Name": "Black Hat","HatId": 5,"Type": 0,"InterfaceList": [0, 2, 3, 12, 16]}, {"Name": "Purple Hat","HatId": 7,"Type": 0,"InterfaceList": [0, 3, 12, 16, 17, 18]}];
var HatList = [{"Name": "none","HatId": 0,"Type": 0,"InterfaceList": [0, 1, 2, 3, 8, 10, 12, 16, 17, 18]}, {"Name": "Blue Hat","HatId": 1,"Type": 0,"InterfaceList": [0, 1, 2, 3, 8, 10, 12, 16]}, {"Name": "Brown Hat","HatId": 2,"Type": 0,"InterfaceList": [0, 2, 3, 12]}, {"Name": "Yellow Hat","HatId": 3,"Type": 0,"InterfaceList": [0, 1, 2, 3, 8, 10, 12, 16]}, {"Name": "Green Hat","HatId": 4,"Type": 0,"InterfaceList": [0, 1, 2, 3, 8, 10, 12, 16]}, {"Name": "Red Hat Shield","HatId": 6,"Type": 0,"InterfaceList": [2, 16]}, {"Name": "Black Hat","HatId": 5,"Type": 0,"InterfaceList": [0, 2, 3, 12, 16]}, {"Name": "Purple Hat","HatId": 7,"Type": 0,"InterfaceList": [0, 3, 12, 16, 17, 18]},{"Name": "Silver Hat","HatId": 8,"Type": 0,"InterfaceList": [0,1,2,3,10,12]}];
//var	ALMList = [{"Name": "Event Handler","ALMId": 0,"InterfaceList": [2, 3, 12, 16],"Type": 1}];
var	ALMList = [{"Name": "Event Handler","ALMId": 0,"InterfaceList": [2, 3, 12, 16],"Type": 1},{"Name": "SecEl Handler","ALMId": 1,"InterfaceList": [],"Type": 1},{"Name": "Voice Watcher","ALMId": 2,"InterfaceList": [],"Type": 1}];
var	ServerList = [{"Name": "MQTT Broker Gateway","ServerId": 0,"InterfaceList": [1, 2, 3, 10, 12, 16],"Type": 1},{"Name": "Loconet lbServer","ServerId": 1,"InterfaceList": [2, 3, 12, 16],"Type": 1},{"Name": "WiThrottle Server","ServerId": 2,"InterfaceList": [2, 3, 12, 16],"Type": 1},{"Name": "Loconet Subnet","ServerId": 3,"InterfaceList": [3, 12],"Type": 1}];

/* Available command sources: 
 * 0: none
 * 1: DCC Interface
 * 2: Loconet interface
 * 3: Loconet from MQTT
 * 8: MQTT with topics
 * 10: DCC from MQTT
 * 12: lbServer client
 * 16: Loconet Loopback
 * 17: WiThrottle client
 * 
*/
	jsonFileVersion = "1.0.4";
	var thisVersion = jsonData.Version;
//	console.log(thisVersion);
	if (thisVersion == jsonFileVersion)
		return jsonData;
	if (thisVersion == undefined)
//	if ((isNaN(thisVersion)) || (thisVersion = undefined))
	{
		//upgrade from noversion to 1.0.0
		console.log("upgrade Node Config from noversion to 1.0.0");
		jsonData.Version = "1.0.0";
		jsonData.InterfaceTypeList = JSON.parse(JSON.stringify(InterfaceList));
		jsonData.InterfaceIndex = 0;
		jsonData.HatTypeList = JSON.parse(JSON.stringify(HatList));
		jsonData.HatIndex = 0;
	}

	if (jsonData.Version == "1.0.0")
	{
		jsonData.ALMTypeList = JSON.parse(JSON.stringify(ALMList));
		jsonData.HatTypeList = JSON.parse(JSON.stringify(HatList));
		jsonData.Version = "1.0.1";
		console.log("upgrade Node Config from version 1.0.0 to 1.0.1");
	}
	if (jsonData.Version == "1.0.1")
	{
		jsonData.useLissy = 0;
		jsonData.ALMTypeList = JSON.parse(JSON.stringify(ALMList));
		jsonData.HatTypeList = JSON.parse(JSON.stringify(HatList));
		jsonData.Version = "1.0.2";
		console.log("upgrade Node Config from version 1.0.1 to 1.0.2");
	}
	if (jsonData.Version == "1.0.2")
	{
		jsonData.InterfaceTypeList = JSON.parse(JSON.stringify(InterfaceList));
		jsonData.HatTypeList = JSON.parse(JSON.stringify(HatList));
		jsonData.ALMTypeList = JSON.parse(JSON.stringify(ALMList));
		jsonData.ServerTypeList = JSON.parse(JSON.stringify(ServerList));
		jsonData.Version = "1.0.3";
		console.log("upgrade Node Config from version 1.0.2 to 1.0.3");
	}
	if (jsonData.Version == "1.0.3")
	{
		jsonData.InterfaceTypeList = JSON.parse(JSON.stringify(InterfaceList));
		jsonData.HatTypeList = JSON.parse(JSON.stringify(HatList));
		jsonData.ALMTypeList = JSON.parse(JSON.stringify(ALMList));
		jsonData.ServerTypeList = JSON.parse(JSON.stringify(ServerList));
		jsonData.Version = "1.0.4";
		console.log("upgrade Node Config from version 1.0.3 to 1.0.4");
	}
	return jsonData;
}

function upgradeJSONVersionSilverHat(jsonData)
{
	jsonFileVersion = "1.0";
	var thisVersion = jsonData.Version;
//	console.log(thisVersion);
	if (thisVersion == jsonFileVersion)
		return jsonData;
//	if (jsonData.Version == "1.0.0")
	return jsonData;
}

function upgradeJSONVersionRedHat(jsonData)
{
	var DefaultTracker = {"MaxVal": 6000, "Multiplier": 1, "MainTicks": [ 0, 1000, 2000, 3000, 4000, 5000, 6000], "SampleSize": 50, "ShowGauge": false};
	jsonFileVersion = "1.0.3";
	var thisVersion = jsonData.Version;
//	console.log(thisVersion);
	if (thisVersion == jsonFileVersion)
		return jsonData;
	if (thisVersion == undefined)
	{
		//upgrade from noversion to 1.0.0
		console.log("upgrade RedHat Config from noversion to 1.0.0");
		jsonData.Version = "1.0.0";
	}
	if (jsonData.Version == "1.0.0")
	{
		console.log("upgrade RedHat Config from 1.0.0 to 1.0.1");
//		console.log(jsonData.CurrentTracker);
		var oldTracker;
		if (jsonData.CurrentTracker == undefined)
			oldTracker = JSON.parse(JSON.stringify(DefaultTracker));
		else
			oldTracker = JSON.parse(JSON.stringify(jsonData.CurrentTracker));
		var Gauges = ["Main", "Prog"];
		jsonData.CurrentTracker = [];
		for (var i = 0; i < 2; i++)
		{
			var newTracker = JSON.parse(JSON.stringify(oldTracker));
			newTracker.GaugeName = Gauges[i];
			newTracker.ShowGauge = (newTracker.ReportMode & (1<<i)) > 0 ? true : false;
			newTracker.PinNr = i;
			newTracker.Offset = 0;
			delete newTracker.ReportMode;
			jsonData.CurrentTracker.push(newTracker); 
		}
		jsonData.Version = "1.0.1";
	}
	if (jsonData.Version == "1.0.1")
	{
		console.log("upgrade RedHat Config from 1.0.1 to 1.0.2");
		if (jsonData.AutomationSettings == undefined)
			jsonData.AutomationSettings = JSON.parse(JSON.stringify({"Routes": []}));
		if (jsonData.RosterSettings == undefined)
			jsonData.RosterSettings = JSON.parse(JSON.stringify({"Locos": []}));
		jsonData.Version = "1.0.2";
	}
	if (jsonData.Version == "1.0.2")
	{
		console.log("upgrade RedHat Config from 1.0.2 to 1.0.3");
		if (jsonData.TrackSettings == undefined)
			jsonData.TrackSettings = JSON.parse(JSON.stringify({"TrackModes": [{"Status":1},{"Status":2}]}));
		jsonData.Version = "1.0.3";
	}
	return jsonData;
}

function upgradeJSONVersionPurpleHat(jsonData)
{
	jsonFileVersion = "1.0.4";
	var thisVersion = jsonData.Version;
	console.log(thisVersion);
	if (thisVersion == jsonFileVersion)
		return jsonData;
	if (thisVersion == undefined)
	{
		//upgrade from noversion to 1.0.0
		console.log("upgrade PurpleHat Config from noversion to 1.0.0");
		jsonData.Version = "1.0.0";
	}
	if (jsonData.Version == "1.0.0")
	{
		
		console.log("upgrade PurpleHat Config from 1.0.0 to 1.0.1");
		jsonData.ProgMethod = 0;
		jsonData.ProgMode = 0;
		jsonData.Version = "1.0.1";
	}
	if (jsonData.Version == "1.0.1")
	{
		jsonData.LEDColsSpeedDisp = JSON.parse(JSON.stringify([{"Name": "Background","RGBVal": [145, 145, 145]},{"Name": "Phys. Speed","RGBVal": [255, 50, 50]}, {"Name": "Scale Speed","RGBVal": [30, 128, 30]},{"Name": "Speed Step","RGBVal": [50, 50, 255]}]));
		jsonData.LEDColsAngleDisp = JSON.parse(JSON.stringify([{"Name": "Background","RGBVal": [145, 145, 145]},{"Name": "Grade","RGBVal": [255, 50, 50]}, { "Name": "Superelevation","RGBVal": [30, 128, 30]}]));
		jsonData.LEDColsStability = JSON.parse(JSON.stringify([{"Name": "Background","RGBVal": [145, 145, 145]},{"Name": "250ms","RGBVal": [255, 50, 50]},{"Name": "1s","RGBVal": [30, 128, 30]},{"Name": "5s","RGBVal": [50, 50, 255]}]));
		jsonData.LEDColsThrottle = JSON.parse(JSON.stringify([{"Name": "Background","RGBVal": [255,50,50]},{"Name": "Throttle Curve","RGBVal": [150,150,250]}]));
		jsonData.LEDColsProfile = JSON.parse(JSON.stringify([{"Name": "Background","RGBVal":[30,128,30]},{"Name": "Speed Limit Curve","RGBVal": [128,128,128]},{"Name": "Forward Curve","RGBVal": [7,161,228]},{"Name": "Backward Curve","RGBVal": [212,42,12]}]));
		jsonData.LEDColsTable = JSON.parse(JSON.stringify([{"Name": "Background","RGBVal": [50,50,255]},{"Name": "Speed Table Curve","RGBVal": [255,50,255]}]));
		console.log("upgrade PurpleHat Config from 1.0.1 to 1.0.2");
		jsonData.Version = "1.0.2";
	}
	if (jsonData.Version == "1.0.2")
	{
		console.log("upgrade PurpleHat Config from 1.0.2 to 1.0.3");
		jsonData.CalcMode = 0;
		jsonData.TrimMode = 0;
		jsonData.Version = "1.0.3";
	}
	if (jsonData.Version == "1.0.3")
	{
		console.log("upgrade PurpleHat Config from 1.0.3 to 1.0.4");
		jsonData.SpeedDispSize = 100;
		jsonData.Version = "1.0.4";
	}
	return jsonData;
}

function upgradeJSONVersionLBServer(jsonData)
{
	jsonFileVersion = "1.0.4";
	var thisVersion = jsonData.Version;
//	console.log(thisVersion);
	if (thisVersion == jsonFileVersion)
		return jsonData;
	if (thisVersion == undefined)
	{
		//upgrade from noversion to 1.0.0
		console.log("upgrade LBServer Config from noversion to 1.0.0");
		jsonData.Version = "1.0.0";
	}
	if (jsonData.Version == "1.0.0")
	{
		console.log("upgrade LBServer Config from 1.0.0 to 1.0.1");
		jsonData.ServerPortNr = 1234;
		jsonData.Version = "1.0.1";
	}
	if (jsonData.Version == "1.0.1")
	{
		console.log("upgrade LBServer Config from 1.0.1 to 1.0.2");
		if (jsonData.PowerMode == undefined) //lbServer
			jsonData.ServerType = 0;
		else
			jsonData.ServerType = 1;
		switch (jsonData.ServerType)
		{
			case 1: 
				jsonData.UpdateFC = true;
				break;
			default: break;
		}
		jsonData.Version = "1.0.2";
		
	}
	if (jsonData.Version == "1.0.2")
	{
		console.log("upgrade LBServer Config from 1.0.2 to 1.0.3");
		if (jsonData.Locos == undefined) //radiobox local list vs command station
			jsonData.Locos == []; //local roster list
		if (jsonData.DefinitionSource == undefined) //radiobox local list vs command station
			jsonData.DefinitionSource = 0;
		if (jsonData.TrainServerIP == undefined) //TrainServer
			jsonData.TrainServerIP = "192.168.0.1";
		jsonData.Version = "1.0.3";
		
	}
	if (jsonData.Version == "1.0.3")
	{
		console.log("upgrade LBServer Config from 1.0.3 to 1.0.4");
		if (jsonData.Sensors == undefined) //radiobox local list vs command station
			jsonData.Sensors = []; //local roster list
		if (jsonData.Functions == undefined) //radiobox local list vs command station
			jsonData.Functions = ["Light", "Bell", "Horn", "F3", "F4", "F5"]; //local function list
		jsonData.Version = "1.0.4";
		
	}
	return jsonData;
}

function upgradeJSONVersionGH(jsonData)
{
	jsonFileVersion = "1.0.0";
	var thisVersion = jsonData.Version;
	console.log(thisVersion);
	if (thisVersion == jsonFileVersion)
		return jsonData;
	if (thisVersion == undefined)
//	if ((isNaN(thisVersion)) || (thisVersion = undefined))
	{
		//upgrade from noversion to 1.1.1
		console.log("upgrade GreenHat from noversion to 1.0.0");
		jsonData.Version = "1.0.0";
	}
	return jsonData;
}

function upgradeJSONVersionSwitch(jsonData)
{
	jsonFileVersion = "1.0.1";
	var thisVersion = jsonData.Version;
//	console.log(thisVersion);
	if (thisVersion == jsonFileVersion)
		return jsonData;
	if (thisVersion == undefined)
//	if ((isNaN(thisVersion)) || (thisVersion = undefined))
	{
		//upgrade from noversion to 1.1.1
		console.log("upgrade Switches from noversion to 1.0.0");
		jsonData.Version = "1.0.0";
	}
	if (jsonData.Version == "1.0.0")
	{
		for (var i = 0; i < jsonData.Drivers.length; i++)
		{
			jsonData.Drivers[i].DevType = "servo";
			jsonData.Drivers[i].DevOption = 3;
		}
		jsonData.ServoMinPos = 175;
		jsonData.ServoMaxPos = 875;
		jsonData.Version = "1.0.1";
		console.log("upgrade Switches from version 1.0.0 to 1.0.1");
	}
	return jsonData;
}

function upgradeJSONVersionLED(jsonData)
{
	jsonFileVersion = "1.1.2";
	var thisVersion = jsonData.Version;
	if (thisVersion == jsonFileVersion)
		return jsonData;
	if (thisVersion == undefined)
//	if ((isNaN(thisVersion)) || (thisVersion == undefined))
	{
		//upgrade from noversion to 1.1.0
		console.log("upgrading LEDs to 1.1.0");
		jsonData.Version = "1.1.0";
	}
	if (jsonData.Version == "1.1.0")
	{
		console.log("upgrading LEDs to 1.1.1");
		jsonData.MQTT = {"Subscribe": [{"Topic": "LEDSET","InclAddr": false}, {"Topic": "LEDASK","InclAddr": false}],"Publish": [{"Topic": "LEDREPLY","InclAddr": false}]};
		jsonData.Version = "1.1.1";
	}
	if (jsonData.Version == "1.1.1")
	{
		console.log("upgrading LEDs to 1.1.2");
		for (var i = 0; i < jsonData.LEDDefs.length; i++)
		{
			var isArray = Array.isArray(jsonData.LEDDefs[i].LEDCmd.CtrlAddr);
			if (!isArray)
				jsonData.LEDDefs[i].LEDCmd.CtrlAddr = [jsonData.LEDDefs[i].LEDCmd.CtrlAddr];
			if (jsonData.LEDDefs[i].CtrlSource == "switch")
				if (jsonData.LEDDefs[i].LEDCmd.length == 4)
				{
					jsonData.LEDDefs[i].LEDCmd.pop(); //remove last entry
					jsonData.LEDDefs[i].LEDCmd.splice(1,1); //remove 2nd entry
					jsonData.LEDDefs[i].LEDCmd[0].Val = 0;
					jsonData.LEDDefs[i].LEDCmd[1].Val = 1;
				}
			if (jsonData.LEDDefs[i].CtrlSource == "signalstat")
				jsonData.LEDDefs[i].CtrlSource = "switch";
		}

		jsonData.Version = "1.1.2";
		console.log(jsonData);
	}
	return jsonData;
}

function upgradeJSONVersionBtn(jsonData)
{
	jsonFileVersion = "1.1.3";
	var thisVersion = jsonData.Version;
	if (thisVersion == jsonFileVersion)
		return jsonData;
	if (thisVersion == undefined)
//	if ((isNaN(thisVersion)) || (thisVersion = undefined))
	{
		//upgrade from noversion to 1.1.1
		console.log("upgrade buttons from noversion to 1.1.1");
		for (var i=0; i<jsonData.Buttons.length;i++)
		{
			if (jsonData.Buttons[i].EventMask == undefined)
				jsonData.Buttons[i].EventMask = 0x1F;
		}
		jsonData.Version = "1.1.1";
	}
	if (jsonData.Version == "1.1.1")
	{
		console.log("upgrade buttons to 1.1.2");
		jsonData.MQTT = {"Subscribe": [{"Topic": "BTNASK","InclAddr": false}],"Publish": [{"Topic": "BTNREPORT","InclAddr": false}, {"Topic": "BTNREPLY","InclAddr": false}]};
		jsonData.Version = "1.1.2";
	}
	if (jsonData.Version == "1.1.2")
	{
		console.log("upgrade buttons to 1.1.3");
		jsonData.RefreshInterval = 30000;
		jsonData.Sensitivity = 2;
		jsonData.Version = "1.1.3";
	}
	return jsonData;
}

function upgradeJSONVersionBtnHdlr(jsonData)
{
	jsonFileVersion = "1.3.2";
	var thisVersion = jsonData.Version;
	if (thisVersion == jsonFileVersion)
		return jsonData;
	if (thisVersion == undefined)
	{
		//upgrade from noversion to 1.3.0
		console.log("upgrade button handler from noversion to 1.3.0");
		for (var i=0; i<jsonData.ButtonHandler.length;i++)
		{
			for (var j=0; j<jsonData.ButtonHandler[i].CtrlCmd.length;j++)
			{
				if (jsonData.ButtonHandler[i].CtrlCmd[j].EventType == "onanalog")
					jsonData.ButtonHandler[i].CtrlCmd.splice(j, 1);
			}
			for (var j=0; j<jsonData.ButtonHandler[i].CtrlCmd.length;j++)
			{
				if ((jsonData.ButtonHandler[i].CtrlCmd[j].EventType == "onbtndown") || (jsonData.ButtonHandler[i].CtrlCmd[j].EventType == "onbtnhold"))
					if (jsonData.ButtonHandler[i].CtrlCmd[j].BtnCondAddr == undefined)
						jsonData.ButtonHandler[i].CtrlCmd[j].BtnCondAddr = 0xFFFF;
			}
		}
		jsonData.Version = "1.3.0";
		thisVersion = "1.3.0";
	}
	if (thisVersion == "1.3.0")
	{
		//upgrade from 1.3.0 to 1.3.1
		console.log("upgrade button handler from 1.3.0 to 1.3.1");
		for (var i=0; i<jsonData.ButtonHandler.length;i++)
		{
			jsonData.ButtonHandler[i].EventSource = "button";
			jsonData.ButtonHandler[i].CurrDisp = 0;
			jsonData.ButtonHandler[i].CondData = [0];
			var currVal = jsonData.ButtonHandler[i].ButtonNr;
			jsonData.ButtonHandler[i].ButtonNr = [];
			if (!isNaN(currVal))
				jsonData.ButtonHandler[i].ButtonNr.push(currVal);
			for (var j=0; j<jsonData.ButtonHandler[i].CtrlCmd.length;j++)
			{
				delete jsonData.ButtonHandler[i].CtrlCmd[j].EventType;
				var currVal = jsonData.ButtonHandler[i].CtrlCmd[j].BtnCondAddr;
				jsonData.ButtonHandler[i].CtrlCmd[j].BtnCondAddr = [];
				if (!isNaN(currVal))
					if (currVal != 0xFFFF)
						jsonData.ButtonHandler[i].CtrlCmd[j].BtnCondAddr.push(currVal);
			}
		}
		jsonData.Version = "1.3.1";
		thisVersion = "1.3.1";
	}
	if (thisVersion == "1.3.1")
	{
		console.log("upgrade button handler from 1.3.1 to 1.3.2");
		for (var i=0; i<jsonData.ButtonHandler.length;i++)
		{
			jsonData.ButtonHandler[i].EnableSource = "on";
			jsonData.ButtonHandler[i].EnableAddr = 0;
			jsonData.ButtonHandler[i].EnableState = 0;			
		}
		jsonData.Version = "1.3.2";
		thisVersion = "1.3.2";
	}
	return jsonData;
}

function addFileSeqLED(ofObj, cfgData) //object specific function to include partial files
{
	console.log(ofObj);
	console.log(cfgData);
	for (var j=0; j<ofObj.LEDDefs.length; j++)
	{
		cfgData[1].LEDDefs.push(JSON.parse(JSON.stringify(ofObj.LEDDefs[j])));
		cfgData[2] = JSON.parse(JSON.stringify(cfgData[1]));
	}
}

function addFileSeqBtnHdlr(ofObj, cfgData) //object specific function to include partial files
{
	console.log(ofObj);
	console.log(cfgData);
	for (var j=0; j<ofObj.ButtonHandler.length; j++)
	{
		cfgData[1].ButtonHandler.push(JSON.parse(JSON.stringify(ofObj.ButtonHandler[j])));
		cfgData[2] = JSON.parse(JSON.stringify(cfgData[1]));
	}
}

function addFileSeqSwi(ofObj, cfgData) //object specific function to include partial files
{
	console.log(ofObj);
	console.log(cfgData);
	for (var j=0; j<ofObj.Drivers.length; j++)
	{
		cfgData[1].Drivers.push(JSON.parse(JSON.stringify(ofObj.Drivers[j])));
		cfgData[2] = JSON.parse(JSON.stringify(cfgData[1]));
	}
}

function prepareFileSeqLED(configData, transferIndex) //object specific function to create partial files
{
	function addEntry()
	{
		var newEntry = {"LEDDefs":[]}
		transferData[transferIndex].FileList.push(newEntry);
		return newEntry;
	}
	
	var thisEntry = addEntry();
	thisEntry.Version = jsonFileVersion;
	thisEntry.ChainParams = JSON.parse(JSON.stringify(configData.ChainParams));
	thisEntry.MQTT = JSON.parse(JSON.stringify(configData.MQTT));
	thisEntry.LEDCols = JSON.parse(JSON.stringify(configData.LEDCols));
	var thisFileLength = 0;
	
	for (var j=0; j<configData.LEDDefs.length;j++)
	{
		var thisElementStr = JSON.stringify(configData.LEDDefs[j]);
		thisFileLength += thisElementStr.length;
		thisEntry.LEDDefs.push(JSON.parse(thisElementStr));
		if ((thisFileLength > targetSize) && (j < (configData.LEDDefs.length - 1)))
		{
			thisEntry = addEntry();
			thisFileLength = 0;
		}
	}
	console.log(transferData);
}

function prepareFileSeqBtnEvt(configData, transferIndex) //object specific function to create partial files
{
	function addEntry()
	{
		var newEntry = {"ButtonHandler":[]}
		transferData[transferIndex].FileList.push(newEntry);
		return newEntry;
	}
	
	var thisEntry = addEntry();
	thisEntry.Version = jsonFileVersion;
	var thisFileLength = 0;
	
	for (var j=0; j < configData.ButtonHandler.length;j++)
	{
		var thisElementStr = JSON.stringify(configData.ButtonHandler[j]);
		thisFileLength += thisElementStr.length;
		thisEntry.ButtonHandler.push(JSON.parse(thisElementStr));
		if ((thisFileLength > targetSize) && (j < (configData.ButtonHandler.length - 1)))
		{
			thisEntry = addEntry();
			thisFileLength = 0;
		}
	}
}

function prepareFileSeqSwi(configData, transferIndex) //object specific function to create partial files
{
	function addEntry()
	{
		var newEntry = {"Drivers":[]}
		transferData[transferIndex].FileList.push(newEntry);
		return newEntry;
	}
	
	var thisEntry = addEntry();
	thisEntry.Version = JSON.parse(JSON.stringify(configData.Version));
	thisEntry.ServoMinPos = JSON.parse(JSON.stringify(configData.ServoMinPos));
	thisEntry.ServoMaxPos = JSON.parse(JSON.stringify(configData.ServoMaxPos));
	var thisFileLength = 0;
	
	for (var j=0; j < configData.Drivers.length;j++)
	{
		var thisElementStr = JSON.stringify(configData.Drivers[j]);
		thisFileLength += thisElementStr.length;
		thisEntry.Drivers.push(JSON.parse(thisElementStr));
		if ((thisFileLength > targetSize) && (j < (configData.Drivers.length - 1)))
		{
			thisEntry = addEntry();
			thisFileLength = 0;
		}
	}
}
