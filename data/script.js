var rainbowEnable = false;
var connection = new WebSocket('ws://' + location.hostname + ':81/', ['arduino']);

connection.onopen = function () 
{
  connection.send('Connect ' + new Date());
};
connection.onerror = function (error) 
{
  console.log('WebSocket Error ', error);
};
connection.onmessage = function (e) 
{
  console.log('Server: ', e.data);
};
connection.onclose = function () 
{
  console.log('WebSocket connection closed');
};

function sendRGB () 
{
  var r = document.getElementById('r').value;

  var rgb = r;
  var rgbstr = rgb.toString(4);

  connection.send(rgbstr);
}

function rainbowEffect () 
{

}