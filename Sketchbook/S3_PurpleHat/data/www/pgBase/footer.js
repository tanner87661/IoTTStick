function constructFooterContent(footerTab)
{
	var tempObj;
	tempObj = createEmptyDiv(footerTab, "div", "tile-1_4", "footerstatsdiv1");
		createDispText(tempObj, "", "Date / Time:", "n/a", "sysdatetime");
		createDispText(tempObj, "", "System Uptime:", "n/a", "uptime");
		createDispText(tempObj, "", "Core Temp.:", "n/a", "temp");
		createDispText(tempObj, "", "Ext. Voltage:", "n/a", "uin");
	tempObj = createEmptyDiv(footerTab, "div", "tile-1_4", "footerstatsdiv2");
		createDispText(tempObj, "", "IP Address:", "n/a", "IPID");
		createDispText(tempObj, "", "Signal Strength:", "n/a", "SigStrengthID");
		createDispText(tempObj, "", "Access Point:", "n/a", "SSID");
	tempObj = createEmptyDiv(footerTab, "div", "tile-1_4", "footerstatsdiv3");
		createDispText(tempObj, "", "Firmware Version:", "n/a", "firmware");
		createDispText(tempObj, "", "Available RAM/Flash:", "n/a", "heapavail");
		createDispText(tempObj, "", "Bat. Voltage:", "n/a", "ubat");
		createDispText(tempObj, "", "Bat. Current:", "n/a", "ibat");
}

function processStatsData(jsonData)
{
//	console.log(jsonData);
	writeTextField("sysdatetime", jsonData.systime);
	writeTextField("uptime", formatTime(Math.trunc(jsonData.uptime/1000)));
	writeTextField("IPID", jsonData.ipaddress);
	writeTextField("SigStrengthID", jsonData.sigstrength + " dBm");
	writeTextField("SSID", jsonData.apname);
	writeTextField("firmware", jsonData.version);
	writeTextField("heapavail", jsonData.freemem + " / " + jsonData.freedisk + " Bytes");
	writeTextField("temp", jsonData.temp.toFixed(2) + "\u00B0C");
	writeTextField("uin", jsonData.uin.toFixed(2) + "V");
	writeTextField("ubat", jsonData.ubat.toFixed(2) + "V");
	writeTextField("ibat", jsonData.ibat.toFixed(2) + "mA");
}

