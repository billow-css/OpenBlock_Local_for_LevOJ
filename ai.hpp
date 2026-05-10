#pragma once

#include "locenv.hpp"
typedef std::vector<std::pair<int,int>> ans;

// ==================== 你的解题逻辑 ====================
static bool can_connect(const Cell& a, const Cell& b) {
    return (a.color() == b.color() || a.color() == 0 || b.color() == 0);
}

bool is_neig(std::pair<int,int> a, std::pair<int,int> b)
{
    if((abs(a.first - b.first) + abs(a.second - b.second)) >= 2){return false;}
    else {return true;}
}

void DFS(int r,int c,const Board& board,
    std::vector<std::pair<int,int>>& route, int colorlock)
{
    if(!route.empty() && !is_neig(std::make_pair(r,c),route.back())){return;}
    route.emplace_back(r,c);
    for(int i = 0;i < 4;i++)
    {
        int nr = r + DR[i];
        int nc = c + DC[i];

        if (!board.in_bounds(nr, nc)){continue;}//越界检查
        if (std::find(route.begin(), route.end(),std::make_pair(nr, nc)) != route.end()){continue;}//重复检查

        if ((board.at(nr,nc).color() == colorlock || board.at(nr,nc).color() == 0) && can_connect(board.at(r,c), board.at(nr,nc))) {
            DFS(nr, nc, board, route, colorlock);   // 递归探索，不提前返回
        }
    }
}

void best_path_elem(std::vector<std::pair<std::vector<std::pair<int,int>>,int>> &rouPlen,const Board &board,int N){
    for (int r = 0; r < N; ++r) 
    {
        for (int c = 0; c < N; ++c) 
        {
            std::vector<std::pair<int,int>> route;
            double score = 0;
            int cnt=0;
            int extra =0;
            DFS(r,c,board,route,board.at(r,c).color());//获取可行的路径
            if(route.size() < 2){continue;}
            for(auto i : route)//获取路径可消除的块数
            {
                score=score+sqrt(i.first+1)*0.3+1;
                ++cnt;
                if(board.at(i.first,i.second).is_bomb())
                {
                    for(int j = 0; j < 8; j++)
                    {
                        int canr = i.first + DR8[j];
                        int canc = i.second + DC8[j];
                        if(find(route.begin(),route.end(),std::make_pair(canr,canc)) != route.end()
                            || canr < 0 || canr >= N || canc < 0 || canc >= N){continue;}
                        else{
                            extra++;
                        }
                    }
                }
            }
            score+=10*cnt+floor(18.0*(sqrt(cnt)-1)*(sqrt(cnt)-1))+10*extra;
            rouPlen.emplace_back(route,score);
        }
    }
}

ans find_best_path(const Board &board) {
    int N = board.N;
    /*候选池初始化*/
    std::vector<std::pair<std::vector<std::pair<int,int>>,int>> rouPlen;
    rouPlen.reserve(N*N/2);
    best_path_elem(rouPlen , board,N);
    // for(auto i : rouPlen){
    //    std::vector<std::pair<std::vector<std::pair<int,int>>,int>> rouPlenNext; 
    //    rouPlenNext.reserve(N*N/2);
    //    best_path_elem(rouPlen ,board.preview(i.first),N);
    //    sort(rouPlenNext.begin(),rouPlenNext.end(),[](std::pair<std::vector<std::pair<int,int>>,int> a,
    //         std::pair<std::vector<std::pair<int,int>>,int> b){return a.second > b.second;});
    //     double score_extra = rouPlenNext[0].second;
    //     i.second+=score_extra;
    // }
    for (auto& i : rouPlen) {
        std::vector<std::pair<std::vector<std::pair<int,int>>, int>> rouPlenNext;
        best_path_elem(rouPlenNext, board.preview(i.first), N);
        if (!rouPlenNext.empty()) {
            std::sort(rouPlenNext.begin(), rouPlenNext.end(),
                [](const auto& a, const auto& b) { return a.second > b.second; });
            i.second += rouPlenNext[0].second;  // 累加后续最佳启发分
        }
    }
    sort(rouPlen.begin(),rouPlen.end(),[](std::pair<std::vector<std::pair<int,int>>,int> a,
    std::pair<std::vector<std::pair<int,int>>,int> b){return a.second > b.second;});
    return rouPlen[0].first;
    // 兜底防御机制：理论上死局评测机会结束，若未结束强制返回两格防报错1
    return {{0, 0}, {0, 1}};
}

