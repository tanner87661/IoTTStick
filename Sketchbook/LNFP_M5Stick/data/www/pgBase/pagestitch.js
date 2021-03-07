function upgradeJSONVersionNode(jsonData)
{
	jsonFileVersion = "1.0.0";
	var thisVersion = jsonData.Version;
	console.log(thisVersion);
	if (thisVersion == jsonFileVersion)
		return jsonData;
	if ((isNaN(thisVersion)) || (thisVersion = undefined))
	{
		//upgrade from noversion to 1.0.0
		console.log("upgrade Node Config from noversion to 1.0.0");
		jsonData.Version = "1.0.0";
	}
	return jsonData;
}

function upgradeJSONVersionLBServer(jsonData)
{
	jsonFileVersion = "1.0.0";
	var thisVersion = jsonData.Version;
	console.log(thisVersion);
	if (thisVersion == jsonFileVersion)
		return jsonData;
	if ((isNaN(thisVersion)) || (thisVersion = undefined))
	{
		//upgrade from noversion to 1.0.0
		console.log("upgrade LBServer Config from noversion to 1.0.0");
		jsonData.Version = "1.0.0";
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
	if ((isNaN(thisVersion)) || (thisVersion = undefined))
	{
		//upgrade from noversion to 1.1.1
		console.log("upgrade GreenHat from noversion to 1.0.0");
		jsonData.Version = "1.0.0";
	}
	return jsonData;
}

function upgradeJSONVersionSwitch(jsonData)
{
	jsonFileVersion = "1.0.0";
	var thisVersion = jsonData.Version;
//	console.log(thisVersion);
	if (thisVersion == jsonFileVersion)
		return jsonData;
	if ((isNaN(thisVersion)) || (thisVersion = undefined))
	{
		//upgrade from noversion to 1.1.1
		console.log("upgrade Switches from noversion to 1.0.0");
		jsonData.Version = "1.0.0";
	}
	return jsonData;
}

function upgradeJSONVersionLED(jsonData)
{
	jsonFileVersion = "1.1.2";
	var thisVersion = jsonData.Version;
	if (thisVersion == jsonFileVersion)
		return jsonData;
	if ((isNaN(thisVersion)) || (thisVersion == undefined))
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
	if ((isNaN(thisVersion)) || (thisVersion = undefined))
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
	jsonFileVersion = "1.3.1";
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

