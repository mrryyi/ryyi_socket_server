#include <iostream>
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

const char* DEFAULT_PORT = "27015";
const int DEFAULT_BUFLEN = 512;
const int WINSOCK_SUCCESS = 0;
const int RYYI_SUCCESS = 1;
const int RYYI_FAILURE = -1;
// Need to link with Ws2_32.lib

#pragma comment (lib, "Ws2_32.lib")

#define ever ;;

class server
{
public:
  server() {
    printf("started new server\n");
    if (initialize_winsock() == RYYI_FAILURE) return; 
    if (resolve_address_and_port() == RYYI_FAILURE) return; 
    if (create_socket() == RYYI_FAILURE) return;
    if (bind_socket() == RYYI_FAILURE) return;
    if (listen_socket() == RYYI_FAILURE) return;
    if (accept_socket() == RYYI_FAILURE) return;

    // No need to return early if socket errors occur at this stage.
    run_until_disconnect();
    shutdown_socket();
  }
  
private:
  auto initialize_winsock() -> int {
    // Initialize Winsock
    iResult = WSAStartup (MAKEWORD (2, 2), &m_wsaData);
    if (iResult != WINSOCK_SUCCESS) {
      std::cout << "WSAStartup failed with error: " << iResult << std::endl;
      return RYYI_FAILURE;
    }
    return RYYI_SUCCESS;
  }

  auto resolve_address_and_port() -> int {
    ZeroMemory (&m_hints, sizeof (m_hints));
    m_hints.ai_family = AF_INET;
    m_hints.ai_socktype = SOCK_STREAM;
    m_hints.ai_protocol = IPPROTO_TCP;
    m_hints.ai_flags = AI_PASSIVE;
    
    // Resolve the server address and port
    iResult = getaddrinfo (NULL, DEFAULT_PORT, &m_hints, &m_result);
    if (iResult != WINSOCK_SUCCESS) {
      std::cout << "getaddrinfo failed with error: " << iResult << std::endl;
      return RYYI_FAILURE;
    }
    return RYYI_SUCCESS;
  }

  auto create_socket() -> int {
    // Create a SOCKET for connecting to server
    if (m_result == nullptr)
      return RYYI_FAILURE;
    
    m_listen_socket = socket (m_result->ai_family,
                              m_result->ai_socktype,
                              m_result->ai_protocol);
    if (m_listen_socket == INVALID_SOCKET) {
      std::cout << "socket failed with error: " << 
                             WSAGetLastError () << std::endl;
      freeaddrinfo (m_result);
      return RYYI_FAILURE;
    }

    return RYYI_SUCCESS;
  }

  auto bind_socket() -> int {
    // Setup the TCP listening socket
    iResult = bind (m_listen_socket, m_result->ai_addr, (int) m_result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
      std::cout << "bind failed with error: " << WSAGetLastError () << std::endl;
      freeaddrinfo (m_result);
      closesocket (m_listen_socket);
      return RYYI_FAILURE;
    }

    freeaddrinfo (m_result);
    return RYYI_SUCCESS;
  }

  auto listen_socket() -> int {
    iResult = listen (m_listen_socket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
      std::cout << "listen failed with error: " << WSAGetLastError () << std::endl;
      closesocket (m_listen_socket);
      return RYYI_FAILURE;
    }
    return RYYI_SUCCESS;
  }

  auto accept_socket() -> int {
    
    // Accept a client socket
    m_client_socket = accept (m_listen_socket, NULL, NULL);
    if (m_client_socket == INVALID_SOCKET) {
      std::cout << "accept failed with error: " << WSAGetLastError () << std::endl;
      closesocket (m_listen_socket);
      return RYYI_FAILURE;
    }

    // No longer need server socket
    closesocket (m_listen_socket);
    return RYYI_SUCCESS;
  }

  auto run_until_disconnect() -> int {
    // Receive until the peer shuts down the connection
    do {
      iResult = recv (m_client_socket, recvbuf, recvbuflen, 0);
      if (iResult > 0) {
        std::cout << "Bytes received: " << iResult << std::endl;

        // Echo the buffer back to the sender
        iSendResult = send (m_client_socket, recvbuf, iResult, 0);
        if (iSendResult == SOCKET_ERROR) {
          std::cout << "send failed with error: " << WSAGetLastError () << std::endl;
          closesocket (m_client_socket);
          return RYYI_FAILURE;
        }
        std::cout << "Bytes sent: " << iSendResult << std::endl;
      }
      else if (iResult == 0) {
        std::cout << "Connection closing..." << std::endl;
      } 
      else {
        std::cout << "recv failed with error: " << WSAGetLastError () << std::endl;
        closesocket (m_client_socket);
        return RYYI_FAILURE;
      }
    } while (iResult > 0);

    return RYYI_SUCCESS;
  }

  
  auto shutdown_socket() -> int {
    // shutdown the connection since we're done
    iResult = shutdown (m_client_socket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
      std::cout << "shutdown failed with error: " << WSAGetLastError () << std::endl;
      closesocket (m_client_socket);
      return RYYI_FAILURE;
    }

    // cleanup
    closesocket (m_client_socket);
    return RYYI_SUCCESS;
  }

  WSADATA m_wsaData;
  int iResult;

  SOCKET m_listen_socket = INVALID_SOCKET;
  SOCKET m_client_socket = INVALID_SOCKET;

  struct addrinfo *m_result = NULL;
  struct addrinfo m_hints;

  int iSendResult;
  char recvbuf[DEFAULT_BUFLEN];
  int recvbuflen = DEFAULT_BUFLEN;
};

auto main (int, char**) -> void {
  for(ever) server s;

  WSACleanup ();
}