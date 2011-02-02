// See test_tipc.cc for an example of how to use this class

#include <string>

class TIPC {
public:
  static bool getline(int fd, std::string &line, bool debug);
  static bool putline(int fd, const std::string &line, bool debug);
};

class TIPC_Client {
protected:
  std::string _remote_host;
  int _remote_port;
  bool _debug;
public:
  TIPC_Client(const std::string &remote_addr="", bool debug=true) : _debug(debug) {
    if (remote_addr != "") set_remote_address(remote_addr);
  }
  // Remote address is in form hostname:portnum
  void set_remote_address(const std::string &remote_addr);
  // Returns true if success, false if failure
  // If failure, returns error in result
  bool request(const std::string &request, std::string &result);
};

class TIPC_Server {
protected:
  bool _debug;
  struct Connection_State {
    TIPC_Server *server;
    int client_fd;
    Connection_State(TIPC_Server *server_init,
                     int client_fd_init) :
      server(server_init), client_fd(client_fd_init) {}
  };
public:
  TIPC_Server(bool debug= true) : _debug(debug) {}
  bool serve(int port, std::string &error_result);
  static void *serve_one_handler(void *arg);
  virtual void serve_one(int client_fd);
  virtual void process_request(const std::string &request, std::string &result);
};
  
