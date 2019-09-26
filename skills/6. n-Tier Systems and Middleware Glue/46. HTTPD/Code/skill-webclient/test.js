var express = require('express');
var app = require('express')();
var http = require('http').Server(app);
var io = require('socket.io')(http);
var options = {
  hostname: '192.168.1.114',
  //hostname: 'ptsv2.com', 
  //port: 3000,
  path:'/led',
  //path:'/t/meow/post',
  method: 'POST',
  headers: {
      'Content-Type': 'application/json',
  }
};

var req = http.request(options, function(res) {
  console.log('Status: ' + res.statusCode);
  console.log('Headers: ' + JSON.stringify(res.headers));
  // res.setEncoding('utf8');
  res.on('data', function (body) {
    console.log('Body: ' + body);
  });
});
req.on('error', function(e) {
  console.log('problem with request: ' + e.message);
});
// write data to request body
//req.write('1');
req.send({led_status: '1'} );
req.end();
// var xhr = new XMLHttpRequest();
// xhr.open("POST", '192.168.1.114', true);

// //Send the proper header information along with the request
// xhr.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");

// xhr.onreadystatechange = function() { // Call a function when the state changes.
//     if (this.readyState === XMLHttpRequest.DONE && this.status === 200) {
//         // Request finished. Do processing here.
//     }
// }
// xhr.send("1"); 

// // xhr.send(new Int8Array()); 
// // xhr.send(document);