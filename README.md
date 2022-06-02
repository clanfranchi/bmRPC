# bmRPC – a Bare Metal Remote Procedure Call framework

bmRPC is a remote procedure call framework
suitable for operating system-less embedded devices.
This [rpc framework](https://en.wikipedia.org/wiki/Remote_procedure_call
) allows to invoke and execute functions located on one device from a connected and trusted other device.
It can solve the “Last mile” problem for the implementation of the Industrial Internet of Things [IIoT - Industry 4.0](https://en.wikipedia.org/wiki/Industrial_internet_of_things) by simplifying the connection of non-IIoT-enabled devices.
The installation of this framework allows the data access and device control from Internet in the least intrusive way:
- no other external application or libraries.
- no development of custom protocols.
- no heavy source code modification and regression testing.


## License

Copyright 2022 Claudio Lanfranchi.

bmRPC is licensed under Apache License Version 2.0.
See [LICENSE](LICENSE) for more information.

## Features

-	Transport layer independent. It is required only the provision of a driver implementing the Data Link layer (e.g. I2C/SPI/UART driver). The data streaming is implemented with optimized circular buffers. CAN/USB/BT/TCPIP can also be adopted.
-	bmRPC does not require Interface Description Language (IDL) and other applications for code generation. Function prototypes are used to exchange information between Client and Server. The RPC identification is automatically created from the function prototype.
-	Unlimited number of parameters.
-	Several parameter types (fundamental types, std::strings, std::vectors). Other types support can be easily added.
-	Parameters passed by reference.
-	Arguments number and type control at compilation time.
-	Function overloading support. The text format of the complete function prototype is used as RPC identification.
-	Multiple client invokation instances of the same RPC.
-	Text protocol and binary protocol with endianness handling.
-	Reduced code footprint by limiting the use of template meta programming for arguments marshalling and serialization.
-	Minimum overhead for RPC call. Use of static polymorphism instead of dynamic polymorphism.

## Requirements

-	Support for version C++17 or higher.
-	STL library.
-	Data Link driver: I2C/SPI/UART...
-	std::chrono or other high resolution timer.
-	Any C++ IDEs.

## How to build

Take the following steps:
- Includes bmRPC files in you projects for compilation.
- Implement the Data Link driver using provided Comm class interface located in the [bmRPCDataLink.h](src/bmRPCDataLink.h) file.
- Configure user settings located in the [bmRPC.h](src/bmRPC.h) file and reported in the table below:
  
<table>
  <tr>
    <td><c>BMRPC_SERVER:</c></td>
    <td><c>Enable server compilation</c></td>
  </tr>
  <tr>
    <td><c>BMRPC_CLIENT:</c></td>
    <td><c>Enable client compilation</c></td>
  </tr>
  <tr>
    <td><c>BINARY_BASED_PROTOCOL:</c></td>
    <td><c>Select binary or text protocol</c></td>
  </tr>
  <tr>
    <td><c>P64:</c></td>
    <td><c>Select the P64 fundamental types support</c></td>
  </tr>
  <tr>
    <td><c>SRV_NATIVE:</c></td>
    <td><c>Set server endianess</c></td>
  </tr>
    <tr>
        <td><c>CLN_NATIVE:</c></td>
        <td><c>Set client endianess</c></td>
      </tr>
</table>



## How to use

Take the following steps:

Server side. Register the function to be called from the client: 
```C++
Skeleton<Data>* func_skeleton = server.CONNECT(func_name);
```
where func_name is the function prototype name.

Client side. Register the function to be executed in the server: 
```C++
RpcHandle<Stub<Data>> func_handle =  client.CONNECT(func_pointer_name);
```
where func_pointer_name is the pointer name to the function type equivalent to the one registered by the server.

Client side. Invoke the function execution:
```C++
Client.ASYNC_RPC_WITH_CB(func_pointer_name, func_handle, callback lambda, arguments…);
```
Refer to the file [main.cpp](src/main.cpp) for details.

The framework setup requires the following:

Server side.
1. Create the Data Link driver:
```C++
auto server_com = ServerCom<DataItem>();
```
3. Create the RPC Server:
```C++
RpcServer server = CREATE_SERVER(ServerCom<DataItem>, server_com);
```
4. Open the Data Link com driver:
```C++
server_com.open();
```
5. Init RPC Server loop:
```C++
server.init_loop();
```
6. Invoke ```server.doLoop();``` in the main working thread.
7. Disconnect registered functions before shutdown:
```C++
server.disconnect(func_skeleton);
```

Client side.
7. Create the Data Link driver:
```C++
auto client_com = ClientCom<DataItem>();
```
8. Create the RPC Client:
```C++
RpcClient client = CREATE_CLIENT(ClientCom<DataItem>, client_com);
```
9. Open the Data Link com driver:
```C++
client_com.open();
```
10. Init RPC Client loop:
```C++
client.init_loop();
```
11. Invoke ```client.doLoop();``` in the main working thread.
12. Disconnect registered functions before shutdown:
```C++
client.disconnect(func_handle);
```

## Credits

-	[Google’s gRPC](https://github.com/grpc/grpc)
-	[Rui Figuera’s czRPC](https://github.com/ruifig/czrpc)
-	[Godbolt’s Compiler Explorer](https://godbolt.org)


## Future Developments

-	Improve optimization.
-	Implement bidirectional rpc invokation: allow the server to call functions registered by the client.
-	Implement multithreading support.
-	Framework porting to Heap-less memory solution.
-	Implement function prototypes discovery.
-	Make bmRPC reusable as Inter-Process Communication (IPC).
-	Cognitive RPC.


## To start contributing

Contributions are welcome!
Contribution guide document is pendiing.
