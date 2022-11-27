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
const int WINSOCK_SUCCESS = 0;

auto initialize_winsock() -> int;

class MultipleClientsMessageHandler {
public:
  MultipleClientsMessageHandler() {};
  
  auto initialize(const std::string port) {
    initialize_winsock();

    { // Resolve the server address and port into m_addr_info
      struct addrinfo hints;
      ZeroMemory (&hints, sizeof (hints));
      hints.ai_family = AF_INET;
      hints.ai_socktype = SOCK_STREAM;
      hints.ai_protocol = IPPROTO_TCP;
      hints.ai_flags = AI_PASSIVE;
    
      int iResult = getaddrinfo (NULL, port.c_str(), &hints, &m_addr_info);
      if (iResult != WINSOCK_SUCCESS) {
        std::cout << "getaddrinfo failed with error: " << iResult << std::endl;
        freeaddrinfo (m_addr_info);
        return RYYI_FAILURE;
      };
    }

    // Create listening socket
    m_listen_socket = socket (m_addr_info->ai_family,
                              m_addr_info->ai_socktype,
                              m_addr_info->ai_protocol);

    if (m_listen_socket == INVALID_SOCKET) {
      std::cout << "socket failed with error: " << WSAGetLastError () << std::endl;
      freeaddrinfo (m_addr_info);
      return RYYI_FAILURE;
    }

    // Bind listening socket to ip address and port
    int iResult = bind (m_listen_socket,
                        m_addr_info->ai_addr,
                        (int) m_addr_info->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
      std::cout << "bind failed with error: " << WSAGetLastError () << std::endl;
      freeaddrinfo (m_addr_info);
      closesocket (m_listen_socket);
      return RYYI_FAILURE;
    }

    // tell winsock that the socket is for listening
    listen(m_listen_socket, SOMAXCONN);

    // set file descriptor to zero
    FD_ZERO(&m_fd_main);

    // Add listening socket to main file descriptor
    // Without this we won't hear incoming connections.
    FD_SET(m_listen_socket, &m_fd_main);

    return RYYI_SUCCESS;
  }

  // Message getting thread
  auto run() {

    auto accept_inbound_connection = [this]() {
      // (no reason to keep this socket, it is but a pointer).
      SOCKET client = accept(m_listen_socket, nullptr, nullptr);
      // Add the new connection to the list of connected clients
      FD_SET(client, &m_fd_main);
    };

    while (true) {
      // We don't want to overwrite the main file descriptor.
      // select() overwrites any argument file descriptor.
      // The job of the copy is to see who is talking to us.
      fd_set fd_copy = m_fd_main;

      // select() blocks until readable message (last argument == nullptr)
      // stops blocking on error as well.
      int socket_count = select(0, &fd_copy, nullptr, nullptr, nullptr);

      if (socket_count == SOCKET_ERROR)
        run_select_handle_WSAError();

      for(int socket_i = 0; socket_i < socket_count; ++socket_i) {
        SOCKET in_sock = fd_copy.fd_array[socket_i];

        // New inbound connection
        if (in_sock == m_listen_socket)
          accept_inbound_connection();
        else /* New inbound message from existing client. */ {
          const int bufsize = 4096;
          char buf[bufsize];
          
          // Receive the message
          int bytes_in = recv(in_sock, buf, bufsize, 0);
          if (bytes_in <= 0) {
            // Some sort of error, drop the client.
            closesocket(in_sock);
            // Remove the client from the list of connected clients
            FD_CLR(in_sock, &m_fd_main);
          }
          else {
            // Send message to all clients except the one who sent them
            for (u_int i = 0; i < m_fd_main.fd_count; ++i) {
              SOCKET out_sock = m_fd_main.fd_array[i];
              if (out_sock != m_listen_socket && out_sock != in_sock)
                send(out_sock, buf, bytes_in, 0);
            }
          }
        }
      }
    }; // end while(true)
  }

private:

  auto run_select_handle_WSAError() -> void{
    int WSAError =  WSAGetLastError();
    std::cout << "select failed with error: " << WSAError << std::endl;

    // Handle error if necessary
    switch (WSAError) {
      case WSANOTINITIALISED:
        initialize_winsock();
        break;
      case WSAENETDOWN:
        // Network subsystem is down
        break;
      case WSAEFAULT:
        // The Windows Sockets implementation was unable
        // to allocate needed resources for its internal operations,
        // or the readfds, writefds, exceptfds, or timeval parameters 
        // are not part of the user address space.
        break;
      case WSAEINVAL:
        // The time-out value is not valid, or all three descriptor sets
        // were null.
        break;
      case WSAENOTSOCK:
        // One of the descriptor sets contains an entry that is not a socket.
        break;
        
      // WSAINTR, WSAEINPROGRESS not applicable, since we don't use WSA 1.1
      default:
        break;
    };
  };

  // Used to listen for incoming connections.
  SOCKET m_listen_socket;

  // Result of resolving this server address and port.
  struct addrinfo *m_addr_info;
  
  // Main file descriptor
  fd_set m_fd_main;

};


// OMR Creates a socket, listens for a connection, and receives a message from a client when bound.
class OneMessageReceiver
{
public:
  OneMessageReceiver();

  auto get_message() -> int;
  auto is_valid() -> const int { return m_is_valid; }
  
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
