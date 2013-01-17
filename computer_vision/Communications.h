#pragma once
#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
/*
int main()
{
	struct addrinfo host_nfo, client_nfo;
	struct addrinfo *host_nfo_list;
	memset(&host_nfo, 0, sizeof(host_nfo));
	memset(&client_nfo, 0, sizeof(client_nfo));
	std::cout << "setting up structs\n";
	host_nfo.ai_family=AF_UNSPEC;
	host_nfo.ai_socktype=SOCK_STREAM;
	host_nfo.ai_flags=AI_PASSIVE;
	int status=getaddrinfo(NULL, "5556", &host_nfo, &host_nfo_list);
	if (status==-1) std::cout << "failed getting host info\n";
	int host_socket=socket(host_nfo_list->ai_family, host_nfo_list->ai_socktype, host_nfo_list->ai_protocol);
	if (host_socket==-1) std::cout << "socket error\n";
	int yes=1;
	setsockopt(host_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
	if (bind(host_socket, host_nfo_list->ai_addr, host_nfo_list->ai_addrlen)==-1) std::cout << "couldn't bind socket\n";
	if (listen(host_socket, 3)==-1) std::cout << "listen error\n";
	else std::cout << "listening now\n";
	struct sockaddr_storage their_addr;
    	socklen_t addr_size = sizeof(their_addr);
    	int client_sock = accept(host_socket, (struct sockaddr *)&their_addr, &addr_size);
	freeaddrinfo(host_nfo_list);
	close(client_sock);
	close(host_socket);
}
*/

class SocketServer
{
  private:
	struct addrinfo host_nfo, client_nfo;
	struct addrinfo *host_nfo_list;
  struct sockaddr_storage their_addr;
  int host_socket, client_socket;
  public:
  bool sendnow=false;
  bool bye=false;
  SocketServer();
  ~SocketServer();
  SocketServer(std::string port);
  int waitmessage();
  void sendmessage(std::string message);
};
