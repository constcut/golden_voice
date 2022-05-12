#include "RequestClient.hpp"

#include <QDebug>

#include <QNetworkRequest>

#include <QNetworkReply>
#include <QThread>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <QFile>


using namespace diaryth;




bool RequestClient::logIn(QString username, QString password)
{
    QString urlString = QString("http://localhost:8000/login?password=%1&login=%2")
                                .arg(password, username);

    _lastRequest = QNetworkRequest(QUrl(urlString));
    auto getReply = _mgr.get(_lastRequest);

    _username = username;

    QObject::connect(getReply, &QNetworkReply::finished, [this, getReply=getReply]()
    {
         QString result = getReply->readAll();
         //qDebug() << result << " : login reply !";

         this->_loggedIn = result == "Logged in!";
         this->loginNotification();
    });

    return false; //Как-то обыграть асинхронно
}


void RequestClient::loginNotification()
{
    qDebug() << "Notification of login result: " << _loggedIn;

    emit loggedIn(_loggedIn);
}


void RequestClient::sendAudioFile(QString filename)
{
    if (_loggedIn == false)
    {
        qDebug() << "Not logged in to send file";
        return;
    }

    QString urlString = QString("http://localhost:8000/audio?login=%1").arg(_username);
    QUrl url(urlString);

    QNetworkRequest req(url);
    QFile* f = new QFile(filename);
    f->open(QIODevice::ReadOnly);

    qDebug() << "File was open: " << f->isOpen();

    auto reply = _mgr.put(req, f);
    f->setParent(reply); //For auto delete

    QObject::connect(reply, &QNetworkReply::finished, [reply=reply]()
    {
        QString result = reply->readAll();
        qDebug() << result << " REPLY !";

        auto doc = QJsonDocument::fromJson(result.toLocal8Bit());
        auto rootObject = doc.object();

        if (rootObject.contains("done"))
            qDebug() << "Contains done! id = " << rootObject["id"].toString()
                     << " key = " << rootObject["key"].toString();
        else
        {
            qDebug() << "Field done not found" << doc.isArray() << doc.isEmpty() << doc.isNull() << doc.isObject();


            for (auto& k: rootObject.keys())
                qDebug() << "Objects inside: " << k;

        }

        reply->deleteLater();
    });


    if (reply->error() == QNetworkReply::NoError) //Try connect slot?
        qDebug() << "Reply has no error";

}


/*

    //Uppload file
    QNetworkAccessManager mgr;


 */
