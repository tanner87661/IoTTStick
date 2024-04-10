var canvasSizeSpeed = [screen.width-300, 400, 50, 50, 150, 50];
var canvasSizeAngle = [screen.width-300, 400, 50, 50, 150, 50];
var canvasSizeRadius = [screen.width-300, 400, 50, 50, 150, 50];
var canvasSizeStability = [screen.width-300, 400, 50, 50, 150, 50];

var bgColor = "rgb(160,160,160)";

var graphColorSpeed = "rgb(165,165,165)";
var graphColorRadius = "rgb(165,165,165)";
var graphColorGrade = "rgb(165,165,165)";
var graphColorStability = "rgb(165,165,165)";

var graphColorTechSpeed = "rgb(255, 50, 50)";
var graphColorScaleSpeed = "rgb(30, 128, 30)";
var graphColorSpeedStep = "rgb(50, 50, 255)";

var graphColorSlope = "rgb(255, 50, 50)";
var graphColorSElevation = "rgb(30, 128, 30)";

var graphColorStab1 = "rgb(255, 50, 50)";
var graphColorStab2 = "rgb(30, 128, 30)";
var graphColorStab3 = "rgb(50, 50, 255)";

var lineGraphTemplate = {"Name":"Title", "Dim":"km/h", "Color":"blue","DefaultVals":[0,100], "Show" : true,  "DataElements":[]};
var DataElementTemplate = {"x":0,"y":0};
var lineDispTemplate = {"Title":"title", "DispDim":[0,0], "DispIncr" : 1, "XDim" : 0, "MaxXRange":120, "FieldCol": "rgb(100,100,100)", "BoxCol": "rgb(120,120,120)", "LineGraphs":[]};

var lineGraphTechSpeed;
var lineGraphScaleSpeed;
var lineGraphSpeedStep;
var speedGraph;

var lineGraphRadius;
var lineGraphHeading;
var radiusGraph;

var lineGraphGrade;
var lineGraphElevation;
var angleGraph;

var lineGraphStab1;
var lineGraphStab2;
var lineGraphStab3;
var stabilityGraph;


function setGraphBaseLine(baseLineIndex)
{
	speedGraph.XDim = baseLineIndex;
	speedGraph.MaxXRange = ((baseLineIndex === 0) ? 120 : 1000);
	radiusGraph.XDim = baseLineIndex;
	radiusGraph.MaxXRange = ((baseLineIndex === 0) ? 120 : 1000);
	angleGraph.XDim = baseLineIndex;
	angleGraph.MaxXRange = ((baseLineIndex === 0) ? 120 : 1000);
	stabilityGraph.XDim = baseLineIndex;
	stabilityGraph.MaxXRange = ((stabilityGraph === 0) ? 120 : 1000);
//	console.log("changed ", baseLineIndex);
}

function clearLineGraphData(thisGraph)
{
	for (var i = 0; i < thisGraph.LineGraphs.length; i++)
		thisGraph.LineGraphs[i].DataElements = [];
}

function clearAllGraphData()
{
	clearLineGraphData(speedGraph);
	clearLineGraphData(radiusGraph);
	clearLineGraphData(angleGraph);
	clearLineGraphData(stabilityGraph);
}

function loadSpeedGraphData()
{
	lineGraphTechSpeed = JSON.parse(JSON.stringify(lineGraphTemplate));
	lineGraphTechSpeed.Name = "Phys. Speed";
	lineGraphTechSpeed.Dim = "mm/s";
	lineGraphTechSpeed.Color = graphColorTechSpeed;
	lineGraphTechSpeed.DefaultVals = [-700,700];

	lineGraphScaleSpeed = JSON.parse(JSON.stringify(lineGraphTemplate));
	lineGraphScaleSpeed.Name = "Scale Speed";
	lineGraphScaleSpeed.Dim = (configData[workCfg].Units == 0) ? "km/h" : "mph";
	lineGraphScaleSpeed.Color = graphColorScaleSpeed;
	lineGraphScaleSpeed.DefaultVals = [-200,200];

	lineGraphSpeedStep = JSON.parse(JSON.stringify(lineGraphTemplate));
	lineGraphSpeedStep.Name = "Speed Step";
	lineGraphSpeedStep.Dim = "%";
	lineGraphSpeedStep.Color = graphColorSpeedStep;
	lineGraphSpeedStep.DefaultVals = [-128,128];

	speedGraph = JSON.parse(JSON.stringify(lineDispTemplate));
	speedGraph.Title = "Speed Data";
	speedGraph.DispDim = canvasSizeSpeed;
	speedGraph.MaxDispElX = 100;
	speedGraph.FieldCol = bgColor;
	speedGraph.BoxCol = graphColorSpeed;
	speedGraph.LineGraphs.push(lineGraphTechSpeed);
	speedGraph.LineGraphs.push(lineGraphScaleSpeed);
	speedGraph.LineGraphs.push(lineGraphSpeedStep);
//	console.log(speedGraph);

	lineGraphRadius = JSON.parse(JSON.stringify(lineGraphTemplate));
	lineGraphRadius.Name = "Curve Radius";
	lineGraphRadius.Dim = (configData[workCfg].Units == 0) ? "mm" : "in";;
	lineGraphRadius.Color = graphColorTechSpeed;
	lineGraphRadius.DefaultVals = [-2500,2500];

	lineGraphHeading = JSON.parse(JSON.stringify(lineGraphTemplate));
	lineGraphHeading.Name = "Heading";
	lineGraphHeading.Dim = "deg";
	lineGraphHeading.Color = graphColorScaleSpeed;
	lineGraphHeading.DefaultVals = [0,360];

	radiusGraph = JSON.parse(JSON.stringify(lineDispTemplate));
	radiusGraph.Title = "Track Direction";
	radiusGraph.DispDim = canvasSizeRadius;
	radiusGraph.MaxDispElX = 100;
	radiusGraph.FieldCol = graphColorRadius;
	radiusGraph.LineGraphs.push(lineGraphRadius);
	radiusGraph.LineGraphs.push(lineGraphHeading);
//	console.log(radiusGraph);

	lineGraphGrade = JSON.parse(JSON.stringify(lineGraphTemplate));
	lineGraphGrade.Name = "Grade";
	lineGraphGrade.Dim = "%";;
	lineGraphGrade.Color = graphColorSlope;
	lineGraphGrade.DefaultVals = [-5,5];

	lineGraphElevation = JSON.parse(JSON.stringify(lineGraphTemplate));
	lineGraphElevation.Name = "Superelevation";
	lineGraphElevation.Dim = "%";
	lineGraphElevation.Color = graphColorSElevation;
	lineGraphElevation.DefaultVals = [-5,5];

	angleGraph = JSON.parse(JSON.stringify(lineDispTemplate));
	angleGraph.Title = "Track Angles";
	angleGraph.DispDim = canvasSizeAngle;
	angleGraph.MaxDispElX = 100;
	angleGraph.FieldCol = bgColor;
	angleGraph.BoxCol = graphColorGrade;
	angleGraph.LineGraphs.push(lineGraphGrade);
	angleGraph.LineGraphs.push(lineGraphElevation);
//	console.log(angleGraph);
}

function loadStabilityGraphData()
{
	lineGraphStab1 = JSON.parse(JSON.stringify(lineGraphTemplate));
	lineGraphStab1.Name = "0.25 sec Avg. Speed";
	lineGraphStab1.Dim = "mm/s";
	lineGraphStab1.Color = graphColorStab1;
	lineGraphStab1.DefaultVals = [-128,128];

	lineGraphStab2 = JSON.parse(JSON.stringify(lineGraphTemplate));
	lineGraphStab2.Name = "1 sec Avg. Speed";
	lineGraphStab2.Dim = "mm/s";
	lineGraphStab2.Color = graphColorStab2;
	lineGraphStab2.DefaultVals = [-128,128];

	lineGraphStab3 = JSON.parse(JSON.stringify(lineGraphTemplate));
	lineGraphStab3.Name = "5 sec Avg. Speed";
	lineGraphStab3.Dim = "mm/s";
	lineGraphStab3.Color = graphColorStab3;
	lineGraphStab3.DefaultVals = [-128,128];

	stabilityGraph = JSON.parse(JSON.stringify(lineDispTemplate));
	stabilityGraph.Title = "Speed Stability";
	stabilityGraph.DispDim = canvasSizeStability;
	stabilityGraph.MaxDispElX = 100;
	stabilityGraph.FieldCol = bgColor;
	stabilityGraph.BoxCol = graphColorStability;
	stabilityGraph.LineGraphs.push(lineGraphStab1);
	stabilityGraph.LineGraphs.push(lineGraphStab2);
	stabilityGraph.LineGraphs.push(lineGraphStab3);

/*
	speedVerifyProfileGraph = JSON.parse(JSON.stringify(profileTemplate));
	speedVerifyProfileGraph.Title = "Throttle Position to Scale Speed";
	speedVerifyProfileGraph.DispDim = canvasSizeSpeedVerify;
	speedVerifyProfileGraph.BoxCol = graphBoxSpeedVerify;
	speedVerifyProfileGraph.FieldCol = graphBackgroundSpeedVerify;

	profileBox = JSON.parse(JSON.stringify(profileGraphTemplate));
	profileBox.DimX = "Throttle Step";
	profileBox.DimY = (configData[workCfg].Units == 0) ? "Scale km/h" : "Scale mph";
//	profileBox.ValsX = [0,100,10];
//	profileBox.ValsY = [0,150,10];
	profileBox.ValsX = [0,throttleDef.SpeedSteps,10];
	profileBox.ValsY = [0,50 * (Math.trunc(throttleDef.VMax/50) + 1),10];
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
	speedVerifyProfileGraph.LineGraphs.push(profileBox);

	profileBox = JSON.parse(JSON.stringify(profileGraphTemplate));
	profileBox.DimX = "Scale Speed fw";
	profileBox.DimY = (configData[workCfg].Units == 0) ? "km/h" : "mph";
	profileBox.ValsX = [0,100,10];
	profileBox.ValsY = [0,150,10];
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
	speedVerifyProfileGraph.LineGraphs.push(profileBox);

	profileBox = JSON.parse(JSON.stringify(profileGraphTemplate));
	profileBox.DimX = "Scale Speed bw";
	profileBox.DimY = (configData[workCfg].Units == 0) ? "km/h" : "mph";
	profileBox.ValsX = [0,100,10];
	profileBox.ValsY = [0,150,10];
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
	speedVerifyProfileGraph.LineGraphs.push(profileBox);
*/
}

function drawLineGraph(ofCanvas, dispObj)
{
	var context = ofCanvas.getContext( "2d" );  

	ofCanvas.width = dispObj.DispDim[0];
	ofCanvas.height = dispObj.DispDim[1];
   
	// declare graph start and end  
	var TOP_FRAME = 50;
	var BOTTOM_FRAME = 50;
	var LEFT_FRAME = 150;
	var RIGHT_FRAME = 50;

	var GRAPH_TOP = TOP_FRAME;  
	var GRAPH_BOTTOM = ofCanvas.height - BOTTOM_FRAME;  
	var GRAPH_LEFT = LEFT_FRAME;  
	var GRAPH_RIGHT = ofCanvas.width - RIGHT_FRAME;  
	var GRAPH_HEIGHT = ofCanvas.height - TOP_FRAME - BOTTOM_FRAME;   
	var GRAPH_WIDTH = ofCanvas.width - LEFT_FRAME - RIGHT_FRAME;   
	var GRAPH_CENTER =  TOP_FRAME + Math.round(GRAPH_HEIGHT/2);
   
	// clear canvas (if another graph was previously drawn)  
	//context.clearRect( 0, 0, ofCanvas.getAttribute("width"), ofCanvas.getAttribute("height") );   
	context.fillStyle = dispObj.FieldCol;
	context.fillRect(0, 0, ofCanvas.width, ofCanvas.height);

	context.fillStyle = dispObj.FieldCol;
	context.strokeStyle = "black";
	context.fillRect(0, 0, ofCanvas.width, ofCanvas.height);
	context.stroke();  

	context.fillStyle = dispObj.BoxCol;
	context.fillRect(GRAPH_LEFT, GRAPH_TOP, GRAPH_RIGHT-GRAPH_LEFT, GRAPH_BOTTOM-GRAPH_TOP);
	context.stroke();  

	context.font = "14px Arial";  
	context.fillStyle = "black";
	context.strokeStyle = "black";
	//analyze data lines and calculate min/max
	var hasNegativeVals = false;
	for (var i = 0; i < dispObj.LineGraphs.length; i++)
	{
		if (dispObj.LineGraphs[i].Show)
		{
			for (var j = 0; j < dispObj.LineGraphs[i].DataElements.length; j++)
			{
				if (dispObj.LineGraphs[i].DataElements[j].y < dispObj.LineGraphs[i].DefaultVals[0])
					dispObj.LineGraphs[i].DefaultVals[0] = dispObj.LineGraphs[i].DataElements[j].y;
				if (dispObj.LineGraphs[i].DataElements[j].y > dispObj.LineGraphs[i].DefaultVals[1])
					dispObj.LineGraphs[i].DefaultVals[1] = dispObj.LineGraphs[i].DataElements[j].y;
			}
			hasNegativeVals |= (dispObj.LineGraphs[i].DefaultVals[0] < 0);
		}
	}

	// draw X and Y axis  
	context.beginPath();  
	context.moveTo( GRAPH_LEFT, GRAPH_TOP );  
	context.lineTo( GRAPH_LEFT, GRAPH_BOTTOM );
	context.moveTo( GRAPH_LEFT-5, hasNegativeVals ? GRAPH_CENTER : GRAPH_TOP );  
	context.lineTo( GRAPH_RIGHT, hasNegativeVals ? GRAPH_CENTER : GRAPH_TOP );  
	context.font = "16px Arial";  
	context.fillStyle = "black";
	context.textAlign = "center";
	context.fillText( "0", GRAPH_LEFT + 10, 20 + (hasNegativeVals ? GRAPH_CENTER : GRAPH_BOTTOM ));  
	context.fillText( dispObj.XDim === 0 ? "[s]" : configData[workCfg].Units === 0 ? "[cm]" : "in", GRAPH_RIGHT - Math.round(GRAPH_WIDTH / 20), 20 + (hasNegativeVals ? GRAPH_CENTER : GRAPH_BOTTOM ));  
	for (var i = 1; i < 11; i++)
	{
		context.moveTo( GRAPH_LEFT + (i * Math.round(GRAPH_WIDTH / 10)), (hasNegativeVals ? GRAPH_CENTER : GRAPH_BOTTOM ));  
		context.lineTo( GRAPH_LEFT + (i * Math.round(GRAPH_WIDTH / 10)), 5 + (hasNegativeVals ? GRAPH_CENTER : GRAPH_BOTTOM ));  
		context.fillText(Math.round(i * dispObj.MaxXRange/10).toString(), GRAPH_LEFT + (i * Math.round(GRAPH_WIDTH / 10)), 20 + (hasNegativeVals ? GRAPH_CENTER : GRAPH_BOTTOM ));  
	}
	context.stroke();  

	// draw reference lines
	context.beginPath();   
	// set light grey color for reference lines  
	context.strokeStyle = "#BBB";  
	var numRefLines = 5;
	context.beginPath();  
	for (var i = 1; i <= numRefLines; i++)
		if (hasNegativeVals)
		{
			context.moveTo( GRAPH_LEFT-5, Math.round(GRAPH_CENTER - ((GRAPH_HEIGHT/(2*numRefLines)) * i)));  
			context.lineTo( GRAPH_RIGHT, Math.round(GRAPH_CENTER - ((GRAPH_HEIGHT/(2*numRefLines)) * i)));  
			context.moveTo( GRAPH_LEFT-5, Math.round(GRAPH_CENTER + ((GRAPH_HEIGHT/(2*numRefLines)) * i)));  
			context.lineTo( GRAPH_RIGHT, Math.round(GRAPH_CENTER + ((GRAPH_HEIGHT/(2*numRefLines)) * i)));  
		}
		else
		{
			context.moveTo( GRAPH_LEFT-5, Math.round(GRAPH_BOTTOM - ((GRAPH_HEIGHT/numRefLines) * i)));  
			context.lineTo( GRAPH_RIGHT, Math.round(GRAPH_BOTTOM - ((GRAPH_HEIGHT/numRefLines) * i)));  
		}
	context.stroke();  

    // draw titles  

	context.font = "20px Arial";  
	context.fillStyle = "black";
	context.textAlign = "center";
	context.fillText( dispObj.Title, Math.round(GRAPH_RIGHT / 2), 30);  
	for (var i = 0; i < dispObj.LineGraphs.length; i++)
	{
		if (dispObj.LineGraphs[i].Show)
		{
			context.font = "16px Arial";  
			context.fillStyle = dispObj.LineGraphs[i].Color;
			context.textAlign = "left";
			context.fillText( dispObj.LineGraphs[i].Name, GRAPH_LEFT + i * 150, ofCanvas.height - 5);  
			context.textAlign = "right";
		}
	}
	context.stroke();  

//finally, draw the data. x axis is same for all
	var xScale = GRAPH_WIDTH / dispObj.MaxXRange;
	
	for (var i = 0; i < dispObj.LineGraphs.length; i++)
	{
		if (dispObj.LineGraphs[i].DataElements.length > 0)
		{
			var xOrigin = dispObj.LineGraphs[i].DataElements[0].x;
//			console.log(xOrigin);
		
			if (dispObj.LineGraphs[i].Show)
			{
				var maxVal = Math.max(Math.abs(dispObj.LineGraphs[i].DefaultVals[0]), Math.abs(dispObj.LineGraphs[i].DefaultVals[1]));
				var yScale = GRAPH_HEIGHT / maxVal;
				if (hasNegativeVals)
					yScale /= 2;
				context.strokeStyle = dispObj.LineGraphs[i].Color;
				context.moveTo( GRAPH_LEFT, (hasNegativeVals ? GRAPH_CENTER : GRAPH_BOTTOM) - (yScale * dispObj.LineGraphs[i].DataElements[0].y));  
				context.beginPath();  
				for (var j = 1; j < dispObj.LineGraphs[i].DataElements.length; j++)
					if ((dispObj.LineGraphs[i].DataElements[j].length == 3) || (dispObj.LineGraphs[i].DataElements[j-1].length == 3))
						context.moveTo( GRAPH_LEFT + (xScale * (dispObj.LineGraphs[i].DataElements[j].x - xOrigin) / 1000), (hasNegativeVals ? GRAPH_CENTER : GRAPH_BOTTOM) - (yScale * dispObj.LineGraphs[i].DataElements[j].y));  
					else
						context.lineTo( GRAPH_LEFT + (xScale * (dispObj.LineGraphs[i].DataElements[j].x - xOrigin) / 1000), (hasNegativeVals ? GRAPH_CENTER : GRAPH_BOTTOM) - (yScale * dispObj.LineGraphs[i].DataElements[j].y));  
				context.stroke();  

//display the scale
				context.font = "16px Arial";  
				context.fillStyle = dispObj.LineGraphs[i].Color;
				context.textAlign = "right";
				for (var j = 1; j <= numRefLines; j++)
				{
					var thisVal = j * maxVal/numRefLines;
					context.fillText(+thisVal.toFixed(2).toString(), GRAPH_LEFT - (50 * i) - 10, (hasNegativeVals ? GRAPH_CENTER : GRAPH_BOTTOM) + 7 - (yScale * thisVal)); 
					if (hasNegativeVals)
						context.fillText(-thisVal.toFixed(2).toString(), GRAPH_LEFT - (50 * i) - 10, (hasNegativeVals ? GRAPH_CENTER : GRAPH_BOTTOM) + 7 + (yScale * thisVal)); 
				} 
				context.fillText( "[" + dispObj.LineGraphs[i].Dim + "]", GRAPH_LEFT - 10 - (50 * i), GRAPH_TOP - 15);  
			}
		}
	}



//context.beginPath();  
//context.lineJoin = "round";  
//context.strokeStyle = "black";  
  
//context.moveTo( GRAPH_LEFT, ( GRAPH_HEIGHT - dataArr[ 0 ] / largest * GRAPH_HEIGHT ) + GRAPH_TOP );  
// draw reference value for day of the week  
//context.fillText( "1", 15, GRAPH_BOTTOM + 25);  
/*
for( var i = 1; i < arrayLen; i++ )
{  
	context.lineTo( GRAPH_RIGHT / arrayLen * i + GRAPH_LEFT, ( GRAPH_HEIGHT - dataArr[ i ] / largest * GRAPH_HEIGHT ) + GRAPH_TOP );  
    // draw reference value for day of the week  
    context.fillText( ( i + 1 ), GRAPH_RIGHT / arrayLen * i, GRAPH_BOTTOM + 25);  
}
*/  
//	context.stroke();  
	
}
