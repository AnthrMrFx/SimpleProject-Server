#include "server.h"

Server::Server(QWidget *parent)
    : QWidget(parent), m_nNextBlockSize(0)
{
    serverStatusLabel = new QLabel;
    serverStatusLabel->setText(tr("Сервер отключен"));

    m_ptxt = new QTextEdit;
    m_ptxt->setReadOnly(true);

    init_btn = new QPushButton(tr("Запуск сервера"));
    init_btn->setEnabled(false);
    connect(init_btn, &QPushButton::clicked,
            this, &Server::initServer);

    closeServerButton = new QPushButton(tr("Отключить сервер"));
    closeServerButton->setEnabled(false);

    port_edit = new QLineEdit;
    connect(port_edit, &QLineEdit::textChanged,
            this, &Server::enableInitBtn);

    QVBoxLayout* pvbxLayout = new QVBoxLayout;
    pvbxLayout->addWidget(new QLabel(tr("<H1>Сервер</H1>")));
    pvbxLayout->addWidget(serverStatusLabel);
    pvbxLayout->addWidget(m_ptxt);
    pvbxLayout->addWidget(port_edit);
    pvbxLayout->addWidget(init_btn);
    pvbxLayout->addWidget(closeServerButton);
    setLayout(pvbxLayout);
}

void Server::initServer()
{
    int nPort = 0;
    if(!(nPort = port_edit->text().toInt()))
    {
        QMessageBox::critical(this, tr("Сервер"), tr("Введен некорректный адрес порта"));
        return;
    }

    m_ptcpServer = new QTcpServer(this);
    if (!m_ptcpServer->listen(QHostAddress::Any, nPort)) {
        QMessageBox::critical(this, tr("Сервер"),
                              tr("Ошибка при запуске сервера: %1.")
                              .arg(m_ptcpServer->errorString()));
        return;
    }

    serverStatusLabel->setText(tr("Сервер включен по порту ") + QVariant(nPort).toString());
    closeServerButton->setEnabled(true);

    connect(m_ptcpServer, SIGNAL(newConnection()),
            this, SLOT(slotNewConnection()));
    connect(closeServerButton, &QPushButton::clicked,
            this, &Server::closeServer);
}

void Server::closeServer()
{
    m_ptcpServer->close();
    closeServerButton->setEnabled(false);
    serverStatusLabel->setText(tr("Сервер отключен"));
}

void Server::slotNewConnection()
{
    QTcpSocket* pClientSocket = m_ptcpServer->nextPendingConnection();

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

    for (;;)
    {
        if ( !m_nNextBlockSize)
        {
            if (pClientSocket->bytesAvailable() < sizeof (quint16))
            {
                break;
            }
            in >> m_nNextBlockSize;
        }
        if (pClientSocket->bytesAvailable() < m_nNextBlockSize)
        {
            break;
        }

        QTime time;
        QString str;

        in >> time >> str;

        QString strMessage = time.toString() + " " + "Client has sent - " + str;
        m_ptxt->append(strMessage);

        m_nNextBlockSize = 0;
        sendToClient(pClientSocket, tr("Server Response: Received \"") + str + tr("\""));
    }
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

void Server::enableInitBtn()
{
    init_btn->setEnabled(!port_edit->text().isEmpty());
}

Server::~Server()
{
    this->close();
}

