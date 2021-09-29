var mainScrollBox;
var pingDelay;

function saveConfigFileSettings()
{
	//step 1: save greenhat.cfg
	saveJSONConfig(scriptList.Pages[currentPage].ID, scriptList.Pages[currentPage].FileName, configData[workCfg], null);
}

function downloadSettings(sender)
{
	downloadConfig(0x0800); //send just this
}

function setUseKeyword(sender)
{
	console.log(sender);
	if (sender.id == "cbUseStop")
		configData[2].UseStop = sender.checked; 
	if (sender.id == "cbUseGo")
		configData[2].UseGo = sender.checked; 
}

function constructPageContent(contentTab)
{
	var tempObj;
	mainScrollBox = createEmptyDiv(contentTab, "div", "pagetopicboxscroll-y", "nodeconfigdiv");
		createPageTitle(mainScrollBox, "div", "tile-1", "BasicCfg_Title", "h1", "Voice Watcher Configuration");

			createPageTitle(mainScrollBox, "div", "tile-1", "", "h2", "Select Keywords to listen for");
			tempObj = createEmptyDiv(mainScrollBox, "div", "tile-1", "");
			createCheckbox(tempObj, "tile-1_4", "STOP", "cbUseStop", "setUseKeyword(this)");
			createCheckbox(tempObj, "tile-1_4", "GO", "cbUseGo", "setUseKeyword(this)");

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
	writeCBInputField("cbUseStop", jsonData.UseStop);
	writeCBInputField("cbUseGo", jsonData.UseGo);
}
