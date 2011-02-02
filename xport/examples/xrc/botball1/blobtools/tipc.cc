#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "tipc.h"
using namespace std;

bool TIPC::getline(int fd, string &line, bool debug) {
  line="";
  char buf[4096];
  while (1) {
    int ret= read(fd, buf, sizeof(buf));
    if (ret <= 0) {
      if (errno == EINTR || errno == EAGAIN) continue;
      break;
    }
    const char *crpos= strchr(buf, '\r');
    const char *nlpos= strchr(buf, '\n');
    if (crpos || nlpos) {
      line.append(buf, (crpos<nlpos?crpos:nlpos)-buf);
      if (debug) fprintf(stderr, "Got >%s<\n", line.c_str());
      return true;
    }
    line.append(buf, ret);
  }
  if (debug) fprintf(stderr, "Error waiting for response\n");
  line= "Error waiting for response";
  return false;
}
bool TIPC::putline(int fd, const string &line, bool debug) {
  string line_plus_newline= line + "\n";
  int writepos= 0, writelen= line_plus_newline.length();
  while (writepos < writelen) {
    int nwritten= write(fd, line_plus_newline.c_str()+writepos,
                        writelen-writepos);
    if (nwritten < 0) {
      if (errno == EINTR || errno == EAGAIN) continue;
      return false;
    }
    writepos += nwritten;
  }
  return true;
}

#if 0
// Remote address is in form hostname:portnum
void TIPC_Client::set_remote_address(const string &remote_addr) {
  if (remote_addr.find(':') != remote_addr.npos) {
    _remote_host= remote_addr.substr(0, remote_addr.find(':'));
    _remote_port= atoi(remote_addr.substr(remote_addr.find(':')+1).c_str());
  } else {
    _remote_host= remote_addr;
    _remote_port= 9090;
  }
}
// Returns true if success, false if failure
// If failure, returns error in result
bool TIPC_Client::request(const string &request, string &result) {
  struct sockaddr_in sin;
  struct hostent host;
  

  
  ACE_INET_Addr remote_addr(_remote_port, _remote_host.c_str());
  //
  // Connect
  //
  if (_debug) fprintf(stderr, "Connecting to %s:%d...\n",
                      _remote_host.c_str(), _remote_port);
  ACE_SOCK_Connector connect(stream, remote_addr, &timeout);
  if (stream.get_handle() == ACE_INVALID_HANDLE) {
    if (_debug) fprintf(stderr, "Error connecting\n");
    result= "Error connecting";
    return false;
  }
  //
  // Send request, terminated by newline
  //
  if (!TIPC::putline(stream, request, _debug)) { return false; }
  //
  // Receive response
  //
  return TIPC::getline(stream, result, _debug);
}
#endif

bool TIPC_Server::serve(int port, string &error_result) {
  int fd=socket(AF_INET,SOCK_STREAM,0);
  if (fd < 0) {
    fprintf(stderr, "Couldn't create socket\n");
    return false;
  }
  // Allow port to be reused quickly */
  int val = 1;
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) < 0) {
    fprintf(stderr, "Couldn't setsockopt\n");
    return false;
  }
  
  struct sockaddr_in sin;
  memset((char *)&sin, 0, sizeof(sin));
  sin.sin_family= AF_INET;
  sin.sin_port= htons(port);
  sin.sin_addr.s_addr= htonl(INADDR_ANY);

  if (bind(fd, (struct sockaddr*) &sin, sizeof(sin)) < 0) {
    fprintf(stderr, "Couldn't bind to socket\n");
    return false;
  }
  if (listen(fd, 100) < 0) {
    fprintf(stderr, "Couldn't listen to socket\n");
    return false;
  }
  fprintf(stderr, "Serving on port %d\n", port);
  while (1) {
    fprintf(stderr, "Waiting for accept\n");
    int client_fd= accept(fd, NULL, NULL);
    fprintf(stderr, "Got accept\n");
    if (client_fd >= 0) {
      serve_one_handler((void*) new Connection_State(this, client_fd));
    }
  }
}
void *TIPC_Server::serve_one_handler(void *arg) {
  Connection_State *state= (Connection_State*)arg;
  state->server->serve_one(state->client_fd);
  delete state;
  return NULL;
}
void TIPC_Server::serve_one(int client_fd) {
  //
  // Receive request
  //
  string request;
  TIPC::getline(client_fd, request, _debug);
  //
  // Send response
  //
  string response;
  process_request(request, response);
  if (_debug) fprintf(stderr, "Response: >%s<\n", response.c_str());
  TIPC::putline(client_fd, response, _debug);
  close(client_fd);
}

// Subclass TIPC_Server and override this to process requests
void TIPC_Server::process_request(const string &request, string &result) {
  result= "process_request needs to be overridden";
}
  
