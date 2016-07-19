// Copyright (c) 2016, Intel Corporation.

// Reimplementation of Arduino - Digital - Button example
// * Turns on an LED whenever a button is being pressed

// import gpio module
var gpio = require("gpio");
var pins = require("arduino101_pins");
print("opening GPIO");
//var led = gpio.open({pin: pins.LED0, direction: 'out'});
gpio.open({pin: pins.IO4, direction: 'in', edge: 'any'}).then(function(pin) {
	print("Promise fulfilled");
	pin.onChange = function(event) {
		print("Button changed");
	};
}).docatch(function(error) {
	print("Error opening GPIO pin");
});
