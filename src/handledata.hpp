#pragma once

#include "string"
#include <unordered_map>
#include <string>
#include <thread>
#include <atomic>
#include <functional>
#include <iostream>
#include <algorithm>
#include <Windows.h>


bool handleData(std::string packet);

void mainLoop();

