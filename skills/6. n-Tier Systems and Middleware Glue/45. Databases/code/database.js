/* Database.js extract and send all lines from the smoke.txt files into an array and send to database*/

// Modules
var level = require('level')
var express = require('express');
var app = require('express')();
var http = require('http').Server(app);
var io = require('socket.io')(http);

// Create or open the underlying LevelDB store
var db = level('./mydb', {valueEncoding: 'json'});


var fs = require('fs');
var parse = require('csv-parse');

var inputFile='Smoke.txt';
console.log("Processing Smoke.txt");

var parser = parse({delimiter: '\t'}, function (err, data) {
    // when all countries are available,then process them
    // note: array element at index 0 contains the row of headers that we should skip
    var i = 10000;
    data.forEach(function(line) {
      // create smoke object out of parsed fields
      var smoke = { Time : line[0]
                    , ID : line[1]
                    , Smoke : line[2]
                    , Temperature : line[3]
                    };

     //db.put(['000'+i.toString()], [smoke],... still does not work for single digits like 0002
     db.put(['000'+i.toString()], [smoke], function (err) {
      if (err) return console.log('Ooops!', err) // some kind of I/O error
    })
     i++;
     console.log(JSON.stringify(smoke) + 'hi' + i);
    });

    console.log('done?');
    readDB();
});


// Function to stream from database
function readDB(arg) {
  console.log('Reading from db...');
  var i = 0;
  db.createReadStream()
    .on('data', function (data) {
      console.log(data.key, '=', data.value)
      // Parsed the data into a structure but don't have to ...


      var dataIn = {[i]: data.value};
      console.log(dataIn[i][0].Temp + ' ' + i);
      i++;


      // Stream data to client
      io.emit('message', dataIn);
    })
    .on('error', function (err) {
      console.log('Oh my!', err)
    })
    .on('close', function () {
      console.log('Stream closed')
    })
    .on('end', function () {
      console.log('Stream ended')
    })
}


// read the inputFile, feed the contents to the parser
fs.createReadStream(inputFile).pipe(parser);
