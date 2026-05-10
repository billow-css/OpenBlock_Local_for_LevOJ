#pragma once

#if __cplusplus < 201700L
#error "C++17 required"
#endif

#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <random>
#include <fstream>      // 用于文件输出

#include "asset.hpp"
#include "tools.hpp"
#include "engine.hpp"