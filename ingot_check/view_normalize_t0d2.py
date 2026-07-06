#!/usr/bin/env python3
# """
# t0d2 交互式 strip 查看器 (64条正面 + 64条背面)

# 用法:
#     python view_t0d2.py

# 第一步: 处理 ROOT 文件并生成输出文件 (对应 notebook 第一个 cell)。
# 第二步: 进入交互模式，命令行查看 strip。

# 交互命令:
#     f-<N>   : 查看 front strip N (0-63)
#     b-<N>   : 查看 back strip N (0-63)
#     <回车>  : 自动切换到下一条 (f-63 -> f-0, b-63 -> b-0)
#     h/help  : 显示帮助
#     q/quit  : 退出
#     Ctrl+C  : 退出
# """

import ROOT
import sys
import os
import re

# ============================================================
# Part 1: 处理数据 (对应 notebook 第一个 cell)
# ============================================================

INPUT_FILE = "/data/disk1/ribll2026_www_data/normalize/t0d2_t1_0060_0060.root"
OUTPUT_DIR = "/home/ribll2026/ribll2026_www/github_code/jupyter/ingot_check/root_file"
OUTPUT_FILE = os.path.join(OUTPUT_DIR, "normalize_t0d2.root")

NSTRIPS = 64  # d2 正面背面都是 64 条

print("=" * 60)
print("Opening input file: {}".format(INPUT_FILE))
f = ROOT.TFile.Open(INPUT_FILE)
if not f or f.IsZombie():
    print("ERROR: Cannot open input file!")
    sys.exit(1)

os.makedirs(OUTPUT_DIR, exist_ok=True)
# print("Creating output file: {}".format(OUTPUT_FILE))
# opt = ROOT.TFile(OUTPUT_FILE, "recreate")

# 存储读取的对象
gf = [None] * NSTRIPS
resf = [None] * NSTRIPS
gb = [None] * NSTRIPS
resb = [None] * NSTRIPS

print("\nProcessing {} strips...".format(NSTRIPS))
for i in range(NSTRIPS):
    c1 = ROOT.TCanvas("c1_{}".format(i), "front_back_{}".format(i), 1000, 500)
    c2 = ROOT.TCanvas("c2_{}".format(i), "front_back_{}".format(i), 1000, 500)
    c1.Divide(2, 1)
    c2.Divide(2, 1)

    gf[i] = f.Get("gf{}".format(i))
    resf[i] = f.Get("resf{}".format(i))

    if not gf[i] or not resf[i]:
        print("  Warning: Cannot find gf{} or resf{}".format(i, i))
        continue

    c1.cd(1)
    gf[i].SetTitle("front strip {}".format(i))
    # gf[i].Draw("AP*")
    c1.cd(2)
    # resf[i].Draw()

    gb[i] = f.Get("gb{}".format(i))
    resb[i] = f.Get("resb{}".format(i))

    if not gb[i] or not resb[i]:
        print("  Warning: Cannot find gb{} or resb{}".format(i, i))
        continue

    c2.cd(1)
    gb[i].SetTitle("back strip {}".format(i))
    # gb[i].Draw("AP*")
    c2.cd(2)
    # resb[i].Draw()

    # opt.cd()
    # c1.Write("front{:02d}".format(i))
    # c2.Write("back{:02d}".format(i))

# opt.Close()
print("Output file created: {}".format(OUTPUT_FILE))
print("=" * 60)


# ============================================================
# Part 2: 交互式查看模式
# ============================================================

def show_help():
    print("""
    ===== 交互式 strip 查看器 =====
    命令:
      f-<N>     查看 front strip N (0-63)
      b-<N>     查看 back strip N (0-63)
      <回车>    自动切换到下一条 strip
                (f-63 按回车回到 f-0, b-63 按回车回到 b-0)
      h/help    显示帮助
      q/quit    退出

    示例:
      > f-32     (查看正面第32条)
      > <回车>   (自动切到正面第33条)
      > b-0      (查看背面第0条)
      > <回车>   (自动切到背面第1条)
    """)


# 创建持久化 canvas 用于查看
c_view = ROOT.TCanvas("c_view", "Strip Viewer", 800, 400)
c_view.Divide(2, 1)

current_type = None   # 'f' 或 'b'
current_index = 0     # 0-63

print("\n交互模式已就绪。输入 'h' 查看帮助, 'q' 退出.\n")


def show_strip(strip_type, index):
    """在 canvas 上显示指定的 strip."""
    global current_type, current_index
    current_type = strip_type
    current_index = index

    c_view.Clear()
    c_view.Divide(2, 1)

    c_view.cd(1)
    if strip_type == 'f':
        gr = gf[index]
        hist = resf[index]
        title = "front strip {}".format(index)
    else:
        gr = gb[index]
        hist = resb[index]
        title = "back strip {}".format(index)

    if not gr or not hist:
        print("  ERROR: Data for {}-{} not available!".format(strip_type, index))
        return

    gr.SetTitle(title)
    gr.Draw("AP*")
    c_view.cd(2)
    hist.Draw()
    c_view.Update()
    print("  -> 显示 {} strip {}".format(
        "front" if strip_type == 'f' else "back", index))


def advance_strip():
    """切换到下一条 strip，循环回绕."""
    global current_type, current_index
    if current_type is None:
        print("  尚未选择 strip，请先输入 f-<N> 或 b-<N>.")
        return
    next_index = (current_index + 1) % NSTRIPS
    show_strip(current_type, next_index)


def parse_command(cmd):
    """解析用户命令. 返回 True 继续, False 退出."""
    cmd = cmd.strip()

    if not cmd:
        advance_strip()
        return True

    if cmd.lower() in ('q', 'quit'):
        print("退出...")
        return False

    if cmd.lower() in ('h', 'help'):
        show_help()
        return True

    # 解析 f-<N> 或 b-<N>
    m = re.match(r'^([fb])-(\d+)$', cmd)
    if m:
        strip_type = m.group(1)
        index = int(m.group(2))
        if index < 0 or index >= NSTRIPS:
            print("  ERROR: Strip 编号必须在 0-{} 之间!".format(NSTRIPS - 1))
            return True
        show_strip(strip_type, index)
        return True

    # 格式错误
    print("  ERROR: 无效命令 '{}'".format(cmd))
    print("  有效格式: f-<N> (0-63), b-<N> (0-63), <回车>, q, h")
    print("  输入 'h' 查看完整帮助.")
    return True


# 主交互循环
running = True
while running:
    try:
        cmd = raw_input("> ")
    except NameError:
        cmd = input("> ")
    except EOFError:
        print("\n退出...")
        break
    except KeyboardInterrupt:
        print("\n退出...")
        break

    running = parse_command(cmd)

print("Done.")