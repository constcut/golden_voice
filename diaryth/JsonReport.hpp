#ifndef JSONREPORT_HPP
#define JSONREPORT_HPP

#include <QObject>

class JsonReport : public QObject
{
    Q_OBJECT
public:
    explicit JsonReport(QObject *parent = nullptr);

signals:

};

#endif // JSONREPORT_HPP
