#pragma once
#include <QString>
class PaddleInferOcr {
public:
    static bool available();
    static QString recognize(const QString& imagePath);
};
