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
	setVisibility((configWorkData.wifiMode & 0x02) > 0, configAPModuleBox);
	setVisibility((configWorkData.wifiMode & 0x02) > 0, configAPBox);
	setVisibility((configWorkData.wifiMode & 0x01) > 0, configNTPSection);
	setVisibility(configWorkData.useNTP, configNTPBox);
	setVisibility((configWorkData.wifiMode & 0x01) > 0, configDHCPSection);
	setVisibility(configWorkData.useStaticIP, configDHCPBox);
	setVisibility((configWorkData.InterfaceIndex > 1) && (configWorkData.InterfaceIndex != 9), configBushbyBox);
}

function setProdType(sender)
{

	function  verifyALM(item, index) 
	{
		if (configWorkData.ALMTypeList[item].Type > configWorkData.InterfaceTypeList[oldCommMode].Type)
			configWorkData.ALMIndex.splice(index,1);
	}

	var oldHatMode = configWorkData.HatIndex;
	var oldCommMode = configWorkData.InterfaceIndex;
	if (sender.id == "selecthattype")
		oldHatMode = sender.selectedIndex;
	if (sender.id == "selectcommtype")
		oldCommMode = sender.selectedIndex;
		
	if (configWorkData.HatTypeList[oldHatMode].InterfaceList.indexOf(configWorkData.InterfaceTypeList[oldCommMode].IntfId) < 0) 
	{
		alert("Invalid Combination of Hat and Interface");
		if (sender.id == "selecthattype")
			setDropdownValue("selecthattype", configWorkData.HatIndex);
		if (sender.id == "selectcommtype")
			setDropdownValue("selectcommtype", configWorkData.InterfaceIndex);
		return;
	}

	if ((configWorkData.InterfaceTypeList[oldCommMode].ReqSTA == 1) && (configWorkData.wifiMode == 2))
	{
		alert("This interface requires active WiFi connection");
		if (sender.id == "selectcommtype")
			setDropdownValue("selectcommtype", configWorkData.InterfaceIndex);
		return;
	}
	configWorkData.ALMIndex.forEach(verifyALM);
	loadALMOptions(configWorkData);

	configWorkData.HatIndex = oldHatMode;
	configWorkData.InterfaceIndex = oldCommMode;
	if (configWorkData.InterfaceIndex >= 2)
	{
//		configWorkData.useWifiTimeout = false;
//		writeCBInputField("cbUseWifi", false);
	}	
	setDisplayOptions();
	console.log(configWorkData);
}

function setUseALM(sender)
{
	
	function checkIndex(e) 
	{
		return e.ALMId == ALMId;
	}
	
	var oldCommMode = configWorkData.InterfaceIndex;
	var ALMId = parseInt(sender.getAttribute("ALMId"));
	var ALMIndex = configWorkData.ALMTypeList.findIndex(checkIndex);
	if (sender.checked)
	{
		console.log(oldCommMode, ALMId, ALMIndex);
		if ((configWorkData.InterfaceTypeList[oldCommMode].Type >= configWorkData.ALMTypeList[ALMIndex].Type))
		{
			if (configWorkData.ALMIndex.indexOf(ALMId) < 0)
				configWorkData.ALMIndex.push(ALMId);
			else
			{}
		}
		else
		{
			alert("This Logic Module does not work with the selected command source");
			sender.checked = false;
		}
	}
	else
	{
		var elPos = configWorkData.ALMIndex.indexOf(ALMId);
		if (elPos >= 0)
			configWorkData.ALMIndex.splice(elPos,1);
	}
	console.log(configWorkData.ALMIndex);
}

function setWifiStatus(sender)
{
	configWorkData.useWifiTimeout = sender.checked ? 1:0;
	setDisplayOptions();
}

function setNTPStatus(sender)
{
	configWorkData.useNTP = sender.checked ? 1:0;
	setDisplayOptions();
}

function setNTPServer(sender)
{
	configWorkData.ntpConfig.NTPServer = sender.value;
}

function setNTPTimeZone(sender)
{
	configWorkData.ntpConfig.ntpTimeZone = sender.value;
}

function setNodeName(sender)
{
	configWorkData.devName = sender.value;
}

function setUseMac(sender)
{
	configWorkData.inclMAC = sender.checked ? 1:0;
}

function setUseDHCP(sender)
{
	configWorkData.useStaticIP = sender.checked ? 1:0;
	setDisplayOptions();
}

function setDHCP(sender)
{
	if (sender.id == "staticip")
		configWorkData.staticConfig.staticIP = sender.value;
	if (sender.id == "gatewayip")
		configWorkData.staticConfig.staticGateway = sender.value;
	if (sender.id == "netmask")
		configWorkData.staticConfig.staticNetmask = sender.value;
	if (sender.id == "dnsserver")
		configWorkData.staticConfig.staticDNS = sender.value;
	console.log(configWorkData.staticConfig);
}

function setAP(sender)
{
	if (sender.id == "ap_ip")
		configWorkData.apConfig.apGateway = sender.value;
	if (sender.id == "ap_password")
		configWorkData.apConfig.apPassword = sender.value;
	console.log(configWorkData.apConfig);
}

function setWifiMode(sender, id)
{
	if (sender.id == "selectwifimode_0")
		configWorkData.wifiMode = 1;
	if (sender.id == "selectwifimode_1")
	{
		if (configWorkData.InterfaceTypeList[configWorkData.InterfaceIndex].ReqSTA == 1)
		{
			alert("This interface requires active WiFi connction");
			writeRBInputField("selectwifimode", configWorkData.wifiMode-1);
			return;
		}
		configWorkData.wifiMode = 2;
	}
	setDisplayOptions();
}

function setUseBushbyBit(sender)
{
	configWorkData.useBushby = sender.checked ? 1:0;
}

function constructPageContent(contentTab)
{
	var tempObj;
	mainScrollBox = createEmptyDiv(contentTab, "div", "pagetopicboxscroll-y", "nodeconfigdiv");
		createPageTitle(mainScrollBox, "div", "tile-1", "", "h1", "Node Configuration");
		createPageTitle(mainScrollBox, "div", "tile-1", "", "h2", "Basic Setup");
		tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
			createDropdownselector(tempObj, "tile-1_4", "Command Source:", [], "selectcommtype", "setProdType(this, id)");
			createDropdownselector(tempObj, "tile-1_4", "Hat Module:", [], "selecthattype", "setProdType(this, id)");
		tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "configBushbyBox");
			createCheckbox(tempObj, "tile-1_4", "respect Bushby Bit", "cbUseBushbyBit", "setUseBushbyBit(this)");
		
		createPageTitle(mainScrollBox, "div", "tile-1", "", "h2", "Embedded Logic Modules Activation");
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
		createCheckbox(currDispBox, "tile-1_4", jsonData.ALMTypeList[i].Name, "cbUseALM_" + i.toString(), "setUseALM(this)");
		writeCBInputField("cbUseALM_" + i.toString(), jsonData.ALMIndex.indexOf(jsonData.ALMTypeList[i].ALMId) >=0);
		document.getElementById("cbUseALM_" + i.toString()).setAttribute("ALMId", jsonData.ALMTypeList[i].ALMId);
	}
}

function loadDataFields(jsonData)
{
//	console.log(jsonData);

	loadHatOptions(jsonData);
	loadInterfaceOptions(jsonData);
	loadALMOptions(jsonData);
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

	writeCBInputField("cbUseBushbyBit", jsonData.useBushby);
	
	writeCBInputField("cbUseNTP", jsonData.useNTP);
	setVisibility(jsonData.useNTP, configNTPBox);
	writeInputField("ntpserverurl", jsonData.ntpConfig.NTPServer);
	writeInputField("ntptimezone", jsonData.ntpConfig.ntpTimeZone);
	
	
	setDisplayOptions();
}
