var locoDef; //parsed JMRI XML data
var validLocoDef = false; //true if JMRI file is loaded
var fileName = "";
var throttleDef = {"DataType" : "ThrottleDef", "Version" : "1.0", "ProfName":"new profile", "VMax":80, "AtStep":90, "SpeedSteps":100, "GraphData" : {}, "FileName":""};
var validThrottleDef = true;
var validTechSpeedDef = false; //true after speed analysis
var validTableDef = false; //true after table caclulation

var throttleProfileGraph;
var techSpeedProfileGraph;
var speedTableProfileGraph;
//var speedVerifyProfileGraph;

var speedProfileDef = {"DataType" : "TechSpeedDef", "Version" : "1.0", "FileName":"", "ProfName":"new profile", "VMax":80, "GraphData" : {}};
//var validSpeedProfile = false;

var canvasSizeThrottle = [screen.width/2, 400, 50, 50, 150, 50]; //width, height, frame size top, bottom, left, right
var canvasSizeTechSpeed = [screen.width/2, 400, 50, 50, 150, 50];
var canvasSizeSpeedTable = [screen.width/2, 400, 50, 50, 150, 50];
//var canvasSizeSpeedVerify = [screen.width/2, 400, 50, 50, 150, 50];

var canvasElementThrottle;
var canvasElementTechSpeed;
var canvasElementSpeedTable;
//var canvasElementSpeedVerify;

var graphBackgroundThrottle = bgColor;
var graphBackgroundTechSpeed = bgColor;
var graphBackgroundSpeedTable =bgColor;
//var graphBackgroundSpeedVerify =bgColor;

var graphBoxThrottle = "rgb(255,50,50)";
var graphBoxTechSpeed = "rgb(30,128,30)";
var graphBoxSpeedTable = "rgb(50,50,255)";
//var graphBoxSpeedVerify = "rgb(50,50,255)";

var graphColorThrottle = "rgb(150,150,250)";
var graphColorSpeedDot = "rgb(150,150,0)";
var graphLineThrottle = 3;
var graphColorMinMax1 = "rgb(255, 50, 50)";
var graphColorMinMax2 = "rgb(30, 128, 30)";
var graphColorMinMax3 = "rgb(50, 50, 255)";

var graphColorTechProfileSpeedBw = "rgb(30,128,30)";
var graphColorTechProfileSpeedFw = "rgb(128,128,30)";
var graphColorTechProfileSpeedLimit = "rgb(128,128,128)";
var graphLineTechSpeed = 1;
var graphColorSpeedTable = "rgb(255,50,255)";
var graphHLColorThrottle = "rgb(255,150,150)";
var graphHLColorTechProfileSpeed = "rgb(130,128,130)";
var graphHLColorSpeedTable = "rgb(150,150,255)";
var graphLineSpeedTable = 2;

var profileGraphTemplate = {"DimX":"%", "DimY":"km/h", "mouseOverLine" : -1, "Color":"blue", "Width": 1, "HLColor":"green", "ValsX":[0,100,10], "ValsY":[0,160,10], "Show" : true,  "DataElements":[]};
var DataElementTemplate = {"x":0,"y":0, "m":0};
var SpeedGraphTemplate = {"x":0, "Min1y":0, "Max1y":0, "Last1y":0, "Min2y":0, "Max2y":0, "Last2y":0, "Min3y":0, "Max3y":0, "Last3y":0, "m":1, "evtCtr" : 0, "graphColor" : "rgb(150,150,0)"};
var profileTemplate = {"Title":"title", "DispDim":[0,0], "FieldCol": "rgb(100,100,100)", "BoxCol": "rgb(100,100,100)", "LineGraphs":[]};

var xmlFile = "";
var jsonFileThrottle = "";
var jsonFileSpeedProfile = "";

var cvId = -1;
var cvVal = -1;
var cvArray = [];

var cvVal_2 = 0;
var cvVal_5 = 0;
var cvVal_6 = 0;
var cvVal_29 = 0;

//var tableMode = 0; //table mode
//var trimMode = 0; //trim mode 0-200 vs 0-100 ->moved to phcfg.cfg
var stepMode = 1;
var moveNode = -1; //selected node for dragging
//--------------------------------------------------------------JMRI Decoder functions
function loadJMRIDecoder(sender)
{
	var fileInput = document.getElementById("btnLoadDecoder");
	var fileName = fileInput.files[0];
	var reader = new FileReader();
	fileInput.value = "";
	var parser;
    reader.onload = function()
    {
        try 
        {
			parser = new DOMParser();
			locoDef = parser.parseFromString(reader.result,"text/xml");
//			console.log(reader.result);
		}
		catch(err) 
		{
			fileName = "";
			alert("Configuration data not valid");
			return;
		}
        validLocoDef = displayDecoder(locoDef);
		setButtonStatus();
    };
    xmlFile = fileName;
    reader.readAsText(fileName);	
}

function saveJMRIDecoder(sender)
{
	if (validLocoDef)
		writeDiskFile(new Blob([xml2Str(locoDef)], {type: "text/xml"}), xmlFile.name);
}

function xml2Str(xmlNode) {
   try {
      // Gecko- and Webkit-based browsers (Firefox, Chrome), Opera.
      return (new XMLSerializer()).serializeToString(xmlNode);
  }
  catch (e) {
     try {
        // Internet Explorer.
        return xmlNode.xml;
     }
     catch (e) {  
        //Other browsers without XML Serializer
        alert('XML Serializer not supported');
     }
   }
   return false;
}

function findXMLAttribute(inCollection, thisAttr, attrVal)
{
	for (var i = 0; i < inCollection.length; i++)
	{
//		console.log(i, inCollection[i].getAttribute(thisAttr));
		if (inCollection[i].getAttribute(thisAttr) == attrVal)
			return inCollection[i];
	}
	return null;
}

function displayDecoder(xmlNode)
{
	var isLoco = xmlNode.getElementsByTagName("locomotive-config").length > 0;
	var varVals = xmlNode.getElementsByTagName("varValue");
	var hasSpeedTable = -1;
	var cvVal = [];
	if (varVals.length > 0)
	{
//		console.log(varVals.length);
		var attr = findXMLAttribute(varVals, "item", "Speed Table Definition");
//		console.log(attr);
		if (attr)
			hasSpeedTable = attr.getAttribute("value");
		if (hasSpeedTable == 1)
		{
			cvVal = xmlNode.getElementsByTagName("CVvalue");
			speedTableProfileGraph.LineGraphs[0].DataElements.length = 0;//delete old data
			var newEl = JSON.parse(JSON.stringify(DataElementTemplate));
			newEl.x = 0;
			newEl.y = 0;
			speedTableProfileGraph.LineGraphs[0].DataElements.push(newEl);
//			console.log(xmlNode);
			for (var i = 0; i < 28; i++) //28 table entries by definition
			{
				var cv = findXMLAttribute(cvVal, "name", (i+67).toString());
				if (cv)
				{
					newEl = JSON.parse(JSON.stringify(DataElementTemplate));
					newEl.x = i+1;
					newEl.y = cv.getAttribute("value");
					speedTableProfileGraph.LineGraphs[0].DataElements.push(newEl);
				}
			}
		}
	}
	drawProfileBox(canvasElementSpeedTable, speedTableProfileGraph);
	if (isLoco)
	{
		document.getElementById("jmrifilename").innerHTML = xmlNode.getElementsByTagName("locomotive")[0].getAttribute("fileName");
		document.getElementById("dccloco").innerHTML = xmlNode.getElementsByTagName("locomotive")[0].getAttribute("id");
		document.getElementById("jmriaddr").innerHTML = xmlNode.getElementsByTagName("locomotive")[0].getAttribute("dccAddress");
		document.getElementById("decbrand").innerHTML = findCVVal(xmlNode, 8);
		document.getElementById("decfamily").innerHTML = xmlNode.getElementsByTagName("decoder")[0].getAttribute("family");
		document.getElementById("decmodel").innerHTML = xmlNode.getElementsByTagName("decoder")[0].getAttribute("model");
		writeTextField("cv02", findCVVal(xmlNode, 2));
		writeTextField("cv05", findCVVal(xmlNode, 5));
		writeTextField("cv06", findCVVal(xmlNode, 6));
		writeTextField("cv29", findCVVal(xmlNode, 29));
		writeInputField("cvn02", findCVVal(xmlNode, 2));
		writeInputField("cvn05", findCVVal(xmlNode, 5));
		writeInputField("cvn06", findCVVal(xmlNode, 6));
		writeInputField("cvn29", findCVVal(xmlNode, 29));
		cvVal_2 = findCVVal(xmlNode, 2);
		cvVal_5 = findCVVal(xmlNode, 5);
		cvVal_6 = findCVVal(xmlNode, 6);
		cvVal_29 = findCVVal(xmlNode, 29);
		
		var vMax = xmlNode.getElementsByTagName("locomotive")[0].getAttribute("maxSpeed");
		writeInputField("thscalespeed", vMax);
		document.getElementById("jmrivmax").innerHTML = vMax;
		setThrottleProfile(document.getElementById("thscalespeed"));
		var newAddr = xmlNode.getElementsByTagName("locomotive")[0].getAttribute("dccAddress");
		locoAddr = -1;
		reqDCCAssign(newAddr);
	}
	else
		console.log("Not Locomotive data");
	return isLoco;
}

function findCVVal(xmlNode, cvNr)
{
//	return speedTableProfileGraph.LineGraphs[0].DataElements[cvNr].y;
	
	var cvVals = xmlNode.getElementsByTagName("CVvalue");
	for (var i = 0; i < cvVals.length; i++)
		if (cvVals[i].getAttribute("name") == cvNr) 
			return cvVals[i].getAttribute("value");
	return -1;
}

function setCV(sender)
{
	switch (sender.id)
	{
		case "cvid":
			cvId = verifyNumber(sender.value, cvId);
			break;
		case "cvval":
			cvVal = verifyNumber(sender.value, cvVal);
			break;
	}
}

function setCVVal(sender)
{
	switch (sender.id)
	{
		case "cvn02":
			cvVal_2 = verifyNumber(sender.value, cvVal_2);
			break;
		case "cvn05":
			cvVal_5 = verifyNumber(sender.value, cvVal_5);
			break;
		case "cvn06":
			cvVal_6 = verifyNumber(sender.value, cvVal_6);
			break;
		case "cvn29":
			cvVal_29 = verifyNumber(sender.value, cvVal_29);
			break;
	}
}

//--------------------------------------------------------------Throttle Profile functions
function getMotorValueForSpeed(thisSpeed)
{
	var motorValue = [0,0];
	for (var j = 1; j <= 2; j++) //techSpeedProfileGraph.LineGraphs
	{
	//find speedstep that has desired speed
		for (var i = 0; i < techSpeedProfileGraph.LineGraphs[j].DataElements.length-1; i++)
			if ((thisSpeed >= techSpeedProfileGraph.LineGraphs[j].DataElements[i].y) && ( thisSpeed <= techSpeedProfileGraph.LineGraphs[j].DataElements[i+1].y))
			{
//				console.log(thisSpeed, i);
				var y1 = techSpeedProfileGraph.LineGraphs[j].DataElements[i].y
				var y2 = techSpeedProfileGraph.LineGraphs[j].DataElements[i+1].y
				var dy = y2 - y1;
				var dx = 127 / 27; 
//				console.log(thisSpeed, j, i, y1, y2, dy);
				//for 27 step mode, interpolate to 127 step
				if (techSpeedProfileGraph.LineGraphs[j].DataElements.length < 30)
				{
					if ((i == 0) || (dy == 0))
						motorValue[j-1] = 0;
					else
						motorValue[j-1] = (i * dx) + (((thisSpeed - y1) / dy) * dx);
//						motorValue[j-1] = Math.round((i * dx) + (((thisSpeed - y1) / dy) * dx));
				}
				else
					if (i === 0)
						motorValue[j-1] = 0;
					else
					{
//						if (dy === 0)
//							motorValue[j-1] = 127;
//						else
						motorValue[j-1] = i + ((thisSpeed - y1) / dy);
//						console.log(i + ((thisSpeed - y1) / dy), i);
					}
//						motorValue[j-1] = thisSpeed > (y1 + (dy / 2)) ? i + 1 : i;
				//times 2 for motor value
				motorValue[j-1] = (motorValue[j-1] * 2); //adjust from 127 to 255 max value
//				console.log(motorValue);
				break;
			}
			else
				if ((thisSpeed > techSpeedProfileGraph.LineGraphs[j].DataElements[i].y) && (techSpeedProfileGraph.LineGraphs[j].DataElements[i].y == techSpeedProfileGraph.LineGraphs[j].DataElements[i+1].y))
					motorValue[j-1] = 255;
	}
	return motorValue;
}

function getSpeedForThrottleStep(thisStep)
{
//	var tStep = Math.round((thisStep / 28) * throttleProfileGraph.LineGraphs[0].ValsX[1]);
	var tStep = (thisStep / 28) * throttleProfileGraph.LineGraphs[0].ValsX[1];
	var lineSegment = 0;
	for (var i = 0; i < throttleProfileGraph.LineGraphs[0].DataElements.length-1; i++)
		if ((tStep >= throttleProfileGraph.LineGraphs[0].DataElements[i].x) && ( tStep <= throttleProfileGraph.LineGraphs[0].DataElements[i+1].x))
		{
			lineSegment = i;
			break;
		}
	var x1 = throttleProfileGraph.LineGraphs[0].DataElements[lineSegment].x;
	var y1 = throttleProfileGraph.LineGraphs[0].DataElements[lineSegment].y;
	var dx = throttleProfileGraph.LineGraphs[0].DataElements[lineSegment+1].x - throttleProfileGraph.LineGraphs[0].DataElements[lineSegment].x;
	var dy = throttleProfileGraph.LineGraphs[0].DataElements[lineSegment+1].y - throttleProfileGraph.LineGraphs[0].DataElements[lineSegment].y;
	var scaleSpeed = y1 + ((tStep - x1) * dy/dx);
//	console.log(thisStep, tStep.toFixed(2), lineSegment, x1, y1, dx, dy);
    if (configData[workCfg].Units == 1) //imperial
		scaleSpeed *= 1.6;
	scaleSpeed /= configData[workCfg].ScaleList[configData[workCfg].ScaleIndex].Scale;
//	scaleSpeed = Math.round((scaleSpeed * 10000) / 36); //[mm/s]
	scaleSpeed = ((scaleSpeed * 10000) / 36); //[mm/s]
	return scaleSpeed;
}

function createThrottleGraph(throttleDef, myThrottleGraph)
{
	
	myThrottleGraph.LineGraphs[0].ValsY[1] = 50 * (Math.trunc(throttleDef.VMax/50) + 1); 
	myThrottleGraph.LineGraphs[0].ValsX[1] = throttleDef.SpeedSteps;
//	console.log(myThrottleGraph.LineGraphs[0].DataElements.length);
	while (myThrottleGraph.LineGraphs[0].DataElements.length < 3)
	{
		var newEl = JSON.parse(JSON.stringify(DataElementTemplate));
		myThrottleGraph.LineGraphs[0].DataElements.push(newEl); //add elements to minimum length
	}
	var currEl = myThrottleGraph.LineGraphs[0].DataElements[0];
	currEl.x = 0;
	currEl.y = 0;
	currEl = myThrottleGraph.LineGraphs[0].DataElements[myThrottleGraph.LineGraphs[0].DataElements.length-2];
	currEl.x = throttleDef.AtStep;
	currEl.y = throttleDef.VMax;
	currEl.m = 2;
	currEl = myThrottleGraph.LineGraphs[0].DataElements[myThrottleGraph.LineGraphs[0].DataElements.length-1];
	currEl.x = myThrottleGraph.LineGraphs[0].ValsX[1];
	currEl.y = throttleDef.VMax;
	myThrottleGraph.SpeedGraph = JSON.parse(JSON.stringify(SpeedGraphTemplate));
	myThrottleGraph.SpeedGraph.graphColor = graphColorSpeedDot;
	return myThrottleGraph;
}

function setThrottleProfile(sender)
{
	if (sender.id == "thprofilename")
	{
		throttleDef.ProfName = sender.value;
	}
	if (sender.id == "thscalespeed")
	{
		throttleDef.VMax = verifyNumber(sender.value, throttleDef.VMax);
		throttleProfileGraph = createThrottleGraph(throttleDef, throttleProfileGraph);
		drawProfileBox(canvasElementThrottle, throttleProfileGraph);
		document.getElementById("testmaxspeed").value = throttleDef.VMax;
		setTestSpeed(document.getElementById("testmaxspeed"));
	}
	if (sender.id == "thvmax")
	{
		throttleDef.AtStep = Math.trunc(verifyNumber(sender.value, throttleDef.AtStep));
		if (throttleDef.AtStep > throttleDef.SpeedSteps)
			throttleDef.AtStep = throttleDef.SpeedSteps;
		throttleProfileGraph = createThrottleGraph(throttleDef, throttleProfileGraph);
		drawProfileBox(canvasElementThrottle, throttleProfileGraph);
	}
	if (sender.id == "thnumsteps")
	{
		throttleDef.SpeedSteps = verifyNumber(sender.value, throttleDef.SpeedSteps);
		throttleProfileGraph.LineGraphs[0].ValsX[1] = throttleDef.SpeedSteps;
		throttleProfileGraph = createThrottleGraph(throttleDef, throttleProfileGraph);
		drawProfileBox(canvasElementThrottle, throttleProfileGraph);
	}
}

function setTechProfile(sender)
{
	speedProfileDef.ProfName = sender.value;
}

function saveProfile(sender)
{
	if (validTechSpeedDef)
	{
		fileName.name = speedProfileDef.FileName;
		writeDiskFile(new Blob([JSON.stringify(speedProfileDef)], {type: "application/json"}), jsonFileSpeedProfile.name);
	}
}

function loadProfile(sender)
{
	var fileInput = document.getElementById("btnLoadProfile");
	var fileName = fileInput.files[0];
	var reader = new FileReader();
	fileInput.value = "";
    reader.onload = function()
    {
        try 
        {
			speedProfileDef = JSON.parse(reader.result);
			speedProfileDef.FileName = fileName.name;
		}
		catch(err) 
		{
			speedProfileDef.FileName = "";
			alert("Not a valid speed profile");
			return;
		}
        validTechSpeedDef = speedProfileDef.DataType == "TechSpeedDef";
        if (!validTechSpeedDef)
        {
			speedProfileDef.FileName = "";
			alert("Not a valid speed profile");
			return;
		}
		displayTechSpeedDef(speedProfileDef);
        validTableDef = false;
		processSpeedTableInput(speedProfileDef.GraphData, false);
		setButtonStatus();
    };
    reader.onerror = function() 
    {
          alert(reader.error);
    }
    jsonFileSpeedProfile = fileName;
    reader.readAsText(fileName);	
}

function displayTechSpeedDef(jsonNode)
{
	writeInputField("techprofilename", jsonNode.ProfName);
	writeTextField("techprofilefilename", jsonNode.FileName);
}

function saveThrottle(sender)
{
	if (validThrottleDef)
	{
		fileName.name = throttleDef.FileName;
		throttleDef.GraphData = JSON.parse(JSON.stringify(throttleProfileGraph.LineGraphs[0]));
		writeDiskFile(new Blob([JSON.stringify(throttleDef)], {type: "application/json"}), jsonFileThrottle.name);
	}
}

function loadThrottle(sender)
{
	var fileInput = document.getElementById("btnLoadThrottle");
	var fileName = fileInput.files[0];
	var reader = new FileReader();
	fileInput.value = "";
    reader.onload = function()
    {
        try 
        {
			throttleDef = JSON.parse(reader.result);
			throttleDef.GraphData.DimY = (configData[workCfg].Units == 0) ? "km/h" : "mph";
			throttleDef.FileName = fileName.name;
//			console.log("Read Throttle");
		}
		catch(err) 
		{
			throttleDef.fileName = "";
			alert("Not a valid throttle profile");
			return;
		}
        validThrottleDef = throttleDef.DataType == "ThrottleDef";
        if (!validThrottleDef)
        {
			throttleDef.FileName = "";
			alert("Not a valid throttle profile");
			return;
		}
		throttleProfileGraph.LineGraphs[0] = JSON.parse(JSON.stringify(throttleDef.GraphData));
		throttleProfileGraph.LineGraphs[0].Color = JSON.parse(JSON.stringify(graphColorThrottle));
		throttleProfileGraph.LineGraphs[0].Width = JSON.parse(JSON.stringify(graphLineThrottle));
		throttleProfileGraph = createThrottleGraph(throttleDef, throttleProfileGraph);
        displayThrottleDef(throttleDef);
		drawProfileBox(canvasElementThrottle, throttleProfileGraph);
		setButtonStatus();
    };
    reader.onerror = function() 
    {
          alert(reader.error);
    }
    jsonFileThrottle = fileName;
    reader.readAsText(fileName);	
}

function displayThrottleDef(jsonNode)
{
	writeTextField("throttlefilename", jsonNode.FileName);
	writeInputField("thprofilename", jsonNode.ProfName);
	writeInputField("thscalespeed", jsonNode.VMax);
	
	writeInputField("thvmax", jsonNode.AtStep);
	writeInputField("thnumsteps", jsonNode.SpeedSteps);

	writeInputField("testmaxspeed", jsonNode.VMax);
	setTestSpeed(document.getElementById("testmaxspeed"));
	
}



//--------------------------------------------------------------Graph Data
function loadSpeedMagicGraphData()
{
	throttleProfileGraph = JSON.parse(JSON.stringify(profileTemplate));
	throttleProfileGraph.Title = "Throttle Step to Scale Speed";
	throttleProfileGraph.DispDim = canvasSizeThrottle;
	throttleProfileGraph.BoxCol = graphBoxThrottle;
	throttleProfileGraph.FieldCol = graphBackgroundThrottle;

	var profileBox = JSON.parse(JSON.stringify(profileGraphTemplate));
	profileBox.DimX = "Throttle Step";
	profileBox.DimY = (configData[workCfg].Units == 0) ? "km/h" : "mph";
	profileBox.Width = graphLineThrottle;
	profileBox.Color = graphColorThrottle;
	profileBox.HLColor = graphHLColorThrottle;
	profileBox.ValsX = [0,throttleDef.SpeedSteps,10];
	profileBox.ValsY = [0,50 * (Math.trunc(throttleDef.VMax/50) + 1),10];
	throttleProfileGraph.LineGraphs.push(profileBox);


	throttleProfileGraph = createThrottleGraph(throttleDef, throttleProfileGraph);

	techSpeedProfileGraph = JSON.parse(JSON.stringify(profileTemplate));
	techSpeedProfileGraph.Title = "Technical Speed Profile";
	techSpeedProfileGraph.DispDim = canvasSizeTechSpeed;
	techSpeedProfileGraph.BoxCol = graphBoxTechSpeed;
	techSpeedProfileGraph.FieldCol = graphBackgroundTechSpeed;

	profileBox = JSON.parse(JSON.stringify(profileGraphTemplate));
	profileBox.DimX = "Speed Step";
	profileBox.DimY = "mm/s";
	profileBox.ValsX = [0,126,13];
	profileBox.ValsY = [0,500,10];
	var startEl = JSON.parse(JSON.stringify(DataElementTemplate));
	startEl.x = profileBox.ValsX[0];
	startEl.y = 0;
	profileBox.DataElements.push(startEl);
	var endEl = JSON.parse(JSON.stringify(DataElementTemplate));
	endEl.x = profileBox.ValsX[1];
	endEl.y = 0;
	profileBox.DataElements.push(endEl);
	profileBox.Width = graphLineTechSpeed;
	profileBox.Color = graphColorTechProfileSpeedLimit;
	profileBox.HLColor = graphHLColorTechProfileSpeed;
	techSpeedProfileGraph.LineGraphs.push(profileBox);

	profileBox = JSON.parse(JSON.stringify(profileGraphTemplate));
	profileBox.DimX = "Speed Step fw";
	profileBox.DimY = "mm/s";
	profileBox.ValsX = [0,126,13];
	profileBox.ValsY = [0,500,10];
	var startEl = JSON.parse(JSON.stringify(DataElementTemplate));
	startEl.x = profileBox.ValsX[0];
	startEl.y = profileBox.ValsY[0];
	profileBox.DataElements.push(startEl);
	var endEl = JSON.parse(JSON.stringify(DataElementTemplate));
	endEl.x = profileBox.ValsX[1];
	endEl.y = 0;
	profileBox.DataElements.push(endEl);
	profileBox.Color = graphColorTechProfileSpeedFw;
	profileBox.HLColor = graphHLColorTechProfileSpeed;
	techSpeedProfileGraph.LineGraphs.push(profileBox);

	profileBox = JSON.parse(JSON.stringify(profileGraphTemplate));
	profileBox.DimX = "Speed Step bw";
	profileBox.DimY = "mm/s";
	profileBox.ValsX = [0,126,13];
	profileBox.ValsY = [0,500,10];
	var startEl = JSON.parse(JSON.stringify(DataElementTemplate));
	startEl.x = profileBox.ValsX[0];
	startEl.y = profileBox.ValsY[0];
	profileBox.DataElements.push(startEl);
	var endEl = JSON.parse(JSON.stringify(DataElementTemplate));
	endEl.x = profileBox.ValsX[1];
	endEl.y = 0;
	profileBox.DataElements.push(endEl);
	profileBox.Color = graphColorTechProfileSpeedBw;
	profileBox.HLColor = graphHLColorTechProfileSpeed;
	techSpeedProfileGraph.LineGraphs.push(profileBox);

	speedTableProfileGraph = JSON.parse(JSON.stringify(profileTemplate));
	speedTableProfileGraph.Title = "Speed Table Settings";
	speedTableProfileGraph.DispDim = canvasSizeSpeedTable;
	speedTableProfileGraph.BoxCol = graphBoxSpeedTable;
	speedTableProfileGraph.FieldCol = graphBackgroundSpeedTable;
	profileBox = JSON.parse(JSON.stringify(profileGraphTemplate));
	profileBox.DimX = "Table Entry";
	profileBox.DimY = "CV Value";
	profileBox.ValsX = [0,28,28];
	profileBox.ValsY = [0,255,10];
	var startEl = JSON.parse(JSON.stringify(DataElementTemplate));
	startEl.x = profileBox.ValsX[0];
	startEl.y = profileBox.ValsY[0];
	profileBox.DataElements.push(startEl);
	var endEl = JSON.parse(JSON.stringify(DataElementTemplate));
	endEl.x = profileBox.ValsX[1];
	endEl.y = profileBox.ValsY[1];
	profileBox.DataElements.push(endEl);
	profileBox.Width = graphLineSpeedTable;
	profileBox.Color = graphColorSpeedTable;
	profileBox.HLColor = graphHLColorSpeedTable;
	speedTableProfileGraph.LineGraphs.push(profileBox);
}

//--------------------------------------------------------------Graphic Functions
function drawProfileBox(ofCanvas, dispObj)
{
	var context = ofCanvas.getContext( "2d" );  

	ofCanvas.width = dispObj.DispDim[0];
	ofCanvas.height = dispObj.DispDim[1];
   
	context.fillStyle = dispObj.FieldCol;
	context.strokeStyle = "black";
	context.fillRect(0, 0, ofCanvas.width, ofCanvas.height);
	context.stroke();  

	// declare graph start and end  
//DispDim = [screen.width/2, 400, 50, 50, 150, 50]; //width, height, frame size top, bottom, left, right
	var TOP_FRAME = dispObj.DispDim[2];
	var BOTTOM_FRAME = dispObj.DispDim[3];
	var LEFT_FRAME = dispObj.DispDim[4];
	var RIGHT_FRAME = dispObj.DispDim[5];

	var GRAPH_TOP = TOP_FRAME;  
	var GRAPH_BOTTOM = ofCanvas.height - BOTTOM_FRAME;  
	var GRAPH_LEFT = LEFT_FRAME;  
	var GRAPH_RIGHT = ofCanvas.width - RIGHT_FRAME;  
	var GRAPH_HEIGHT = ofCanvas.height - TOP_FRAME - BOTTOM_FRAME;   
	var GRAPH_WIDTH = ofCanvas.width - LEFT_FRAME - RIGHT_FRAME;   
	var GRAPH_CENTER =  TOP_FRAME + Math.round(GRAPH_HEIGHT/2);
	
	// draw profile box  
	var my_gradient = context.createLinearGradient(GRAPH_LEFT, GRAPH_TOP, GRAPH_RIGHT-GRAPH_LEFT, GRAPH_BOTTOM-GRAPH_TOP);
	my_gradient.addColorStop(1, dispObj.BoxCol);
	my_gradient.addColorStop(0, "white");
	context.fillStyle = my_gradient;
	context.fillRect(GRAPH_LEFT, GRAPH_TOP, GRAPH_RIGHT-GRAPH_LEFT, GRAPH_BOTTOM-GRAPH_TOP);
	context.stroke();  
	context.beginPath();  
	context.strokeStyle = "black";
	context.moveTo( GRAPH_LEFT, GRAPH_TOP );  
	context.lineTo( GRAPH_LEFT, GRAPH_BOTTOM );
	context.lineTo( GRAPH_RIGHT, GRAPH_BOTTOM );
	context.lineTo( GRAPH_RIGHT, GRAPH_TOP );
	context.lineTo( GRAPH_LEFT, GRAPH_TOP );
//	context.strokeRect(GRAPH_LEFT, GRAPH_TOP ,GRAPH_RIGHT, GRAPH_BOTTOM);
	context.stroke();  
	context.font = "16px Arial";  
	context.fillStyle = "black";
	context.textAlign = "center";

	context.fillText( dispObj.Title, Math.round((GRAPH_RIGHT + GRAPH_LEFT) / 2), 30);  

	context.textAlign = "right";
//	for (var j = 0; j < dispObj.LineGraphs.length; j++)
	{
		var j = 0;
		for (var i = 0; i <= dispObj.LineGraphs[j].ValsY[2]; i++)
		{
			var yPos = GRAPH_BOTTOM - (i * (GRAPH_HEIGHT/(dispObj.LineGraphs[j].ValsY[2])));
			context.moveTo(GRAPH_LEFT-5, yPos);  
			context.lineTo(GRAPH_LEFT, yPos);
			context.fillText( Math.round(dispObj.LineGraphs[j].ValsY[0] + ((dispObj.LineGraphs[j].ValsY[1]/dispObj.LineGraphs[j].ValsY[2]) * i)), GRAPH_LEFT - 10, yPos+8);  
		}
	}
	context.fillText( dispObj.LineGraphs[0].DimY, GRAPH_LEFT - 10, GRAPH_TOP - 20);  
	context.fillText( dispObj.LineGraphs[0].DimX, GRAPH_RIGHT, 40 + GRAPH_BOTTOM);  
	context.textAlign = "left";
	context.fillText("0", GRAPH_LEFT, 20 + GRAPH_BOTTOM );  
	context.textAlign = "center";
	for (var i = 1; i <= dispObj.LineGraphs[0].ValsX[2]; i++)
	{
		var xPos = GRAPH_LEFT + (i * (GRAPH_WIDTH/(dispObj.LineGraphs[0].ValsX[2])));
		context.moveTo(xPos, GRAPH_BOTTOM+5);  
		context.lineTo(xPos, GRAPH_BOTTOM);
		context.fillText( Math.round(dispObj.LineGraphs[0].ValsX[0] + ((dispObj.LineGraphs[0].ValsX[1]/dispObj.LineGraphs[0].ValsX[2]) * i)), xPos, 20 + GRAPH_BOTTOM );  
	}
	context.stroke();  
	//draw line
	for (var j = 0; j < dispObj.LineGraphs.length; j++)
	{
		if (dispObj.LineGraphs[j].DataElements.length > 1)
		{
			var xPos = GRAPH_LEFT + (dispObj.LineGraphs[j].DataElements[0].x * (GRAPH_WIDTH/(dispObj.LineGraphs[j].ValsX[1] - dispObj.LineGraphs[j].ValsX[0])));
			var yPos = GRAPH_BOTTOM - (dispObj.LineGraphs[j].DataElements[0].y * (GRAPH_HEIGHT/(dispObj.LineGraphs[j].ValsY[1] - dispObj.LineGraphs[j].ValsY[0])));
//			console.log(xPos, yPos, dispObj.LineGraphs[j].DataElements[0].x, dispObj.LineGraphs[j].DataElements[j].y);
			context.strokeStyle = dispObj.LineGraphs[j].mouseOverLine >= 0 ? dispObj.LineGraphs[j].HLColor : dispObj.LineGraphs[j].Color;
			context.fillStyle = dispObj.LineGraphs[j].mouseOverLine >= 0 ? dispObj.LineGraphs[j].HLColor : dispObj.LineGraphs[j].Color;
			context.lineWidth = dispObj.LineGraphs[j].Width;
			context.beginPath();
			context.moveTo(xPos, yPos);
//			context.arc(xPos, yPos, 5, 0, 2 * Math.PI);
			for (var i = 1; i < dispObj.LineGraphs[j].DataElements.length; i++)
			{
				xPos = GRAPH_LEFT + (dispObj.LineGraphs[j].DataElements[i].x * (GRAPH_WIDTH/(dispObj.LineGraphs[j].ValsX[1] - dispObj.LineGraphs[j].ValsX[0])));
				yPos = GRAPH_BOTTOM - (dispObj.LineGraphs[j].DataElements[i].y * (GRAPH_HEIGHT/(dispObj.LineGraphs[j].ValsY[1] - dispObj.LineGraphs[j].ValsY[0])));
//				console.log(xPos, yPos, dispObj.LineGraphs[j].DataElements[i].x, dispObj.LineGraphs[j].DataElements[i].y);
//				context.stroke(); 
				context.lineTo(xPos, yPos);  
				context.stroke(); 
				context.beginPath();
				switch (dispObj.LineGraphs[j].DataElements[i].m)
				{
					case 1: 
//						context.beginPath();
						context.arc(xPos, yPos, 3, 0, 2 * Math.PI);
						context.fill(); 
						break;
					case 2:
//						context.beginPath();
						context.rect(xPos-2, yPos-2, 4, 4);
						context.fill(); 
						break;
				}
				context.moveTo(xPos, yPos);  
				context.stroke(); 
			}
		}
		else
			console.log("No data");
	}
	context.stroke();  
	if (dispObj.SpeedGraph)
		if ((dispObj.SpeedGraph.m == 0) && (document.getElementById("cbCurrSpeed").checked))
		{
			var yPos1;
				context.beginPath();
				xPos = GRAPH_LEFT + (dispObj.SpeedGraph.x * (GRAPH_WIDTH/(dispObj.LineGraphs[0].ValsX[1] - dispObj.LineGraphs[0].ValsX[0]))); //dispObj.LineGraphs[0].Width);
				yPos = GRAPH_BOTTOM - (dispObj.SpeedGraph.Last2y * (GRAPH_HEIGHT/(dispObj.LineGraphs[0].ValsY[1] - dispObj.LineGraphs[0].ValsY[0])));
				context.strokeStyle = dispObj.SpeedGraph.graphColor;
				context.fillStyle = dispObj.SpeedGraph.graphColor;
				context.arc(xPos, yPos, 6, 0, 2 * Math.PI);
				context.fill(); 
				context.stroke(); 

		}
	
}

function getDataPtr(forCanvas)
{
	if (forCanvas.id == "glCanvasThrottle")
		return throttleProfileGraph;
	if (forCanvas.id == "glCanvasTechSpeed")
		return techSpeedProfileGraph;
	if (forCanvas.id == "glCanvasSpeedTable")
		return speedTableProfileGraph;
	return null;
}

function setCanvasMouseEvents(canvasElement)
{
	canvasElement.setAttribute("onclick", "canvasMouseClick(this, event)");
	canvasElement.setAttribute("ondblclick", "canvasMouseDblClick(this, event)");
	canvasElement.setAttribute("oncontextmenu", "canvasMouseRightClick(this)");
	canvasElement.setAttribute("onmousemove", "canvasMouseMove(this, event)");
	canvasElement.setAttribute("onmousedown", "canvasMouseDown(this, event)");
	canvasElement.setAttribute("onmouseup", "canvasMouseUp(this, event)");
 	canvasElement.setAttribute("onmouseleave", "canvasMouseLeave(this, event)");
	canvasElement.setAttribute("onmouseout", "canvasMouseOut(this, event)");
	canvasElement.setAttribute("onmouseenter", "canvasMouseEnter(this, event)");
	canvasElement.setAttribute("onmouseover", "canvasMouseOver(this, event)");
}

function getMousePos(canvas, evt) 
{
	var rect = canvas.getBoundingClientRect();
	var retVal = {"x":0,"y":0};
	retVal.x = evt.clientX - rect.left - 150;
	retVal.y = -(evt.clientY - rect.top) + 350;
    return retVal;
}

function getMouseVal(graphData, graphPoint)
{
	var dispWidth = graphData.DispDim[0] - graphData.DispDim[4] - graphData.DispDim[5];
	var dispHeight = graphData.DispDim[1] - graphData.DispDim[2] - graphData.DispDim[3];
	var retVal = {"x":0,"y":0};
	retVal.x = Math.round(graphPoint.x * (graphData.LineGraphs[0].ValsX[1] - graphData.LineGraphs[0].ValsX[0]) / dispWidth);
	retVal.y = Math.round(graphPoint.y * (graphData.LineGraphs[0].ValsY[1] - graphData.LineGraphs[0].ValsY[0]) / dispHeight);
	return retVal;
}
 
function isMouseOver(dataPoint, dataLine)
{
	if (dataLine.length < 2) return -1;
	for (var i = 1; i < dataLine.length; i++)
	{
		if ((dataLine[i-1].x <= dataPoint.x) && (dataLine[i].x > dataPoint.x) && (dataLine[i-1].y <= dataPoint.y) && (dataLine[i].y > dataPoint.y))
		{
			var lineSlope = (dataLine[i].x - dataLine[i-1].x) / (dataLine[i].y - dataLine[i-1].y);
			var pointSlope = (dataPoint.x - dataLine[i-1].x) / (dataPoint.y - dataLine[i-1].y);
			if (dataPoint.x * Math.abs((lineSlope - pointSlope)/lineSlope) < 0.50)
				return i;
		}
	}
	return -1;
}
 
function mouseOverPoint(graphData, posVal)
{
	for (var i = 1; i < (graphData.LineGraphs[0].DataElements.length - 1); i++)
	{
		var dtVal = graphData.LineGraphs[0].DataElements[i];
//		console.log(dtVal, posVal);
		if ((posVal.x >= (dtVal.x - 2)) && (posVal.x <= (dtVal.x + 2)) && (posVal.y >= (dtVal.y - 2)) && (posVal.y <= (dtVal.y + 2)))
		{
//			console.log(i);
			return i;
		}
	}
	return -1; 
}
 
function canvasMouseClick(sender, e)
{
	var graphData = getDataPtr(sender);
	var mousePos = getMousePos(sender, e);
	var mouseVal = getMouseVal(graphData, mousePos);
	if ((graphData == throttleProfileGraph) && (mouseVal.x > 0) && (mouseVal.y > 0) && (mouseVal.x < graphData.LineGraphs[0].ValsX[1]) && (mouseVal.y < graphData.LineGraphs[0].ValsY[1]))
	{
		var selLeg = mouseOverPoint(graphData, mouseVal);
		if (e.ctrlKey && selLeg > 0 && (selLeg < (graphData.LineGraphs[0].DataElements.length - 2)))
		{
			graphData.LineGraphs[0].DataElements.splice(selLeg, 1);
			graphData = createThrottleGraph(throttleDef, graphData);
			drawProfileBox(canvasElementThrottle, throttleProfileGraph);
			return;
		}
		else
		{
			for (var i = 1; i < (graphData.LineGraphs[0].DataElements.length - 1); i++)
			{
				if ((mouseVal.x > graphData.LineGraphs[0].DataElements[i-1].x) && (mouseVal.x < graphData.LineGraphs[0].DataElements[i].x))
				{
					var newEl = JSON.parse(JSON.stringify(DataElementTemplate));
					graphData.LineGraphs[0].DataElements.splice(i, 0, newEl); //add elements to minimum length
					newEl.x = mouseVal.x;
					newEl.y = Math.max(0, Math.min(mouseVal.y, throttleDef.VMax));
					newEl.m = 1;
					graphData = createThrottleGraph(throttleDef, graphData);
					drawProfileBox(canvasElementThrottle, throttleProfileGraph);
					return;
				}
			}
		}
	}
}

function canvasMouseRightClick(sender)
{
	var graphData = getDataPtr(sender);
}

function isMouseOverLine(thisPt, thisLine)
{
}

function canvasMouseMove(sender, e)
{-1
	var graphData = getDataPtr(sender);
	var mousePos = getMousePos(sender, e);
	var mouseVal = getMouseVal(graphData, mousePos);
	if (graphData == throttleProfileGraph)
		if (moveNode > 0)
		{
			if ((mouseVal.x > graphData.LineGraphs[0].DataElements[moveNode-1].x) && (mouseVal.y > graphData.LineGraphs[0].DataElements[moveNode-1].y) && (mouseVal.x < graphData.LineGraphs[0].DataElements[moveNode+1].x) && (mouseVal.y < (1.1 * graphData.LineGraphs[0].ValsY[1])))
			{
				if (moveNode == graphData.LineGraphs[0].DataElements.length - 2)
				{
					throttleDef.VMax = Math.max(0, Math.min(mouseVal.y, Math.round(1.1 * graphData.LineGraphs[0].ValsY[1])));
					throttleDef.AtStep = Math.max(graphData.LineGraphs[0].DataElements[moveNode-1].x + 1, Math.min(mouseVal.x, graphData.LineGraphs[0].ValsX[1]-1));
					writeInputField("thscalespeed", throttleDef.VMax);
					writeInputField("thvmax", throttleDef.AtStep);
				}
				graphData.LineGraphs[0].DataElements[moveNode].x = mouseVal.x;
				graphData.LineGraphs[0].DataElements[moveNode].y = Math.max(0, Math.min(mouseVal.y, throttleDef.VMax));
				if (moveNode == graphData.LineGraphs[0].DataElements.length - 2)
					graphData.LineGraphs[0].DataElements[moveNode+1].y = graphData.LineGraphs[0].DataElements[moveNode].y;
				graphData = createThrottleGraph(throttleDef, graphData);
				drawProfileBox(canvasElementThrottle, throttleProfileGraph);
			}
		}
//	graphData.LineGraphs[0].mouseOverLine = isMouseOver(mouseVal, graphData.LineGraphs[0].DataElements);

//	if (oldVal != graphData.LineGraphs[0].mouseOverLine)
//		drawProfileBox(sender, graphData);
}

function canvasMouseDblClick(sender)
{
	var graphData = getDataPtr(sender);
	
}

function canvasMouseDown(sender, e)
{
	var graphData = getDataPtr(sender);
	var mousePos = getMousePos(sender, e);
	var mouseVal = getMouseVal(graphData, mousePos);
	if ((graphData == throttleProfileGraph) && (mouseVal.x > 0) && (mouseVal.y > 0) && (mouseVal.x < graphData.LineGraphs[0].ValsX[1]) && (mouseVal.y < graphData.LineGraphs[0].ValsY[1]))
	{
		var selLeg = mouseOverPoint(graphData, mouseVal);
		if (selLeg > 0 && (selLeg < (graphData.LineGraphs[0].DataElements.length - 1)))
			moveNode = selLeg;
	}
}

function canvasMouseUp(sender, e)
{
	var graphData = getDataPtr(sender);
	moveNode = -1;
}

function canvasMouseLeave(sender)
{
	var graphData = getDataPtr(sender);
}

function canvasMouseOut(sender)
{
	var graphData = getDataPtr(sender);
}

function canvasMouseEnter(sender)
{
	var graphData = getDataPtr(sender);
}

function canvasMouseOver(sender)
{
	var graphData = getDataPtr(sender);
}

function setProgMode(sender)
{
	if (sender.id == "rbdefprogmode")
	{
		configData[workCfg].ProgMode = sender.selectedIndex;
		writeRBInputField("rbprogmode", configData[workCfg].ProgMode);
		setButtonStatus();
	}
	else
	{
		configData[workCfg].ProgMode = parseInt(sender.id[sender.id.length - 1]);
		setDropdownValue("rbdefprogmode", configData[workCfg].ProgMode);
	}
//	console.log(sender.id, configData[workCfg].ProgMode);
}

function setProgMethod(sender)
{
	if (sender.id == "rbdefprogmethod")
	{
		configData[workCfg].ProgMethod = sender.selectedIndex;
		writeRBInputField("rbprogmethod", configData[workCfg].ProgMethod);
	}
	else
	{
		configData[workCfg].ProgMethod = parseInt(sender.id[sender.id.length - 1]);
		setDropdownValue("rbdefprogmethod", configData[workCfg].ProgMethod);
	}
//	console.log(sender.id, configData[workCfg].ProgMethod);
}

function setTrimReserve(sender)
{
	trimReserve = verifyNumber(sender.value, trimReserve);
	trimReserve = Math.min(15, Math.max(0, trimReserve));
	sender.value = trimReserve;
}

function setTestMode(sender)
{
	var newMode = parseInt(sender.id[sender.id.length - 1]);
	if (newMode != stepMode)
	{
		stepMode = newMode;
		for (var i = 0; i < 3; i++)
		{
			techSpeedProfileGraph.LineGraphs[i].ValsX[1] = stepMode == 0 ? 28 :127;
			if (techSpeedProfileGraph.LineGraphs[i].DataElements.length > 2)
				techSpeedProfileGraph.LineGraphs[i].DataElements.slice(1, techSpeedProfileGraph.LineGraphs[i].DataElements.length-2);
			techSpeedProfileGraph.LineGraphs[i].DataElements[1].x = techSpeedProfileGraph.LineGraphs[i].ValsX[1];
		}
//		console.log(techSpeedProfileGraph.LineGraphs);
		drawProfileBox(canvasElementTechSpeed, techSpeedProfileGraph);
	}
}


function setCalcBase(sender)
{
	configData[workCfg].CalcMode = parseInt(sender.id[sender.id.length - 1]);
//	console.log(configData[workCfg].CalcMode);
}

function setTrimMode(sender)
{
	configData[workCfg].TrimMode = parseInt(sender.id[sender.id.length - 1]);
	setButtonStatus();
//	console.log(configData[workCfg].TrimMode);
}

function setTableMode(sender)
{
//	tableMode = parseInt(sender.id[sender.id.length - 1]);
//	console.log(tableMode);
}

function setTrim(sender)
{
	if ((sender.id == "fwtrim") || (sender.id == "fwtrimadj"))
	{
		fwTrim = verifyNumber(sender.value, fwTrim);
		writeInputField("fwtrim", fwTrim);
		writeInputField("fwtrimadj", fwTrim);
	}
	if ((sender.id == "bwtrim") || (sender.id == "bwtrimadj"))
	{
		bwTrim = verifyNumber(sender.value, bwTrim);
		writeInputField("bwtrim", bwTrim);
		writeInputField("bwtrimadj", bwTrim);
	}
}
