这是我学习 [mit 6.S081](https://pdos.csail.mit.edu/6.828/2021/schedule.html) (原 mit 6.828 课程)的 lab 代码。这十个实验的相关笔记在以下博客地址，课程的学习笔记也在我的博客中。

实验名 | 介绍 | 笔记地址 | 代码分支
--- | --- | --- | ---
Lab: Xv6 and Unix utilities | 是熟悉xv6的代码与Unix系统调用流程 | [lab1](https://qifanwang.github.io/labs/2022/07/03/mit-6s081-lab1/) | [util](https://github.com/QifanWang/learning_6s081/tree/util)
Lab: System calls | 为xv6内核添加两个系统调用，负责追踪系统调用信息与收集系统信息 | [lab2](https://qifanwang.github.io/labs/2022/07/09/mit-6s081-lab2/) | [syscall](https://github.com/QifanWang/learning_6s081/tree/syscall)
Lab: page tables | 跟踪xv6内核如何操作用户进程页表，并添加页表相关功能代码 | [lab3](https://qifanwang.github.io/labs/2022/07/23/mit-6s081-lab3/) | [pgtbl](https://github.com/QifanWang/learning_6s081/tree/pgtbl)
Lab: traps | 学习内核如何 Trap 阅读汇编代码回答问题，为 xv6 添加 backtrace 与 支持计时器中断的用户 handler 的系统调用 | [lab4](https://qifanwang.github.io/labs/2022/07/30/mit-6s081-lab4/) | [traps](https://github.com/QifanWang/learning_6s081/tree/traps)
Lab 5: Copy-on-Write Fork for xv6 | 为 xv6 添加 Copy-on-Write 的 fork 机制 | [lab5](https://qifanwang.github.io/os/2022/09/03/mit-6s081-lab5/) | [cow](https://github.com/QifanWang/learning_6s081/tree/cow)
Lab 6: Multithreading | 为 xv6 添加用户级线程与使用 pthread 库实现线程安全的hash table 与 barrier | [lab6](https://qifanwang.github.io/os/2022/11/07/mit-6s081-lab6/) | [thread](https://github.com/QifanWang/learning_6s081/tree/thread)
Lab 7: Network driver | 为 xv6 编写网卡设备驱动程序 | [lab7](https://qifanwang.github.io/os/2023/03/01/mit-6s081-lab7/) | [net](https://github.com/QifanWang/learning_6s081/tree/net)
Lab 8: Locks | 为 xv6 的堆与buf缓冲区提供细粒度锁机制，减少 lock contention 以提高性能 | [lab8](https://qifanwang.github.io/os/2023/03/12/mit-6s081-lab8/) | [lock](https://github.com/QifanWang/learning_6s081/tree/lock)
Lab 9: File System | 为 xv6 文件系统添加大文件读写与符号链接 | [lab9](https://qifanwang.github.io/os/2023/03/14/mit-6s081-lab9/) | [fs](https://github.com/QifanWang/learning_6s081/tree/fs)
Lab 10: mmap | 为 xv6 文件系统添加添加mmap和munmap，重点关注内存映射文件 | [lab10](https://qifanwang.github.io/os/2023/03/18/mit-6s081-lab10/) | [mmap](https://github.com/QifanWang/learning_6s081/tree/mmap)
