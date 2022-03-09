var locoDef;
var validLocoDef = false;
var fileName = "";
var throttleDef = {"DataType" : "ThrottleDef", "Version" : "1.0", "ProfName":"new profile", "VMax":80, "SpeedSteps":100, "GraphData" : {}, "FileName":""};
var validThrottleDef = true;

var throttleProfileGraph;
var techSpeedProfileGraph;
var speedTableProfileGraph;

var canvasSizeThrottle = [screen.width/2, 400, 50, 50, 150, 50]; //width, height, frame size top, bottom, left, right
var canvasSizeTechSpeed = [screen.width/2, 400, 50, 50, 150, 50];
var canvasSizeSpeedTable = [screen.width/2, 400, 50, 50, 150, 50];

var canvasElementThrottle;
var canvasElementTechSpeed;
var canvasElementSpeedTable;

var graphBackgroundThrottle = "rgb(160,160,160)";
var graphBackgroundTechSpeed = "rgb(160,160,160)";
var graphBackgroundSpeedTable = "rgb(160,160,160)";

var graphBoxThrottle = "rgb(255,50,50)";
var graphBoxTechSpeed = "rgb(30,128,30)";
var graphBoxSpeedTable = "rgb(50,50,255)";

var graphColorThrottle = "rgb(255,50,50)";
var graphColorTechProfileSpeed = "rgb(30,128,30)";
var graphColorSpeedTable = "rgb(50,50,255)";
var graphHLColorThrottle = "rgb(255,150,150)";
var graphHLColorTechProfileSpeed = "rgb(130,128,130)";
var graphHLColorSpeedTable = "rgb(150,150,255)";

var profileGraphTemplate = {"DimX":"%", "DimY":"km/h", "mouseOverLine" : -1, "Color":"blue", "HLColor":"green", "ValsX":[0,100,10], "ValsY":[0,160,10], "Show" : true,  "DataElements":[]};
var DataElementTemplate = {"x":0,"y":0};
var profileTemplate = {"Title":"title", "DispDim":[0,0], "FieldCol": "rgb(100,100,100)", "BoxCol": "rgb(100,100,100)", "LineGraphs":[]};


var progMode = 1; //main line
var tableMode = 0; //table mode

//--------------------------------------------------------------JMRI Decoder functions
function loadJMRIDecoder(sender)
{
	fileName = document.getElementById("btnLoadDecoder").files[0];
	var reader = new FileReader();
	var parser;
    reader.onload = function()
    {
        try 
        {
			parser = new DOMParser();
			locoDef = parser.parseFromString(reader.result,"text/xml");
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
    reader.readAsText(fileName);	
}

function saveJMRIDecoder(sender)
{
	if (validLocoDef)
		writeDiskFile(new Blob([xml2Str(locoDef)], {type: "text/xml"}), fileName.name);
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
	var varVal = xmlNode.getElementsByTagName("varValue");
	var hasSpeedTable = -1;
	var cvVal = [];
	if (varVal.length > 0)
	{
		var attr = findXMLAttribute(varVal, "item", "Speed Table Definition");
		if (attr)
			hasSpeedTable = attr.getAttribute("value");
		if (hasSpeedTable == 1)
		{
			cvVal = xmlNode.getElementsByTagName("CVvalue");
			speedTableProfileGraph.LineGraphs[0].DataElements.length = 0;//delete old data
			console.log(xmlNode);
			for (var i = 0; i < 28; i++) //28 table entries by definition
			{
				var cv = findXMLAttribute(cvVal, "name", (i+67).toString());
				if (cv)
				{
					var newEl = JSON.parse(JSON.stringify(DataElementTemplate));
					newEl.x = i;
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
		document.getElementById("decbrand").innerHTML = xmlNode.getElementsByTagName("decoder")[0].getAttribute("family");
		document.getElementById("decmodel").innerHTML = xmlNode.getElementsByTagName("decoder")[0].getAttribute("model");
		
	}
	else
		console.log("Not Locomotive data");
	return isLoco;
}

//--------------------------------------------------------------Throttle Profile functions
function setThrottleProfile(sender)
{
	if (sender.id == "thprofilename")
	{
		throttleDef.ProfName = sender.value;

	}
	if (sender.id == "thscalespeed")
	{
		throttleDef.VMax = verifyNumber(sender.value, throttleDef.VMax);
		throttleProfileGraph.LineGraphs[0].ValsY[1] = throttleDef.VMax;
		for (var i = 0; i < throttleProfileGraph.LineGraphs[0].DataElements.length; i++)
			if (throttleProfileGraph.LineGraphs[0].DataElements[i].y > throttleDef.VMax)
				throttleProfileGraph.LineGraphs[0].DataElements[i].y = throttleDef.VMax;
		drawProfileBox(canvasElementThrottle, throttleProfileGraph);
	}
	if (sender.id == "thnumsteps")
	{
		throttleDef.SpeedSteps = verifyNumber(sender.value, throttleDef.SpeedSteps);
		throttleProfileGraph.LineGraphs[0].ValsX[1] = throttleDef.SpeedSteps;

		throttleProfileGraph.LineGraphs[0].DataElements.length = 0;
		var startEl = JSON.parse(JSON.stringify(DataElementTemplate));
		startEl.x = throttleProfileGraph.LineGraphs[0].ValsX[0];
		startEl.y = throttleProfileGraph.LineGraphs[0].ValsY[0];
		throttleProfileGraph.LineGraphs[0].DataElements.push(startEl);
		var endEl = JSON.parse(JSON.stringify(DataElementTemplate));
		endEl.x = throttleProfileGraph.LineGraphs[0].ValsX[1];
		endEl.y = throttleProfileGraph.LineGraphs[0].ValsY[1];
		throttleProfileGraph.LineGraphs[0].DataElements.push(endEl);
		drawProfileBox(canvasElementThrottle, throttleProfileGraph);
	}
}

function saveThrottle(sender)
{
	if (validThrottleDef)
	{
		fileName.name = throttleDef.FileName;
		writeDiskFile(new Blob([JSON.stringify(throttleDef)], {type: "application/json"}), fileName.name);
	}
}

function loadThrottle(sender)
{
	fileName = document.getElementById("btnLoadThrottle").files[0];
//	console.log(fileName);
	var reader = new FileReader();
	var parser;
    reader.onload = function()
    {
        try 
        {
			throttleDef = JSON.parse(reader.result);
			throttleDef.FileName = fileName.name;
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
			throttleDef.fileName = "";
			alert("Not a valid throttle profile");
			return;
		}
//		console.log(throttleDef);
        displayThrottleDef(throttleDef);
		throttleProfileGraph = throttleDef.GraphData;
//		console.log(throttleProfileGraph);
		drawProfileBox(canvasElementThrottle, throttleProfileGraph);
		setButtonStatus();
    };
    reader.readAsText(fileName);	
}

function displayThrottleDef(jsonNode)
{
	writeTextField("throttlefilename", jsonNode.FileName);
	writeInputField("thprofilename", jsonNode.ProfName);
	writeInputField("thscalespeed", jsonNode.VMax);
	writeInputField("thnumsteps", jsonNode.SpeedSteps);
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
	profileBox.DimX = "Speed Step";
	profileBox.DimY = (configData[workCfg].Units == 0) ? "km/h" : "mph";
	profileBox.Color = graphColorThrottle;
	profileBox.HLColor = graphHLColorThrottle;
	profileBox.ValsX = [0,throttleDef.SpeedSteps,10];
	profileBox.ValsY = [0,throttleDef.VMax,10];
	var startEl = JSON.parse(JSON.stringify(DataElementTemplate));
	startEl.x = profileBox.ValsX[0];
	startEl.y = profileBox.ValsY[0];
	profileBox.DataElements.push(startEl);
	var endEl = JSON.parse(JSON.stringify(DataElementTemplate));
	endEl.x = profileBox.ValsX[1];
	endEl.y = profileBox.ValsY[1];
	profileBox.DataElements.push(endEl);
	throttleProfileGraph.LineGraphs.push(profileBox);
	throttleDef.GraphData = throttleProfileGraph;

	techSpeedProfileGraph = JSON.parse(JSON.stringify(profileTemplate));
	techSpeedProfileGraph.Title = "Technical Speed Profile";
	techSpeedProfileGraph.DispDim = canvasSizeTechSpeed;
	techSpeedProfileGraph.BoxCol = graphBoxTechSpeed;
	techSpeedProfileGraph.FieldCol = graphBackgroundTechSpeed;
	profileBox = JSON.parse(JSON.stringify(profileGraphTemplate));
	profileBox.DimX = "Speed Step fw";
	profileBox.DimY = "mm/s";
	profileBox.ValsX = [0,127,13];
	profileBox.ValsY = [0,500,10];
	var startEl = JSON.parse(JSON.stringify(DataElementTemplate));
	startEl.x = profileBox.ValsX[0];
	startEl.y = profileBox.ValsY[0];
	profileBox.DataElements.push(startEl);
	var endEl = JSON.parse(JSON.stringify(DataElementTemplate));
	endEl.x = profileBox.ValsX[1];
	endEl.y = profileBox.ValsY[1];
	profileBox.DataElements.push(endEl);
	profileBox.Color = graphColorTechProfileSpeed;
	profileBox.HLColor = graphHLColorTechProfileSpeed;
	techSpeedProfileGraph.LineGraphs.push(profileBox);

	profileBox = JSON.parse(JSON.stringify(profileGraphTemplate));
	profileBox.DimX = "Speed Step bw";
	profileBox.DimY = "mm/s";
	profileBox.ValsX = [0,127,13];
	profileBox.ValsY = [0,500,10];
	var startEl = JSON.parse(JSON.stringify(DataElementTemplate));
	startEl.x = profileBox.ValsX[0];
	startEl.y = profileBox.ValsY[0];
	profileBox.DataElements.push(startEl);
	var endEl = JSON.parse(JSON.stringify(DataElementTemplate));
	endEl.x = profileBox.ValsX[1];
	endEl.y = 1.5 * profileBox.ValsY[1];
	profileBox.DataElements.push(endEl);
	profileBox.Color = graphColorTechProfileSpeed;
	profileBox.HLColor = graphHLColorTechProfileSpeed;
	techSpeedProfileGraph.LineGraphs.push(profileBox);

	speedTableProfileGraph = JSON.parse(JSON.stringify(profileTemplate));
	speedTableProfileGraph.Title = "Speed Table Settings";
	speedTableProfileGraph.DispDim = canvasSizeSpeedTable;
	speedTableProfileGraph.BoxCol = graphBoxSpeedTable;
	speedTableProfileGraph.FieldCol = graphBackgroundSpeedTable;
	profileBox = JSON.parse(JSON.stringify(profileGraphTemplate));
	profileBox.DimX = "Speed Step";
	profileBox.DimY = "Pwr. Incr.";
	profileBox.ValsX = [0,27,27];
	profileBox.ValsY = [0,255,10];
	var startEl = JSON.parse(JSON.stringify(DataElementTemplate));
	startEl.x = profileBox.ValsX[0];
	startEl.y = profileBox.ValsY[0];
	profileBox.DataElements.push(startEl);
	var endEl = JSON.parse(JSON.stringify(DataElementTemplate));
	endEl.x = profileBox.ValsX[1];
	endEl.y = profileBox.ValsY[1];
	profileBox.DataElements.push(endEl);
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
//	for (var j = 0; j < dispObj.LineGraphs.Length; j++)
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
	if (dispObj.LineGraphs[0].DataElements.length > 1)
	{
		var xPos = GRAPH_LEFT + (dispObj.LineGraphs[0].DataElements[0].x * (GRAPH_WIDTH/(dispObj.LineGraphs[0].ValsX[1] - dispObj.LineGraphs[0].ValsX[0])));
		var yPos = GRAPH_BOTTOM - (dispObj.LineGraphs[0].DataElements[0].y * (GRAPH_HEIGHT/(dispObj.LineGraphs[0].ValsY[1] - dispObj.LineGraphs[0].ValsY[0])));
//		console.log(xPos, yPos, dispObj.LineGraphs[0].DataElements[0].x, dispObj.LineGraphs[0].DataElements[0].y);
		context.moveTo(xPos, yPos);
		context.strokeStyle = dispObj.LineGraphs[0].mouseOverLine >= 0 ? dispObj.LineGraphs[0].HLColor : dispObj.LineGraphs[0].Color;
		context.fillStyle = dispObj.LineGraphs[0].mouseOverLine >= 0 ? dispObj.LineGraphs[0].HLColor : dispObj.LineGraphs[0].Color;
		context.beginPath();
		context.arc(xPos, yPos, 5, 0, 2 * Math.PI);
		for (var i = 1; i < dispObj.LineGraphs[0].DataElements.length; i++)
		{
			xPos = GRAPH_LEFT + (dispObj.LineGraphs[0].DataElements[i].x * (GRAPH_WIDTH/(dispObj.LineGraphs[0].ValsX[1] - dispObj.LineGraphs[0].ValsX[0])));
			yPos = GRAPH_BOTTOM - (dispObj.LineGraphs[0].DataElements[i].y * (GRAPH_HEIGHT/(dispObj.LineGraphs[0].ValsY[1] - dispObj.LineGraphs[0].ValsY[0])));
//			console.log(xPos, yPos, dispObj.LineGraphs[0].DataElements[i].x, dispObj.LineGraphs[0].DataElements[i].y);
			context.arc(xPos, yPos, 5, 0, 2 * Math.PI);
			context.lineTo(xPos, yPos);  
		}
		context.stroke(); 
		context.fill(); 
	}
	else
		console.log("No data");
	context.stroke();  
	
	
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
	canvasElement.setAttribute("onclick", "canvasMouseClick(this)");
	canvasElement.setAttribute("ondblclick", "canvasMouseDblClick(this)");
	canvasElement.setAttribute("oncontextmenu", "canvasMouseRightClick(this)");
	canvasElement.setAttribute("onmousemove", "canvasMouseMove(this, event)");
	canvasElement.setAttribute("onmousedown", "canvasMouseDown(this)");
	canvasElement.setAttribute("onmouseup", "canvasMouseUp(this)");
 	canvasElement.setAttribute("onmouseleave", "canvasMouseLeave(this)");
	canvasElement.setAttribute("onmouseout", "canvasMouseOut(this)");
	canvasElement.setAttribute("onmouseenter", "canvasMouseEnter(this)");
	canvasElement.setAttribute("onmouseover", "canvasMouseOver(this)");
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
	retVal.x = graphPoint.x * (graphData.LineGraphs[0].ValsX[1] - graphData.LineGraphs[0].ValsX[0]) / dispWidth;
	retVal.y = graphPoint.y * (graphData.LineGraphs[0].ValsY[1] - graphData.LineGraphs[0].ValsY[0]) / dispHeight;
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
 
function canvasMouseClick(sender)
{
	var graphData = getDataPtr(sender);
}

function canvasMouseRightClick(sender)
{
	var graphData = getDataPtr(sender);
}

function isMouseOverLine(thisPt, thisLine)
{
}

function canvasMouseMove(sender, e)
{
	var graphData = getDataPtr(sender);
	var oldVal = graphData.LineGraphs[0].mouseOverLine;
	var mousePos = getMousePos(sender, e);
	var mouseVal = getMouseVal(graphData, mousePos);
//	console.log(mouseVal);
	
	graphData.LineGraphs[0].mouseOverLine = isMouseOver(mouseVal, graphData.LineGraphs[0].DataElements);

	if (oldVal != graphData.LineGraphs[0].mouseOverLine)
		drawProfileBox(sender, graphData);
}

function canvasMouseDblClick(sender)
{
	var graphData = getDataPtr(sender);
	
}

function canvasMouseDown(sender)
{
	var graphData = getDataPtr(sender);
}

function canvasMouseUp(sender)
{
	var graphData = getDataPtr(sender);
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
	progMode = parseInt(sender.id[sender.id.length - 1]);
	console.log(progMode);
}

function setTableMode(sender)
{
	tableMode = parseInt(sender.id[sender.id.length - 1]);
	console.log(tableMode);
}
