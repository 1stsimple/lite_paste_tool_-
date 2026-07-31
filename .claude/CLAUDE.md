# 项目名称

**LitePaste（暂定）——高性能轻量化剪贴板与多模式粘贴工具**

## 项目描述

LitePaste 是一款使用 **C++11 + Qt 5.14** 开发的桌面剪贴板应用，面向编程、文本编辑和批量录入场景，提供剪贴板历史管理、纯文本粘贴、整行替换、顺序粘贴和矩形粘贴等功能。

项目优先保证启动速度、低空闲占用、粘贴响应速度和键盘操作效率。首个版本以 Windows 10/11 为主要运行平台，同时通过平台抽象层为后续适配 Linux 预留接口。

## 核心目标

- 使用原生 C++ 与 Qt Widgets 构建轻量桌面应用，不引入 Web 运行时和不必要的大型依赖。
- 在后台持续监听文本剪贴板，并以较低的 CPU 与内存开销维护历史记录。
- 支持全局快捷键，在不切换当前窗口的情况下快速调用不同粘贴模式。
- 对 VS Code、Visual Studio、记事本和常见文本编辑器提供良好的兼容性。
- 将剪贴板采集、文本转换、粘贴执行、历史存储和界面显示拆分为独立模块，便于持续优化。

## 核心功能

### 1. 剪贴板历史

- 自动记录复制过的文本内容。
- 支持搜索、预览、置顶、删除和清空历史记录。
- 自动合并连续出现的相同内容，避免重复记录。
- 支持设置最大记录数量、单条文本大小和保留时间。
- 支持临时暂停记录，避免保存密码、验证码等敏感内容。

### 2. 普通粘贴

- 将选中的历史记录写入系统剪贴板，并发送标准粘贴指令。
- 保留原始换行和 Unicode 文本内容。
- 粘贴完成后可按配置恢复用户原来的剪贴板内容。

### 3. 纯文本粘贴

- 去除 HTML、富文本和其他格式，只保留可见文本。
- 统一 Windows、Linux 和 macOS 风格的换行符。
- 可选清理行尾空格、连续空行和不可见控制字符。

### 4. 单行整行替换

用于将一段单行文本快速替换当前编辑器中的整行内容。

推荐执行流程：

1. 判断待粘贴内容是否为单行文本。
2. 向当前窗口发送“选中当前行”的按键序列。
3. 用目标文本替换整行内容。
4. 根据设置决定是否保留当前行缩进和行尾换行。

该模式需要针对不同编辑器维护快捷键策略，不能假定所有软件的“选中当前行”快捷键完全一致。

### 5. 矩形粘贴 / 列粘贴

重点支持 VS Code 中通过 **Shift + Alt + 鼠标左键拖动** 创建的矩形选区或多光标选区。

处理流程：

1. 将待粘贴文本按行拆分，并统一换行符。
2. 检测空行、行数和每行长度。
3. 根据矩形选区的多光标行为整理待粘贴数据。
4. 将每一行文本对应到矩形选区中的一行或一个光标位置。
5. 当文本行数与选区行数不一致时，根据配置选择循环填充、空行补齐、截断或取消操作。

需要明确：普通剪贴板接口无法直接获取 VS Code 矩形选区的完整几何信息。首版通过“文本预处理 + VS Code 多光标机制 + 按键模拟”实现兼容；若需要精确读取选区行数、列位置和编辑器状态，应增加一个可选的 VS Code 扩展，与桌面程序通过本地 IPC 通信。

### 6. 顺序粘贴

- 将多条剪贴板记录组成待粘贴队列。
- 每次触发快捷键时自动粘贴下一条内容。
- 支持循环、撤销上一步和重置队列。
- 适用于表单录入、代码片段填充和批量文本处理。

### 7. 系统托盘与快捷键

- 后台运行时默认驻留系统托盘。
- 支持显示或隐藏主窗口、打开历史面板和暂停监听。
- 每种粘贴模式可配置独立的全局快捷键。
- 检测快捷键冲突，并在注册失败时给出明确提示。

## 性能目标

以下内容属于开发目标，必须通过实际测试验证，不能直接视为已经达成：

- 空闲状态 CPU 占用接近 0，不使用高频轮询读取剪贴板。
- 普通短文本从触发快捷键到发出粘贴指令的内部处理时间尽量控制在 50 ms 内。
- 冷启动时间尽量控制在 1 秒内。
- 空闲内存占用尽量控制在 60 MB 内。
- 默认支持至少 1,000 条文本历史记录，并保持搜索和列表滚动流畅。
- 对大文本设置尺寸上限，避免单次复制导致界面卡顿或内存突增。

## 技术栈

- 开发语言：C++11
- GUI 框架：Qt 5.14 Widgets
- 构建系统：CMake
- 剪贴板接口：QClipboard、QMimeData
- 界面组件：QMainWindow、QListView、QAbstractListModel、QSystemTrayIcon
- 配置与轻量持久化：QSettings、JSON 或二进制文件
- Windows 平台能力：Win32 API、全局快捷键、窗口识别和按键发送
- 并发机制：Qt 信号槽、QThread 或 C++11 标准线程
- 单元测试：Qt Test
- 打包部署：windeployqt

说明：Qt 6 不适合作为本项目的默认版本，因为项目要求以 C++11 为基础；首版固定使用 Qt 5.14，避免语言标准和框架版本发生冲突。

## 总体架构

```text
系统剪贴板
    │
    ▼
ClipboardMonitor
    │  采集、去重、过滤
    ▼
ClipboardRepository ─────► HistoryStorage
    │                         持久化、容量限制
    │
    ├──────────────► ClipboardHistoryModel ─────► Qt Widgets UI
    │
    ▼
PasteController
    │
    ├── NormalPasteStrategy
    ├── PlainTextPasteStrategy
    ├── WholeLinePasteStrategy
    ├── RectanglePasteStrategy
    └── SequentialPasteStrategy
    │
    ▼
PlatformAdapter
    │  全局快捷键、活动窗口、按键模拟
    ▼
目标应用程序
```

## 项目结构

```text
LitePaste/
├── CMakeLists.txt
├── README.md
├── docs/
│   ├── architecture.md          # 架构和模块边界
│   ├── paste-modes.md           # 各粘贴模式的行为定义
│   └── performance.md           # 性能指标与测试结果
├── resources/
│   ├── icons/
│   └── litepaste.qrc
├── src/
│   ├── main.cpp
│   ├── app/
│   │   ├── Application.h
│   │   └── Application.cpp
│   ├── clipboard/
│   │   ├── ClipboardItem.h
│   │   ├── ClipboardMonitor.h
│   │   ├── ClipboardMonitor.cpp
│   │   ├── ClipboardRepository.h
│   │   └── ClipboardRepository.cpp
│   ├── paste/
│   │   ├── PasteController.h
│   │   ├── PasteController.cpp
│   │   ├── PasteStrategy.h
│   │   ├── NormalPasteStrategy.cpp
│   │   ├── PlainTextPasteStrategy.cpp
│   │   ├── WholeLinePasteStrategy.cpp
│   │   ├── RectanglePasteStrategy.cpp
│   │   └── SequentialPasteStrategy.cpp
│   ├── platform/
│   │   ├── PlatformAdapter.h
│   │   └── windows/
│   │       ├── WindowsPlatformAdapter.h
│   │       └── WindowsPlatformAdapter.cpp
│   ├── storage/
│   │   ├── HistoryStorage.h
│   │   └── FileHistoryStorage.cpp
│   ├── ui/
│   │   ├── MainWindow.h
│   │   ├── MainWindow.cpp
│   │   ├── ClipboardHistoryModel.h
│   │   ├── ClipboardHistoryModel.cpp
│   │   └── SettingsDialog.cpp
│   └── utils/
│       ├── TextNormalizer.h
│       └── TextNormalizer.cpp
└── tests/
    ├── test_text_normalizer.cpp
    ├── test_clipboard_repository.cpp
    └── test_rectangle_paste.cpp
```

## 关键数据结构

```cpp
struct ClipboardItem
{
    std::string id;
    QString text;
    QDateTime createdAt;
    bool pinned;
};

enum class PasteMode
{
    Normal,
    PlainText,
    WholeLine,
    Rectangle,
    Sequential
};
```

数据结构应保持简单。首版只处理文本内容，不要在尚未需要时加入图片、文件、HTML 编辑和云同步等功能。

## 编码规范

- 所有代码必须兼容 C++11，不使用 C++14、C++17 或更高版本语法。
- 类名使用 PascalCase，函数和变量使用 camelCase，常量使用清晰统一的命名方式。
- 头文件使用 `#pragma once` 或统一的 include guard，项目内保持一致。
- 资源所有权优先使用 RAII 和智能指针；QObject 对象优先使用 Qt 父子对象机制管理生命周期。
- 不在 UI 线程执行大文本解析、磁盘批量写入或其他可能阻塞界面的操作。
- 模块之间通过接口、信号槽或明确的数据结构通信，避免 UI 直接操作平台 API。
- 平台相关代码必须放入 `platform/` 目录，不能散落在业务逻辑中。
- 粘贴模式使用策略模式实现，避免在一个函数中堆积大量条件分支。
- 每个公开类和复杂算法必须说明职责、输入、输出、边界条件和失败行为。
- 所有文本处理必须考虑 UTF-8、中文、Emoji、CRLF/LF 换行差异和空文本。
- 不允许吞掉错误；全局快捷键注册失败、剪贴板访问失败和按键发送失败必须可追踪。

## 线程与性能约束

- 剪贴板变化优先使用系统事件通知，不使用固定间隔高频轮询。
- UI 线程只负责界面更新和轻量调度。
- 历史记录写入采用延迟合并或批量写入，避免每次复制都同步刷新整个文件。
- 历史列表采用 Model/View，不为每条记录创建复杂 QWidget。
- 搜索采用延迟触发，避免用户每输入一个字符就执行高成本全量操作。
- 对超大文本进行截断预览，完整内容按需加载。
- 矩形粘贴前必须先在内存中完成文本拆分和规则校验，再执行系统按键操作。

## 首版范围

首版必须完成：

- Windows 文本剪贴板监听。
- 剪贴板历史列表、搜索、置顶和删除。
- 普通粘贴与纯文本粘贴。
- 单行整行替换。
- VS Code 矩形选区兼容模式。
- 顺序粘贴队列。
- 系统托盘和可配置全局快捷键。
- 基础配置持久化。
- 关键文本转换逻辑的单元测试。

首版暂不实现：

- 图片和文件剪贴板历史。
- 账号系统、云同步和局域网同步。
- OCR、翻译和 AI 文本处理。
- 跨设备共享。
- 复杂富文本编辑器。
- 插件市场。

## 开发阶段

### 阶段一：基础框架

- 初始化 CMake 与 Qt Widgets 工程。
- 创建主窗口、托盘菜单和配置模块。
- 完成 ClipboardMonitor 与 ClipboardRepository。

### 阶段二：基础粘贴

- 实现普通粘贴和纯文本粘贴。
- 实现 Windows 全局快捷键注册。
- 实现活动窗口识别与按键发送。

### 阶段三：高级粘贴

- 实现单行整行替换。
- 实现顺序粘贴队列。
- 实现矩形文本拆分、行数匹配和异常处理。
- 对 VS Code 不同选区和多光标场景进行兼容性测试。

### 阶段四：性能与稳定性

- 增加容量限制、大文本保护和延迟持久化。
- 测量启动时间、空闲内存、空闲 CPU 和粘贴延迟。
- 修复剪贴板重复触发、快捷键冲突和焦点切换问题。

### 阶段五：可选扩展

- 评估 VS Code 扩展与桌面程序的本地 IPC 协作方案。
- 抽象 Linux 平台适配接口。
- 增加导入、导出和备份功能。

## 当前开发状态

- 当前处于需求整理和架构设计阶段。
- 技术路线确定为 C++11、Qt 5.14 Widgets 和 CMake。
- 首个目标平台确定为 Windows 10/11。
- 矩形粘贴的行为规则已经初步定义，仍需通过原型验证 VS Code 的多光标粘贴表现。
- 项目代码、性能测试和兼容性测试尚未开始，不得在文档中描述为已经完成。

## 验收标准

- 连续复制相同文本时，历史记录不会无限重复增加。
- 关闭并重新启动应用后，历史记录和设置能够正确恢复。
- 普通粘贴和纯文本粘贴在 VS Code、Visual Studio 和记事本中可正常使用。
- 单行整行替换不会误删相邻行，并能正确处理文件末尾没有换行符的情况。
- 矩形粘贴能够正确处理等行数、少行、多行、空行和中英文混合文本。
- 全局快捷键冲突时应用不会崩溃，并能提示用户重新配置。
- 复制超大文本时应用不会长时间冻结或无限增长内存。
- 退出应用后不残留全局钩子、后台线程或未释放的系统资源。

## 注意事项

- 不要把“高性能”和“轻量化”只写成宣传语，必须用启动时间、内存、CPU 和延迟数据验证。
- Qt 本身存在基础运行时开销，因此性能目标要以实测为准，不应预先宣称达到极低内存占用。
- VS Code 矩形选区是编辑器能力，桌面剪贴板程序只能进行兼容和协作，不能无条件获得编辑器内部选区信息。
- 模拟按键前必须保存当前活动窗口，并防止粘贴面板抢占焦点后把内容粘贴到自身。
- 应防止程序写回剪贴板时再次触发监听，从而形成重复记录或循环处理。
- 剪贴板历史可能包含隐私信息，必须提供暂停记录、清空记录和进程黑名单能力。
- 所有新功能先创建独立 Git 分支，再进行开发、测试和合并。