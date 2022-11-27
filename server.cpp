#include "ryyisocket/ryyisocket.h"
#define ever ;;

auto main (int, char**) -> void {
  MultipleClientsMessageHandler server;
  server.initialize ("27015");
  server.run ();
}
