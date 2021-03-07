var mainScrollBox;
var pingDelay;

function saveConfigFileSettings()
{
	//step 1: save greenhat.cfg
	saveJSONConfig(scriptList.Pages[currentPage].ID, scriptList.Pages[currentPage].FileName, configData[workCfg], null);
}

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
			configData[2].BaudRate = parseInt(sender.options[e].text); 
	}
}

function constructPageContent(contentTab)
{
	var tempObj;
	mainScrollBox = createEmptyDiv(contentTab, "div", "pagetopicboxscroll-y", "nodeconfigdiv");
		createPageTitle(mainScrollBox, "div", "tile-1", "BasicCfg_Title", "h1", "USB Serial Interface Configuration");

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

function loadDataFields(jsonData)
{
	console.log(jsonData);
	setDropdownValueByText("selectrate", jsonData.BaudRate.toString());
}
