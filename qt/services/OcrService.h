#pragma once
#include <QString>
#include <functional>
class QObject;
class OcrService {
public:
    // 是否可用（Paddle/Tesseract）
    static bool available();
    // 对指定图片执行文字识别
    static QString recognize(const QString& imagePath);
    // 是否具备 PaddleOCR 能力
    static bool hasPaddle();
    static bool hasBaiduWebApi();
    static void recognizeAsync(const QString& imagePath, QObject* context, std::function<void(const QString& text)> onDone);
};
