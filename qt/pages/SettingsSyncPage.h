#pragma once
#include <QWidget>
class QLineEdit; class QPushButton; class QLabel;

class SettingsSyncPage : public QWidget {
    Q_OBJECT
public:
    explicit SettingsSyncPage(QWidget* parent=nullptr);
private:
    QLineEdit* cloudDir_{}; QLabel* info_{}; QPushButton* btnChoose_{}; QPushButton* btnSync_{};
    void setupUi(); void chooseCloud(); void syncNow();
};