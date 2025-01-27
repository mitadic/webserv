#ifndef WEBSERV_H
# define WEBSERV_H

# define OK 0

# include <sys/socket.h>	// socket(), bind(), listen(), accept()
# include <netinet/in.h>	// sockaddr_in
# include <arpa/inet.h>     // htons()
# include <cstdlib>			// exit() and EXIT_FAILURE
# include <unistd.h>		// read()
# include <sys/epoll.h>     // struct epoll_event, epoll_create1(), epoll_ctl(), epoll_wait(), EPOLLIN, EPOLLET, EPOLL_CTL_ADD
# include <fcntl.h>         // F_GETFL, F_SETFL, O_NONBLOCK
# include <cstring>         // strlen()

# include <iostream>
# include <string>
# include <stack>



#endif