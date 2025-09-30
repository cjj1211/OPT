import argparse
import re
import matplotlib.pyplot as plt
import numpy as np
import os
import time
from collections import Counter

EVENT_TYPES = [
    "NO LOG",
    "BATTERY_UNDERVOLTAGE",
    "CASE_TEMP_OVERHEAT",
    "BATT_TEMP_OVERHEAT",
    "DISCHARGE_OVERCURR",
    "PROBE_OVERCURR",
    "UNKNOWN_CRITICAL_TYPE"
]

EVENT_COLORS = {
    "NO LOG": "red",
    "BATTERY_UNDERVOLTAGE": "orange",
    "CASE_TEMP_OVERHEAT": "blue",
    "BATT_TEMP_OVERHEAT": "green",
    "DISCHARGE_OVERCURR": "purple",
    "PROBE_OVERCURR": "brown",
    "UNKNOWN_CRITICAL_TYPE": "gray"
}

def parse_log_line(line):
    time_match = re.search(r'(\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}[.,]\d{3})', line)
    if not time_match or 'CRITICAL' not in line:
        return None
    t_str = time_match.group(1)
    type_match = re.search(r'Type:\s*([A-Z_]+)', line)
    type_name = type_match.group(1) if type_match else None
    if type_name not in EVENT_TYPES:
        return None
    data_match = re.search(r'Data:\s*(\d+)', line)
    data_val = int(data_match.group(1)) if data_match else 0
    return t_str, type_name, data_val

def main():
    parser = argparse.ArgumentParser(description="CRITICAL事件分析工具")
    parser.add_argument("logfile", help="日志文件路径")
    args = parser.parse_args()

    critical_records = []
    levels = Counter()
    keywords = Counter()
    delays = []
    prev_time_sec = None

    with open(args.logfile, 'r', encoding='utf8') as f:
        for line in f:
            # CRITICAL事件解析
            parsed = parse_log_line(line)
            if parsed:
                critical_records.append(parsed)

            # 关键字统计
            msg_match = re.search(r'changed|timeout|error', line, re.IGNORECASE)
            has_changed = bool(msg_match and msg_match.group(0).lower() == 'changed')
            if msg_match:
                keywords[msg_match.group(0).lower()] += 1

            # 延迟统计（仅包含 changed）
            if has_changed:
                time_match = re.search(r'(\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}[.,]\d{3})', line)
                if time_match:
                    t_str = time_match.group(1).replace(',', '.')
                    h, m, s_ms = t_str.split(' ')[1].split(':')
                    s, ms = s_ms.split('.')
                    t_sec = int(h)*3600 + int(m)*60 + int(s) + int(ms)/1000
                    if prev_time_sec is not None:
                        delays.append((t_sec - prev_time_sec)*1000)
                    prev_time_sec = t_sec

            # 日志等级统计
            level_match = re.search(r'\b(INFO|WARN|ERROR|DEBUG|FATAL|NORMAL|CRITICAL)\b', line)
            if level_match:
                levels[level_match.group(1)] += 1

    # 创建保存目录
    timestamp_str = time.strftime("%Y%m%d_%H%M%S")
    result_dir = os.path.join(os.getcwd(), timestamp_str)
    os.makedirs(result_dir, exist_ok=True)

    # 保存统计信息TXT
    type_counts = Counter([r[1] for r in critical_records])
    stats_text = (
        f"延时统计 (ms, 仅 changed 行):\n"
        f"  样本数量: {len(delays)}\n"
        f"  最小值: {np.min(delays) if delays else 0:.3f}\n"
        f"  最大值: {np.max(delays) if delays else 0:.3f}\n"
        f"  平均值: {np.mean(delays) if delays else 0:.3f}\n"
        f"  95%分位: {np.percentile(delays, 95) if delays else 0:.3f}\n\n"
        f"日志等级统计:\n{dict(levels)}\n\n"
        f"关键字统计:\n{dict(keywords)}\n\n"
        f"CRITICAL事件类型统计:\n{dict(type_counts)}\n"
    )
    with open(os.path.join(result_dir, "log_stats.txt"), "w", encoding="utf8") as f:
        f.write(stats_text)

    # 绘图
    fig, axes = plt.subplots(3, 1, figsize=(18, 18))

    # 1. 延时折线图
    axes[0].plot(delays, marker='o', color='royalblue')
    axes[0].set_title("Time Delay Between 'changed' Logs (ms)")
    axes[0].set_xlabel("Changed Log Index")
    axes[0].set_ylabel("Delay (ms)")
    axes[0].grid(True)

    # 2. 延时分布柱状图
    delays_arr = np.array(delays) if delays else np.array([0])
    bins = [0, 10, 20, 50, 100, max(100, np.max(delays_arr))]
    hist, bin_edges = np.histogram(delays_arr, bins=bins)
    bin_labels = [f"{int(bin_edges[i])}-{int(bin_edges[i+1])}ms" for i in range(len(bin_edges)-1)]
    bars = axes[1].barh(bin_labels, hist, color='skyblue')
    axes[1].set_title("Time Delay Distribution (changed)")
    axes[1].set_xlabel("Count")
    axes[1].set_ylabel("Delay Range")
    axes[1].bar_label(bars, labels=[str(h) for h in hist], padding=3)
    axes[1].grid(axis='x', linestyle='--', alpha=0.7)

    # 3. CRITICAL事件时间分布图
    y_map = {etype: i for i, etype in enumerate(EVENT_TYPES)}
    if critical_records:
        y_vals = [y_map[r[1]] for r in critical_records]
        x_vals = [r[0] for r in critical_records]
        data_vals = [r[2] for r in critical_records]
        colors = [EVENT_COLORS[r[1]] for r in critical_records]

        axes[2].scatter(range(len(x_vals)), y_vals, c=colors, s=50)
        for i, (x, y, d) in enumerate(zip(range(len(x_vals)), y_vals, data_vals)):
            axes[2].text(x, y, str(d), fontsize=8, ha='left', va='bottom')
        axes[2].set_xticks(range(len(x_vals)))
        axes[2].set_xticklabels(x_vals, rotation=45, fontsize=8)
    else:
        axes[2].scatter([], [])
        axes[2].set_xticks([])

    axes[2].set_yticks(range(len(EVENT_TYPES)))
    axes[2].set_yticklabels(EVENT_TYPES)
    axes[2].set_xlabel("Timestamp")
    axes[2].set_ylabel("CRITICAL Event Type")
    axes[2].set_title("CRITICAL Event Data Over Time")
    axes[2].grid(True)

    plt.subplots_adjust(top=0.95, bottom=0.1, hspace=0.5)
    out_file = os.path.join(result_dir, "critical_event_time_scatter_colored.png")
    plt.savefig(out_file, dpi=150)
    plt.show()

    print(f"分析结果已保存到文件夹: {result_dir}")

if __name__ == '__main__':
    main()
