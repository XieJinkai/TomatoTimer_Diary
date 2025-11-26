#pragma once
#include <QString>
class PaddleInferOcr {
public:
    // 是否已配置 Paddle C++ 推理模型
    static bool available();
    // 使用 Paddle 推理识别图片文字
    static QString recognize(const QString& imagePath);
};
