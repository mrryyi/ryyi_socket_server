#pragma once
#include <iostream>
#define WIN32_LEAN_AND_MEAN

// Windows socket stuff
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

// Link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")



// Application-specific stuff
const int RYYI_SUCCESS = 1;
const int RYYI_FAILURE = -1;


auto initialize_winsock() -> int;

// OMR Creats a socket, listens for a connection, and receives a message from the client when bound.
class OneMessageReceiver
{
public:
  OneMessageReceiver();

  auto get_message() -> int;
  auto is_valid() -> int { return m_is_valid; }
  
private:
  auto resolve_local_address_and_port() -> int;
  auto create_listen_socket() -> int;
  auto bind_listen_socket() -> int;
  auto listen_socket() -> int;
  auto accept_client_socket() -> int;
  auto invalidate() -> void;
  auto shutdown_client_socket() -> int;

  bool m_is_valid = true;

  SOCKET m_listen_socket = INVALID_SOCKET;
  SOCKET m_client_socket = INVALID_SOCKET;

  // Socket stuff
  const char* DEFAULT_PORT = "27015";

  // Address info
  struct addrinfo *m_result = NULL;
  struct addrinfo m_hints;

  // Buffer stuff
  int iSendResult;
  char recvbuf[512];
  int recvbuflen = 512;
};
