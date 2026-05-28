<<<<<<< HEAD
# Online Shopping System

Our project is an online shopping system which utilizes the abilities of C++ language while abiding by the rules and regulations of
OOP. The project is a console application which primarily consists of two pieces of code:

1. The networking layer.
2. The management layer.
 
## Dependencies

The project has used the windows api of threading and socket programming - `Winsock2` library. Therefore, the project can only be compiled to windows machines that support this library. Also, make sure to link the object file with winsock modules.

The networking layer also uses the local host `127.0.0.1` ip as default. You can change it in the [Server.h](./Server/Headers/Server.h)

## Networking layer

The networking layer consists of those classes and functions which
are responsible for establishing a connection between a client and
the remote sever.

### Technology

The networking is carried out with the help of Winsock (2.2) programming - windows APIs of socket programming.

### Classes

The `Server` - which uses the `Thread` class - and `Client` class.

![networking - classes](./images/network_classes.jpg)

### Usage

The networking layer is independent of the management layer; it can used in any C++ program. You can download the following files and use them in your own project.

- Server: [Server.h](./Server/Headers/Server.h) - [Server.cpp](./Server/Headers/Server.cpp) and [Thread.h](./Server/Headers/Thread.h) - [Thread.cpp](./Server/Headers/Thread.cpp)
- Client: [Client.h](./Client/Headers/Client.h) - [Client.cpp](./Client/Headers/Client.cpp)

You may utilize the networking functions like:

#### Server:

```cpp

#include "Server.h"

void worker(Thread& server)
{
    // code to deal with client
    server.Rec(buffer);     // receive a msg from client
    server.Send(msg);       // send a msg to client
    server.endServer();     // close the server
}

int main()
{
    Server server(worker);
    if (server.start())
        server.run();

    return 0;
}
```


#### Client:

```cpp

#include "Client.h"

int main()
{
    Client client;
    if (client.start())
    {
        // code to deal with server
        client.Rec(buffer);     // receive a msg from server
        client.Send(msg);       // send a msg to server
    }
        
    return 0;
}
```

### Working

![networking - working](./images/network_working.jpg)

## Management layer

This layer uses the networking layer to implement a networked shopping app.

### Features

- Customers can sign-up or login.
- Customers can buy items.
- Before confirming, the bill is displayed. Then, they have the option to confirm, cancel, add or remove anything.
- Customers can write complaints.
- Any employee (or admin) can answer the complain.
- Employees and admin will avail 20% discount on purchases.
- Employee accounts are made by the admin.
- Customers and employees alike can view their profile.
- Automatic reorder is placed if goods fall below a certain level.
- Admin can view record of customers & employees.
- Admin can fire any employees and ban any customers.
- Admin can view the inventory level and manually order goods.
- Admin can also manually update balance.
- Admin can search for customers and employees.
- Admin also sets the reorder level, amount & unit price.

### Classes

![management - classes](./images/manage_classes.jpg)

## Documentation

For a detailed explanation of everything, refer to this [report.](./REPORT.pdf)

## Last Updated

_May, 2021_

## Authors

- github : [@saad0510](https://www.github.com/saad0510)
- email  : k200161@nu.edu.pk or ayyansaad46@gmail.com

=======
# Cpp-Repo-New
>>>>>>> db96088fd096429e541fe9dd000b2ebcede84d91
