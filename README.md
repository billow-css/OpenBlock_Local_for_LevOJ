# OpenBlock_Local_for_LevOJ

本地化LevOJ嘉豪测评机与可视化播放器

![Stone Badge](https://stone.professorlee.work/api/stone/billow-css/OpenBlock_Local_for_LevOJ)

## 使用前请仔细阅读以下条目

1. 请确保你的环境包含`g++`或`clang++`其中之一；
2. 建议你的环境包含`python`；
3. 代码**不保证在全部环境下可以完美运行**，如有问题，请丢**issue**, 欢迎提交Pull Request；
4. 该测评机与官方测评机存在一定的分数差异，**但是分差不多**，因此仅作参考！

## 软件包组成

```plaintext
OpenBlock_Local_for_LevOJ/
├── run.py          # 自动化脚本
├── asset.hpp       # 游戏资产
├── engine.hpp      # 随机数生成器
├── localenv.hpp    # 一堆include
├── ai.hpp          # AI 函数（用户需编写）
├── tools.hpp       # 工具函数
├── local_judge.cpp # 本地测评机
└── visualizer.html # 可视化播放器
```

## 如何使用

1. 将项目克隆到本地
   ```bash
   git clone https://github.com/billow-css/OpenBlock_Local_for_LevOJ.git
   ```
2. 对于解题逻辑的导入：我们**非常荣幸**的向您宣告，您现在只需要修改`ai.hpp`即可！为了您**宝贵的**用户体验，文件格式简单到令人发指！！
   ```cpp
   #pragma once

   #include "locenv.hpp"
   typedef std::vector<std::pair<int,int>> ans;

   /*相关辅助函数*/

   ans find_best_path(const Board &board){
      /*code*/
      }
   ```
3. - 如果你的环境包含Python，运行

     ```bash
     python run.py
     ```

   - 否则需要你手动编译 `local_judge.cpp` 并运行 `local_judge` 可执行文件

     ```bash
     g++ local_judge.cpp -o local_judge
     ```

     ```bash
     ./local_judge
     ```

     然后打开 `visualizer.html` 可视化播放器

     ```bash
     start visualizer.html
     ```

4. 在 `visualizer.html` 中选择 `game_log.json` 文件，即可开始播放.
