#include "ImageToolsPage.h"
#include "../ui/ImageView.h"
#include "../services/OcrService.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QFileDialog>
#include <QImage>
#include <QPixmap>
#include <QStandardPaths>
#include <QDir>

#ifdef WITH_OPENCV
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#endif

ImageToolsPage::ImageToolsPage(QWidget* parent): QWidget(parent){ setupUi(); }

void ImageToolsPage::setupUi(){
    auto* lay = new QVBoxLayout(this);
    info_ = new QLabel("打开图片以预览。选中四个点（顺时针）可进行透视调整。", this);
    view_ = new ImageView(this);
    btnOpen_ = new QPushButton("打开图片", this);
    btnAdjust_ = new QPushButton("调整图片", this);
    btnOcr_ = new QPushButton("文字识别", this);
    btnSave_ = new QPushButton("保存图片", this);
    ocrOut_ = new QLabel(this); ocrOut_->setWordWrap(true);
    auto* row = new QHBoxLayout();
    row->addWidget(btnOpen_); row->addWidget(btnAdjust_); row->addWidget(btnOcr_); row->addWidget(btnSave_);
    lay->addWidget(info_); lay->addWidget(view_); lay->addLayout(row); lay->addWidget(ocrOut_); lay->addStretch();
    connect(btnOpen_, &QPushButton::clicked, this, &ImageToolsPage::openImage);
    connect(btnAdjust_, &QPushButton::clicked, this, &ImageToolsPage::adjustPerspective);
    connect(btnOcr_, &QPushButton::clicked, this, &ImageToolsPage::recognizeText);
    connect(btnSave_, &QPushButton::clicked, this, &ImageToolsPage::saveImage);
}

void ImageToolsPage::openImage(){
    currentPath_ = QFileDialog::getOpenFileName(this, "选择图片", QString(), "Images (*.png *.jpg *.jpeg)");
    if(currentPath_.isEmpty()) return;
    QImage img(currentPath_);
    currentImage_ = img;
    view_->setImage(img);
    view_->clearPoints();
    ocrOut_->setText(QString());
}

 

void ImageToolsPage::adjustPerspective(){
#ifdef WITH_OPENCV
    if(currentImage_.isNull()) return;
    auto pts = view_->points(); if(pts.size()!=4) return;
    QImage imgRgb = currentImage_.convertToFormat(QImage::Format_RGB888);
    cv::Mat m(imgRgb.height(), imgRgb.width(), CV_8UC3, const_cast<uchar*>(imgRgb.bits()), imgRgb.bytesPerLine());
    cv::Mat src; cv::cvtColor(m, src, cv::COLOR_RGB2BGR);
    cv::Point2f s[4]; for(int i=0;i<4;++i){ s[i] = cv::Point2f((float)pts[i].x(), (float)pts[i].y()); }
    double wA = cv::norm(s[1]-s[0]); double wB = cv::norm(s[2]-s[3]); double hA = cv::norm(s[3]-s[0]); double hB = cv::norm(s[2]-s[1]);
    int W = (int)std::max(wA, wB); int H = (int)std::max(hA, hB);
    cv::Point2f d[4]; d[0]=cv::Point2f(0,0); d[1]=cv::Point2f((float)(W-1),0); d[2]=cv::Point2f((float)(W-1),(float)(H-1)); d[3]=cv::Point2f(0,(float)(H-1));
    cv::Mat M = cv::getPerspectiveTransform(s, d);
    cv::Mat warped; cv::warpPerspective(src, warped, M, cv::Size(W,H));
    cv::Mat warpedRgb; cv::cvtColor(warped, warpedRgb, cv::COLOR_BGR2RGB);
    QImage out(warpedRgb.data, warpedRgb.cols, warpedRgb.rows, warpedRgb.step, QImage::Format_RGB888);
    currentImage_ = out.copy();
    view_->setImage(currentImage_);
    view_->clearPoints();
#else
    info_->setText("未检测到 OpenCV，无法进行透视调整。");
#endif
}

void ImageToolsPage::recognizeText(){
    if(currentImage_.isNull()) return;
    QString dir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    if(dir.isEmpty()) dir = QDir::currentPath();
    QString path = QDir(dir).filePath("ocr_tmp.png");
    currentImage_.save(path);
    QString res = OcrService::recognize(path);
    ocrOut_->setText(res.isEmpty() ? QString("未获取到识别结果") : res);
}

void ImageToolsPage::saveImage(){
    if(currentImage_.isNull()) return;
    QString save = QFileDialog::getSaveFileName(this, "保存图片", QString(), "PNG (*.png);;JPEG (*.jpg *.jpeg)");
    if(save.isEmpty()) return;
    currentImage_.save(save);
}
