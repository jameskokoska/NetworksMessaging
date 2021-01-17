# Networks Messaging Program
A simple chat messaging program using UNIX TCP sockets across a local network with several features.
## Usage
1. Compile the respective programs. For e.g., using gcc:   
```gcc deliver.c -lpthread -o deliver``` and ```gcc server.c -lpthread -o server``` 
2. Get the current local IP . For e.g., using curl:  
```curl ifconfig.me```
3. Run the compiled ```deliver``` and ```server``` programs on the same network  
Use the following syntax to run the programs:
```deliver``` and ```server <port number>```
4. Login through ```deliver```  
You can use these logins provided in ```server.c```  
| User | Password |
| ------------- | ------------- |
| Yash | 000 |
| James | 123 |
| jim | 456 |
| bob | 789 |
5. Use the login command with the following syntax:  
```/login <User> <Password> <local IP/server address> <port>```  
The server does not need any login and should be already running (see step 3)
6. Multiple users may connect to the same server (repeat steps 3 for ```deliver```)

## Features
* Server registers new users to a database and assigns a unique ID
* Server maintains a database of IDs and passwords for login
* When a client logs in, a bind is created between the client and the server until the client disconnects
* Client to client communication is passed through the server
* Server manages input and distribution of chat messages
* Console colours for ease of differentiating unique types of messages
* Special commands and room creation
```
List of valid commands: 
 /help 
 /createsession <roomname> 
 /joinsession <roomname> 
 /leavesession <roomname> 
 /leaveallsessions 
 /quit 
 /logout 
 /list 
 /colour <red, green, yellow, blue, purple, cyan, white> 
 /time 
 /invite <name> <roomname>
```

## Images
![Example Opening conference](/images/ExampleConference.png)