//https://instructobit.com/tutorial/90/Creating-line-graphs-with-Javascript-using-an-HTML-canvas
var mainScrollBox;
var listViewerRot;
var listViewerSingle;

var flowActiveSingle = true;
var flowActiveRot = true;

var wayPoints = [];
var drawPoints = [];

var trackRecFile = "";
var trackRecMode = false;

var testPoints = [0,0,0, 1,0,0, 0.5,0.5,0, 1,0,0, -0.5,-0.5,0, 1,0,0];
var dispSize = [2,2,2];
var canvasSizeGPS = [screen.width, 480];
var dispBoundaries = [-1000, 1000, -1000, 1000, -1000, 1000]; //minX, maxX, minY, maxY, minZ, maxZ
var dispOrigin = [0,0,0];
var dispMode = false;
//var ScalingFactor = [Math.min(dispSize[0], dispSize[1], dispSize[2])/Math.max(dispBoundaries[1] - dispBoundaries[0], dispBoundaries[3] - dispBoundaries[2], dispBoundaries[5] - dispBoundaries[4]);
var ScalingFactor = [dispSize[0]/(dispBoundaries[1] - dispBoundaries[0]), dispSize[1]/(dispBoundaries[3] - dispBoundaries[2])];

function resetDisplay()
{
	wayPoints = [];
	drawPoints = [];
	dispOrigin = [0,0,0];
	dispBoundaries = [-1000, 1000, -1000, 1000, -1000, 1000]; //minX, maxX, minY, maxY, minZ, maxZ
//	ScalingFactor = Math.min(dispSize[0], dispSize[1], dispSize[2])/Math.max(dispBoundaries[1] - dispBoundaries[0], dispBoundaries[3] - dispBoundaries[2], dispBoundaries[5] - dispBoundaries[4]);
	ScalingFactor = [dispSize[0]/(dispBoundaries[1] - dispBoundaries[0]), dispSize[1]/(dispBoundaries[3] - dispBoundaries[2])];
}

function checkBoundaries(newWayPoint)
{
	var needRecalc = false;
	if (newWayPoint[0] < dispBoundaries[0])
	{
		dispBoundaries[0] = 100 * (Math.trunc(newWayPoint[0] / 100) - 1);
		needRecalc = true;
	}
	if (newWayPoint[0] > dispBoundaries[1])
	{
		dispBoundaries[1] = 100 * (Math.trunc(newWayPoint[0] / 100) + 1);
		needRecalc = true;
	}
	if (newWayPoint[1] < dispBoundaries[2])
	{
		dispBoundaries[2] = 100 * (Math.trunc(newWayPoint[1] / 100) - 1);
		needRecalc = true;
	}
	if (newWayPoint[1] > dispBoundaries[3])
	{
		dispBoundaries[3] = 100 * (Math.trunc(newWayPoint[1] / 100) + 1);
		needRecalc = true;
	}
	if (newWayPoint[2] < dispBoundaries[4])
	{
		dispBoundaries[4] = 100 * (Math.trunc(newWayPoint[2] / 100) - 1);
		needRecalc = true;
	}
	if (newWayPoint[2] > dispBoundaries[5])
	{
		dispBoundaries[5] = 100 * (Math.trunc(newWayPoint[2] / 100) + 1);
		needRecalc = true;
	}
	if (needRecalc)
	{
		dispOrigin[0] = Math.round((dispBoundaries[1] + dispBoundaries[0]) / 2);
		dispOrigin[1] = Math.round((dispBoundaries[3] + dispBoundaries[2]) / 2);
		dispOrigin[2] = Math.round((dispBoundaries[5] + dispBoundaries[4]) / 2);
		var newMax = Math.max(dispBoundaries[1] - dispBoundaries[0], dispBoundaries[3] - dispBoundaries[2], dispBoundaries[5] - dispBoundaries[4]); 
		ScalingFactor = [dispSize[0]/(dispBoundaries[1] - dispBoundaries[0]), dispSize[1]/(dispBoundaries[3] - dispBoundaries[2])];
		drawPoints = [];
		for (var i = 0; i < wayPoints.length; i++)
		{
			if ((i % 6) < 3)
				drawPoints.push((wayPoints[i] - dispOrigin[i % 3]) * ScalingFactor[i % 2]); //dirty trick! z axis uses scalingfactor 0
			else
				drawPoints.push(wayPoints[i]); //RGB values
		}
//		console.log(dispBoundaries[0],dispBoundaries[1],dispBoundaries[2],dispBoundaries[3],dispBoundaries[4],dispBoundaries[5]);
	}
}

function addWayPoint(newWayPoint)
{
	checkBoundaries(newWayPoint);
	wayPoints.push(newWayPoint[0]); //x
	drawPoints.push((newWayPoint[0] - dispOrigin[0]) * ScalingFactor[0]);
	wayPoints.push(newWayPoint[1]); //y
	drawPoints.push((newWayPoint[1] - dispOrigin[1]) * ScalingFactor[1]);
	wayPoints.push(newWayPoint[2]); //z
	drawPoints.push((newWayPoint[2] - dispOrigin[2]) * ScalingFactor[0]);
	wayPoints.push(1); //R
	wayPoints.push(0); //G
	wayPoints.push(0); //B
	drawPoints.push(1); //R
	drawPoints.push(0); //G
	drawPoints.push(0); //B
}

var vertexShaderText = 
[
	'precision mediump float;',
	'',
	'attribute vec3 vertPosition;',
	'attribute vec3 vertColor;',
	'varying vec3 fragColor;',
	'void main()',
	'{',
	'  fragColor = vertColor;',
	'  gl_Position = vec4(vertPosition, 1.0);',
//	'  gl_PointSize = 10.0;',
	'}'
].join('\n');

var fragmentShaderText =
[
	'precision mediump float;',
	'',
	'varying vec3 fragColor;',
	'void main()',
	'{',
	'  gl_FragColor = vec4(fragColor, 1.0);',
	'}'
].join('\n');

function renderImage(newImage)
{
}

function graphicMain() 
{
	return;
	const canvas = document.querySelector("#glCanvasSpeed");
	// Initialize the GL context
	const gl = canvas.getContext("webgl");

	// Only continue if WebGL is available and working
	if (gl === null) 
	{
		alert("Unable to initialize WebGL. Your browser or machine may not support it.");
		return;
	}

	// Set clear color to black, fully opaque
	gl.clearColor(0.75, 0.85, 0.8, 1.0);
	// Clear the color buffer with specified clear color
	gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

//	var image = new Image();
//	image.src = "sfwb.jpg";  // MUST BE SAME DOMAIN!!!
//	image.onload = function() {renderImage(image);}

	var vertexShader = gl.createShader(gl.VERTEX_SHADER);
	var fragmentShader = gl.createShader(gl.FRAGMENT_SHADER);

	gl.shaderSource(vertexShader, vertexShaderText);
	gl.shaderSource(fragmentShader, fragmentShaderText);

	gl.compileShader(vertexShader);
	if (!gl.getShaderParameter(vertexShader, gl.COMPILE_STATUS)) {
		console.error('ERROR compiling vertex shader!', gl.getShaderInfoLog(vertexShader));
		return;
	}

	gl.compileShader(fragmentShader);
	if (!gl.getShaderParameter(fragmentShader, gl.COMPILE_STATUS)) {
		console.error('ERROR compiling fragment shader!', gl.getShaderInfoLog(fragmentShader));
		return;
	}

	var program = gl.createProgram();
	gl.attachShader(program, vertexShader);
	gl.attachShader(program, fragmentShader);
	gl.linkProgram(program);
	if (!gl.getProgramParameter(program, gl.LINK_STATUS)) {
		console.error('ERROR linking program!', gl.getProgramInfoLog(program));
		return;
	}
	gl.validateProgram(program);
	if (!gl.getProgramParameter(program, gl.VALIDATE_STATUS)) {
		console.error('ERROR validating program!', gl.getProgramInfoLog(program));
		return;
	}

	var triangleVertexBufferObject = gl.createBuffer();
	gl.bindBuffer(gl.ARRAY_BUFFER, triangleVertexBufferObject);
	gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(drawPoints), gl.STATIC_DRAW);

	var positionAttribLocation = gl.getAttribLocation(program, 'vertPosition');
	var colorAttribLocation = gl.getAttribLocation(program, 'vertColor');
	gl.vertexAttribPointer(
		positionAttribLocation, // Attribute location
		3, // Number of elements per attribute
		gl.FLOAT, // Type of elements
		gl.FALSE,
		6 * Float32Array.BYTES_PER_ELEMENT, // Size of an individual vertex
		0 // Offset from the beginning of a single vertex to this attribute
	);
	gl.vertexAttribPointer(
		colorAttribLocation, // Attribute location
		3, // Number of elements per attribute
		gl.FLOAT, // Type of elements
		gl.FALSE,
		6 * Float32Array.BYTES_PER_ELEMENT, // Size of an individual vertex
		3 * Float32Array.BYTES_PER_ELEMENT // Offset from the beginning of a single vertex to this attribute
	);

	gl.enableVertexAttribArray(positionAttribLocation);
	gl.enableVertexAttribArray(colorAttribLocation);

	//
	// Main render loop
	//
	gl.useProgram(program);
	gl.drawArrays(gl.LINE_STRIP, 0, Math.round(drawPoints.length/6)); //wayPoints.length);
//	console.log(Math.round(drawPoints.length/6));

}
