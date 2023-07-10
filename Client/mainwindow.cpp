#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    socket = new QTcpSocket(this);
    connect(socket, &QTcpSocket::readyRead, this, &MainWindow::slotsReadyRead);
    connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);
    nextBlockSize = 0;
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButton_clicked()
{
    QString text = ui->lineEdit_2->text();

    if (text.isEmpty())
         ui->textBrowser->append("Error: Nickame was not found");
    else
        socket->connectToHost("127.0.0.1",2323);
}

void MainWindow::SendToServer(QString str)
{
    Data.clear();
    QDataStream out(&Data, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_2);

    out << quint16(0) << QTime::currentTime() << str << ui->lineEdit_2->text();

    out.device()->seek(0);
    out << quint16(Data.size() - sizeof(quint16));

    socket->write(Data);
    ui->lineEdit->clear();
}

void MainWindow::slotsReadyRead()
{
    QDataStream in(socket);
    in.setVersion(QDataStream::Qt_6_2);

    if(in.status() == QDataStream::Ok)
    {
        for(;;)
        {
            if(nextBlockSize == 0)
            {
                if(socket->bytesAvailable() < 2)
                    break;

                in >> nextBlockSize;
            }
            if(socket->bytesAvailable() < nextBlockSize)
                break;

            QString str;
            QTime time;
            QString nickname;
            in >> time >> str >> nickname;
            nextBlockSize = 0;
            ui->textBrowser->append(time.toString() + " [" + nickname + "] : " + str);
        }
    }
    else
    {
        ui->textBrowser->append("read error");
    }
}


void MainWindow::on_pushButton_2_clicked()
{
    SendToServer(ui->lineEdit->text());
}


void MainWindow::on_lineEdit_returnPressed()
{
    SendToServer(ui->lineEdit->text());
}


void MainWindow::on_lineEdit_2_editingFinished()
{
    QString text = ui->lineEdit_2->text();

    if (!text.isEmpty())
        ui->lineEdit_2->setDisabled(true);
}

