# 双模式登录与 SQLite 本地 API 服务实现计划

> **给后续执行者的说明：** 本计划用于实现“本地免注册登录”和“SQLite 账号注册登录”双模式。执行时按任务逐项推进，每个任务完成后运行对应验证命令。

**目标：** 让用户既可以不注册直接进入本地账号，也可以通过注册登录功能使用 SQLite 账号体系，并让注册账号的数据能够同步到本地 API 服务端。

**架构：** 客户端登录页提供本地模式、账号登录和账号注册三个入口。本地模式沿用原有 `DataStore::userDir(username)` 文件链路；账号模式通过 HTTP/JSON 调用 C++ 本地 API 服务，服务端使用 SQLite 保存用户账号、选填资料和同步文件。

**技术栈：** Qt Widgets、Qt Network、Qt SQL、SQLite、CMake、Qt Test。

---

### 任务 1：会话模型扩展

**文件：**
- 修改：`qt/services/Session.h`
- 修改：`qt/services/Session.cpp`

- [ ] 增加 `LoginMode`，区分本地模式与账号模式。
- [ ] 增加用户 ID、姓名、学校、专业等远程账号资料字段。
- [ ] 保留 `username()` 接口，确保原有本地存储链路继续可用。

### 任务 2：登录注册页面改造

**文件：**
- 修改：`qt/pages/LoginRegisterPage.h`
- 修改：`qt/pages/LoginRegisterPage.cpp`

- [ ] 使用 `QTabWidget` 拆分“本地模式”“账号登录”“账号注册”。
- [ ] 本地模式提供“进入本地模式”按钮，内部用户名固定为 `local_user`。
- [ ] 注册必填字段仅为用户名、密码、确认密码。
- [ ] 姓名、性别、年龄、手机号、邮箱、学校、专业作为选填字段提交。

### 任务 3：C++ 本地 API 服务

**文件：**
- 新增：`qt/server/LocalApiServer.cpp`
- 修改：`CMakeLists.txt`

- [ ] 新增独立可执行目标 `local_api_server`。
- [ ] 使用 `QTcpServer` 提供轻量 HTTP/JSON 接口。
- [ ] 使用 SQLite 初始化 `users` 和 `cloud_files` 表。
- [ ] 实现 `/health`、`/api/register`、`/api/login`、文件上传、文件清单、文件下载接口。

### 任务 4：客户端账号 API 与同步约束

**文件：**
- 新增或修改：`qt/services/CloudSyncService.h`
- 新增或修改：`qt/services/CloudSyncService.cpp`
- 修改：`qt/pages/SettingsSyncPage.cpp`

- [ ] 账号登录和注册通过本地 API 服务完成。
- [ ] 本地模式下点击同步时提示需要注册或登录账号。
- [ ] 账号模式下按用户 ID 将本地用户目录文件上传到服务端 SQLite。

### 任务 5：测试与验证

- [ ] 更新登录流程测试，覆盖本地模式入口。
- [ ] 新增或更新同步服务测试，覆盖用户 ID 上传路径。
- [ ] 运行 `cmake --build build --config Debug --target tomato_timer`。
- [ ] 运行 `ctest --test-dir build -C Debug --output-on-failure`。
