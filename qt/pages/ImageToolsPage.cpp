#include "ImageToolsPage.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QFileDialog>
#include <QImage>
#include <QPixmap>

#ifdef WITH_OPENCV
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#endif

ImageToolsPage::ImageToolsPage(QWidget* parent): QWidget(parent){ setupUi(); }

void ImageToolsPage::setupUi(){
    auto* lay = new QVBoxLayout(this);
    info_ = new QLabel("打开图片以预览。若安装 OpenCV 将启用增强/降噪。", this);
    btnOpen_ = new QPushButton("打开图片", this);
    btnEnhance_ = new QPushButton("增强/降噪", this);
    lay->addWidget(info_); lay->addWidget(btnOpen_); lay->addWidget(btnEnhance_); lay->addStretch();
    connect(btnOpen_, &QPushButton::clicked, this, &ImageToolsPage::openImage);
    connect(btnEnhance_, &QPushButton::clicked, this, &ImageToolsPage::enhance);
}

void ImageToolsPage::openImage(){
    currentPath_ = QFileDialog::getOpenFileName(this, "选择图片", QString(), "Images (*.png *.jpg *.jpeg)");
    if(currentPath_.isEmpty()) return;
    QImage img(currentPath_);
    info_->setPixmap(QPixmap::fromImage(img).scaled(480, 360, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void ImageToolsPage::enhance(){
#ifdef WITH_OPENCV
    if(currentPath_.isEmpty()) return;
    cv::Mat src = cv::imread(currentPath_.toStdString(), cv::IMREAD_GRAYSCALE);
    if(src.empty()) return;
    cv::Mat denoise; cv::medianBlur(src, denoise, 3);
    cv::Mat enhanced; cv::equalizeHist(denoise, enhanced);
    std::vector<unsigned char> buf; cv::imencode(".png", enhanced, buf);
    QImage img(buf.data(), (int)buf.size(), 1, QImage::Format_Grayscale8);
    info_->setPixmap(QPixmap::fromImage(QImage::fromData(buf.data(), (int)buf.size())).scaled(480,360,Qt::KeepAspectRatio,Qt::SmoothTransformation));
#else
    info_->setText("未检测到 OpenCV，当前使用占位。安装 OpenCV 后启用增强。");
#endif
}