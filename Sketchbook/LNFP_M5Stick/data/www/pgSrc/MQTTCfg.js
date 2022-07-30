var mainScrollBox;
var pingDelay;
var dccDisp;
var lnDisp;

mqttDefault = {"MQTTServer": "broker.hivemq.com", "MQTTPort": 1883, "MQTTUser": "", "MQTTPassword": "", "NodeName": "LNCTC", "inclMAC": true, "doPing": true, "pingDelay": 300, "BCTopic": "IoTT_LNBroadcast", 	"EchoTopic": "IoTT_LNEcho", "PingTopic": "IoTT_LNPing", "DCCTopic": "dccBC"};

jsonFileVersion = "1.1.1";

function saveConfigFileSettings()
{
	//step 1: save greenhat.cfg
	console.log("Saving ", scriptList.Pages[currentPage].FileName, configData[workCfg]);
	saveJSONConfig(scriptList.Pages[currentPage].ID, scriptList.Pages[currentPage].FileName, configData[workCfg], null);
}

function upgradeJSONVersion(jsonData)
{
	var thisVersion = jsonData.Version;
	console.log(thisVersion);
//	console.log(jsonData);
	if (thisVersion == jsonFileVersion)
		return jsonData;
	if ((isNaN(thisVersion)) || (thisVersion = undefined))
	{
		//upgrade from noversion to 1.1.0
		console.log("upgrading to 1.1.0");
		jsonData.DCCTopic = "dccBC";
		jsonData.Version = "1.1.0";
	}
	if (jsonData.Version == "1.1.0")
	{
		console.log("upgrading to 1.1.1");
		var oldSettings = JSON.parse(JSON.stringify(jsonData));
		delete oldSettings["Version"];
		jsonData = {};
		jsonData.Version = "1.1.1";
		jsonData.ClientSettings = JSON.parse(JSON.stringify(oldSettings));
		jsonData.ServerSettings = JSON.parse(JSON.stringify(oldSettings));
	}
//	console.log(jsonData);
	return jsonData;
}

function setMQTTBasics(sender)
{
	if (pageParam == 's')
	{
		if (sender.id == "serverID")
			configData[workCfg].ServerSettings.MQTTServer = sender.value;
		if (sender.id == "serverport")
			configData[workCfg].ServerSettings.MQTTPort = verifyNumber(sender.value, configData[workCfg].ServerSettings.MQTTPort);
		if (sender.id == "mqttuser")
			configData[workCfg].ServerSettings.MQTTUser = sender.value;
		if (sender.id == "mqttpassword")
			configData[workCfg].ServerSettings.MQTTPassword = sender.value;
		if (sender.id == "pingtopic")
			configData[workCfg].ServerSettings.PingTopic = sender.value;
		if (sender.id == "dcctopic")
			configData[workCfg].ServerSettings.DCCTopic = sender.value;
		if (sender.id == "bctopic")
			configData[workCfg].ServerSettings.BCTopic = sender.value;
		if (sender.id == "echotopic")
			configData[workCfg].ServerSettings.EchoTopic = sender.value;
	} 
	else
	{
		if (sender.id == "serverID")
			configData[workCfg].ClientSettings.MQTTServer = sender.value;
		if (sender.id == "serverport")
			configData[workCfg].ClientSettings.MQTTPort = verifyNumber(sender.value, configData[workCfg].ClientSettings.MQTTPort);
		if (sender.id == "mqttuser")
			configData[workCfg].ClientSettings.MQTTUser = sender.value;
		if (sender.id == "mqttpassword")
			configData[workCfg].ClientSettings.MQTTPassword = sender.value;
		if (sender.id == "pingtopic")
			configData[workCfg].ClientSettings.PingTopic = sender.value;
		if (sender.id == "dcctopic")
			configData[workCfg].ClientSettings.DCCTopic = sender.value;
		if (sender.id == "bctopic")
			configData[workCfg].ClientSettings.BCTopic = sender.value;
		if (sender.id == "echotopic")
			configData[workCfg].ClientSettings.EchoTopic = sender.value;
	} 
//	console.log(configData[workCfg]);
}

function setNodeName(sender)
{
	if (sender.id == "nodename")
		if (pageParam == 's')
			configData[workCfg].ServerSettings.NodeName = sender.value;
		else
			configData[workCfg].ClientSettings.NodeName = sender.value;
//	console.log(configData[workCfg]);
}

function setUseMac(sender)
{
	if (pageParam == 's')
		configData[workCfg].ServerSettings.inclMAC = sender.checked ? 1:0;
	else
		configData[workCfg].ClientSettings.inclMAC = sender.checked ? 1:0;
//	console.log(configData[workCfg]);
}

function setUsePing(sender)
{
	if (pageParam == 's')
		configData[workCfg].ServerSettings.doPing = sender.checked ? 1:0;
	else
		configData[workCfg].ClientSettings.doPing = sender.checked ? 1:0;
	setVisibility(sender.checked, pingDelay);
}

function setPingDelay(sender)
{
	if (sender.id == "pingdelay")
		if (pageParam == 's')
			configData[workCfg].ServerSettings.pingDelay = verifyNumber(sender.value, configData[workCfg].ServerSettings.pingDelay);
		else
			configData[workCfg].ClientSettings.pingDelay = verifyNumber(sender.value, configData[workCfg].ClientSettings.pingDelay);
//	console.log(configData[workCfg]);
}

function downloadSettings(sender)
{
	downloadConfig(0x0002); //send just this
}

function constructPageContent(contentTab)
{
	var tempObj;
	mainScrollBox = createEmptyDiv(contentTab, "div", "pagetopicboxscroll-y", "nodeconfigdiv");
	createPageTitle(mainScrollBox, "div", "tile-1", "BasicCfg_Title", "h1", "MQTT Configuration");

			createPageTitle(mainScrollBox, "div", "tile-1", "", "h2", "Broker Settings");
			tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
				createTextInput(tempObj, "tile-1_4", "MQTT Server:", "n/a", "serverID", "setMQTTBasics(this)");
				createTextInput(tempObj, "tile-1_4", "MQTT Port:", "n/a", "serverport", "setMQTTBasics(this)");
			tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
				createTextInput(tempObj, "tile-1_4", "MQTT User Name:", "n/a", "mqttuser", "setMQTTBasics(this)");
				createTextInput(tempObj, "tile-1_4", "MQTT Password:", "n/a", "mqttpassword", "setMQTTBasics(this)");

			createPageTitle(mainScrollBox, "div", "tile-1", "", "h2", "Communication Settings");
			tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
				createTextInput(tempObj, "tile-1_4", "Node Name:", "n/a", "nodename", "setNodeName(this)");
				createCheckbox(tempObj, "tile-1_4", "Add MAC Address", "cbUseMac", "setUseMac(this)");
			tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
				createCheckbox(tempObj, "tile-1_4", "Send Periodic Ping", "cbUsePing", "setUsePing(this)");
				pingDelay = createEmptyDiv(tempObj, "div", "tile-1", "");
				pingDelay.style.display = "none";
					createTextInput(pingDelay, "tile-1_4", "Ping Delay:", "n/a", "pingdelay", "setPingDelay(this)");
					createTextInput(pingDelay, "tile-1_4", "Ping Topic:", "n/a", "pingtopic", "setMQTTBasics(this)");
			lnDisp = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
				createTextInput(lnDisp, "tile-1_4", "LN Broadcast Topic:", "n/a", "bctopic", "setMQTTBasics(this)");
				createTextInput(lnDisp, "tile-1_4", "LN Echo Topic:", "n/a", "echotopic", "setMQTTBasics(this)");
			dccDisp = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
				createTextInput(dccDisp, "tile-1_4", "DCC Broadcast Topic:", "n/a", "dcctopic", "setMQTTBasics(this)");

		tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
			createButton(tempObj, "", "Save & Restart", "btnSave", "saveSettings(this)");
			createButton(tempObj, "", "Cancel", "btnCancel", "cancelSettings(this)");
		tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
			createButton(tempObj, "", "Save to File", "btnDownload", "downloadSettings(this)");
}

function configDisplay()
{
	switch (pageParam)
	{
		case 'c' :  switch (thisIntfID)
					{
						case 3: document.getElementById("BasicCfg_Title").innerHTML = "<h1>Loconet from MQTT Configuration<\h1>"; break;
						case 10: document.getElementById("BasicCfg_Title").innerHTML = "<h1>DCC from MQTT Configuration<\h1>"; break;
						default: document.getElementById("BasicCfg_Title").innerHTML = "<h1>MQTT Configuration<\h1>"; break;
					}
					break;
		case 's' :  document.getElementById("BasicCfg_Title").innerHTML = "<h1>MQTT Gateway Configuration<\h1>";
					break;
	}
	setVisibility(([1,10].indexOf(thisIntfID) >= 0), dccDisp);
	setVisibility(([2,3,12,16].indexOf(thisIntfID) >= 0), lnDisp);
}

function loadNodeDataFields(jsonData)
{
//	console.log(jsonData);
	configDisplay();
}

function loadDataFields(jsonData)
{
//	console.log(jsonData);
	configData[workCfg] = upgradeJSONVersion(jsonData);
	configDisplay();
//	console.log(configData[workCfg]);
	var jsonSubData = JSON.parse(JSON.stringify(configData[workCfg].ClientSettings));
	switch (pageParam)
	{
//		case 'c' : jsonSubData = jsonData.ClientSettings; break;
		case 's' : jsonSubData = JSON.parse(JSON.stringify(configData[workCfg].ServerSettings)); break;
	}
//	console.log(jsonSubData);
	writeInputField("serverID", jsonSubData.MQTTServer);
	writeInputField("serverport", jsonSubData.MQTTPort);
	writeInputField("mqttuser", jsonSubData.MQTTUser);
	writeInputField("mqttpassword", jsonSubData.MQTTPassword);
	writeInputField("nodename", jsonSubData.NodeName);
	writeCBInputField("cbUseMac", jsonSubData.inclMAC);

	writeCBInputField("cbUsePing", jsonSubData.doPing);
	setVisibility(jsonSubData.doPing, pingDelay);
	writeInputField("pingdelay", jsonSubData.pingDelay);
	writeInputField("pingtopic", jsonSubData.PingTopic);
	writeInputField("bctopic", jsonSubData.BCTopic);
	writeInputField("dcctopic", jsonSubData.DCCTopic);
	writeInputField("echotopic", jsonSubData.EchoTopic);
}
