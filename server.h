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
    explicit Server(int nPort, QWidget *parent = nullptr);
    ~Server();

public slots:
    virtual void slotNewConnection();
    void slotReadClient();

private:
    void initServer(int nPort);
    void sendToClient(QTcpSocket* pSocket, const QString& str);

    QTcpServer* tcpServer = nullptr;
    QTextEdit* txt;
//    quint16 m_nNextBlockSize;
    QLabel* serverStatusLabel = nullptr;
    QVBoxLayout* vBoxLayout;
    QDataStream in;
};
#endif // SERVER_H
