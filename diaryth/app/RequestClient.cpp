#include "RequestClient.hpp"

#include <QDebug>

#include <QNetworkRequest>

#include <QNetworkReply>
#include <QThread>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <QTextCodec>

#include <QFile>


using namespace diaryth;



void RequestClient::logIn(QString username, QString password)
{

    //qDebug() << QSslSocket::supportsSsl() << QSslSocket::sslLibraryBuildVersionString() << QSslSocket::sslLibraryVersionString();


    QString urlString = QString("https://accumerite.ru/login?login=%1&password=%2")
                                .arg(username, password);

    _lastRequest = QNetworkRequest(QUrl(urlString));
    auto getReply = _mgr.get(_lastRequest);

    _username = username;

    QObject::connect(getReply, &QNetworkReply::finished, [this, getReply=getReply]()
    {
         QString result = getReply->readAll();

         qDebug() << "Result from server: " << result;

         this->_loggedIn = result == "Logged in!";
         this->loginNotification();

         getReply->deleteLater();
    });

}


void RequestClient::loginNotification()
{
    qDebug() << "Notification of login result: " << _loggedIn;

    emit loggedIn(_loggedIn);
}


void RequestClient::sendAudioFile(QString filename)
{
    sendFile("audio", filename);
}


void RequestClient::sendImageFile(QString filename)
{
    sendFile("image", filename);
}


void RequestClient::sendFile(QString type, QString filename)
{
    if (_loggedIn == false)
    {
        qDebug() << "Not logged in to send file";
        return;
    }

    QString urlString = QString("https://accumerite.ru/%1?login=%2").arg(type, _username);
    QUrl url(urlString);

    QNetworkRequest req(url);
    QFile* f = new QFile(filename);
    f->open(QIODevice::ReadOnly);

    qDebug() << "File was open: " << f->isOpen();

    auto reply = _mgr.put(req, f);
    f->setParent(reply);

    QObject::connect(reply, &QNetworkReply::finished, [this, reply=reply, type=type]()
    {
        QString result = reply->readAll();
        fileSentNotification(type, result);
        reply->deleteLater();
    });


    if (reply->error() == QNetworkReply::NoError) //Try connect slot?
        qDebug() << "Reply has no error";
}


void RequestClient::fileSentNotification(QString type, QString result)
{
    qDebug() << result << " : Send file reply FOR " << type;
    emit fileSent(type, result);
}


void RequestClient::requestCompleteStatus(QString id, QString key)
{
    QString urlString = QString("https://accumerite.ru/processed?id=%1&key=%2&login=%3")
                                .arg(id, key, _username);

    _lastRequest = QNetworkRequest(QUrl(urlString));
    auto reply = _mgr.get(_lastRequest);

    QObject::connect(reply, &QNetworkReply::finished, [this, reply=reply, id=id]()
    {
        QString result = reply->readAll();

        this->processedNotification(id, result);
        reply->deleteLater();
    });

}


void RequestClient::processedNotification(QString id, QString result)
{
    emit fileProcessed(id, result);
}

