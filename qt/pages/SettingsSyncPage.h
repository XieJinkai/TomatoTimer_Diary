#pragma once

#include <QWidget>

class CloudSyncService;
class QLabel;
class QLineEdit;
class QPushButton;

class SettingsSyncPage : public QWidget {
    Q_OBJECT
public:
    explicit SettingsSyncPage(QWidget* parent=nullptr);

private:
    void setupUi();
    bool applyServerUrl();
    void testConnection();
    void uploadNow();
    void downloadNow();

    QLineEdit* serverUrl_{};
    QLabel* info_{};
    QPushButton* btnTest_{};
    QPushButton* btnUpload_{};
    QPushButton* btnDownload_{};
    CloudSyncService* sync_{};
};
