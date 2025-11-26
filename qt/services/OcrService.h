#pragma once
#include <QString>
class OcrService {
public:
    static bool available();
    static QString recognize(const QString& imagePath);
    static bool hasPaddle();
};
