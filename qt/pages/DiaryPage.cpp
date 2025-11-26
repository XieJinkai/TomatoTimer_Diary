#include "DiaryPage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCalendarWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QFileDialog>
#include <QDate>
#include <QLineEdit>
#include "../services/Session.h"
#include "../services/DiaryStore.h"

DiaryPage::DiaryPage(QWidget* parent): QWidget(parent){ setupUi(); }

void DiaryPage::setupUi(){
    auto* lay = new QVBoxLayout(this);
    calendar_ = new QCalendarWidget(this);
    editor_ = new QTextEdit(this);
    mediaInfo_ = new QLineEdit(this); mediaInfo_->setPlaceholderText("附件：图片/音频（文件路径记录）");
    btnSave_ = new QPushButton("保存", this);
    btnInsertImage_ = new QPushButton("插入图片", this);
    btnAttachAudio_ = new QPushButton("附加音频", this);
    auto* row = new QHBoxLayout(); row->addWidget(btnSave_); row->addWidget(btnInsertImage_); row->addWidget(btnAttachAudio_);
    lay->addWidget(calendar_); lay->addWidget(editor_); lay->addWidget(mediaInfo_); lay->addLayout(row);

    connect(calendar_, &QCalendarWidget::selectionChanged, this, &DiaryPage::loadSelected);
    connect(btnSave_, &QPushButton::clicked, this, &DiaryPage::save);
    connect(btnInsertImage_, &QPushButton::clicked, this, &DiaryPage::insertImage);
    connect(btnAttachAudio_, &QPushButton::clicked, this, &DiaryPage::attachAudio);

    loadSelected();
}

void DiaryPage::loadSelected(){
    if(!Session::instance().isLoggedIn()){ editor_->setPlainText("请先登录"); return; }
    const auto u = Session::instance().username();
    const auto d = calendar_->selectedDate();
    editor_->setPlainText(DiaryStore::load(u, d));
}

void DiaryPage::save(){
    if(!Session::instance().isLoggedIn()) return;
    const auto u = Session::instance().username();
    const auto d = calendar_->selectedDate();
    DiaryStore::save(u, d, editor_->toPlainText() + "\n" + mediaInfo_->text());
}

void DiaryPage::insertImage(){
    const QString path = QFileDialog::getOpenFileName(this, "选择图片", QString(), "Images (*.png *.jpg *.jpeg)");
    if(path.isEmpty()) return;
    mediaInfo_->setText(mediaInfo_->text() + (mediaInfo_->text().isEmpty()?"":"; ") + QString("[Image]%1").arg(path));
}

void DiaryPage::attachAudio(){
    const QString path = QFileDialog::getOpenFileName(this, "选择音频", QString(), "Audio (*.wav *.mp3)");
    if(path.isEmpty()) return;
    mediaInfo_->setText(mediaInfo_->text() + (mediaInfo_->text().isEmpty()?"":"; ") + QString("[Audio]%1").arg(path));
}