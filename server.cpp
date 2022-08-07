#include "server.h"

Server::Server(int nPort, QWidget *parent)
    : QWidget(parent),
//      m_nNextBlockSize(0),
      txt(new QTextEdit),
      serverStatusLabel(new QLabel)
{
    initServer(nPort);

    txt->setReadOnly(true);


    QVBoxLayout* vBoxLayout = new QVBoxLayout;
    vBoxLayout->addWidget(new QLabel(tr("<H1>Сервер</H1>")));
    vBoxLayout->addWidget(serverStatusLabel);
    vBoxLayout->addWidget(txt);

    setLayout(vBoxLayout);
}

void Server::initServer(int nPort)
{
    tcpServer = new QTcpServer(this);
    if (!tcpServer->listen(QHostAddress::Any, nPort)) {
        QMessageBox::critical(this, tr("Сервер"),
                              tr("Ошибка при запуске сервера: %1.")
                              .arg(tcpServer->errorString()));
        return;
    }

    serverStatusLabel->setText(tr("Сервер прослушивает соединения по порту %1").arg(nPort));

    connect(tcpServer, &QTcpServer::newConnection,
            this, &Server::slotNewConnection);
}

//void Server::closeServer()
//{
//    tcpServer->close();
//    closeServerButton->setEnabled(false);
//    serverStatusLabel->setText(tr("Сервер отключен"));
//}

void Server::slotNewConnection()
{
    QTcpSocket* clientSocket = tcpServer->nextPendingConnection();

    in.setDevice(clientSocket);
    in.setVersion(QDataStream::Qt_5_10);

    connect(clientSocket, &QTcpSocket::disconnected,
            clientSocket, &QTcpSocket::deleteLater);
    connect(clientSocket, &QTcpSocket::readyRead,
            this, &Server::slotReadClient);

    sendToClient(clientSocket, tr("Сервер: Соединение установлено"));
}

void Server::slotReadClient()
{
    QTcpSocket* clientSocket = (QTcpSocket*)sender();
//    QDataStream in(clientSocket);
    in.setDevice(clientSocket);
    in.setVersion(QDataStream::Qt_5_10);

    in.startTransaction();

    QTime time;
    QString msg;

    in >> time >> msg;

    if (!in.commitTransaction())
    {
        return;
    }

    QString str = time.toString() + " " + msg;

    txt->append(str);
    sendToClient(clientSocket, tr("Сервер: Получено \"") + msg + tr("\""));

//    for (;;)
//    {
//        if ( !m_nNextBlockSize)
//        {
//            if (clientSocket->bytesAvailable() < sizeof (quint16))
//            {
//                break;
//            }
//            in >> m_nNextBlockSize;
//        }
//        if (clientSocket->bytesAvailable() < m_nNextBlockSize)
//        {
//            break;
//        }

//        QTime time;
//        QString str;

//        in >> time >> str;

//        QString str = time.toString() + " " + "Client has sent - " + str;
//        txt->append(str);

//        m_nNextBlockSize = 0;
//        sendToClient(clientSocket, tr("Server Response: Received \"") + str + tr("\""));
//    }
}

void Server::sendToClient(QTcpSocket* socket, const QString& str)
{
    QByteArray arrBlock;
    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_10);
    out << /*quint16(0) <<*/ QTime::currentTime() << str;

//    out.device()->seek(0);
//    out << quint16(arrBlock.size() - sizeof(quint16));

    socket->write(arrBlock);
}

Server::~Server()
{
    this->close();
}

