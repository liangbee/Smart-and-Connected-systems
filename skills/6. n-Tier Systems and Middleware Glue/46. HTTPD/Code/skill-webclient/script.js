 var express = require('express');
 var app = require('express')();
 var http = require('http').Server(app);
 var io = require('socket.io')(http);
 var bodyParser = require('body-parser');
 var request = require('request');

app.use(bodyParser.text({type: 'application/x-www-form-urlencoded'}));

app.get('/', function(req, res){
  res.sendFile(__dirname + '/index.html');
});

app.get('/button', function(req, res){
  res.set('Content-Type', 'application/json');
    res.send({msg: '1'});
});

// request.post(
//     '192.168.1.136',
//     { json: { key: '1' } },
//     function (error, response, body) {
//         if (!error && response.statusCode == 200) {
//             console.log(body)
//         }
//     }
// );

app.post('/', (req, res) => {
  var data = req.body;
  console.log('POST request received!');
  var button = '1';
  res.end();
});

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
http.listen(3000, () => {
  console.log('listening on 3000');
});
