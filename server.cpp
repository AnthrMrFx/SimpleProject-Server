#include "server.h"

Server::Server(int nPort, QWidget *parent)
    : QWidget(parent),
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

    db = QSqlDatabase::addDatabase("QPSQL");
    db.setDatabaseName("qt_projects");
    db.setUserName("postgres");
    db.setPassword("root");

    if(!db.open())
    {
        QMessageBox::critical(this, tr("Сервер"),
                              tr("Ошибка при запуске сервера: %1.")
                              .arg(tcpServer->errorString()));
        return;
    }

    if (!db.tables().contains( QLatin1String("messages")))
    {
        createTable();
    }

    serverStatusLabel->setText(tr("Сервер принимает соединения по порту %1").arg(nPort));

    connect(tcpServer, &QTcpServer::newConnection,
            this, &Server::slotNewConnection);
}

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

    in.setDevice(clientSocket);
    in.setVersion(QDataStream::Qt_5_10);

    in.startTransaction();

    QTime time;
    QString msg;
    QString user;

    in >> time >> msg;
    user = QString::number(clientSocket->socketDescriptor());

    if (!in.commitTransaction())
    {
        return;
    }

    QSqlQuery query;

    query.prepare("INSERT INTO messages ( time, "
                                         " message   , "
                                         " userr      ) "
                  "VALUES (:time, :message, :userr);");
    query.bindValue(":time",    time.toString());
    query.bindValue(":message", msg);
    query.bindValue(":userr",    user);


    if(!query.exec())
    {
        QMessageBox::critical(this, tr("Сервер"),
                         tr("Ошибка при добавлении записи в таблицу: %1.")
                         .arg(query.lastError().text()));
    }

    QString str = time.toString() + " " + msg;

    txt->append(str);
    sendToClient(clientSocket, tr("Сервер: Получено \"") + msg + tr("\""));
}

void Server::sendToClient(QTcpSocket* socket, const QString& str)
{
    QByteArray arrBlock;
    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_10);
    out << QTime::currentTime() << str;

    socket->write(arrBlock);
}

void Server::createTable()
{
    QSqlQuery query;

    if(!(query.exec("CREATE TABLE messages("
                   "id BIGSERIAL NOT NULL PRIMARY KEY, "
                   "time VARCHAR(8) NOT NULL, "
                   "message VARCHAR(255) NOT NULL, "
                   "userr VARCHAR(40) NOT NULL"
               ")")))
    {
     QMessageBox::critical(this, tr("Сервер"),
                      tr("Ошибка при создании таблицы: %1.")
                      .arg(query.lastError().text()));
     return;
    }
}

