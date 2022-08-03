#include "server.h"

Server::Server(int nPort, QWidget *parent)
    : QWidget(parent),
      m_nNextBlockSize(0),
      serverStatusLabel(new QLabel)
{
    initServer(nPort);

    m_ptxt = new QTextEdit;
    m_ptxt->setReadOnly(true);

    QVBoxLayout* pvbxLayout = new QVBoxLayout;
    pvbxLayout->addWidget(new QLabel(tr("<H1>Сервер</H1>")));
    pvbxLayout->addWidget(serverStatusLabel);
    pvbxLayout->addWidget(m_ptxt);

    setLayout(pvbxLayout);
}

void Server::initServer(int nPort)
{
    m_ptcpServer = new QTcpServer(this);
    if (!m_ptcpServer->listen(QHostAddress::Any, nPort)) {
        QMessageBox::critical(this, tr("Сервер"),
                              tr("Ошибка при запуске сервера: %1.")
                              .arg(m_ptcpServer->errorString()));
        return;
    }

    serverStatusLabel->setText(tr("Сервер прослушивает соединения по порту %1").arg(nPort));

    connect(m_ptcpServer, SIGNAL(newConnection()),
            this, SLOT(slotNewConnection()));
}

//void Server::closeServer()
//{
//    m_ptcpServer->close();
//    closeServerButton->setEnabled(false);
//    serverStatusLabel->setText(tr("Сервер отключен"));
//}

void Server::slotNewConnection()
{
    QTcpSocket* pClientSocket = m_ptcpServer->nextPendingConnection();

    in.setDevice(pClientSocket);
    in.setVersion(QDataStream::Qt_5_10);

    connect(pClientSocket, &QTcpSocket::disconnected,
            pClientSocket, &QTcpSocket::deleteLater);
    connect(pClientSocket, &QTcpSocket::readyRead,
            this, &Server::slotReadClient);

    sendToClient(pClientSocket, tr("Сервер: Соединение установлено"));
}

void Server::slotReadClient()
{
    QTcpSocket* pClientSocket = (QTcpSocket*)sender();
    QDataStream in(pClientSocket);
    in.setVersion(QDataStream::Qt_5_10);

    in.startTransaction();

    QTime time;
    QString msg;
    in >> time >> msg;

    if (!in.commitTransaction())
        return;

    QString strMessage = time.toString() + msg;

    m_ptxt->append(strMessage);
    sendToClient(pClientSocket, tr("Server Response: Received \"") + msg + tr("\""));

//    for (;;)
//    {
//        if ( !m_nNextBlockSize)
//        {
//            if (pClientSocket->bytesAvailable() < sizeof (quint16))
//            {
//                break;
//            }
//            in >> m_nNextBlockSize;
//        }
//        if (pClientSocket->bytesAvailable() < m_nNextBlockSize)
//        {
//            break;
//        }

//        QTime time;
//        QString str;

//        in >> time >> str;

//        QString strMessage = time.toString() + " " + "Client has sent - " + str;
//        m_ptxt->append(strMessage);

//        m_nNextBlockSize = 0;
//        sendToClient(pClientSocket, tr("Server Response: Received \"") + str + tr("\""));
//    }
}

void Server::sendToClient(QTcpSocket* pSocket, const QString& str)
{
    QByteArray arrBlock;
    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_10);
    out << quint16(0) << QTime::currentTime() << str;

    out.device()->seek(0);
    out << quint16(arrBlock.size() - sizeof(quint16));

    pSocket->write(arrBlock);
}

Server::~Server()
{
    this->close();
}

