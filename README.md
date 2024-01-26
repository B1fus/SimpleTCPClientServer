A simple tcp server and a client that sends the client name, the server writes this name to log.txt
# Install
```cmake
cmake .
cmake --build .
```
# Usage
Create server
```shell
server [port]
```
Create client
```shell
client [name] [port] [period in seconds]
```
