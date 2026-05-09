#if __cplusplus < 201700L
#error "C++17 required"
#endif

#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <vector>

// ============================================================
// 数据模型（带底层完美沙盘推演）
// ============================================================

struct Cell {
  int value = 1;
  int color() const { return std::abs(value); }
  bool is_bomb() const { return value < 0; }
  bool is_wildcard() const { return value == 0; }
};

struct Board {
  int N = 0;
  int level = 1;
  std::vector<std::vector<Cell>> grid;

  // 底层引擎变量：未来掉落物预知队列
  std::vector<std::vector<int>> drop_queue;
  std::vector<int> queue_ptr;

  explicit Board(int n = 0) : N(n), grid(n, std::vector<Cell>(n)) {}

  Cell &at(int r, int c) { return grid[r][c]; }
  const Cell &at(int r, int c) const { return grid[r][c]; }
  bool in_bounds(int r, int c) const {
    return r >= 0 && r < N && c >= 0 && c < N;
  }

  // 【核心功能：未来预知引擎】
  // 输入：一条打算行走的路径
  // 返回：走出这条路径后的完美盘面预测（包含确切的新掉落方块！）
  Board preview(const std::vector<std::pair<int, int>> &path) const {
    Board next_b = *this;
    if (path.size() < 2)
      return next_b;

    std::vector<std::vector<bool>> in_path(N, std::vector<bool>(N, false));
    for (auto p : path)
      in_path[p.first][p.second] = true;

    std::vector<std::vector<bool>> to_remove = in_path;

    // 模拟炸弹爆炸
    if (level >= 4) {
      for (auto [r, c] : path) {
        if (!at(r, c).is_bomb())
          continue;
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

    // 模拟重力下落与新块生成
    for (int c = 0; c < N; ++c) {
      std::vector<Cell> remaining;
      for (int r = 0; r < N; ++r) {
        if (!to_remove[r][c])
          remaining.push_back(at(r, c));
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

  // 是否死局检测
  bool is_deadlocked() const {
    for (int r = 0; r < N; ++r) {
      for (int c = 0; c < N; ++c) {
        int ac = at(r, c).color();
        if (c + 1 < N) {
          int ac2 = at(r, c + 1).color();
          if (ac == ac2 || ac == 0 || ac2 == 0)
            return false;
        }
        if (r + 1 < N) {
          int ac2 = at(r + 1, c).color();
          if (ac == ac2 || ac == 0 || ac2 == 0)
            return false;
        }
      }
    }
    return true;
  }
};

// ============================================================
// 中间件：GameController
// ============================================================

class GameController {
  Board _board;
  int _level = 0;
  int _step = 0;
  int _score = 0;
  bool _done = false;
  std::string _pending_line;
  std::vector<std::pair<int, int>> _last_path;

  static int try_parse_level(const std::string &line, int &level, int &seed) {
    int lv, sd, N, steps;
    if (std::sscanf(line.c_str(), "LEVEL %d SEED %d SIZE %d STEPS %d", &lv, &sd,
                    &N, &steps) == 4) {
      level = lv;
      seed = sd;
      return N;
    }
    return 0;
  }

  static bool try_parse_step(const std::string &line, int &step, int &score,
                             bool &valid) {
    char buf[16] = {};
    if (std::sscanf(line.c_str(), "STEP %d SCORE %d %15s", &step, &score,
                    buf) >= 3) {
      valid = (std::string(buf) == "VALID");
      return true;
    }
    return false;
  }

  static int gen_block(std::mt19937 &rng, int level) {
    if (level <= 2)
      return (rng() % 5) + 1;
    else if (level == 3)
      return ((rng() % 100) < 15) ? 0 : (rng() % 5) + 1;
    else if (level == 4) {
      int color = (rng() % 5) + 1;
      return ((rng() % 100) < 10) ? -color : color;
    } else {
      if ((rng() % 100) < 15)
        return 0;
      int base = (rng() % 5) + 1;
      return ((rng() % 100) < 10) ? -base : base;
    }
  }

  static void init_queues(Board &b, int seed, int N, int level) {
    b.level = level;
    std::mt19937 rng(seed);
    b.drop_queue.assign(N, std::vector<int>(1000));
    b.queue_ptr.assign(N, 0);
    for (int c = 0; c < N; ++c) {
      for (int i = 0; i < 1000; ++i)
        b.drop_queue[c][i] = gen_block(rng, level);
    }
  }

  bool read_line(std::string &line) {
    if (!_pending_line.empty()) {
      line = std::move(_pending_line);
      _pending_line.clear();
      return true;
    }
    return (bool)std::getline(std::cin, line);
  }

  Board read_board(int N) {
    Board board(N);
    for (int row = 0; row < N; ++row) {
      std::string line;
      read_line(line);
      std::istringstream ls(line);
      for (int c = 0; c < N; ++c)
        ls >> board.at(row, c).value;
    }
    return board;
  }

  void drain_trailing() {
    std::string line;
    while (std::cin.rdbuf()->in_avail() > 0) {
      if (!read_line(line))
        break;
      if (line.empty() || line.find("LEVEL_END") != std::string::npos)
        continue;
      if (line.find("FINAL_SCORE") != std::string::npos) {
        _done = true;
        continue;
      }
      _pending_line = std::move(line);
      break;
    }
  }

public:
  const Board &board() const { return _board; }
  int level() const { return _level; }
  int step() const { return _step; }
  int score() const { return _score; }
  bool done() const { return _done; }

  bool update() {
    std::string first_line;
    while (true) {
      if (!read_line(first_line)) {
        _done = true;
        return false;
      }
      if (!first_line.empty())
        break;
    }

    if (first_line.find("LEVEL_END") != std::string::npos ||
        first_line.find("FINAL_SCORE") != std::string::npos) {
      _done = true;
      return false;
    }

    int seed;
    int new_N = try_parse_level(first_line, _level, seed);
    if (new_N > 0) {
      Board new_board = read_board(new_N);
      init_queues(new_board, seed, new_N, _level);
      _board = std::move(new_board);
      _step = 0;
      _score = 0;
      drain_trailing();
      return true;
    }

    int step, score;
    bool valid;
    if (try_parse_step(first_line, step, score, valid)) {
      _step = step;
      _score = score;

      // 同步指针：如果上一步合法，我们在本地走一遍以消耗掉正确的 queue_ptr
      Board predicted =
          (valid && !_last_path.empty()) ? _board.preview(_last_path) : _board;

      Board new_board = read_board(_board.N);
      new_board.level = _level;
      new_board.drop_queue = std::move(predicted.drop_queue);
      new_board.queue_ptr = std::move(predicted.queue_ptr);
      _board = std::move(new_board);
      _last_path.clear();

      drain_trailing();
      if (!_pending_line.empty()) {
        int next_level, next_seed;
        int next_N = try_parse_level(_pending_line, next_level, next_seed);
        if (next_N > 0) {
          _level = next_level;
          _pending_line.clear();
          Board nb = read_board(next_N);
          init_queues(nb, next_seed, next_N, next_level);
          _board = std::move(nb);
          _step = 0;
          _score = 0;
          drain_trailing();
        }
      }
      return true;
    }

    _done = true;
    return false;
  }

  void respond(const std::vector<std::pair<int, int>> &path) {
    _last_path = path;
    std::cout << path.size();
    for (auto [r, c] : path)
      std::cout << ' ' << r << ' ' << c;
    std::cout << '\n';
    std::cout.flush();
  }
};

// ============================================================
// 工具函数
// ============================================================

constexpr int DR[] = {-1, 1, 0, 0};
constexpr int DC[] = {0, 0, -1, 1};

int path_score(int k) {
  double t = std::sqrt(static_cast<double>(k)) - 1.0;
  return 10 * k + 18 * static_cast<int>(t * t);
}

int path_score(const Board &board,
               const std::vector<std::pair<int, int>> &path) {
  int k = static_cast<int>(path.size());
  int s = path_score(k);

  std::vector<std::vector<bool>> in_path(board.N,
                                         std::vector<bool>(board.N, false));
  for (auto [r, c] : path)
    in_path[r][c] = true;
  std::vector<std::vector<bool>> exploded(board.N,
                                          std::vector<bool>(board.N, false));

  for (auto [r, c] : path) {
    if (!board.at(r, c).is_bomb())
      continue;
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

// ============================================================
// 解题逻辑（如果你不理解这份代码其他部分在干什么，请仅在此处进行策略实现和修改）
// ============================================================

std::vector<std::pair<int, int>> find_best_path(const Board &board) {
  int N = board.N;

  // 遍历整个棋盘，寻找最近的（任意一对）可连接的相邻两格
  // 这是独属于嘉豪的雷霆策略
  for (int r = 0; r < N; ++r) {
    for (int c = 0; c < N; ++c) {
      int color1 = board.at(r, c).color();

      // 检查右侧相邻格子
      if (c + 1 < N) {
        int color2 = board.at(r, c + 1).color();
        if (color1 == color2 || color1 == 0 || color2 == 0) {
          return {{r, c}, {r, c + 1}};
        }
      }

      // 检查下方相邻格子
      if (r + 1 < N) {
        int color2 = board.at(r + 1, c).color();
        if (color1 == color2 || color1 == 0 || color2 == 0) {
          return {{r, c}, {r + 1, c}};
        }
      }
    }
  }

  // 兜底防御机制：理论上死局评测机会结束，若未结束强制返回两格防报错1
  return {{0, 0}, {0, 1}};
}

int main() {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);

  GameController ctl;
  while (ctl.update()) {
    auto path = find_best_path(ctl.board());
    ctl.respond(path);
  }
  return 0;
}