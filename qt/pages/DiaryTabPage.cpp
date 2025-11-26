#include "DiaryTabPage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTextEdit>
#include <QPushButton>
#include <QFileDialog>
#include <QFontComboBox>
#include <QComboBox>
#include <QFont>
#include <QToolTip>
#include <QFontDatabase>
#include <QUrl>
#include <thread>
#include "../services/Session.h"
#include "../services/DiaryStore.h"
#include "../services/SpeechToText.h"
#include "../windows/CalendarWindow.h"

DiaryTabPage::DiaryTabPage(QWidget* parent) : QWidget(parent){
    setupUi();
    date_ = QDate::currentDate();
    if(Session::instance().isLoggedIn()) user_ = Session::instance().username();
    load();
}

void DiaryTabPage::setupUi(){
    auto* lay = new QVBoxLayout(this);
    title_ = new QLabel(this);
    title_->setObjectName("title");
    editor_ = new QTextEdit(this);
    auto* fmt = new QHBoxLayout();
    fontBox_ = new QFontComboBox(this);
    fontBox_->setWritingSystem(QFontDatabase::SimplifiedChinese);
    sizeBox_ = new QComboBox(this);
    for(int sz : QList<int>{12,14,16,18,20,24,28}) sizeBox_->addItem(QString::number(sz));
    btnBold_ = new QPushButton("加粗", this); btnBold_->setCheckable(true); btnBold_->setStyleSheet("font-weight:700");
    btnItalic_ = new QPushButton("斜体", this); btnItalic_->setCheckable(true); btnItalic_->setStyleSheet("font-style:italic");
    fmt->addWidget(new QLabel("字体:")); fmt->addWidget(fontBox_);
    fmt->addWidget(new QLabel("大小:")); fmt->addWidget(sizeBox_);
    fmt->addWidget(btnBold_); fmt->addWidget(btnItalic_);
    fmt->addStretch();
    btnCalendar_ = new QPushButton("日历", this);
    btnVoice_ = new QPushButton("语音输入", this);
    fmt->addWidget(btnCalendar_);
    fmt->addWidget(btnVoice_);
    btnSave_ = new QPushButton("保存", this);
    btnInsertImage_ = new QPushButton("插入图片", this);
    auto* row = new QHBoxLayout(); row->addWidget(btnSave_); row->addWidget(btnInsertImage_);
    lay->addWidget(title_); lay->addLayout(fmt); lay->addWidget(editor_); lay->addLayout(row);
    connect(btnSave_, &QPushButton::clicked, this, &DiaryTabPage::save);
    connect(btnInsertImage_, &QPushButton::clicked, this, &DiaryTabPage::insertImage);
    connect(fontBox_, &QFontComboBox::currentFontChanged, this, &DiaryTabPage::applyFontFamily);
    connect(sizeBox_, &QComboBox::currentTextChanged, this, [this](const QString& s){ applyFontSize(s.toInt()); });
    connect(btnBold_, &QPushButton::toggled, this, [this](bool){ toggleBold(); });
    connect(btnItalic_, &QPushButton::toggled, this, [this](bool){ toggleItalic(); });
    connect(btnVoice_, &QPushButton::clicked, this, &DiaryTabPage::voiceInput);
    connect(btnCalendar_, &QPushButton::clicked, this, &DiaryTabPage::openCalendar);
}

void DiaryTabPage::load(){
    if(!Session::instance().isLoggedIn()){ title_->setText("未登录的日记 - "); editor_->setPlainText("请先登录"); return; }
    user_ = Session::instance().username();
    title_->setText(QString("%1 的日记 - %2").arg(user_).arg(date_.toString("yyyy-MM-dd")));
    const QString content = DiaryStore::load(user_, date_);
    editor_->clear(); images_.clear();
    const auto lines = content.split('\n');
    for(const auto& line : lines){
        if(line.startsWith("[Image]")){
            const QString path = line.mid(7);
            if(!path.isEmpty()){
                images_.append(path);
                const QString url = QUrl::fromLocalFile(path).toString();
                editor_->insertHtml(QString("<br><img src=\"%1\" style=\"max-width:100%;\"><br>").arg(url));
            }
        } else {
            editor_->append(line);
        }
    }
}

void DiaryTabPage::save(){
    if(!Session::instance().isLoggedIn()) return;
    QString txt = editor_->toPlainText();
    for(const auto& img : images_){ txt += "\n[Image]" + img; }
    DiaryStore::save(user_, date_, txt);
}

void DiaryTabPage::insertImage(){
    const QString path = QFileDialog::getOpenFileName(this, "选择图片", QString(), "Images (*.png *.jpg *.jpeg)");
    if(path.isEmpty()) return;
    images_.append(path);
    const QString url = QUrl::fromLocalFile(path).toString();
    editor_->insertHtml(QString("<br><img src=\"%1\" style=\"max-width:100%;\"><br>").arg(url));
}

void DiaryTabPage::applyFontFamily(const QFont& f){ editor_->setCurrentFont(f); }
void DiaryTabPage::applyFontSize(int pt){ editor_->setFontPointSize(pt); }
void DiaryTabPage::toggleBold(){ editor_->setFontWeight(btnBold_->isChecked()? QFont::Bold : QFont::Normal); QToolTip::showText(btnBold_->mapToGlobal(QPoint(0, btnBold_->height())), btnBold_->isChecked()? "已开启加粗" : "已关闭加粗"); }
void DiaryTabPage::toggleItalic(){ editor_->setFontItalic(btnItalic_->isChecked()); QToolTip::showText(btnItalic_->mapToGlobal(QPoint(0, btnItalic_->height())), btnItalic_->isChecked()? "已开启斜体" : "已关闭斜体"); }

void DiaryTabPage::voiceInput(){
    btnVoice_->setEnabled(false);
    std::thread([this]{
        const QString text = SpeechToText::recognizeOnce();
        QMetaObject::invokeMethod(this, [this, text]{
            if(!text.isEmpty()) editor_->append(text);
            btnVoice_->setEnabled(true);
        }, Qt::QueuedConnection);
    }).detach();
}

void DiaryTabPage::openCalendar(){
    auto* cw = new CalendarWindow(true, this);
    connect(cw, &CalendarWindow::datePicked, this, [this, cw](const QDate& d){
        date_ = d; load(); cw->close(); cw->deleteLater();
    });
    cw->show();
}
