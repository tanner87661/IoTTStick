var mainScrollBox;
var pingDelay;

function downloadSettings(sender)
{
	downloadConfig(0x0004); //send just this
}

function setPortSettings(sender)
{
	console.log(sender);
	if (sender.id == "selectrate")
	{
		var e = sender.selectedIndex;
		if (e >= 0)
			configWorkData.BaudRate = parseInt(sender.options[e].text); 
	}
}

function constructFooterContent(footerTab)
{
	var tempObj;
	tempObj = createEmptyDiv(footerTab, "div", "tile-1_4", "footerstatsdiv1");
		createDispText(tempObj, "", "Date / Time", "n/a", "sysdatetime");
		createDispText(tempObj, "", "System Uptime", "n/a", "uptime");
	tempObj = createEmptyDiv(footerTab, "div", "tile-1_4", "footerstatsdiv2");
		createDispText(tempObj, "", "IP Address", "n/a", "IPID");
		createDispText(tempObj, "", "Signal Strength", "n/a", "SigStrengthID");
	tempObj = createEmptyDiv(footerTab, "div", "tile-1_4", "footerstatsdiv3");
		createDispText(tempObj, "", "Firmware Version", "n/a", "firmware");
		createDispText(tempObj, "", "Available RAM/Flash", "n/a", "heapavail");
}

function constructPageContent(contentTab)
{
	var tempObj;
	mainScrollBox = createEmptyDiv(contentTab, "div", "pagetopicboxscroll-y", "nodeconfigdiv");
		createPageTitle(mainScrollBox, "div", "tile-1", "BasicCfg_Title", "h1", "USB Interface Configuration");

			createPageTitle(mainScrollBox, "div", "tile-1", "", "h2", "Basic Settings");
			tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
				createDropdownselector(tempObj, "tile-1_4", "Baud Rate:", ["57600","115200","230400","250000","288000","333333","460800"], "selectrate", "setPortSettings(this)");

		tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
			createButton(tempObj, "", "Save & Restart", "btnSave", "saveSettings(this)");
			createButton(tempObj, "", "Cancel", "btnCancel", "cancelSettings(this)");
		tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
			createButton(tempObj, "", "Save to File", "btnDownload", "downloadSettings(this)");

}

function loadNodeDataFields(jsonData)
{
}

function processStatsData(jsonData)
{
	writeTextField("sysdatetime", jsonData.systime);
	writeTextField("uptime", formatTime(Math.trunc(jsonData.uptime/1000)));
	writeTextField("IPID", jsonData.ipaddress);
	writeTextField("SigStrengthID", jsonData.sigstrength + " dBm");
	writeTextField("firmware", jsonData.version);
	writeTextField("heapavail", jsonData.freemem + " / " + jsonData.freedisk + " Bytes");
}

function loadDataFields(jsonData)
{
	console.log(jsonData);
	setDropdownValueByText("selectrate", jsonData.BaudRate.toString());
}
