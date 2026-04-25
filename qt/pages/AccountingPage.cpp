#include "AccountingPage.h"

#include <QComboBox>
#include <QDateTimeEdit>
#include <QDoubleSpinBox>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSet>
#include <QSignalBlocker>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

#include "../services/AccountingStatsService.h"
#include "../services/Session.h"

namespace {

QString displayType(const AccountingRecord& record){
    return record.type.trimmed().isEmpty() ? QStringLiteral("未分类") : record.type.trimmed();
}

}

AccountingPage::AccountingPage(QWidget* parent): QWidget(parent){
    setupUi();
    refresh();
}

void AccountingPage::setupUi(){
    auto* root = new QVBoxLayout(this);

    auto* titleRow = new QHBoxLayout();
    auto* title = new QLabel("记账", this);
    auto* summary = new QLabel("记录每一笔消费，并按时间和分类查看。", this);
    summary->setStyleSheet("color:#666666;");
    titleRow->addWidget(title);
    titleRow->addSpacing(12);
    titleRow->addWidget(summary);
    titleRow->addStretch();
    root->addLayout(titleRow);

    auto* formRow1 = new QHBoxLayout();
    itemEdit_ = new QLineEdit(this);
    itemEdit_->setPlaceholderText("项目，例如：午餐");
    itemEdit_->setAccessibleName("记账项目");

    amountEdit_ = new QDoubleSpinBox(this);
    amountEdit_->setRange(0.01, 99999999.0);
    amountEdit_->setDecimals(2);
    amountEdit_->setPrefix("¥ ");
    amountEdit_->setValue(25.0);
    amountEdit_->setAccessibleName("记账金额");

    typeEdit_ = new QComboBox(this);
    typeEdit_->setEditable(true);
    typeEdit_->addItems({ "餐饮", "交通", "购物", "住房", "学习", "娱乐", "医疗", "其他" });
    typeEdit_->setAccessibleName("记账分类");

    formRow1->addWidget(new QLabel("项目", this));
    formRow1->addWidget(itemEdit_, 2);
    formRow1->addWidget(new QLabel("金额", this));
    formRow1->addWidget(amountEdit_, 1);
    formRow1->addWidget(new QLabel("分类", this));
    formRow1->addWidget(typeEdit_, 1);
    root->addLayout(formRow1);

    auto* formRow2 = new QHBoxLayout();
    noteEdit_ = new QLineEdit(this);
    noteEdit_->setPlaceholderText("备注，例如：和同学聚餐");
    noteEdit_->setAccessibleName("记账备注");

    timeEdit_ = new QDateTimeEdit(QDateTime::currentDateTime(), this);
    timeEdit_->setDisplayFormat("yyyy-MM-dd HH:mm");
    timeEdit_->setCalendarPopup(true);
    timeEdit_->setAccessibleName("记账时间");

    addButton_ = new QPushButton("新增记录", this);
    clearButton_ = new QPushButton("清空输入", this);
    deleteButton_ = new QPushButton("删除选中", this);

    formRow2->addWidget(new QLabel("备注", this));
    formRow2->addWidget(noteEdit_, 2);
    formRow2->addWidget(new QLabel("时间", this));
    formRow2->addWidget(timeEdit_, 1);
    formRow2->addWidget(addButton_);
    formRow2->addWidget(clearButton_);
    formRow2->addWidget(deleteButton_);
    root->addLayout(formRow2);

    auto* filterRow = new QHBoxLayout();
    rangeFilter_ = new QComboBox(this);
    rangeFilter_->addItems({ "今天", "本周", "本月", "今年", "全部" });
    rangeFilter_->setAccessibleName("记账范围筛选");

    typeFilter_ = new QComboBox(this);
    typeFilter_->addItem("全部分类");
    typeFilter_->setAccessibleName("记账分类筛选");

    searchEdit_ = new QLineEdit(this);
    searchEdit_->setPlaceholderText("按项目或备注搜索");
    searchEdit_->setAccessibleName("记账搜索");

    filterRow->addWidget(new QLabel("范围", this));
    filterRow->addWidget(rangeFilter_);
    filterRow->addWidget(new QLabel("分类", this));
    filterRow->addWidget(typeFilter_);
    filterRow->addWidget(searchEdit_, 1);
    root->addLayout(filterRow);

    table_ = new QTableWidget(this);
    table_->setAccessibleName("记账表格");
    table_->setColumnCount(5);
    table_->setHorizontalHeaderLabels({ "时间", "项目", "金额", "分类", "备注" });
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->setSelectionMode(QAbstractItemView::SingleSelection);
    table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table_->verticalHeader()->setVisible(false);
    table_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    table_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    table_->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    table_->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    table_->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);
    root->addWidget(table_, 1);

    connect(addButton_, &QPushButton::clicked, this, &AccountingPage::addRecord);
    connect(clearButton_, &QPushButton::clicked, this, &AccountingPage::clearForm);
    connect(deleteButton_, &QPushButton::clicked, this, &AccountingPage::deleteSelected);
    connect(rangeFilter_, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [this](int){ refresh(); });
    connect(typeFilter_, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [this](int){ refresh(); });
    connect(searchEdit_, &QLineEdit::textChanged, this, [this](const QString&){ refresh(); });
}

AccountingRange AccountingPage::selectedRange() const{
    switch(rangeFilter_->currentIndex()){
    case 0: return AccountingRange::Day;
    case 1: return AccountingRange::Week;
    case 2: return AccountingRange::Month;
    case 3: return AccountingRange::Year;
    default: return AccountingRange::All;
    }
}

QList<AccountingRecord> AccountingPage::filteredRecords() const{
    if(!Session::instance().isLoggedIn()){
        return {};
    }

    QList<AccountingRecord> records = AccountingStatsService::recordsInRange(Session::instance().username(), selectedRange());
    const QString selectedType = typeFilter_->currentData().toString();
    const QString keyword = searchEdit_->text().trimmed();

    QList<AccountingRecord> result;
    for(const auto& record : records){
        const QString normalizedType = displayType(record);
        if(!selectedType.isEmpty() && normalizedType != selectedType){
            continue;
        }
        if(!keyword.isEmpty()
            && !record.item.contains(keyword, Qt::CaseInsensitive)
            && !record.note.contains(keyword, Qt::CaseInsensitive)){
            continue;
        }
        result.append(record);
    }
    return result;
}

void AccountingPage::refreshTypeFilter(const QList<AccountingRecord>& records){
    const QString current = typeFilter_->currentData().toString();
    QSet<QString> types;
    const QStringList preferredOrder{ "餐饮", "交通", "购物", "住房", "学习", "娱乐", "医疗", "其他", "未分类" };
    for(const auto& record : records){
        types.insert(displayType(record));
    }

    QSignalBlocker blocker(typeFilter_);
    typeFilter_->clear();
    typeFilter_->addItem("全部分类", "");
    for(const auto& type : preferredOrder){
        if(types.contains(type)){
            typeFilter_->addItem(type, type);
        }
    }

    const int index = typeFilter_->findData(current);
    typeFilter_->setCurrentIndex(index >= 0 ? index : 0);
}

void AccountingPage::refresh(){
    if(!Session::instance().isLoggedIn()){
        table_->setRowCount(1);
        for(int col = 0; col < table_->columnCount(); ++col){
            auto* item = new QTableWidgetItem(col == 0 ? "请先登录" : "");
            table_->setItem(0, col, item);
        }
        return;
    }

    const auto allInRange = AccountingStatsService::recordsInRange(Session::instance().username(), selectedRange());
    refreshTypeFilter(allInRange);
    const auto records = filteredRecords();

    table_->setRowCount(records.size());
    for(int row = 0; row < records.size(); ++row){
        const auto& record = records.at(row);
        auto* timeItem = new QTableWidgetItem(record.time.toString("yyyy-MM-dd HH:mm"));
        timeItem->setData(Qt::UserRole, record.time.toString(Qt::ISODate));
        table_->setItem(row, 0, timeItem);
        table_->setItem(row, 1, new QTableWidgetItem(record.item));
        table_->setItem(row, 2, new QTableWidgetItem(QString::number(record.amount, 'f', 2)));
        table_->setItem(row, 3, new QTableWidgetItem(displayType(record)));
        table_->setItem(row, 4, new QTableWidgetItem(record.note));
    }
}

void AccountingPage::addRecord(){
    if(!Session::instance().isLoggedIn()){
        QMessageBox::information(this, "记账", "请先登录后再记账。");
        return;
    }

    const QString item = itemEdit_->text().trimmed();
    const QString type = typeEdit_->currentText().trimmed();
    if(item.isEmpty()){
        QMessageBox::warning(this, "记账", "请输入记账项目。");
        return;
    }

    AccountingRecord record;
    record.time = timeEdit_->dateTime();
    record.item = item;
    record.amount = amountEdit_->value();
    record.type = type;
    record.note = noteEdit_->text().trimmed();

    if(!AccountingStore::append(Session::instance().username(), record)){
        QMessageBox::warning(this, "记账", "保存失败，请稍后重试。");
        return;
    }

    clearForm();
    refresh();
}

void AccountingPage::clearForm(){
    itemEdit_->clear();
    amountEdit_->setValue(25.0);
    typeEdit_->setCurrentIndex(0);
    typeEdit_->setEditText(typeEdit_->currentText());
    noteEdit_->clear();
    timeEdit_->setDateTime(QDateTime::currentDateTime());
}

void AccountingPage::deleteSelected(){
    if(!Session::instance().isLoggedIn()){
        return;
    }

    const int row = table_->currentRow();
    if(row < 0){
        QMessageBox::information(this, "记账", "请先选中一条记录。");
        return;
    }

    const QString timeValue = table_->item(row, 0) ? table_->item(row, 0)->data(Qt::UserRole).toString() : QString();
    const QString itemValue = table_->item(row, 1) ? table_->item(row, 1)->text() : QString();
    const double amountValue = table_->item(row, 2) ? table_->item(row, 2)->text().toDouble() : 0.0;
    const QString typeValue = table_->item(row, 3) ? table_->item(row, 3)->text() : QString();
    const QString noteValue = table_->item(row, 4) ? table_->item(row, 4)->text() : QString();

    auto records = AccountingStore::loadAll(Session::instance().username());
    bool removed = false;
    for(int i = 0; i < records.size(); ++i){
        const auto& record = records.at(i);
        if(record.time.toString(Qt::ISODate) == timeValue
            && record.item == itemValue
            && qFuzzyCompare(record.amount + 1.0, amountValue + 1.0)
            && displayType(record) == typeValue
            && record.note == noteValue){
            records.removeAt(i);
            removed = true;
            break;
        }
    }

    if(!removed || !AccountingStore::saveAll(Session::instance().username(), records)){
        QMessageBox::warning(this, "记账", "删除失败，请稍后重试。");
        return;
    }

    refresh();
}
