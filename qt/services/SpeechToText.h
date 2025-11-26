#pragma once
#include <QString>

class SpeechToText {
public:
    // 进行一次语音识别并返回文本（Windows SAPI）
    static QString recognizeOnce();
};
