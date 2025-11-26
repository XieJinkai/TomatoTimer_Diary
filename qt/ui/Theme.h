#pragma once
namespace Theme {
enum class Mode { Light, Dark };
// 应用主题到全局
void apply(Mode m);
// 保存主题首选项
void save(Mode m);
// 读取主题首选项
Mode load();
}
