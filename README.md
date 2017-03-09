# Caching Multithreaded Scheduling Web Server
As part of our Operating Systems course at Dalhousie University, we are developing a Caching Multithreaded Scheduling Web Server in C.

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

## Our Project Tasks
1. Implement scheduling algorithms (SJF, RR, and Multilevel Queue)
2. Implement Multithreading
3. Implement caching (bonus).

