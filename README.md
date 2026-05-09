# OpenBlock_Local_for_LevOJ

本地化LevOJ嘉豪测评机与可视化播放器

## 使用前请仔细阅读以下条目

1. 请确保电脑里面有Python，不过这**不是必须的**；
2. 代码**不保证在全部环境下可以完美运行**，如有问题，请丢**issue**；
3. 该测评机与官方测评机存在一定的分数差异，**但是分差不多**，因此仅作参考！

## 软件包组成

- run.py 自动化脚本
- local_judge.cpp 本地测评机与ai函数
- visualizer.html 可视化播放器

## 如何使用

1. 将项目克隆到本地
   ```bash
   git clone https://github.com/billow-css/OpenBlock_Local_for_LevOJ.git
   ```
2. 将你的 `find_best_path` 函数粘贴到 `local_judge.cpp Line 170 - 200`(附近) 中
   ```cpp
   std::vector<std::pair<int, int>> find_best_path(const Board &board);
   ```
3. - 如果你的环境包含Python，运行

     ```bash
     python run.py
     ```

   - 否则需要你手动编译 `local_judge.cpp` 并运行 `local_judge` 可执行文件

     ```bash
     g++ local_judge.cpp -o local_judge
     ```

     然后打开 `visualizer.html` 可视化播放器

     ```bash
     start visualizer.html
     ```

4. 在 `visualizer.html` 中选择 `game_log.json` 文件，即可开始播放.
