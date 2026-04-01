# PromptLab

PromptLab 是一个基于 Qt Widgets 和 SQLite 的本地桌面应用，用于管理、整理和评估提示词（Prompt）。它面向高频使用 AI 工具的用户，提供提示词的结构化存储、分类与标签管理、搜索筛选、评分记录和模型效果对比等能力。

项目当前以“本地优先、轻量可用”为目标，适合作为个人提示词资产库或小型提示词实验室使用。

## 项目简介

随着大模型工具的普及，提示词逐渐成为重要的生产资料。PromptLab 希望解决以下问题：

- 提示词分散在文档、聊天记录和笔记软件中，难以统一管理
- 缺少分类、标签、评分等结构化整理能力
- 难以记录同一提示词在不同模型下的表现
- 缺乏一个适合长期沉淀和复用的本地工具

当前版本是一个单机桌面应用，数据默认保存在程序目录下的 `data/promptlab.db` 中。

## 核心功能

当前代码中已实现的主要功能包括：

- 提示词管理：新建、编辑、删除、查看、复制提示词
- 分类管理：创建、修改、删除分类
- 标签管理：创建、修改、删除标签
- 搜索与筛选：支持关键词搜索、按分类筛选、按标签筛选
- 提示词评分：对提示词整体进行星级评分
- 模型评分：记录不同模型下的评分与备注
- 仪表盘：查看提示词总数、分类数、标签数、收藏数、平均评分
- 本地数据存储：使用 SQLite 保存全部数据
- 基础界面样式：支持 QSS 样式资源加载

## 界面说明

PromptLab 是一个典型的桌面双栏工作区界面：

- 左侧区域：搜索框、分类树、标签列表，以及分类/标签管理入口
- 右侧区域：
  - 提示词列表视图：展示当前筛选结果
  - 提示词详情视图：展示标题、元信息、正文内容、描述、评分与模型评分
  - 仪表盘视图：展示项目统计信息

## 技术栈

- 语言：C++17
- UI 框架：Qt 6
- 界面技术：Qt Widgets
- 数据存储：SQLite
- 构建工具：CMake
- 资源管理：Qt Resource System (`.qrc`)
- 样式：QSS

## 安装方式

### 1. 环境要求

- CMake 3.19 或更高
- Qt 6.5 或更高
- 支持 C++17 的编译器

根据 `CMakeLists.txt`，当前项目依赖以下 Qt 模块：

- `Qt::Core`
- `Qt::Widgets`
- `Qt::Sql`

### 2. 克隆项目

```bash
git clone <your-repo-url>
cd PromptLab
```

### 3. 配置与构建

```bash
cmake -S . -B build
cmake --build build --config Release
```

### 4. 运行

构建完成后运行生成的 `PromptLab` 可执行文件即可。

首次启动时程序会自动：

- 创建 `data` 目录
- 初始化 SQLite 数据库
- 创建所需数据表、索引和触发器
- 写入默认模型数据和基础设置

## 使用方法

### 提示词管理

1. 启动应用后，点击工具栏或菜单中的“新建”
2. 输入标题、内容、描述、分类、标签和评分
3. 保存后即可在列表中查看

### 搜索与筛选

- 在左侧搜索框中输入关键词，按标题或内容搜索
- 点击左侧分类可按分类筛选
- 点击左侧标签可按标签筛选

### 查看与复制

- 在列表中选择一条提示词
- 右侧详情区会显示正文、说明和评分信息
- 点击“复制”可将提示词内容复制到剪贴板

### 模型评分

- 在详情区中可以为当前提示词添加模型评分
- 支持记录模型、评分和备注
- 可对已有评分进行编辑和删除

### 仪表盘

- 通过菜单或工具栏切换到“仪表盘”
- 查看当前库中的提示词数量、分类数量、标签数量、收藏数量和平均评分

## 项目结构说明

```text
PromptLab/
├── CMakeLists.txt
├── src/
│   ├── main.cpp
│   ├── model/          # 数据模型，如 Prompt、Category、Tag、ModelScore
│   ├── database/       # 数据库初始化、建表、升级、索引、触发器
│   ├── repository/     # 数据访问层，负责 SQL 查询与持久化
│   ├── service/        # 业务服务层，连接 UI 与 Repository
│   ├── ui/             # Qt Widgets 界面层
│   └── utils/          # 工具类，如翻译管理
├── resources/
    ├── resources.qrc
    └── styles/
        └── style.qss

```

### 主要模块说明

- `src/ui/main_window.*`
  - 主窗口，负责组织整体界面、菜单、工具栏和主工作区
- `src/ui/prompt_list_widget.*`
  - 提示词列表与筛选结果展示
- `src/ui/prompt_detail_widget.*`
  - 提示词详情、复制、评分和模型评分管理
- `src/ui/prompt_editor_dialog.*`
  - 提示词新建与编辑对话框
- `src/ui/category_manager_dialog.*`
  - 分类管理界面
- `src/ui/tag_manager_dialog.*`
  - 标签管理界面
- `src/ui/dashboard_widget.*`
  - 统计面板
- `src/service/*`
  - 业务服务层，负责对上层 UI 提供统一调用入口
- `src/repository/*`
  - 数据访问层，封装数据库 CRUD、搜索、筛选和统计逻辑
- `src/database/database_manager.*`
  - 数据库初始化、表结构维护与默认数据写入

## 架构说明

当前项目采用分层架构，整体数据流如下：

```text
UI（Qt Widgets）
  -> Service
  -> Repository
  -> SQLite
```

特点：

- UI 层负责交互与展示
- Service 层负责业务协调与信号通知
- Repository 层负责 SQL 和数据映射
- Database 层负责数据库生命周期管理

## 当前状态与已知边界

以下内容在数据库或设计文档中已有痕迹，但当前代码中尚未完整实现为用户可用功能：

- 提示词版本管理
- 示例（Examples）管理
- 导入 / 导出能力
- 更完整的排序与高级筛选能力
- 更完整的模型管理界面

因此 README 仅描述当前代码中已落地的功能。

## 未来规划

可作为后续迭代方向的内容包括：

- 版本管理与历史回滚
- 导入 / 导出（JSON、Markdown 等）
- 更完整的筛选与排序能力
- 模型管理界面完善
- 更丰富的提示词评估维度
- 测试覆盖与工程化提升
- 多语言与国际化完善

## 开发说明

如果你准备继续开发这个项目，建议优先关注以下文件：

- `src/main.cpp`
- `src/ui/main_window.cpp`
- `src/ui/prompt_detail_widget.cpp`
- `src/repository/prompt_repository.cpp`
- `src/repository/model_score_repository.cpp`
- `src/database/database_manager.cpp`

## 开源协议
  MIT License

Copyright (c) 2026 walkeroliver

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.


