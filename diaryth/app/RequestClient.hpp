#ifndef REQUESTCLIENT_HPP
#define REQUESTCLIENT_HPP

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>


namespace diaryth {


    class RequestClient : public QObject
    {
        Q_OBJECT
    public:

        Q_INVOKABLE void logIn(QString username, QString password);

        Q_INVOKABLE void sendAudioFile(QString filename);
        Q_INVOKABLE void sendImageFile(QString filename);

        Q_INVOKABLE void requestCompleteStatus(QString id, QString key);


    signals:

        void loggedIn(bool value);
        void fileSent(QString type, QString result); //Maybe we also need filename?
        void fileProcessed(QString id, QString result);

    private:

        void loginNotification();
        void fileSentNotification(QString type, QString result);
        void processedNotification(QString id, QString result);

        void sendFile(QString type, QString filename);


        bool _loggedIn;
        QString _username;
        QNetworkAccessManager _mgr;

        QNetworkRequest _lastRequest;


    };

}

#endif // REQUESTCLIENT_HPP
