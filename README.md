# Hasht-Client

Client-side program to use a hashmap provided by the server through POSIX shared memory.<br>
The server-side implementation can be found [here](https://github.com/corvulpex/hasht-server).

## Building 

You can build the program by either using CMake or build it with the included Makefile directly.<br>
Note that this program relies on POSIX functionality, so it won't build on Windows.

## How to use

### Starting the client

Start the client by providing the name of the shared memory object (the same as for the server).
```
./hasht-client [name_of_shared_memory]
```

Optionally you can change the number of requests that can be queued at the same time. By default this number will be 30.
```
./hasht-client [name_of_shared_memory] [number_of_queue_slots]
```
> [!WARNING]
> The number of queue slots has to be the same for the server or the queue will break.

### Using the running client

There are 3 operations that can be performed on the hashmap.

#### Insert

This will insert the value 1337 with key 4.
```
i 4 1337
```

#### Remove 
```
r 4 
```
This will remove the value with key 4.

#### Get
```
g 4 
```
This will fetch the value with key 4 and print it to std::cout.


To quit simply in input 'q'.

