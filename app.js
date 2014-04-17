var express = require("express.io");
var serial = require("serialport").SerialPort;

// Listen for messages from Arduino
var port = new serial("/dev/tty.usbmodem1421", {
  baudrate: 9600
});

port.on("open", function() {
  console.log("> serial connection opened");

  port.on("data", function(data) {
    if (data == 0) {
      // TODO ~ Ignore superfluous fires
      console.log("> up button pushed");
    }
  });
});