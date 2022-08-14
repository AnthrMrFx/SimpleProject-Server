#ifndef SERVER_H
#define SERVER_H

#include <QDialog>
#include <QObject>
#include <QSql>
#include <QSqlError>
#include <QString>
#include <QList>
#include <QTcpServer>
#include <QTcpSocket>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QtWidgets>
#include <QtNetwork>

#define TABLE_WORD          "word"
#define TABLE_DEFINITION    "definition"
#define TABLE_EXAMPLE       "example"
#define TABLE               "dictionary"

class Server : public QWidget
{
    Q_OBJECT

public:
    explicit Server(int nPort, QWidget *parent = nullptr);

public slots:
    virtual void slotNewConnection();
    void slotReadClient();

private:
    void initServer(int nPort);
    void sendToClient(QTcpSocket* pSocket, const QString& str);
    void createTable();

    QTcpServer* tcpServer = nullptr;
    QSqlDatabase db;
    QTextEdit* txt;
//    quint16 m_nNextBlockSize;
    QLabel* serverStatusLabel = nullptr;
    QVBoxLayout* vBoxLayout;
    QDataStream in;
};
#endif // SERVER_H
