#!/usr/bin/env python3
"""
LevOJ 本地测评与可视化整合脚本
功能：
  1. 自动编译 local_judge.cpp （C++17）
  2. 运行测评机，生成 game_log.json，同时实时显示终端输出
  3. 自动打开 visualizer.html 以便手动导入生成的 JSON 日志
"""

import os
import sys
import platform
import subprocess
import webbrowser
import shutil
from pathlib import Path

# ---------- 配置 ----------
CPP_SOURCE = "local_judge.cpp"  # 测评机源码文件名
OUTPUT_BIN = "local_judge"  # 输出的可执行文件名（Windows 会自动加 .exe）
HTML_FILE = "visualizer.html"  # 可视化页面文件名
JSON_LOG = "game_log.json"  # 生成的日志文件名


# ---------- 工具函数 ----------
def is_windows():
    return platform.system() == "Windows"


def is_macos():
    return platform.system() == "Darwin"


def get_compiler():
    """根据平台返回编译器命令和标准库选项"""
    if is_windows():
        # Windows 常用 MinGW 的 g++
        if shutil.which("g++"):
            return "g++", []
        else:
            print("❌ 未找到 g++，请安装 MinGW 或配置 g++ 到 PATH。")
            sys.exit(1)
    elif is_macos():
        if shutil.which("clang++"):
            return "clang++", ["-std=c++17", "-O2"]
        else:
            print("❌ 未找到 clang++。")
            sys.exit(1)
    else:  # Linux / other Unix
        if shutil.which("g++"):
            return "g++", ["-std=c++17", "-O2"]
        else:
            print("❌ 未找到 g++，请安装 build-essential。")
            sys.exit(1)


def compile_source(compiler, flags):
    """编译 C++ 源码"""
    source_path = Path(CPP_SOURCE)
    if not source_path.exists():
        print(f"❌ 找不到源文件 {CPP_SOURCE}，请将脚本与测评机源码放在同一目录。")
        sys.exit(1)

    out_name = OUTPUT_BIN
    if is_windows():
        out_name += ".exe"

    cmd = [compiler] + flags + ["-o", out_name, str(source_path)]
    print(f"🔨 编译命令: {' '.join(cmd)}")
    result = subprocess.run(cmd, capture_output=True, text=True)
    if result.returncode != 0:
        print("❌ 编译失败，错误信息：")
        print(result.stderr)
        sys.exit(1)
    print("✅ 编译成功。")
    return out_name


def run_executable(executable):
    """运行测评机，打印输出并生成 JSON"""
    exe_path = Path(executable)
    if is_windows() and not exe_path.exists():
        print(f"❌ 找不到可执行文件 {executable}")
        sys.exit(1)
    # 如果是 Unix-like，可能没有执行权限
    if not is_windows():
        os.chmod(exe_path, 0o755)

    print(f"🚀 运行测评机: {executable}")
    # 实时输出到终端
    process = subprocess.Popen(
        [f"./{executable}" if not is_windows() else executable],
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        bufsize=1,
    )
    for line in process.stdout:
        print(line, end="")
    process.wait()
    if process.returncode != 0:
        print(f"❌ 测评机运行出错，返回码 {process.returncode}")
        sys.exit(1)
    print("✅ 测评完成，已生成 game_log.json")


def open_html():
    """用默认浏览器打开可视化页面"""
    html_path = Path(HTML_FILE).resolve()
    if not html_path.exists():
        print(f"⚠️ 可视化文件 {HTML_FILE} 不存在，请确保放在同一目录。")
        return
    print(f"🌐 打开可视化页面: {html_path}")
    webbrowser.open(f"file://{html_path}")


# ---------- 主流程 ----------
def main():
    print("===============================================")
    print("   LevOJ 本地测评 · 可视化一键整合脚本")
    print("===============================================")

    # 1. 编译
    compiler, base_flags = get_compiler()
    flags = base_flags + (["-std=c++17"] if "-std=c++17" not in base_flags else [])
    # 确保包含 C++17 标志
    if not any("-std=" in f for f in flags):
        flags.append("-std=c++17")
    executable = compile_source(compiler, flags)

    # 2. 运行
    run_executable(executable)

    # 3. 检查 JSON 是否生成
    if not Path(JSON_LOG).exists():
        print(f"⚠️ 警告：未找到 {JSON_LOG}，可能测评机未正确输出。")
    else:
        print(f"✅ 日志文件 {JSON_LOG} 已生成。")

    # 4. 打开 HTML
    open_html()

    print("\n🎉 全部完成！在浏览器中手动导入 game_log.json 即可开始分析。")


if __name__ == "__main__":
    main()
