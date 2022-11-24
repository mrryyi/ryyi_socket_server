#include "ryyisocket.h"

#define WINSOCK_SUCCESS 0

auto initialize_winsock() -> int {
  // Clean up on proper exit of program.
  static std::shared_ptr<void> on_death (nullptr, [] (void*) {
    WSACleanup ();
  });

  static auto initialized = false;
  if (initialized)
    return RYYI_SUCCESS;
  
  WSADATA wsaData;
  int iResult= WSAStartup(MAKEWORD(2, 2), &wsaData);
  if (iResult != WINSOCK_SUCCESS) {
    initialized = false;
    std::cout << "WSAStartup failed with error:" << iResult << std::endl;
    return RYYI_FAILURE;
  }

  initialized = true;
  return RYYI_SUCCESS;
};

OneMessageReceiver::OneMessageReceiver() {
    std::cout << "Initializing new socket...\n"; 
    initialize_winsock(); // just in case
    if (resolve_local_address_and_port() == RYYI_FAILURE) return; 
    if (create_listen_socket() == RYYI_FAILURE) return;
    if (bind_listen_socket() == RYYI_FAILURE) return;
    if (listen_socket() == RYYI_FAILURE) return;
    if (accept_client_socket() == RYYI_FAILURE) return;
  }

auto OneMessageReceiver::get_message() -> int {

  if (!this->is_valid())
    return RYYI_FAILURE;

  // Receive until the peer shuts down the connection
  int iResult;
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
  
  // This object has been used up
  this->invalidate();

  return RYYI_SUCCESS;
}

  
auto OneMessageReceiver::resolve_local_address_and_port() -> int {
  ZeroMemory (&m_hints, sizeof (m_hints));
  m_hints.ai_family = AF_INET;
  m_hints.ai_socktype = SOCK_STREAM;
  m_hints.ai_protocol = IPPROTO_TCP;
  m_hints.ai_flags = AI_PASSIVE;
  
  // Resolve the server address and port
  int iResult = getaddrinfo (NULL, DEFAULT_PORT, &m_hints, &m_result);
  if (iResult != WINSOCK_SUCCESS) {
    std::cout << "getaddrinfo failed with error: " << iResult << std::endl;
    return RYYI_FAILURE;
  }
  return RYYI_SUCCESS;
}

auto OneMessageReceiver::create_listen_socket() -> int {
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

auto OneMessageReceiver::bind_listen_socket() -> int {
  // Setup the TCP listening socket
  int iResult = bind (m_listen_socket, m_result->ai_addr, (int) m_result->ai_addrlen);
  if (iResult == SOCKET_ERROR) {
    std::cout << "bind failed with error: " << WSAGetLastError () << std::endl;
    freeaddrinfo (m_result);
    closesocket (m_listen_socket);
    return RYYI_FAILURE;
  }

  freeaddrinfo (m_result);
  return RYYI_SUCCESS;
}

auto OneMessageReceiver::listen_socket() -> int {
  int iResult = listen (m_listen_socket, SOMAXCONN);
  if (iResult == SOCKET_ERROR) {
    std::cout << "listen failed with error: " << WSAGetLastError () << std::endl;
    closesocket (m_listen_socket);
    return RYYI_FAILURE;
  }
  return RYYI_SUCCESS;
}

auto OneMessageReceiver::accept_client_socket() -> int { 
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

auto OneMessageReceiver::invalidate() -> void {
  m_is_valid = false;
  this->shutdown_client_socket();
}

auto OneMessageReceiver::shutdown_client_socket() -> int {
  // shutdown the connection since we're done
  int iResult = shutdown (m_client_socket, SD_SEND);
  if (iResult == SOCKET_ERROR) {
    std::cout << "shutdown failed with error: " << WSAGetLastError () << std::endl;
    closesocket (m_client_socket);
    return RYYI_FAILURE;
  }

  // cleanup
  closesocket (m_client_socket);
  return RYYI_SUCCESS;
}