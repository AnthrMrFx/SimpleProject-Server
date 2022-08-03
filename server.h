#ifndef SERVER_H
#define SERVER_H

#include <QDialog>
#include <QString>
#include <QList>
#include <QTcpServer>
#include <QTcpSocket>
#include <QtWidgets>
#include <QtNetwork>

class Server : public QWidget
{
    Q_OBJECT

public:
    explicit Server(QWidget *parent = nullptr);
    ~Server();

public slots:
    virtual void slotNewConnection();
    void slotReadClient();
    void initServer();
    void closeServer();

private slots:
    void enableInitBtn();

private:
    void sendToClient(QTcpSocket* pSocket, const QString& str);

    QTcpServer* m_ptcpServer = nullptr;
    QTextEdit* m_ptxt;
    QPushButton* init_btn;
    QPushButton* closeServerButton;
    QLineEdit* port_edit;
    quint16 m_nNextBlockSize;
    QLabel* serverStatusLabel = nullptr;
};
#endif // SERVER_H
