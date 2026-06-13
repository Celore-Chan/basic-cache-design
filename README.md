```markdown
# basic-cache-design
```
[![Language](https://img.shields.io/badge/language-C-blue)](https://en.wikipedia.org/wiki/C_(programming_language))
[![Algorithm](https://img.shields.io/badge/algorithm-LRU-green)](https://en.wikipedia.org/wiki/Cache_replacement_policies#LRU)

**LRU Cache Simulator** —— 使用 C 语言实现的 LRU（Least Recently Used）缓存替换算法模拟器

本项目从零实现一个高速缓存（Cache）模拟器，核心实现 **LRU 替换策略**，用于理解 Cache 的工作机制、地址映射、命中/缺失统计以及时间局部性原理。

## 📖 项目背景

在计算机存储层次结构中，Cache 对 CPU 性能至关重要。本模拟器通过软件方式模拟 Cache 的行为：给定一个访问地址序列，模拟 Cache 的命中/缺失过程，并采用 **LRU（Least Recently Used）** 算法在缓存满时替换最久未使用的缓存块。

## ✨ 核心功能

- **Cache 访问模拟**：输入内存地址序列，模拟 Cache 的查找、命中、缺失及替换过程
- **命中/缺失统计**：自动计算总访问次数、命中次数、缺失次数及命中率
- **LRU 替换策略**：当 Cache 行满时，淘汰最长时间未被访问的缓存行
- **可配置参数**：支持自定义 Cache 容量、相联度、块大小等关键参数

## 🧠 LRU 算法原理

LRU 基于 **时间局部性** 原理：如果某个数据最近被访问过，那么它在不久的将来很可能再次被访问；反之，如果某个数据长时间未被访问，则未来被访问的概率较低。

### 实现方式（本模拟器采用）

- **计数器法**（或时间戳法）：为每个 Cache 行维护一个计数器（或“年龄”）
  - 每次访问某行时，将其计数器清零，其他所有有效行的计数器加 1
  - 替换时选择计数器值最大的行（即最久未使用）
- **链表法**（可选拓展）：将被访问的行移到链表头部，替换时淘汰链表尾部的行

### 示例
假设一个容量为 3 的 Cache，访问序列为 `A, B, C, D, A`：

| 访问 | Cache 状态（LRU顺序） | 命中/缺失 | 替换操作 |
|------|----------------|-----------|----------|
| A    | [A]            | 缺失      | 加载 A   |
| B    | [A, B]         | 缺失      | 加载 B   |
| C    | [A, B, C]      | 缺失      | 加载 C   |
| D    | [B, C, D]      | 缺失      | 淘汰 A，加载 D |
| A    | [C, D, A]      | 缺失      | 淘汰 B，加载 A |

## 🛠️ 技术实现

### 核心数据结构

```c
typedef struct {
    int valid;        // 有效位
    int tag;          // 标签位
    int last_used;    // 最近使用时间戳（LRU 计数器）
    // 可根据需要添加 dirty 位等
} CacheLine;

typedef struct {
    CacheLine *lines; // 缓存行数组
    int sets;         // 组数
    int ways;         // 相联度
    int block_size;   // 块大小（字节）
    int timestamp;    // 全局时间戳
} Cache;
```

### 关键函数

| 函数 | 描述 |
|------|------|
| `init_cache()` | 初始化 Cache 结构体，分配内存 |
| `access_cache()` | 模拟一次内存访问，返回命中/缺失 |
| `find_lru()` | 在指定组中找出需要替换的行索引 |
| `update_lru()` | 访问后更新所有行的 LRU 计数器 |
| `print_stats()` | 输出总访问数、命中数、缺失数和命中率 |

### 工作流程

```
输入地址 → 提取 Tag、Index、Offset → 定位到组 → 遍历组内行
  ↓
如果 Tag 匹配且有效 → 命中 → 更新 LRU 计数器
  ↓
如果未命中 → 若组内有空行 → 直接加载
          → 若组内已满 → 调用 LRU 替换策略 → 覆盖被淘汰行
  ↓
更新统计信息
```

## 🚀 快速开始

### 编译与运行

```bash
# 克隆仓库
git clone https://github.com/Celore-Chan/basic-cache-design.git
cd basic-cache-design

# 编译
gcc -o cache_simulator src/cache_simulator.c src/lru.c -lm

# 运行（使用内置测试序列）
./cache_simulator

# 或从文件读取访问地址
./cache_simulator access_sequences.txt
```

### 配置参数

在 `cache_simulator.h` 中可修改以下宏定义：

```c
#define CACHE_SIZE    (64 * 1024)  // 64KB 总容量
#define BLOCK_SIZE    64           // 64 字节/块
#define ASSOCIATIVITY 4            // 4 路组相联
#define NUM_SETS      (CACHE_SIZE / (BLOCK_SIZE * ASSOCIATIVITY))
```

### 示例输入输出

**输入**（内存访问地址序列，十六进制）：
```
0x1A4F
0x2B80
0x1A4F
0x3C12
0x2B80
```

**输出**：
```
=== LRU Cache Simulation Results ===
Total accesses : 5
Hits          : 2
Misses        : 3
Hit rate      : 40.00%
```

## 📂 项目结构

```
basic-cache-design/
├── src/
│   ├── cache_simulator.c   # 主程序，仿真流程控制
│   ├── lru.c               # LRU 算法具体实现
│   └── cache_simulator.h   # 数据结构与函数声明
├── tests/                  # 测试用例与地址序列文件
├── results/                # 仿真结果输出目录
├── README.md               # 项目文档
└── Makefile                # 编译脚本
```

## 📊 验证与测试

### 测试用例 1：时间局部性
重复访问同一地址序列 `A, A, A`：
- 预期：首次缺失，后续全部命中
- 验证 Cache 的命中率提升效果

### 测试用例 2：LRU 与 FIFO 对比
使用序列 `A, B, C, D, A`：
- **LRU**：缺失次数为 5（替换顺序 A→B→C→D→B）
- **FIFO**：缺失次数也可能为 5，但替换的块不同（A→B→C→D→A？需要仔细分析）
- 通过本模拟器可直观对比两种策略的性能差异

## 📈 学习目标

通过本项目可以深入理解：

- **Cache 基本结构**：有效位、标签位、数据块、偏移量
- **地址映射方式**：直接映射、组相联、全相联
- **替换策略**：LRU、FIFO、随机替换的区别与适用场景
- **局部性原理**：时间局部性与空间局部性对命中率的影响
- **性能评估**：命中率、缺失代价与平均访问时间（AMAT）的计算

## 🔧 扩展建议

可进一步扩展以下功能：

- [ ] **支持多种替换策略**：FIFO、LFU（Least Frequently Used）、Random，并提供对比模式
- [ ] **多级 Cache 模拟**：L1/L2 统一或分离 Cache（指令/数据）
- [ ] **写策略**：增加写直达（Write-Through）与写回（Write-Back）模拟
- [ ] **统计图形化**：使用 Python 脚本（matplotlib）绘制命中率曲线
- [ ] **集成真实 Trace**：支持读取 Valgrind 或 SPEC 产生的内存访问轨迹

## 📜 参考

- 《计算机组成与设计：硬件/软件接口》，Patterson & Hennessy
- Wikipedia: [Cache replacement policies](https://en.wikipedia.org/wiki/Cache_replacement_policies)

---

**作者**：[Celore-Chan](https://github.com/Celore-Chan)  
**许可**：MIT 协议  
**最后更新**：2026年6月
