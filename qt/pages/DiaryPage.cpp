#include "DiaryPage.h"

#include <QCalendarWidget>
#include <QDate>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

#include "../services/DiaryStore.h"
#include "../services/FocusRecordService.h"
#include "../services/Session.h"

DiaryPage::DiaryPage(QWidget* parent): QWidget(parent){ setupUi(); }

void DiaryPage::setupUi(){
    auto* lay = new QVBoxLayout(this);
    calendar_ = new QCalendarWidget(this);
    editor_ = new QTextEdit(this);
    mediaInfo_ = new QLineEdit(this);
    mediaInfo_->setPlaceholderText("附件：图片/音频路径记录");
    btnSave_ = new QPushButton("保存", this);
    btnInsertImage_ = new QPushButton("插入图片", this);
    btnAttachAudio_ = new QPushButton("附加音频", this);
    auto* row = new QHBoxLayout();
    row->addWidget(btnSave_);
    row->addWidget(btnInsertImage_);
    row->addWidget(btnAttachAudio_);
    lay->addWidget(calendar_);
    lay->addWidget(editor_);
    lay->addWidget(mediaInfo_);
    lay->addLayout(row);

    connect(calendar_, &QCalendarWidget::selectionChanged, this, &DiaryPage::loadSelected);
    connect(btnSave_, &QPushButton::clicked, this, &DiaryPage::save);
    connect(btnInsertImage_, &QPushButton::clicked, this, &DiaryPage::insertImage);
    connect(btnAttachAudio_, &QPushButton::clicked, this, &DiaryPage::attachAudio);

    loadSelected();
}

void DiaryPage::loadSelected(){
    if(!Session::instance().isLoggedIn()){
        editor_->setPlainText("请先登录");
        focusLines_.clear();
        mediaInfo_->clear();
        return;
    }
    const auto u = Session::instance().username();
    const auto d = calendar_->selectedDate();
    const QString raw = DiaryStore::load(u, d);
    const DiaryContentParts parts = FocusRecordService::splitDiaryContent(raw);
    focusLines_ = parts.focusLines;
    mediaInfo_->setText(parts.mediaLines.join("\n"));
    editor_->setPlainText(FocusRecordService::renderDiaryText(raw));
}

void DiaryPage::save(){
    if(!Session::instance().isLoggedIn()) return;
    const auto u = Session::instance().username();
    const auto d = calendar_->selectedDate();
    const QStringList mediaLines = mediaInfo_->text().split('\n', Qt::SkipEmptyParts);
    DiaryStore::save(u, d, FocusRecordService::composeDiaryContent(
        focusLines_,
        FocusRecordService::stripRenderedFocusBlock(editor_->toPlainText()),
        mediaLines));
}

void DiaryPage::insertImage(){
    const QString path = QFileDialog::getOpenFileName(this, "选择图片", QString(), "Images (*.png *.jpg *.jpeg)");
    if(path.isEmpty()) return;
    mediaInfo_->setText(mediaInfo_->text() + (mediaInfo_->text().isEmpty() ? "" : "\n") + QString("[Image]%1").arg(path));
}

void DiaryPage::attachAudio(){
    const QString path = QFileDialog::getOpenFileName(this, "选择音频", QString(), "Audio (*.wav *.mp3)");
    if(path.isEmpty()) return;
    mediaInfo_->setText(mediaInfo_->text() + (mediaInfo_->text().isEmpty() ? "" : "\n") + QString("[Audio]%1").arg(path));
}
