#ifndef REQUESTCLIENT_HPP
#define REQUESTCLIENT_HPP

#include <QObject>


namespace diaryth {


    class RequestClient : public QObject
    {
        Q_OBJECT
    public:

        Q_INVOKABLE bool logIn(QString username, QString password);

        Q_INVOKABLE void sendAudioFile(QString filename) {}
        Q_INVOKABLE void sendPhotoFile(QString filename) {}

        Q_INVOKABLE bool requestCompleteStatus(QString id, QString key) {}
        Q_INVOKABLE QString getLastCompleteAnswer() {}

    private:

        QString _username;


    };

}

#endif // REQUESTCLIENT_HPP
