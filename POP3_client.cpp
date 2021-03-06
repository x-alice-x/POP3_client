// POP3_client.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include <iostream>
#include <WS2tcpip.h>
#include <string>

const int port = 110;

#pragma comment(lib, "ws2_32.lib")


int main()
{
	setlocale(0, "rus");
	// Initialize socket
	WSAData data;
	int wsOk = WSAStartup(MAKEWORD(2, 2), &data); 
	if (wsOk != 0)
	{
		std::cout << "Can't initialize WinSock" << std::endl;
		return 0;
	}

	// Initialize socket id
	int sid = socket(AF_INET, SOCK_STREAM, 0); 

	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);

	//IiAddress
	std::string ipAddress;
	std::cout << "ip address: ";
	std::getline(std::cin, ipAddress);
	inet_pton(AF_INET, ipAddress.c_str(), &addr.sin_addr); // convert string to ip

	// Connect to server
	int conn = connect(sid, (sockaddr*)&addr, sizeof(addr));
	if (conn == SOCKET_ERROR)
	{
		std::cerr << "Can't connect to server" << std::endl;
		closesocket(sid);
		WSACleanup();
		return 0;
	}

	// Send and recv data
	// some usefull value
	char buf[4096];
	std::string user, pass, cmd;
	std::string rn = "\r\n";

	// Succesfull connection
	ZeroMemory(buf, 4096);
	int byteRecv = recv(sid, buf, 4096, 0);
	if (byteRecv == SOCKET_ERROR)
	{
		std::cout << "Error in recv()" << std::endl;
		closesocket(sid);
		WSACleanup();
		return 0;
	}
	if (byteRecv == 0)
	{
		std::cout << "Server disconnected" << std::endl;
		closesocket(sid);
		WSACleanup();
		return 0;
	}
	std::cout << "server> " << std::string(buf, 0, byteRecv) << std::endl << "> ";
	if (std::strncmp(buf, "+OK", 3))
	{
		closesocket(sid);            // close everything if it's not +OK
		WSACleanup();
		return 0;
	}
	
	// Send Username and Password
	do 
	{
		//USER
		std::cout << "Username: ";
		std::getline(std::cin, user);
		user = "USER " + user;
		send(sid, user.c_str(), user.size() + 1, 0);            // two pack as telnet
		send(sid, rn.c_str(), rn.size() + 1, 0);				// send \r\n
		ZeroMemory(buf, 4096);
		byteRecv = recv(sid, buf, 4096, 0);
		std::cout << "server> " << std::string(buf, 0, byteRecv) << std::endl; // always +OK

		//PASS
		std::cout << "Password: ";
		std::getline(std::cin, pass);
		pass = "PASS " + pass;
		send(sid, pass.c_str(), pass.size() + 1, 0);
		send(sid, rn.c_str(), rn.size() + 1, 0);
		ZeroMemory(buf, 4096);
		byteRecv = recv(sid, buf, 4096, 0);
		std::cout << "server> " << std::string(buf, 0, byteRecv) << std::endl; // +OK
	}
	while (std::strncmp(buf, "+OK", 3)); // repeat if it's not +OK

	// Other commands
	do
	{
		std::cout << ">";
		std::getline(std::cin, cmd); 
		send(sid, cmd.c_str(), cmd.size(), 0);   // send commands
		send(sid, rn.c_str(), rn.size() + 1, 0); // two pack as telnet
		ZeroMemory(buf, 4096);
		byteRecv = recv(sid, buf, 4096, 0);
		std::cout << "server> " << std::string(buf, 0, byteRecv) << std::endl;
		while ((buf[byteRecv - 2] != '\n') && (buf[byteRecv - 1] != '.')) // "." in empty line; byteRecv = bytes in recv() msg
		{
			if (!std::strncmp(buf, "-ERR", 4)) // repeat if there is wrong command
				break;
			if (cmd == "QUIT")     // exit if it's QUIT
				break;
			ZeroMemory(buf, 4096);
			byteRecv = recv(sid, buf, 4096, 0); // recv() if there is second pack
			std::cout << std::string(buf, 0, byteRecv) << std::endl;
		}
	}
	while (cmd != "QUIT"); // exit if it's QUIT; repaet for another command
	
	// Close everything
	closesocket(sid);
	WSACleanup();
    return 0;
}

