# Injector - Linux 动态库注入工具

`injector` 是一个用于将调试用 `.so` 文件注入到运行中的 Linux 进程中的小型工具。它适用于如下场景：

- **热补丁（Hotpatch）**：无需重启目标程序，通过注入 `.so` 动态库，替换或修复有问题的函数。
- **运行时调试**：注入带有日志或调试逻辑的动态库，实时分析程序行为，便于问题定位。

---

## ✨ 功能特性

- 基于 `ptrace` 实现的安全注入，不依赖目标程序特定实现。
- 可注入任意自定义 `.so` 文件，只要目标函数在符号表中存在。
- 兼容主流 `glibc` 系统，支持 x86_64 Linux。

---

## ⚙️ 使用要求

- 被注入的目标进程必须是当前用户可调试的（或具有 root 权限）。
- 目标进程的可执行文件应未开启 `ptrace_scope` 限制，或你已手动关闭：

```bash
echo 0 | sudo tee /proc/sys/kernel/yama/ptrace_scope

使用方法

```bash
./injector <pid> <full-path-to-hook.so>

