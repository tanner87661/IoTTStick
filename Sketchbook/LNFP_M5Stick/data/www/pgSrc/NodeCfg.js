var mainScrollBox;
var topicStats1;
var topicStats2;
var topicWificb;
var configAPModuleBox;
var configNTPSection;
var configNTPBox;
var configDHCPBox;
var configDHCPSection;
var configAPBox;
var moduleConfig;
var modDCCConfig;
var modHWBtnConfig;
var modLNConfig;
var modALMOnly;
var modAlwaysOn;
var modWifiOnly;
var modDecoderOnly;
var modWifiALMOnly;

var modalDialog = null;
var galleryUploader = null;
var _gaq;




//commMode: 0: DCC, 1: LocoNet, 2: MQTT, 3: Gateway
//workMode: 0: Decoder, 1: ALM

function upgradeJSONVersion(jsonData)
{
	return upgradeJSONVersionNode(jsonData);
}

function saveConfigFileSettings()
{
	//step 1: save greenhat.cfg
	saveJSONConfig(scriptList.Pages[currentPage].ID, scriptList.Pages[currentPage].FileName, configData[workCfg], null);
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
			fileIDName = configArray[0].Type;
			var thisIndex = scriptList.Pages.findIndex(findIDName);
			var valid = (configArray.length > 0) && !isNaN(thisIndex) ;
			if (!valid)
			{
				alert("Error in configuration data");
				return;
			}
		}
		catch(err) 
		{
			alert("Configuration data not valid");
			return;
		}
        uploadConfig(configArray);
    };
    reader.readAsText(fileName);	
}

function downloadSettings(sender)
{
	if (event.ctrlKey) //duplicate entry
		downloadConfig(0x0001); //send just this
	else
		downloadConfig(0xFFFF); //send all
}

function setDisplayOptions()
{
	setVisibility((configData[2].wifiMode & 0x02) > 0, configAPModuleBox);
	setVisibility((configData[2].wifiMode & 0x02) > 0, configAPBox);
	setVisibility((configData[2].wifiMode & 0x01) > 0, configNTPSection);
	setVisibility(configData[2].useNTP, configNTPBox);
	setVisibility((configData[2].wifiMode & 0x01) > 0, configDHCPSection);
	setVisibility(configData[2].useStaticIP, configDHCPBox);
	setVisibility([4,5,6].indexOf(configData[2].InterfaceIndex) >= 0, configLNOptBox);
	setVisibility(([4,5,6].indexOf(configData[2].InterfaceIndex) >= 0) && ([5].indexOf(configData[2].HatIndex) < 0), configFCBox);
//	setVisibility(([4].indexOf(configData[2].InterfaceIndex) >= 0) && ([5].indexOf(configData[2].HatIndex) < 0), configSubnetBox);
	setVisibility(([4,5,6].indexOf(configData[2].InterfaceIndex) >= 0), configRefreshBox);


	for (var i = 0; i < configData[2].ALMTypeList.length; i++)
	{
		var almId = "cbUseALM_" + i;
		if (document.getElementById(almId))
		{
			var isVisible = (configData[2].ALMTypeList[i].InterfaceList.indexOf(configData[2].InterfaceTypeList[configData[2].InterfaceIndex].IntfId)) >= 0;
			//note: InterfaceList settings are overridden by the ones in pagestitch
			if (!isVisible)
			{
				configData[2].ALMIndex = clearElement(configData[2].ALMIndex, configData[2].ALMTypeList[i].ALMId);
				document.getElementById(almId).checked = false;
			}
			setVisibility(isVisible, document.getElementById(almId).parentElement);
		}
	}
	for (var i = 0; i < configData[2].ServerTypeList.length; i++)
	{
		var serverId = "cbUseServer_" + i;
		if (document.getElementById(serverId))
		{
			var isVisible = configData[2].ServerTypeList[i].InterfaceList.indexOf(configData[2].InterfaceTypeList[configData[2].InterfaceIndex].IntfId) >= 0;
			if (!isVisible)
			{
				clearElement(configData[2].ServerIndex, configData[2].ServerTypeList[i].ServerId);
				document.getElementById(serverId).checked = false;
			}
			setVisibility(isVisible, document.getElementById(serverId).parentElement);
		}
	}
//	console.log(configData[2].ALMIndex);
//	console.log(configData[2].ServerIndex);
}

function setProdType(sender)
{

	function  verifyALM(item, index) 
	{
		if (configData[2].ALMTypeList[item].Type > configData[2].InterfaceTypeList[oldCommMode].Type)
			configData[2].ALMIndex.splice(index,1);
	}

	var oldHatMode = configData[2].HatIndex;
	var oldCommMode = configData[2].InterfaceIndex;
	if (sender.id == "selecthattype")
		oldHatMode = sender.selectedIndex;
	if (sender.id == "selectcommtype")
		oldCommMode = sender.selectedIndex;
		
	if (configData[2].HatTypeList[oldHatMode].InterfaceList.indexOf(configData[2].InterfaceTypeList[oldCommMode].IntfId) < 0) 
	{
		alert("Invalid Combination of Hat and Interface");
		if (sender.id == "selecthattype")
			setDropdownValue("selecthattype", configData[2].HatIndex);
		if (sender.id == "selectcommtype")
			setDropdownValue("selectcommtype", configData[2].InterfaceIndex);
		return;
	}

	if ((configData[2].InterfaceTypeList[oldCommMode].ReqSTA == 1) && (configData[2].wifiMode == 2))
	{
		alert("This interface requires active WiFi connection");
		if (sender.id == "selectcommtype")
			setDropdownValue("selectcommtype", configData[2].InterfaceIndex);
		return;
	}
	configData[2].ALMIndex.forEach(verifyALM);
	loadALMOptions(configData[2]);

	configData[2].HatIndex = oldHatMode;
	configData[2].InterfaceIndex = oldCommMode;
	if (configData[2].InterfaceIndex >= 2)
	{
//		configData[2].useWifiTimeout = false;
//		writeCBInputField("cbUseWifi", false);
	}	
	setDisplayOptions();
//	console.log(configData[2]);
}

function setElement(inArray, e)
{
	if (inArray.indexOf(e) < 0)
		inArray.push(e);
	return inArray;
}

function clearElement(inArray, e)
{
	var elPos = inArray.indexOf(e);
	if (elPos >= 0)
		inArray.splice(elPos,1);
	return inArray;
}

function setUseALM(sender)
{
	function checkALMId(e)
	{
		return e.ALMId == ALMId;
	}
	
	var oldCommMode = configData[2].InterfaceIndex;
	var ALMId = parseInt(sender.getAttribute("ALMId"));
	var ALMIndex = configData[2].ALMTypeList.findIndex(checkALMId);
	if (sender.checked)
	{
//		console.log(oldCommMode, ALMId, ALMIndex);
		if ((configData[2].InterfaceTypeList[oldCommMode].Type >= configData[2].ALMTypeList[ALMIndex].Type))
			configData[2].ALMIndex = setElement(configData[2].ALMIndex, ALMId);
		else
		{
			alert("This Logic Module does not work with the selected command source");
			sender.checked = false;
		}
	}
	else
		configData[2].ALMIndex = clearElement(configData[2].ALMIndex, ALMId);
//	console.log(configData[2].ALMIndex);
}

function setUseServer(sender)
{
	function checkServerId(e)
	{
		return e.ServerId == ServerId;
	}

	var oldCommMode = configData[2].InterfaceIndex;
	var ServerId = parseInt(sender.getAttribute("ServerId"));
	var ServerIndex = configData[2].ServerTypeList.findIndex(checkServerId);
	if (sender.checked)
		setElement(configData[2].ServerIndex, ServerId);
	else
		clearElement(configData[2].ServerIndex, ServerId);
//	console.log(configData[2].ServerIndex);
}

function setWifiStatus(sender)
{
	configData[2].useWifiTimeout = sender.checked ? 1:0;
	setDisplayOptions();
}

function setNTPStatus(sender)
{
	configData[2].useNTP = sender.checked ? 1:0;
	setDisplayOptions();
}

function setNTPServer(sender)
{
	configData[2].ntpConfig.NTPServer = sender.value;
}

function setNTPTimeZone(sender)
{
	configData[2].ntpConfig.ntpTimeZone = sender.value;
}

function setNodeName(sender)
{
	configData[2].devName = sender.value;
}

function setUseMac(sender)
{
	configData[2].inclMAC = sender.checked ? 1:0;
}

function setUseDHCP(sender)
{
	configData[2].useStaticIP = sender.checked ? 1:0;
	setDisplayOptions();
}

function setDHCP(sender)
{
	if (sender.id == "staticip")
		configData[2].staticConfig.staticIP = sender.value;
	if (sender.id == "gatewayip")
		configData[2].staticConfig.staticGateway = sender.value;
	if (sender.id == "netmask")
		configData[2].staticConfig.staticNetmask = sender.value;
	if (sender.id == "dnsserver")
		configData[2].staticConfig.staticDNS = sender.value;
//	console.log(configData[2].staticConfig);
}

function setAP(sender)
{
	if (sender.id == "ap_ip")
		configData[2].apConfig.apGateway = sender.value;
	if (sender.id == "ap_password")
		configData[2].apConfig.apPassword = sender.value;
//	console.log(configData[2].apConfig);
}

function setWifiMode(sender, id)
{
	if (sender.id == "selectwifimode_0")
		configData[2].wifiMode = 1;
	if (sender.id == "selectwifimode_1")
	{
		if (configData[2].InterfaceTypeList[configData[2].InterfaceIndex].ReqSTA == 1)
		{
			alert("This interface requires active WiFi connction");
			writeRBInputField("selectwifimode", configData[2].wifiMode-1);
			return;
		}
		configData[2].wifiMode = 2;
	}
	setDisplayOptions();
}

function setUseBushbyBit(sender)
{
	configData[2].useBushby = sender.checked ? 1:0;
}

function setUseLissyBit(sender)
{
	configData[2].useLissy = sender.checked ? 1:0;
}

function setFastClock(sender)
{
	if (sender.id == "cbUseFastClock")
		configData[workCfg].broadcastFC = sender.checked;
	if (sender.id == "fcrefresh")
		configData[workCfg].broadcastFCRate = verifyNumber(sender.value, configData[workCfg].broadcastFCRate); 
}

function setFctRefresh(sender)
{
	if (sender.id == "cbUseFctRefresh")
		configData[workCfg].refreshFct = sender.checked;
}

//function setSubnet(sender)
//{
//	configData[2].subnetMode = sender.checked ? 1:0;
//}

function constructPageContent(contentTab)
{
	var tempObj;
	mainScrollBox = createEmptyDiv(contentTab, "div", "pagetopicboxscroll-y", "nodeconfigdiv");
		createPageTitle(mainScrollBox, "div", "tile-1", "", "h1", "Node Configuration");
		createPageTitle(mainScrollBox, "div", "tile-1", "", "h2", "Basic Setup");
		tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
			createDropdownselector(tempObj, "tile-1_3", "Command Source:", [], "selectcommtype", "setProdType(this, id)");
			createDropdownselector(tempObj, "tile-1_4", "Hat Module:", [], "selecthattype", "setProdType(this, id)");
//		tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
//			createDropdownselector(tempObj, "tile-1_4", "Gateway Options:", ["MQTT", "lbServer", "MQTT and lbServer"], "selectoptiontype", "setProdType(this, id)");
//			createCheckbox(tempObjP, "", "MQTT Gateway", "cbMQTTGateway", "setGatewayMode(this)");
//			createCheckbox(tempObjP, "", "LN over TCP Server", "cbTCPServer", "setGatewayMode(this)");
		tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1_4", "");
//		tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "configSubnetBox");
//		tempObj.style.display = "none";
//			createCheckbox(tempObj, "tile-1_4", "LocoNet Subnet", "cbUseSubnet", "setSubnet(this)");
		tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "configLNOptBox");
			createCheckbox(tempObj, "tile-1_4", "respect Bushby Bit", "cbUseBushbyBit", "setUseBushbyBit(this)");
			createCheckbox(tempObj, "tile-1_4", "Support Uhlenbrock Track display commands", "cbUseLissyBit", "setUseLissyBit(this)");
		tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "configFCBox");
			createCheckbox(tempObj, "tile-1_4", "Support FastClock", "cbUseFastClock", "setFastClock(this)");
			createTextInput(tempObj, "tile-1_4", "Refresh Rate [Sec.]:", "75", "fcrefresh", "setFastClock(this)");
		tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "configRefreshBox");
			createCheckbox(tempObj, "tile-1_4", "Refresh F9-F28", "cbUseFctRefresh", "setFctRefresh(this)");
		
		createPageTitle(mainScrollBox, "div", "tile-1", "", "h2", "Communication Servers");
		tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "ServerBox");

		createPageTitle(mainScrollBox, "div", "tile-1", "", "h2", "Embedded Function Modules Activation");
		tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "ALMBox");

		createPageTitle(mainScrollBox, "div", "tile-1", "", "h2", "Wifi Setup");
		tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
			createTextInput(tempObj, "tile-1_4", "Node Name:", "n/a", "nodename", "setNodeName(this)");
			createCheckbox(tempObj, "tile-1_4", "Add MAC Address", "cbUseMac", "setUseMac(this)");
		tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
//			createRadiobox(tempObj, "tile-1_2", "", ["Connect to WiFi", "Device Access Point", "Wifi and Device AP"], "selectwifimode", "setWifiMode(this, id)");
			createRadiobox(tempObj, "tile-1_2", "", ["Connect to WiFi", "Device A/P on Demand"], "selectwifimode", "setWifiMode(this, id)");
//		tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "wificb");
//			createCheckbox(tempObj, "tile-1_4", "Disable Wifi when not used", "cbUseWifi", "setWifiStatus(this)");

		configDHCPSection = createEmptyDiv(mainScrollBox, "div", "", "");
		createPageTitle(configDHCPSection, "div", "tile-1", "", "h3", "DHCP Configuration");
		tempObj = createEmptyDiv(configDHCPSection, "div", "tile-1", "");
			createCheckbox(tempObj, "tile-1_4", "Use Static IP", "cbDHCP", "setUseDHCP(this)");
		configDHCPBox = createEmptyDiv(configDHCPSection, "div", "", "");
			configDHCPBox.style.display = "none";
			tempObj = createEmptyDiv(configDHCPBox, "div", "tile-1", "");
				createTextInput(tempObj, "tile-1_4", "Static IP:", "n/a", "staticip", "setDHCP(this)");
				createTextInput(tempObj, "tile-1_4", "Gateway IP:", "n/a", "gatewayip", "setDHCP(this)");
			tempObj = createEmptyDiv(configDHCPBox, "div", "tile-1", "");
				createTextInput(tempObj, "tile-1_4", "Netmask:", "n/a", "netmask", "setDHCP(this)");
				createTextInput(tempObj, "tile-1_4", "DNS Server:", "n/a", "dnsserver", "setDHCP(this)");
		configAPModuleBox = createEmptyDiv(mainScrollBox, "div", "", "");
			createPageTitle(configAPModuleBox, "div", "tile-1", "", "h3", "Access Point Configuration");
//			tempObj = createEmptyDiv(configAPModuleBox, "div", "tile-1", "");
//				createCheckbox(tempObj, "tile-1_4", "Provide Access Point", "cbUseAP", "setUseAP(this)");
			configAPBox = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
				configAPBox.style.display = "none";
				tempObj = createEmptyDiv(configAPBox, "div", "tile-1", "");
					createTextInput(tempObj, "tile-1_4", "Access Point IP:", "n/a", "ap_ip", "setAP(this)");
					createTextInput(tempObj, "tile-1_4", "AP Password:", "n/a", "ap_password", "setAP(this)");



		configNTPSection = createEmptyDiv(mainScrollBox, "div", "", "");
		createPageTitle(configNTPSection, "div", "tile-1", "", "h3", "Network Time Setup");
		tempObj = createEmptyDiv(configNTPSection, "div", "tile-1", "");
			createCheckbox(tempObj, "tile-1_4", "Use Internet Time", "cbUseNTP", "setNTPStatus(this)");
		configNTPBox = createEmptyDiv(configNTPSection, "div", "", "");
				configNTPBox.style.display = "none";
				createTextInput(configNTPBox, "tile-1_4", "NTP Server:", "n/a", "ntpserverurl", "setNTPServer(this)");
				createTextInput(configNTPBox, "tile-1_4", "Timezone:", "0", "ntptimezone", "setNTPTimeZone(this)");

		tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
			createButton(tempObj, "", "Save & Restart", "btnSave", "saveSettings(this)");
			createButton(tempObj, "", "Cancel", "btnCancel", "cancelSettings(this)");
		tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
			createButton(tempObj, "", "Save to File", "btnDownload", "downloadSettings(this)");
			createFileDlg(tempObj, "", "Load from File", "btnLoad", ".json", "loadSettings(this)");

}

//function createFileDlg(parentObj, divclass, labelText, btnObjID, acceptfiles, onclick)

function loadNodeDataFields(jsonData)
{
}

function loadHatOptions(jsonData)
{
	var optionsArray = [];
	for (var i = 0; i < jsonData.HatTypeList.length; i++)
		optionsArray.push(jsonData.HatTypeList[i].Name);
	createOptions(document.getElementById("selecthattype"), optionsArray);
	setDropdownValue("selecthattype", jsonData.HatIndex);
}
function loadInterfaceOptions(jsonData)
{
//	console.log(jsonData);
	var optionsArray = [];
	for (var i = 0; i < jsonData.InterfaceTypeList.length; i++)
		optionsArray.push(jsonData.InterfaceTypeList[i].Name);
	createOptions(document.getElementById("selectcommtype"), optionsArray);
	setDropdownValue("selectcommtype", jsonData.InterfaceIndex);
}

function loadALMOptions(jsonData)
{
	var ALMBox = document.getElementById("ALMBox");
	var currDispBox;
	//delete entries
	while (ALMBox.hasChildNodes())
		ALMBox.removeChild(ALMBox.childNodes[0]); //delete rows
	for (var i = 0; i < jsonData.ALMTypeList.length; i++)
	{
		if ((i & 0x01) == 0)
			currDispBox = createEmptyDiv(ALMBox, "div", "tile-1", "");
//		if (jsonData.ALMTypeList[i].InterfaceList.indexOf(jsonData.InterfaceIndex) >= 0)
		{
			createCheckbox(currDispBox, "tile-1_4", jsonData.ALMTypeList[i].Name, "cbUseALM_" + i.toString(), "setUseALM(this)");
			writeCBInputField("cbUseALM_" + i.toString(), jsonData.ALMIndex.indexOf(jsonData.ALMTypeList[i].ALMId) >= 0);
			document.getElementById("cbUseALM_" + i.toString()).setAttribute("ALMId", jsonData.ALMTypeList[i].ALMId);
		}	
	}
}

function loadServerOptions(jsonData)
{
	var ServerBox = document.getElementById("ServerBox");
	var currDispBox;
	//delete entries
	while (ServerBox.hasChildNodes())
		ServerBox.removeChild(ServerBox.childNodes[0]); //delete rows
	for (var i = 0; i < jsonData.ServerTypeList.length; i++)
	{
		if ((i & 0x01) == 0)
			currDispBox = createEmptyDiv(ServerBox, "div", "tile-1", "");
		createCheckbox(currDispBox, "tile-1_4", jsonData.ServerTypeList[i].Name, "cbUseServer_" + i.toString(), "setUseServer(this)");
		writeCBInputField("cbUseServer_" + i.toString(), jsonData.ServerIndex.indexOf(jsonData.ServerTypeList[i].ServerId) >=0);
		document.getElementById("cbUseServer_" + i.toString()).setAttribute("ServerId", jsonData.ServerTypeList[i].ServerId);
	}
}

function loadDataFields(jsonData)
{
//	console.log("Loading ", jsonData);
	configData[workCfg] = upgradeJSONVersion(jsonData);

	loadHatOptions(jsonData);
	loadInterfaceOptions(jsonData);
	loadALMOptions(jsonData);
	loadServerOptions(jsonData);
	writeRBInputField("selectwifimode", jsonData.wifiMode-1);
//	writeCBInputField("cbUseWifi", jsonData.useWifiTimeout);
	writeInputField("nodename", jsonData.devName);
	writeCBInputField("cbUseMac", jsonData.inclMAC);
	writeCBInputField("cbDHCP", jsonData.useStaticIP);
	setVisibility(jsonData.useStaticIP, configDHCPBox, true);
	writeInputField("staticip", jsonData.staticConfig.staticIP);
	writeInputField("gatewayip", jsonData.staticConfig.staticGateway);
	writeInputField("netmask", jsonData.staticConfig.staticNetmask);
	writeInputField("dnsserver", jsonData.staticConfig.staticDNS);
	
	writeInputField("ap_ip", jsonData.apConfig.apGateway);
	writeInputField("ap_password", jsonData.apConfig.apPassword);

//	writeCBInputField("cbUseSubnet", jsonData.subnetMode);
	writeCBInputField("cbUseBushbyBit", jsonData.useBushby);
	if (typeof jsonData.useLissy !== 'undefined')
		writeCBInputField("cbUseLissyBit", jsonData.useLissy);
	if (typeof jsonData.broadcastFC !== 'undefined')
	{
		writeCBInputField("cbUseFastClock", jsonData.broadcastFC);
		writeInputField("fcrefresh", jsonData.broadcastFCRate);
	}
	else
	{
		writeCBInputField("cbUseFastClock", true);
		writeInputField("fcrefresh", 75);
	}
	writeCBInputField("cbUseFctRefresh", jsonData.refreshFct);
	writeCBInputField("cbUseNTP", jsonData.useNTP);
	setVisibility(jsonData.useNTP, configNTPBox);
	writeInputField("ntpserverurl", jsonData.ntpConfig.NTPServer);
	writeInputField("ntptimezone", jsonData.ntpConfig.ntpTimeZone);
	
	
	setDisplayOptions();
}
