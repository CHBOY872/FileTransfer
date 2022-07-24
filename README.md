# FileTransfer

FileTransfer is a simple program to send some files via Network.

## How to build

first of all, change your current directory to src:

```
cd src
```

### How to build server and client

Type in terminal

```
make runall
```

### How to build only client

Type in terminal

```
make runclient
```

### How to build only server

Type in terminal

```
make runserver
```

## How to use

If you want to send some file, you should run a server:

```
./server
```

then put file which you want to send in the directory where server was run.

If you want to receive a file, you should run a client:

```
./client <server ip> <file which you want to receive>
```
