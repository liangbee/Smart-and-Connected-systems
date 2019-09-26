# Smart Key
Author: Ellen Lo, Lin Ma, Biyao Liang, 2018-12-11

## Summary
In this quest, we put together 3 smart key fobs and 1 smart hub which are communicating with a node.js server hosted on raspberry pi. The smart key fobs sends its identification to the smart hub nearby, and the smart hub sends the fob's identification and its own id(denoting location) to the node server via an HTTP put request. Then, the server reads the request and determines whether the key fob should gain access to the smart hub. If so, the node server sends an unlock signal to the key fob. A web client is also made for the users to track communication.

## Evaluation Criteria

We decided on the following specifications for a successful solution to this quest:

- Fob relays {fob_id,code} to security hub, hub sends ID, code and location to server; server responds to fob, fob green line turns on.
- Fob key accesses attempts are logged to database: {time stamp, validation, fob id, code}
- Database with Leveldb and server with Nodejs and Express framework on Raspberry Pi.
- Web-based management interface shows real-time active unlocked fobs and history of unlocked fobs (with actual time of unlock).
- Uses 3 fobs with unique values {id, code}: {1, 1111}, {2, 2222}, and {3, 1111}
- Security vulnerabilities of design are articulated with proposed mitigation: fob 1 and 2 are pre-registered, and fob 1 and 3 have the same code, but only fob 1 has access

## Solution Design
We have put together 3 smart key fobs each with unique ID and code. Fob 1 has code 1111 and has access to the hub 1. Its information is also pre-registered. Fob 2 has code 2222 and does not have access to hub 1. However, its information is pre-registered. Fob 3 has code 1111 and should not have access to hub 1, becuase its information is not pre-registered, making fob 3 itself an illegitimate key fob.

Once the button on a key fob is pressed, the key fob sends its ID and code through the IR LED to nearby Hub. With '0x0A' as the start byte, the following bytes indicate the fob id digit and the 4 digits of code. The hub is always listening, and once it receives a signal from a fob, it sends the fob's information (which is in the signal) and hub's own ID (indicating its location) to the node server via HTTP POST request. The data is a string formatted like this: {FOB_ID}:{CODE}:{HUB_ID}.

The node server is always listening to the hub. Once it receives a signal from hub, it time stamps the request and compare the fob id the code and the hub ID to determine whether the fob should gain access to the hub. All incoming unlock attempts sent to the node server are logged in the database. The node server sends a HTTP PUT request back to the fob with a '1' or '0' indicating successful or failed unlock attempts. 

All log history in the server are displayed on a web client. Figure 2 shown under the Photos section is the interface of the web client. Y-axis shows fob id, x-axis shows time of all unlock attempts. A black circular marker indicates a successful unlocked fob, and a red cross marker indicates a failed attempt to unlock fob. The graph is drawn when the website is loaded at first and updates when a POST request from security hub. 

We successfully ran the web server on pi. By SSH-ing into pi's IP, we host server by running 'node server.js' and it starts listening on port 1111. Our pi's IP is 192.168.1.134, so the graph can be viewed on http://192.168.1.134:1111. 

## Photos

![Fig_1](https://github.com/BU-EC444/Quest5-Team9-Lo-Liang-Ma/blob/master/Artifacts/flow_chart.jpg)

![Fig 2](https://github.com/BU-EC444/Quest5-Team9-Lo-Liang-Ma/blob/master/Artifacts/graph.png)

![Fig 3](https://github.com/BU-EC444/Quest5-Team9-Lo-Liang-Ma/blob/master/Artifacts/console.png)

## Links
-[Video demo](https://www.youtube.com/watch?v=5QzJb4LWRRQ)

## Modules, Tools, Source Used in Solution
-[quest5 on whizzer.bu.edu](http://whizzer.bu.edu/team-quests/primary/smart-key)

## Supporting Artifacts
