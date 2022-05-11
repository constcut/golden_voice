#include "RequestClient.hpp"

#include <QDebug>

#include <QNetworkRequest>

#include <QNetworkReply>
#include <QThread>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>


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
         qDebug() << result << " : login reply !";

         this->_loggedIn = result == "Logged in!";
         //emit this->loggedIn(_loggedIn);
    });

    qDebug() << "Request was sent!";


    return false; //Как-то обыграть асинхронно
}


/*

    //Uppload file
    QNetworkAccessManager mgr;
    QUrl url("http://localhost:8000/q11_test.ogg"); //TODO "http://127.0.0.1:8000/q2_test.ogg?check=test")

    QNetworkRequest req(url);

    QFile f = QFile("C:/Users/constcut/Desktop/local/local_2.ogg"); //stac over fake?
    f.open(QIODevice::ReadOnly);

    qDebug() << "File was open: " << f.isOpen();

    auto reply = mgr.put(req, &f);

    QObject::connect(reply, &QNetworkReply::finished, [reply]()
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
                qDebug() << "KEY: " << k;

        }

        reply->deleteLater();
    });


    if (reply->error() == QNetworkReply::NoError) //Try connect slot?
        qDebug() << "Reply has no error";

 */
