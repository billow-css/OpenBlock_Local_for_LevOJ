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

// ==================== 工具函数 ====================
constexpr int DR[] = {-1, 1, 0, 0};
constexpr int DC[] = { 0, 0,-1, 1};
constexpr int DR8[] = {0,0,1,-1,1,-1,1,-1};
constexpr int DC8[] = {1,-1,0,0,1,-1,-1,1};//special

int path_score(int k) {
    double t = std::sqrt(static_cast<double>(k)) - 1.0;
    return 10 * k + 18 * static_cast<int>(t * t);
}

int path_score(const Board& board, const std::vector<std::pair<int,int>>& path) {
    int k = static_cast<int>(path.size());
    int s = path_score(k);
    if (board.level < 4) return s;

    std::vector<std::vector<bool>> in_path(board.N, std::vector<bool>(board.N, false));
    for (auto [r, c] : path) in_path[r][c] = true;
    std::vector<std::vector<bool>> exploded(board.N, std::vector<bool>(board.N, false));

    for (auto [r, c] : path) {
        if (!board.at(r, c).is_bomb()) continue;
        for (int dr = -1; dr <= 1; ++dr)
            for (int dc = -1; dc <= 1; ++dc) {
                int nr = r + dr, nc = c + dc;
                if (board.in_bounds(nr, nc) && !in_path[nr][nc] && !exploded[nr][nc]) {
                    exploded[nr][nc] = true;
                    s += 10;
                }
            }
    }
    return s;
}