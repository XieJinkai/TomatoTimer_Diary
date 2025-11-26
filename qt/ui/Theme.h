#pragma once
namespace Theme {
enum class Mode { Light, Dark };
void apply(Mode m);
void save(Mode m);
Mode load();
}
