# 薪智汇控 - 智能薪酬管理系统 (SmartSalary)

## 项目介绍
本项目为**软件工程团队作业**。项目旨在通过现代软件工程的设计模式，构建一个高可靠性、智能化、可视化的智能薪金与考勤管理平台。

---
## 团队成员
- 组长：赵思琪
- 组员：陈俊儒 严巩 李子森 黄明涛 王静琪


## 运行环境与依赖要求
* **开发框架**：Qt 5.14.1 (或兼容版本)
* **编译器**：MinGW 64-bit / MSVC
* **数据库**：MySQL 5.7+ / 本地 SQLite 3
* **API 服务**：阿里云百炼平台（DashScope API）

---

## 配置文件 `config.ini` 说明
项目启动时会读取根目录下的 `config.ini` 文件，您需要先创建该文件，并根据实际环境配置以下参数：

```ini
[Database]
; 数据库驱动类型：QMYSQL 或 QSQLITE
Driver=QSQLITE
HostName=localhost
Port=3306
DatabaseName=salary_system
UserName=root
Password=root
; 本地 SQLite 数据库文件名
SqliteDbName=salary_system.db

; 阿里云百炼 API Key
ApiKey=sk-xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
; 模型名称，默认推荐 qwen3.7-max
ModelName=qwen3.7-max
; 是否启用深度思考
EnableThinking=true
```

---

## 编译与构建步骤
1. 打开命令行，进入项目根目录：
   ```bash
   cd SmartSalary
   ```
2. 执行 qmake 生成 Makefile：
   ```bash
   qmake SmartSalary.pro
   ```
3. 执行编译（Windows 环境下使用 mingw32-make）：
   ```bash
   mingw32-make
   ```
4. 运行生成的可执行程序：
   ```bash
   ./release/SmartSalary.exe
   ```

---

