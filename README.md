# C++ windows console server to try out sockets

# This is a WIP: following statements describe my vsion

## What does it do?

- Reads TCP messages on port 6920
- Puts the json content in a sqlite3 database

## Message format

Message format is in json.
You start the message with ```<ryyisock>``` and end with ```</ryyisock>```

In between these you put json.

## Database model

Database has one table for now: jsondump

- RecvDate
- Json
- SentByIP

https://learn.microsoft.com/en-us/windows/win32/winsock/complete-server-code

## How to build

```
CMake -S . -B build\
```

```
MSBuild.exe build\server.vcxproj
```

Don't forget to add your MSBuild executable to your path.