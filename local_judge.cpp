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

// ==================== 数据模型（保持不变） ====================
struct Cell {
    int value = 1;
    int  color()       const { return std::abs(value); }
    bool is_bomb()     const { return value < 0; }
    bool is_wildcard() const { return value == 0; }
};

struct Board {
    int N = 0;
    int level = 1;
    std::vector<std::vector<Cell>> grid;
    std::vector<std::vector<int>> drop_queue;
    std::vector<int> queue_ptr;

    explicit Board(int n = 0) : N(n), grid(n, std::vector<Cell>(n)) {}

    Cell&       at(int r, int c)       { return grid[r][c]; }
    const Cell& at(int r, int c) const { return grid[r][c]; }
    bool in_bounds(int r, int c) const { return r >= 0 && r < N && c >= 0 && c < N; }

    Board preview(const std::vector<std::pair<int,int>>& path) const {
        Board next_b = *this;
        if (path.size() < 2) return next_b;

        std::vector<std::vector<bool>> in_path(N, std::vector<bool>(N, false));
        for (auto p : path) in_path[p.first][p.second] = true;
        std::vector<std::vector<bool>> to_remove = in_path;

        if (level >= 4) {
            for (auto [r, c] : path) {
                if (!at(r, c).is_bomb()) continue;
                for (int dr = -1; dr <= 1; ++dr) {
                    for (int dc = -1; dc <= 1; ++dc) {
                        int nr = r + dr, nc = c + dc;
                        if (in_bounds(nr, nc) && !in_path[nr][nc]) {
                            to_remove[nr][nc] = true;
                        }
                    }
                }
            }
        }

        for (int c = 0; c < N; ++c) {
            std::vector<Cell> remaining;
            for (int r = 0; r < N; ++r) {
                if (!to_remove[r][c]) remaining.push_back(at(r, c));
            }
            int empty = N - (int)remaining.size();
            for (int i = 0; i < empty; ++i) {
                int val = next_b.drop_queue[c][next_b.queue_ptr[c]++];
                next_b.at(i, c).value = val;
            }
            for (int i = 0; i < (int)remaining.size(); ++i) {
                next_b.at(empty + i, c) = remaining[i];
            }
        }
        return next_b;
    }

    bool is_deadlocked() const {
        for (int r = 0; r < N; ++r) {
            for (int c = 0; c < N; ++c) {
                int ac = at(r, c).color();
                if (c + 1 < N) {
                    int ac2 = at(r, c + 1).color();
                    if (ac == ac2 || ac == 0 || ac2 == 0) return false;
                }
                if (r + 1 < N) {
                    int ac2 = at(r + 1, c).color();
                    if (ac == ac2 || ac == 0 || ac2 == 0) return false;
                }
            }
        }
        return true;
    }
};

// ==================== 工具函数 ====================
constexpr int DR[] = {-1, 1, 0, 0};
constexpr int DC[] = { 0, 0,-1, 1};
constexpr int DR8[] = {0,0,1,-1,1,-1,1,-1};
constexpr int DC8[] = {1,-1,0,0,1,-1,-1,1};

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

std::vector<std::pair<int,int>> find_best_path(const Board &board) {
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


// ==================== 简单的 JSON 构建器 ====================
class JsonBuilder {
    std::ostringstream oss;
    bool first = true;
public:
    void start_object() { oss << "{"; first = true; }
    void end_object()   { oss << "}"; }
    void start_array()  { oss << "["; first = true; }
    void end_array()    { oss << "]"; }
    
    // 添加一个键，并准备写值
    void key(const std::string& k) {
        if (!first) oss << ",";
        first = false;
        oss << "\"" << k << "\":";
    }
    
    // 简单值
    void value(int v) { oss << v; }
    void value(const std::string& s) { oss << "\"" << s << "\""; }
    void value_bool(bool b) { oss << (b ? "true" : "false"); }
    
    // 数组（整数数组、二维数组、路径坐标数组）
    void array_int(const std::vector<int>& vec) {
        oss << "[";
        for (size_t i = 0; i < vec.size(); ++i) {
            if (i > 0) oss << ",";
            oss << vec[i];
        }
        oss << "]";
    }
    void array_2d_int(const std::vector<std::vector<int>>& mat) {
        oss << "[";
        for (size_t i = 0; i < mat.size(); ++i) {
            if (i > 0) oss << ",";
            array_int(mat[i]);
        }
        oss << "]";
    }
    void array_pair(const std::vector<std::pair<int,int>>& path) {
        oss << "[";
        for (size_t i = 0; i < path.size(); ++i) {
            if (i > 0) oss << ",";
            oss << "[" << path[i].first << "," << path[i].second << "]";
        }
        oss << "]";
    }
    
    // 在数组中直接插入一个已构造好的 JSON 片段（自动处理逗号）
    void array_element_raw(const std::string& raw_json) {
        if (!first) oss << ",";
        first = false;
        oss << raw_json;
    }
    
    std::string str() const { return oss.str(); }
};

// 将 Board 的 grid 转为 int 的二维数组（便于序列化）
std::vector<std::vector<int>> board_to_int(const Board& board) {
    std::vector<std::vector<int>> res(board.N, std::vector<int>(board.N));
    for (int r = 0; r < board.N; ++r)
        for (int c = 0; c < board.N; ++c)
            res[r][c] = board.at(r, c).value;
    return res;
}

// ==================== 主函数 ====================
int main() {
    const int SEED = 114514;
    const int STEPS_PER_LEVEL = 50;
    int grand_total = 0;

    std::vector<std::string> level_jsons;

    for (int level = 1; level <= 5; ++level) {
        int N = (level <= 4) ? 10 : 12;
        Board board(N);
        init_queues(board, SEED, N, level);

        // 生成初始棋盘
        for (int r = 0; r < N; ++r)
            for (int c = 0; c < N; ++c)
                board.at(r, c).value = board.drop_queue[c][board.queue_ptr[c]++];

        std::vector<std::string> step_jsons;

        // 步骤 0（初始状态）
        {
            JsonBuilder step;
            step.start_object();
            step.key("step"); step.value(0);
            step.key("score"); step.value(0);
            step.key("board"); step.array_2d_int(board_to_int(board));
            step.key("path"); step.start_array(); step.end_array();
            step.end_object();
            step_jsons.push_back(step.str());
        }

        std::cout << "=== LEVEL " << level << " ===\n";
        int level_score = 0;

        for (int step = 1; step <= STEPS_PER_LEVEL; ++step) {
            if (board.is_deadlocked()) {
                std::cout << "Deadlock at step " << step << "\n";
                break;
            }

            auto path = find_best_path(board);
            int score = path_score(board, path);
            level_score += score;

            // 终端输出
            std::cout << path.size();
            for (auto [r, c] : path) std::cout << ' ' << r << ' ' << c;
            std::cout << "  (score: " << score << ")\n";

            Board new_board = board.preview(path);
            {
                JsonBuilder step_builder;
                step_builder.start_object();
                step_builder.key("step"); step_builder.value(step);
                step_builder.key("score"); step_builder.value(score);
                step_builder.key("path"); step_builder.array_pair(path);
                step_builder.key("board"); step_builder.array_2d_int(board_to_int(new_board));
                step_builder.end_object();
                step_jsons.push_back(step_builder.str());
            }

            board = new_board;
        }

        // 构建关卡 JSON
        JsonBuilder level_builder;
        level_builder.start_object();
        level_builder.key("level"); level_builder.value(level);
        level_builder.key("N"); level_builder.value(N);
        level_builder.key("seed"); level_builder.value(SEED);
        level_builder.key("total_score"); level_builder.value(level_score);
        level_builder.key("steps");
        level_builder.start_array();
        for (const auto& s : step_jsons)
            level_builder.array_element_raw(s);
        level_builder.end_array();
        level_builder.end_object();
        level_jsons.push_back(level_builder.str());

        grand_total += level_score;
        std::cout << "Level " << level << " total: " << level_score << "\n\n";
    }

    // 最终 JSON
    JsonBuilder final_json;
    final_json.start_object();
    final_json.key("seed"); final_json.value(SEED);
    final_json.key("final_score"); final_json.value(grand_total);
    final_json.key("levels");
    final_json.start_array();
    for (const auto& lvl : level_jsons)
        final_json.array_element_raw(lvl);
    final_json.end_array();
    final_json.end_object();

    std::ofstream ofs("game_log.json");
    ofs << final_json.str();
    ofs.close();

    std::cout << "FINAL_SCORE: " << grand_total << std::endl;
    std::cout << "Game log saved to game_log.json" << std::endl;

    return 0;
}