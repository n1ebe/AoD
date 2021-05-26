#include "tools.hpp"

std::vector<std::string> parseData(const char* buf, int len)
{
	std::vector<std::string> packets;

	int i = 0;

	while (true)
	{
	   const char* last = buf;

	 while (*buf != 0x01 && i < len) {
			++buf;
			++i;
		}

	 if (i >= len) {
		 break;
	 }

	int packetLen = buf - last;
	packets.push_back(std::string(last, packetLen));

	++buf;
	++i;
   }


	return packets;
}



std::vector<std::string> splitString(std::string  str) {

	std::vector<std::string> tokens;

	int last = 0;
	int i = 0;

	for (; i < str.length(); ++i) {
		if (str[i] != ',') {
			continue;
		}

		tokens.push_back(str.substr(last, i - last));

		++i;
		last = i;
	}

	tokens.push_back(str.substr(last, i - last));

	return tokens;
}



std::vector<std::string> splitStringName(std::string  str) {

	std::vector<std::string> tokens;

	int last = 0;
	int i = 0;

	for (; i < str.length(); ++i) {
		if (str[i] != '<') {
			continue;
		}

		tokens.push_back(str.substr(last, i - last));

		++i;
		last = i;
	}

	tokens.push_back(str.substr(last, i - last));

	return tokens;
}


std::string decrypt(std::string packet) {

	uint8_t token1 = packet[packet.size() - 1] - 0xA;
	uint8_t token2 = packet[packet.size() - 2] - 0xA;


	char keyStr[] = { token2, token1, '\0' };
	int key = atoi(keyStr);

	std::string decryptedPacket = "";
	for (int i = 0; i < packet.size() - 2; ++i)
	{
		decryptedPacket += packet[i] - key;
	}

	return decryptedPacket;
}

std::string encrypt(std::string packet) {

	float keyTemp = rand() / float(RAND_MAX);
	keyTemp = keyTemp * 99.0f;

	char key = (char)round(keyTemp);

	if (key < 0xA) {
		key = 0xA;
	}

	std::string cryptedPacket = "";

	for (int i = 0; i < packet.length(); ++i) {
		cryptedPacket += (packet[i] + key);
	}

	char keyStr[3] = { 0 };
	itoa(key, keyStr, 10);

	char token1 = keyStr[0];
	char token2 = keyStr[1];

	cryptedPacket += (token1 + 0xA);
	cryptedPacket += (token2 + 0xA);

	return cryptedPacket;
}