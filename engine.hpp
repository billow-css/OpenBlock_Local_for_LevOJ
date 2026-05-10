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
#include <fstream>

#include "asset.hpp"

// ==================== 随机生成器 ====================
static int gen_block(std::mt19937& rng, int level) {
    if (level <= 2) return (rng() % 5) + 1;
    else if (level == 3) return ((rng() % 100) < 15) ? 0 : (rng() % 5) + 1;
    else if (level == 4) {
        int color = (rng() % 5) + 1;
        return ((rng() % 100) < 10) ? -color : color;
    } else {
        if ((rng() % 100) < 15) return 0;
        int base = (rng() % 5) + 1;
        return ((rng() % 100) < 10) ? -base : base;
    }
}

static void init_queues(Board& b, int seed, int N, int level) {
    b.level = level;
    std::mt19937 rng(seed);
    b.drop_queue.assign(N, std::vector<int>(1000));
    b.queue_ptr.assign(N, 0);
    for (int c = 0; c < N; ++c) {
        for (int i = 0; i < 1000; ++i) b.drop_queue[c][i] = gen_block(rng, level);
    }
}
