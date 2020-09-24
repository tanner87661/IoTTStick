var mainScrollBox;
var pingDelay;

jsonFileVersion = "1.1.0";

function upgradeJSONVersion(jsonData)
{
	var thisVersion = jsonData.Version;
	console.log(thisVersion);
	console.log(jsonData);
	if (thisVersion == jsonFileVersion)
		return;
	if ((isNaN(thisVersion)) || (thisVersion = undefined))
	{
		//upgrade from noversion to 1.1.0
		console.log("upgrading to 1.1.0");
		jsonData.DCCTopic = "dccBC";
		jsonData.Version = "1.1.0";
	}
}

function setMQTTBasics(sender)
{
	if (sender.id == "serverID")
		configWorkData.MQTTServer = sender.value;
	if (sender.id == "serverport")
		configWorkData.MQTTPort = verifyNumber(sender.value, configWorkData.MQTTPort);
	if (sender.id == "mqttuser")
		configWorkData.MQTTUser = sender.value;
	if (sender.id == "mqttpassword")
		configWorkData.MQTTPassword = sender.value;
	if (sender.id == "pingtopic")
		configWorkData.PingTopic = sender.value;
	if (sender.id == "dcctopic")
		configWorkData.DCCTopic = sender.value;
	if (sender.id == "bctopic")
		configWorkData.BCTopic = sender.value;
	if (sender.id == "echotopic")
		configWorkData.EchoTopic = sender.value;
	console.log(configWorkData);
}

function setNodeName(sender)
{
	if (sender.id == "nodename")
		configWorkData.NodeName = sender.value;
	console.log(configWorkData);
}

function setUseMac(sender)
{
	configWorkData.inclMAC = sender.checked ? 1:0;
	console.log(configWorkData);
}

function setUsePing(sender)
{
	configWorkData.doPing = sender.checked ? 1:0;
	if (sender.checked)
		pingDelay.style.display = "block";
	else
		pingDelay.style.display = "none";
}

function setPingDelay(sender)
{
	if (sender.id == "pingdelay")
		configWorkData.pingDelay = verifyNumber(sender.value, configWorkData.pingDelay);
	console.log(configWorkData);
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

			createPageTitle(mainScrollBox, "div", "tile-1", "", "h2", "Server Settings");
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
			tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
				createTextInput(tempObj, "tile-1_4", "LN Broadcast Topic:", "n/a", "bctopic", "setMQTTBasics(this)");
				createTextInput(tempObj, "tile-1_4", "LN Echo Topic:", "n/a", "echotopic", "setMQTTBasics(this)");
			tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
				createTextInput(tempObj, "tile-1_4", "DCC Broadcast Topic:", "n/a", "dcctopic", "setMQTTBasics(this)");

		tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
			createButton(tempObj, "", "Save & Restart", "btnSave", "saveSettings(this)");
			createButton(tempObj, "", "Cancel", "btnCancel", "cancelSettings(this)");
		tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
			createButton(tempObj, "", "Save to File", "btnDownload", "downloadSettings(this)");
}

function loadNodeDataFields(jsonData)
{
}

function loadDataFields(jsonData)
{
	upgradeJSONVersion(jsonData);
	writeInputField("serverID", jsonData.MQTTServer);
	writeInputField("serverport", jsonData.MQTTPort);
	writeInputField("mqttuser", jsonData.MQTTUser);
	writeInputField("mqttpassword", jsonData.MQTTPassword);
	writeInputField("nodename", jsonData.NodeName);
	writeCBInputField("cbUseMac", jsonData.inclMAC);

	writeCBInputField("cbUsePing", jsonData.doPing);
	setVisibility(jsonData.doPing, pingDelay);
	writeInputField("pingdelay", jsonData.pingDelay);
	writeInputField("pingtopic", jsonData.PingTopic);
	writeInputField("bctopic", jsonData.BCTopic);
	writeInputField("dcctopic", jsonData.DCCTopic);
	writeInputField("echotopic", jsonData.EchoTopic);
	
	
}
