#pragma once
#include <QString>
class OcrService {
public:
    // 是否可用（Paddle/Tesseract）
    static bool available();
    // 对指定图片执行文字识别
    static QString recognize(const QString& imagePath);
    // 是否具备 PaddleOCR 能力
    static bool hasPaddle();
};
