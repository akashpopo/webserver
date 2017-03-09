# Caching Multithreaded Scheduling Web Server
As part of our Operating Systems course at Dalhousie University, we are developing a Caching Multithreaded Scheduling Web Server in C.

## Table of Contents
- Environment Setup
- Compiling and Running the Server
- Testing the Server
- Contribution Workflow
- Background
- Project Goals

## Environment Setup

1. cd into your dev directory, e.g. `cd dev/os/project`
2. clone this git repository: `git clone https://github.com/Mons1Oerjan/webserver.git`

## Compiling and Running the Server
Here is a description on how to compile and run the program:

1. cd into your project directory, e.g. `cd dev/os/project/webserver`
2. Compile all files using the makefile and the make command: `make`
3. Run the server: `./sws portnumber`, e.g. `./sws 6789` or `./sws 12345`

## Testing the Server
We have been provided with a script `hydra.py` to test our web server once it's running. Hydra reads in a test script from stdin, i.e. to run hydra use the command:

```
./hydra.py < test.in
```

where `test.in` is a file containing a test script.  The format of the test file is as follows:
- The first line of the file is a single integer, denoting the port number of the webserver
- The remain lines represent requests to the server.  Each request comprises three parts:
  delay : number of seconds (float) before connecting to the server
  pause : number of seconds (flaot) after connectkng before sending request
  file  : file (path) to requet

Here is an example of a test file that makes hydra connect to the webserver on port 8080 and requests four files:
```
8080
0.0 1.0 sws.c
0.5 0.0 network.c
0.5 0.0 network.h
0.5 0.0 makefile
```
The first request connects immediately, but waits for 1 second before sending the request. The remaining requests all wait 0.5 seconds before connecting. By setting the delay and the pause appropriate we can control how many clients are connected to the webserver at any time.

If the `-t` switch is used, e.g.,
```
./hydra.py -t < test.in
```
the times of the response are printed instead of the output.

## Contribution Workflow
To contribute to this project, please follow these guidelines:

#### Pull the latest changes made to the master branch
```
git checkout master
```
```
git pull
```

#### Create your own branch based on the name of your task 
For example, if you are working on the SJF algorithm, create a new branch called "sjf-algorithm": 
```
git checkout -b sjf-algorithm
```
#### Make your changes and development inside the new branch. 
It is recommended to commit your code when you have reached certain milestones (preferrable when your code is in a working state, e.g. no seg faults and compilation errors): 
```
git commit -a -m "Commit message, e.g. Implemented the SJF Algorithm"
```
#### Push your branch when your task is completed: 
Commit your latest changes, then push up the branch:
```
git push -u origin branchname
```

#### Create a Pull Request 
Go onto Github and create a pull request for your branch targeting the master branch. Assign another person in the team as the code reviewer and wait for their approval. The code reviewer is responsible for commenting on the code and merging the Pull Request into the master branch. The reason for doing code reviews is just to reduce chances of new features breaking in master. 

#### What NOT to do
- Please never commit directly to the master branch.

## Background
### The Web Server
Web servers are almost as ubiquitous as operating systems today. 
Many systems, even consumer-grade ones, can and do run web servers, sometimes as part of the OS itself. 
Furthermore, webservers have to perform many of the same functions and decisions that operating systems do. 
Consequently, using web servers to study how operating systems work is not an uncommon practice. 

The function of a web server is to serve web requests from web clients (web browsers). 
In short, to view a web page or download content, a web client (browser) makes a network connection (over the Internet) to a web server. 
The client then sends a request to the server, specifying the file that the web server should send back. 
The web server receives and parses the request, checks if the file is available, and sends it back if it is. 
The web client then uses and renders the fileas it deems fit. 

When a web server starts up, it first binds to a port, which allows clients to unambiguously specify which server to connect to. 
For example, most web servers bind to port 80. 
However, since our servers will be run as user-level processes, only ports in the range 1024 to 65335 are used. 
Once the server binds to a port and completes any additional initialization, it blocks (goes to sleep) and waits for clients to connect.

## Our Project Goals
1. Implement scheduling algorithms (SJF, RR, and Multilevel Queue)
2. Implement Multithreading
3. Implement caching (bonus).

