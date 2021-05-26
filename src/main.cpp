#include <xhacking/xHacking.h>
#include <xhacking/Utilities/Utilities.h>
#include <xhacking/Loader/Loader.h>
#include <xhacking/Detour/Detour.h>

using namespace xHacking;

#include "handledata.hpp"
#include "tools.hpp"

#include <thread>
#include <mutex>
#include <unordered_map>

Detour<int, SOCKET, sockaddr*, int>* connectDetour = nullptr;



std::mutex serverMutex;
std::mutex clientMutex;

void initBot();
void stopBot();



SOCKET acceptSocket = INVALID_SOCKET;
SOCKET clientSocket = INVALID_SOCKET;
SOCKET serverSocket = INVALID_SOCKET;

std::thread clientBotThread;
std::thread serverBotThread;



void clientBotConnection()
{
	while (true)
	{
		sockaddr_in cliaddr;
		int len = sizeof(cliaddr);
		clientSocket = accept(acceptSocket, (sockaddr*)&cliaddr, &len);

		while (serverSocket == INVALID_SOCKET) {
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}

		char buf[2048];
		while (true)
		{
			memset(buf, 0, 2048);
			int bytes = recv(clientSocket, buf, 2048, 0);
			if (bytes == 0)
			{
				printf("C-->B ERROR\n");
				clientSocket = INVALID_SOCKET;
				break;
			}
			
		/*	auto packets = parseData(buf, bytes);
			for (auto packet : packets) {
				printf("C-->B %s\n", buf);
			}*/

			serverMutex.lock();
			send(serverSocket, buf, bytes, 0);
			serverMutex.unlock();
		}
	}
}


void serverBotConnection() 
{
	char buf[2048];

	while (true)
	{
		memset(buf, 0, 2048);
		int bytes = recv(serverSocket, buf, 2048, 0);

		if (bytes == 0)
		{
			printf("S-->B ERROR\n");
			serverSocket = INVALID_SOCKET;
			break;
		}

		auto packets = parseData(buf, bytes);
		for (auto packet : packets) {
			handleData(packet);
			//printf("S-->B %s\n", buf);	
		}

		clientMutex.lock();
		send(clientSocket, buf, bytes, 0);
		clientMutex.unlock();
	}

}


int WINAPI nuestroConnect(SOCKET s, sockaddr* addr, int len) 
{
	static bool first = true;

	if (first) 
	{  
		initBot();
		clientBotThread = std::thread(clientBotConnection);
		first = false;
	}

	sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(9998);
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	int ret = (*connectDetour)(s, (sockaddr*)&servaddr, sizeof(servaddr));

	serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	ret = (*connectDetour)(serverSocket, addr, len);

	if (serverBotThread.joinable()) 
	{
		serverBotThread.join();
	}
	
	serverBotThread = std::thread(serverBotConnection);

	return 0;
}

void Hooks() 
{

	acceptSocket = socket(AF_INET, SOCK_STREAM, 0);

	sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(9998);
	servaddr.sin_addr.s_addr = INADDR_ANY;
	bind(acceptSocket, (sockaddr*)&servaddr, sizeof(servaddr));
	listen(acceptSocket, 1);


	connectDetour = new Detour<int, SOCKET, sockaddr*, int>();
	connectDetour->Wait("ws2_32.dll", "connect", (BYTE*)nuestroConnect);

}

int sendPacketToSv(std::string packet) {
	serverMutex.lock();
	int ret = send(serverSocket, packet.c_str(), packet.length(), 0);
	serverMutex.unlock();

	return ret;
}

int sendPacketToCli(std::string packet) {
	clientMutex.lock();
	int ret = send(clientSocket, packet.c_str(), packet.length(), 0);
	clientMutex.unlock();
	return ret;
}




BOOL WINAPI DllMain(HINSTANCE instasnce, DWORD reason, DWORD reserved)
{
	if (reason == DLL_PROCESS_ATTACH) {
		CreateConsole();
		Hooks();
	}
	else if (reason = DLL_PROCESS_DETACH) {
		stopBot();
	}
	return true;
}

