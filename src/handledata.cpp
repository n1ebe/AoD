#include "handledata.hpp"
#include "tools.hpp"

#include <map>

DWORD oldTicksTeclas = GetTickCount();

extern int sendPacketToCli(std::string packet);
extern int sendPacketToSv(std::string packet);

bool handleInventory(std::string packet);
bool handlePosition(std::string packet);
bool handleHechizos(std::string data);
bool handleMovePlayer(std::string data);
bool handleCreatePlayer(std::string data);
bool handleClickHordaPlayer(std::string data);
bool handleClickNeutralPlayer(std::string data);
bool handleMeExitMap(std::string data);
bool handlePlayerExitMap(std::string data);

std::thread mainLoopThread;

bool autoPotas = false;
bool autoAim = false;


VOID autoPociones();
VOID castAutoSpell(int posHechi);

HANDLE handlePotas;
HANDLE handleAim;

int targetPosX = 0;
int targetPosY = 0;
int targetID = 0;
struct Player {
	int id;
	std::string name;
	int posX;
	int posY;
	int faction;
	bool isInvisible;
};

struct Item {
	int pos;
	int id;
};

struct Hechizo {
	int pos;
	std::string nombre;
};

std::string positionRemo;
std::string positionInvi;
std::string selectedPlayerName;
std::string nombrePlayerAux;


std::string playerName = "IDO KAFKA";
std::string nombreTargetFinal; 
int selectedPlayerId = 0;



//map<int, Player *> CharactersInMap;

std::unordered_map<int, Player *> CharactersInMap;
std::unordered_map<std::string, Player *> CharactersInRange;
std::unordered_map<int, Player *> CharactersInRangeAux;






unsigned short posX = 0;
int* posY= 0;

struct Bot {
	std::atomic<bool> stopThread = false;
	std::atomic<bool> isInGame = false;
};

Bot bot = { false, false };



struct FunctionMapping {
	std::string opcode;
	std::function<bool(std::string)> func;
};

std::unordered_map<std::string, std::function<bool(std::string)>> mappings =
{
	{"CSI", handleInventory},
//	{"SHS", handleHechizos},
	{"PU", handlePosition},
	{"MP", handleMovePlayer},
	{"CC", handleCreatePlayer},
	{"4K", handleClickHordaPlayer},
	{"1&", handleClickNeutralPlayer},
    {"CM", handleMeExitMap},
    {"QDL", handlePlayerExitMap}
};



std::unordered_map<int, Item> items;
std::unordered_map<int, Hechizo> hechizos;

bool handleData(std::string packet) {
	std::string opcode3 = packet.substr(0, 3);
	auto mapping = mappings.find(opcode3);

	if (mapping != mappings.end()) {
		return mapping->second(packet.substr(3));
	}

	std::string opcode2 = packet.substr(0, 2);
	mapping = mappings.find(opcode2);

	if (mapping != mappings.end()) {
		return mapping->second(packet.substr(2));
	}

	return false;
}

bool handleClickHordaPlayer(std::string data) {
	auto tokens = splitStringName(data);

	std::string nombreTargetHorda = tokens[0];

	targetPosX = 0;
	targetPosY =0;
	targetID = 0;
	nombreTargetFinal = nombreTargetHorda.substr(0, nombreTargetHorda.length() - 1);

	return true;
}

bool handleMeExitMap(std::string data) {

	CharactersInMap.clear();
	CharactersInRange.clear();
	CharactersInRangeAux.clear();
	selectedPlayerId = 0;
	selectedPlayerName = "";
	targetID = 0;

	return true;
}


bool handlePlayerExitMap(std::string data) {

	auto tokens = splitString(data);

	int playerExit = stoi(tokens[0]);

	if (playerExit > 0) {

		if (CharactersInRange.size() > 0)
		{
			auto it2 = CharactersInRangeAux.find(playerExit);

			if (it2 != CharactersInRangeAux.end())
			{
				Player *playersitoMap = it2->second;
				nombrePlayerAux = playersitoMap->name;
			}

			if (nombrePlayerAux != "") {

			auto it = CharactersInRange.find(nombrePlayerAux);

			if (it != CharactersInRange.end())
			{
		    	if (playerExit == targetID)
				{
					selectedPlayerId = 0;
					selectedPlayerName = "";
					targetPosX = 0;
					targetPosY = 0;
					targetID = 0;
				}

				CharactersInRange.erase(it);
			}

			}
		}
	}
	return true;
}


bool handleClickNeutralPlayer(std::string data) {
	auto tokens = splitStringName(data);

	std::string nombreTargetNeutral = tokens[0];

	nombreTargetFinal = nombreTargetNeutral;

	targetPosX = 0;
	targetPosY = 0;

	return false;
}



bool handleCreatePlayer(std::string data) {

	auto tokens = splitString(data);

	if (tokens.size() > 11) {
		std::string name = tokens[11];

		if (name != playerName) {
			Player *newPlayer = new Player();
			newPlayer->id = stoi(tokens[3]);
			newPlayer->posX = stoi(tokens[4]);
			newPlayer->posY = stoi(tokens[5]);
			newPlayer->name = tokens[11];
			newPlayer->faction = stoi(tokens[12]);
			newPlayer->isInvisible = (stoi(tokens[13]) == 1);
			auto it = CharactersInMap.find(newPlayer->id);
			if (it == CharactersInMap.end())
			{
				CharactersInMap.insert(std::pair<int, Player *>(newPlayer->id, newPlayer));
			}
		}
	}
	return false;
}
bool handleMovePlayer(std::string data) {
	
	auto tokens = splitString(data);

	if (tokens.size() != 0) 
	{
		int idPlayer = stoi(tokens[0]);
		int posX = stoi(tokens[1]);
		int posY = stoi(tokens[2]);

		auto itMap = CharactersInMap.find(idPlayer);

		if (itMap != CharactersInMap.end())
		{
			std::string name = itMap->second->name;
			int faction = itMap->second->faction;

			if (CharactersInRange.size() == 0) 
			{
				selectedPlayerName = name;
				selectedPlayerId = idPlayer;
				Player *newPlayer = new Player();
				newPlayer->id = idPlayer;
				newPlayer->posX = posX;
				newPlayer->posY = posY;
				newPlayer->name = name;
				newPlayer->faction = faction;

			
			CharactersInRange.insert(std::pair<std::string, Player *>(name, newPlayer));
			 CharactersInRangeAux.insert(std::pair<int, Player *>(idPlayer, newPlayer));
			}
			else if (CharactersInRange.size() > 0) {
				auto itRange = CharactersInRange.find(name);
				if (itRange != CharactersInRange.end()) {
					itRange->second->posX = posX;
					itRange->second->posY = posY;
				}
				else {
					Player *newPlayer = new Player();
					newPlayer->name = name;
					newPlayer->posX = posX;
					newPlayer->posY = posY;
					newPlayer->faction = faction;

					CharactersInRange.insert(std::pair<std::string, Player *>(name, newPlayer));
					CharactersInRangeAux.insert(std::pair<int, Player *>(idPlayer, newPlayer));
				}
			}
		}
	}

	return true;
}




bool handlePosition(std::string data) {

	
	return true;
}


VOID castAutoSpell(int hechiPos) {

	/*int hechiPos = -1;
	for (auto& pair : hechizos) {
		if (pair.second.nombre == "Invisibilidad") {
			hechiPos = pair.first;
		}
	}*/

	if (hechiPos > 0) {

		std::string UK = "UK1~0";
		std::string LH = ("LH" + std::to_string(hechiPos));

		LH += (char)0x7E;
		LH += (char)0x30;
		LH += (char)0x1;

		std::string WLC = "WLC" + std::to_string(posX) + "," + std::to_string(*posY - 1) + ",1~0";

		UK += (char)0x1;
		WLC += (char)0x1;

		sendPacketToSv(UK);
		sendPacketToSv(LH);
		sendPacketToSv(WLC);

	}

}


VOID castSpellAim(int hechiPos) {

	if (nombreTargetFinal != "") {
	
	  if (targetPosX > 0) {
		 if (hechiPos > 0) {

			std::string UK = "UK1~0";
			std::string LH = ("LH" + std::to_string(hechiPos));

			LH += (char)0x7E;
			LH += (char)0x30;
			LH += (char)0x1;

			UK += (char)0x1;

			sendPacketToSv(UK);
			sendPacketToSv(LH);

			std::string WLC = "WLC" + std::to_string(targetPosX) + "," + std::to_string(targetPosY) + ",1~0";
			WLC += (char)0x1;
			sendPacketToSv(WLC);

		 }
	   }
	}

}

bool handleHechizos(std::string data) {

	auto decrypted = decrypt(data.substr(3));


	//std::string spellPosition = tokens[0];
	//std::string  spellName = tokens[2];
	
	/*Hechizo hechizo = {
	std::atoi(tokens[0].c_str()),
	tokens[1]
	};

	hechizos[hechizo.pos] = hechizo;*/

	return true;
}

bool handleInventory(std::string data) {

	bot.isInGame = true;
	
	auto tokens = splitString(data);

	Item item = {
		stoi(tokens[0]),
		stoi(tokens[4])
	};

	items[item.pos] = item;

	return true;
}


VOID autoAimTarget() {

	while (true) {

		if (nombreTargetFinal != "") {

			auto it = CharactersInRange.find(nombreTargetFinal);
			
			if (it != CharactersInRange.end())
			{
				Player *targetPlayer = it->second;

				targetPosX = targetPlayer->posX;
				targetPosY = targetPlayer->posY;
				targetID = targetPlayer->id;

			}
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}

VOID autoPociones() {

	while (true)	{

	DWORD *maxHpAddress = (DWORD *)(0xAFFA3C);
	DWORD *currentHpAddress = (DWORD *)(0xAFFA40);
	DWORD *maxManaAddress = (DWORD *)(0xAFFA44);
	DWORD *currentManaAddress = (DWORD *)(0xAFFA48);

	int *HPMAX = (int *)maxHpAddress;
	int *HPACT = (int *)currentHpAddress;
	int *MPMAX = (int *)maxManaAddress;
	int *MPACT = (int *)currentManaAddress;
	
	    	if (*HPACT != 0)
			  {
			
				if (*HPACT < *HPMAX)
				{
					int itemPos = -1;
					for (auto& pair : items) {
						if (pair.second.id == 8585) {
							itemPos = pair.first;
							printf("LLEGUE");
						}
					}

					if (itemPos > 0) {

						std::string packet = "USE" + encrypt(" " + std::to_string(itemPos));
						packet += (char)0x7E;
						packet += (char)0x30;
						packet += (char)0x1;
						sendPacketToSv(packet);

						std::this_thread::sleep_for(std::chrono::milliseconds(250));
					}
					
				}
				else if (*MPACT != *MPMAX)
				{
					int itemPos = -1;
					for (auto& pair : items) {
						if (pair.second.id == 8583) {
							itemPos = pair.first;
						}
					}

					if (itemPos > 0) {

						std::string packet = "USE" + encrypt(" " + std::to_string(itemPos));

						packet += (char)0x7E;
						packet += (char)0x30;
						packet += (char)0x1;
						sendPacketToSv(packet);

						std::this_thread::sleep_for(std::chrono::milliseconds(200));
					}
					
				}
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}

void mainLoop()
{
	printf("MAIN loop\n");

	while (!bot.stopThread) {

		DWORD *userPosXAddress = (DWORD *)(0xAFF130); // 
		DWORD *userPosYAddress = (DWORD *)(0xaFF132);

		int *userPosX = (int *)userPosXAddress;
	
		unsigned short mePosX = static_cast<unsigned short>(*userPosX % (1 << (2 * 8)));
		
		posX = mePosX;
		posY = (int *)userPosYAddress;

		if (!bot.isInGame) {
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			continue;
		}

		DWORD newTick = GetTickCount();
		if (newTick - oldTicksTeclas > 100)
		{
			if ((GetKeyState(VK_RBUTTON) & 0x100) != 0)
			{
				castAutoSpell(35);
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
			}

			if ((GetKeyState(VK_MBUTTON) & 0x100) != 0)
			{
				castAutoSpell(29);
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
			}
		
			if ((GetKeyState(VK_LCONTROL) & 0x100) != 0)
			{
				castSpellAim(34);
				std::this_thread::sleep_for(std::chrono::milliseconds(200));
			}

			if ((GetKeyState('P') & 0x100) != 0)
			{
				if (autoAim) {
					sendPacketToCli("||aON ~255~255~255~1~0");
					handleAim = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)autoAimTarget, 0, 0, 0);
					autoAim = false;
				}
				else if (!autoAim)
				{
					sendPacketToCli("||aOFF ~255~255~255~1~0");
					TerminateThread(handleAim, 0);
					autoAim = true;
				}
			}
			
			if ((GetKeyState('F') & 0x100) != 0)
			{
				if (autoPotas) {
					sendPacketToCli("||pON ~255~255~255~1~0");
					handlePotas = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)autoPociones, 0, 0, 0);
					autoPotas = false;
				}
				else if (!autoPotas)
				{
					sendPacketToCli("||pOFF  ~255~255~255~1~0");
					TerminateThread(handlePotas, 0);
					autoPotas = true;
				}
			}


			oldTicksTeclas = GetTickCount();
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
  }




void initBot() 
{
	mainLoopThread = std::thread(mainLoop);
}

void stopBot()
{
	mainLoopThread.join();
	bot.stopThread = true;
}