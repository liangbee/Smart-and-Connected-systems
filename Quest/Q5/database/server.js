// Modules
var level = require('level')
var express = require('express');
var app = require('express')();
var http = require('http').Server(app);
var io = require('socket.io')(http);
var db = level('./smartdb', {valueEncoding: 'json'});
var bodyParser = require('body-parser');
var request = require('request');
var codebase = [ 1111 ]; // fob 1's code
var thisfobbase = [ 1 ]; // only fob 1 has access to this hub
var fobbase = [ 1, 2, 3 ]; // all fobs
var ipbase = [ '192.168.1.143', '192.168.1.114', '192.168.1.122' ]; // ip addresses of fobs

// app.use(bodyParser.urlencoded({extended: false}));
app.use(bodyParser.text({type: 'application/x-www-form-urlencoded'}));


app.get('/', function(req, res){
  res.sendFile(__dirname + '/index.html');
});

// handles post request from security hub
app.post('/hub', (req, res) => {
  var data = req.body.split(':');
  var fob_id = parseInt(data[0], 10);
  var code = parseInt(data[1], 10);
  var hub_id = parseInt(data[2], 10);
  var date = new Date();
  var time = date.getFullYear() + ':' + date.getMonth() + ':' + date.getDate() + ':' + date.getHours() + ':' + date.getMinutes() + ':' + date.getSeconds();
  var validation = validate(code, fob_id);
  send_validate(fob_id, validation);
  log(time, fob_id, code, hub_id, validation);
  read();
  console.log('PUT request received! Fob_id: ' + fob_id + '  Code: ' + code + ' Validation: ' + validation);
  res.end();
});

// tests
// setInterval(test_log, 1000);
// setInterval(test_validate, 2000);
// setInterval(read, 1000);
// setInterval(send_validate, 1000);

// when a new client connects
var clientConnected = 0;
io.on('connection', function(socket){
  console.log('a user connected');
  clientConnected = 0;

  read();
  clientConnected = 1;
  socket.on('disconnect', function(){
    console.log('user disconnected');
  });
});

// listens on port 1111
http.listen(1111, () => {
  console.log('listening on 1111');
});

// reads from database
function read () {
  db.createReadStream()
  .on('data', function (data) {
    var time = data.key;
    var value = data.value[0];
    var fob_id = value.fob_id;
    var hub_id = value.hub_id;
    var validation = value.validation;
    var msg_value = [{fob_id: fob_id,
                      validation: validation
                    }];
    var msg = { [time] : msg_value };
    io.emit('message', msg);
  })
  .on('error', function (err) {
    console.log('Oh my!', err)
  })
  .on('close', function () {
    // console.log('Stream closed')
  })
  .on('end', function () {
    // console.log('Stream ended')
  })
}

// sends validate signal by making POST request
// @param fob_id   ID of fob that makes unlock request
// @param validation   Sends 1 if validation == true, otherwise sends 0
function send_validate (fob_id, validation) {
  var fob_index = fobbase.indexOf(fob_id);
  if(fob_index >= 0) {
    var ip = ipbase[fob_index];
    var valid_key = (validation == true) ? '1' : '0';
    console.log(ip + ' ' + valid_key);
    request.put('http://' + ip + '/ctrl', {form: {key: valid_key}});
  }
}

// logs information received from security hub into database
function log (time, fob_id, code, hub_id, validation) {
  var key = time;
  var value = [ {fob_id: fob_id,
                code: code,
                validation: validation,
                hub_id: hub_id
              } ];
  db.put(key, value, function (err) {
    if(err) return console.log('Oooops', err);
  });
}

// checks if input code of fob_id matches with codebase
// return true if passes validation
// return false otherwise
function validate (input_code, fob_id) {
  var fob_index = thisfobbase.indexOf(fob_id);
  if(fob_index < 0) return false;
  return input_code == codebase[fob_index];
}

// test function to check if log function works
function test_log () {
  var fob_id = getRndInteger(1, 3);
  var code = 1111;
  var hub_id = 1;
  var date = new Date();
  var time = date.getFullYear() + ':' + date.getMonth() + ':' + date.getDate() + ':' + date.getHours() + ':' + date.getMinutes() + ':' + date.getSeconds();
  var validation = validate(code, fob_id);
  log(time, fob_id, code, hub_id, validation);
  console.log(fob_id + ' ' + code + ' ' + validation);
  console.log('testing --- calling log function');
}

// test function to check if validate function works
function test_validate() {
  var fob_id = 3;
  var code = 1111;
  var validation = validate(code, fob_id);
  send_validate(fob_id, validation);
}

// helper function to get random integers
function getRndInteger(min, max) {
    return Math.floor(Math.random() * (max - min + 1) ) + min;
}
