#include "Server.h"

Server::Server()
{
    if(this->listen(QHostAddress::Any, 2323))
    {
        qDebug() << "start";
    }
    else
    {
        qDebug() << "error";
    }
    nextBlockSize = 0;
}

void Server::incomingConnection(qintptr socketDescriptor)
{
    socket = new QTcpSocket;
    socket->setSocketDescriptor(socketDescriptor);
    connect(socket, &QTcpSocket::readyRead, this, &Server::slotReadyRead);
    connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);

    Sockets.push_back(socket);
    qDebug() <<"client connected" << socketDescriptor;
}

void Server::slotReadyRead()
{
    socket = (QTcpSocket*)sender();
    QDataStream in(socket);
    in.setVersion(QDataStream::Qt_6_2);
    if(in.status() == QDataStream::Ok)
    {
        qDebug() << "read...";
        /*QString str;
        in >> str;
        qDebug() << str;
        SendToClient(str);*/
        for(;;)
        {
            if(nextBlockSize == 0)
            {
                qDebug() << "nextBlockSize = 0";
                if(socket->bytesAvailable() < 2)
                {
                    qDebug() << "nextBlockSize < 2";
                    break;
                }

                in >> nextBlockSize;
                qDebug() << "nextBlockSize = " << nextBlockSize;
            }
            if(socket->bytesAvailable() < nextBlockSize)
            {
                qDebug() << "Data not full, break";
                break;
            }

            QString str;
            QTime time;
            QString nickname;
            in >> time >> str >> nickname;
            nextBlockSize = 0;
            qDebug() << str;
            SendToClient(str, nickname);
            break;
        }
    }
    else
        qDebug() << "DataStream error";
}

void Server::SendToClient(const QString& str, const QString& nickname)
{
    Data.clear();
    QDataStream out(&Data, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_2);

    out << quint16(0) << QTime::currentTime() << str << nickname;

    out.device()->seek(0);
    out << quint16(Data.size() - sizeof(quint16));

    for(int i = 0; i < Sockets.size(); i++)
    {
        QTcpSocket* socket = Sockets[i];
        socket->write(Data);
    }
}
