#pragma once
#include <QWidget>

#include "../services/AccountingStore.h"

class QComboBox;
class QDateTimeEdit;
class QDoubleSpinBox;
class QLineEdit;
class QPushButton;
class QTableWidget;

class AccountingPage : public QWidget {
    Q_OBJECT
public:
    explicit AccountingPage(QWidget* parent=nullptr);

private:
    QLineEdit* itemEdit_{};
    QDoubleSpinBox* amountEdit_{};
    QComboBox* typeEdit_{};
    QLineEdit* noteEdit_{};
    QDateTimeEdit* timeEdit_{};
    QComboBox* rangeFilter_{};
    QComboBox* typeFilter_{};
    QLineEdit* searchEdit_{};
    QTableWidget* table_{};
    QPushButton* addButton_{};
    QPushButton* clearButton_{};
    QPushButton* deleteButton_{};

    void setupUi();
    void refresh();
    void refreshTypeFilter(const QList<AccountingRecord>& records);
    QList<AccountingRecord> filteredRecords() const;
    AccountingRange selectedRange() const;

private slots:
    void addRecord();
    void clearForm();
    void deleteSelected();
};
