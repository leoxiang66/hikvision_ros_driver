import math

def read_timestamps(filename):
    """从文件中读取时间戳"""
    with open(filename, 'r') as file:
        return [line.strip() for line in file.readlines()]

def extract_seconds(timestamp):
    """从时间戳中提取秒的小数部分"""
    # 提取小数部分并转换为浮点数
    seconds = float("0." + timestamp.split('.')[-1])
    return seconds

def calculate_sync_errors(timestamps):
    """计算同步误差（以秒为单位）"""
    base_time = extract_seconds(timestamps[0])  # 基准时间（秒的小数部分）
    target_time = base_time  # 初始化目标时间
    errors = []

    for i, timestamp in enumerate(timestamps[1:]):
        actual_time = extract_seconds(timestamp)  # 实际时间（秒的小数部分）
        error = actual_time - target_time  # 计算误差
        errors.append((i + 1, target_time, actual_time, error))
        target_time += 0.05  # 每次目标时间增加 50 毫秒（0.05 秒）
        target_time = target_time - math.floor(target_time)

    return errors

def print_errors(errors):
    """打印同步误差结果"""
    print(f"{'Index':<6} {'Target Time (s)':<20} {'Actual Time (s)':<20} {'Sync Error (s)':<15}")
    print("-" * 65)
    for index, target_time, actual_time, error in errors:
        print(f"{index:<6} {target_time:<20.9f} {actual_time:<20.9f} {error:<15.9f}")

if __name__ == "__main__":
    # 文件名
    filename = "timestamps.txt"

    # 读取时间戳
    timestamps = read_timestamps(filename)

    # 计算同步误差
    errors = calculate_sync_errors(timestamps)

    # 打印误差
    print_errors(errors)
    
    
    print(sum([e[-1] for e in errors])/len(errors))