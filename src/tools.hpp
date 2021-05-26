#pragma once
#include <vector>
#include <string>



std::vector<std::string> splitString(std::string  str);
std::vector<std::string> splitStringName(std::string  str);
std::vector<std::string> parseData(const char* buf, int len);

std::string encrypt(std::string packet);
std::string decrypt(std::string packet);