#include "OcrService.h"
#include <QFile>
#include <QDir>
#include <QProcess>
#include <QCoreApplication>
#include <QStandardPaths>
#include "PaddleInferOcr.h"

static QString guessExe(){ QString base = QDir::fromNativeSeparators(QDir::cleanPath(QCoreApplication::applicationDirPath()+"/../include/bin")); QString exe = base+"/tesseract.exe"; if(QFile::exists(exe)) return exe; return QString(); }
static QString guessPaddleScript(){ QString base = QDir::fromNativeSeparators(QDir::cleanPath(QCoreApplication::applicationDirPath()+"/../include/bin")); QString py = base+"/ocr_paddle.py"; if(QFile::exists(py)) return py; return QString(); }

bool OcrService::available(){ return PaddleInferOcr::available() || !guessPaddleScript().isEmpty() || !guessExe().isEmpty(); }
bool OcrService::hasPaddle(){ return PaddleInferOcr::available() || !guessPaddleScript().isEmpty(); }

QString OcrService::recognize(const QString& imagePath){
    if(PaddleInferOcr::available()){
        return PaddleInferOcr::recognize(imagePath);
    }
    QString script = guessPaddleScript();
    if(!script.isEmpty()){
        QProcess p; QStringList args; args << script << imagePath; p.start("python", args); p.waitForFinished(30000); QByteArray out = p.readAllStandardOutput(); if(out.isEmpty()) out = p.readAllStandardError(); return QString::fromUtf8(out);
    }
    QString exe = guessExe(); if(exe.isEmpty()) return QString(); QProcess p; QStringList args; args << imagePath << "stdout"; p.start(exe, args); p.waitForFinished(15000); return QString::fromUtf8(p.readAllStandardOutput());
}
