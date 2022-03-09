
function setup() {
  createCanvas(800, 600);
  background(0);
}

function draw() {
  noStroke();
  fill(255, 50);
  circle(mouseX, mouseY, 24);
}

function mousePressed() {
  background(0);
}
