#include "PaddleInferOcr.h"
#include <QFile>
#include <QDir>
#include <QCoreApplication>

#ifdef WITH_PADDLE_INFER
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include "paddle_inference_api.h"

static QString recDir(){ return QDir::fromNativeSeparators(QDir::cleanPath(QCoreApplication::applicationDirPath()+"/../include/paddle/models/ch_PP-OCRv4_rec_infer")); }

bool PaddleInferOcr::available(){ QDir d(recDir()); return d.exists(); }

QString PaddleInferOcr::recognize(const QString& imagePath){
    if(!available()) return QString();
    cv::Mat img = cv::imread(imagePath.toStdString()); if(img.empty()) return QString();
    int targetH = 32; int targetW = std::max(32, int(img.cols * (32.0 / std::max(1,img.rows))));
    cv::Mat resized; cv::resize(img, resized, cv::Size(targetW, targetH)); resized.convertTo(resized, CV_32FC3, 1.0/255.0);
    std::vector<float> input(3 * targetH * targetW);
    for(int c=0;c<3;++c){ for(int y=0;y<targetH;++y){ for(int x=0;x<targetW;++x){ input[c*targetH*targetW + y*targetW + x] = resized.at<cv::Vec3f>(y,x)[c]; } } }

    paddle::AnalysisConfig cfg; cfg.SetModel((recDir()+"/inference.pdmodel").toStdString(), (recDir()+"/inference.pdiparams").toStdString());
    cfg.DisableGpu(); cfg.SwitchUseFeedFetchOps(false);
    auto predictor = paddle::CreatePredictor(cfg);
    auto in_names = predictor->GetInputNames(); auto in = predictor->GetInputHandle(in_names[0]);
    in->Reshape({1,3,targetH,targetW}); in->CopyFromCpu(input.data());
    predictor->Run();
    auto out_names = predictor->GetOutputNames(); auto out = predictor->GetOutputHandle(out_names[0]);
    std::vector<float> prob; prob.resize(out->shape()[1]); out->CopyToCpu(prob.data());
    // 字典映射需要加载 keys 文件，这里返回占位文本以表明 C++ 推理已运行
    return QString("Paddle C++ Inference 已运行，字典映射未配置");
}
#else
bool PaddleInferOcr::available(){ return false; }
QString PaddleInferOcr::recognize(const QString&){ return QString(); }
#endif
