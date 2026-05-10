#pragma once

#include <QWidget>

class QLabel;
class QLineEdit;
class QPushButton;
class QNetworkAccessManager;
class QNetworkReply;

class LoginRegisterPage : public QWidget {
    Q_OBJECT
public:
    explicit LoginRegisterPage(QWidget* parent=nullptr);

signals:
    void loggedIn();

private:
    void setupUi();
    void enterLocalMode();
    void testConnection();
    void onRemoteLogin();
    void onRemoteRegister();
    void handleHealthReply(QNetworkReply* reply);
    void handleLoginReply(QNetworkReply* reply);
    void handleRegisterReply(QNetworkReply* reply);
    QString responseErrorMessage(QNetworkReply* reply, const QString& fallback) const;
    QString serverUrlText() const;

    QLabel* info_{};
    QLineEdit* serverUrlEdit_{};
    QLineEdit* loginUserEdit_{};
    QLineEdit* loginPassEdit_{};
    QLineEdit* registerUserEdit_{};
    QLineEdit* registerPassEdit_{};
    QLineEdit* registerConfirmEdit_{};
    QLineEdit* realNameEdit_{};
    QLineEdit* genderEdit_{};
    QLineEdit* ageEdit_{};
    QLineEdit* phoneEdit_{};
    QLineEdit* emailEdit_{};
    QLineEdit* schoolEdit_{};
    QLineEdit* majorEdit_{};
    QPushButton* btnLocal_{};
    QPushButton* btnTestConnection_{};
    QPushButton* btnLogin_{};
    QPushButton* btnRegister_{};
    QNetworkAccessManager* network_{};
};
