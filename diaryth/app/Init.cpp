#include "Init.hpp"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QTextCodec>
#include <QFontDatabase>
#include <QDir>
#include <QTemporaryDir>
#include <QStandardPaths>
#include <QDateTime>

#include <fstream>
#include <signal.h>

#include "app/log.hpp"

#include "app/LogHandler.hpp"
#include "app/Config.hpp"
#include "app/StretchImage.hpp"

#include "diary/SQLBase.hpp"
#include "diary/DiaryCardEngine.hpp"
#include "diary/TestsEngine.hpp"

#include "app/RequestClient.hpp"
#include "app/VisualReport.hpp"
#include "app/JsonReport.hpp"

#ifdef AuralsLegacy
    #include "audio/wave/AudioHandler.hpp"
    #include "audio/wave/WaveShape.hpp"
    #include "audio/spectrum/Spectrograph.hpp"
    #include "audio/spectrum/Cepstrumgraph.hpp"
    #include "audio/features/ACFgraph.hpp"
#endif

#include "audio/Recorder.hpp"

#ifndef Q_OS_WINDOWS //minGW doesn't support web view
    #include <QtWebView>
#endif



using namespace std;
using namespace diaryth;


void posix_death_signal(int signum)
{
    qDebug() << "Crash happend signal:"<<signum;
    signal(signum, SIG_DFL);
    //Replace with Firebase crashlitics
    exit(3);
}


void setLibPath() {
#ifdef WIN32
    QStringList libPath = QCoreApplication::libraryPaths();

    libPath.append(".");
    libPath.append("platforms");
    libPath.append("imageformats");

    QCoreApplication::setLibraryPaths(libPath);
#endif
}


void loadConfig() {
    std::string currentPath = Config::getInst().testsLocation;
    Config& configuration = Config::getInst();
    configuration.checkConfig();
    std::string confFileName = currentPath + "g.config";
    if (QFile::exists(confFileName.c_str())) {
        std::ifstream confFile(confFileName);
        if (confFile.is_open())
            configuration.load(confFile);
    }
    configuration.printValues();
}


void setPosixSignals() {
    signal(SIGSEGV, posix_death_signal);
    signal(SIGILL, posix_death_signal);
    signal(SIGFPE, posix_death_signal);
}



void registerTestsAndCards(const diaryth::SQLBase& sqlBase)
{
    if (sqlBase.checkCardNameExists("DBT") == false)
        sqlBase.addCardFromFile("DBT", ":/cards/DBT.json");

    if (sqlBase.checkCardNameExists("DBT_skills") == false)
        sqlBase.addCardFromFile("DBT_skills", ":/cards/DBT_skills.json");

    if (sqlBase.checkCardNameExists("BPD") == false)
        sqlBase.addCardFromFile("BPD", ":/cards/BPD.json");

    if (sqlBase.checkTestNameExists("Beck") == false)
        sqlBase.addTestFromFile("Beck", ":/tests/Beck.json");

    if (sqlBase.checkTestNameExists("Check") == false)
        sqlBase.addTestFromFile("Check", ":/tests/Check.json");
}



int mainInit(int argc, char *argv[])
{

    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    qmlRegisterType<diaryth::ConsoleLogQML>("diaryth",1,0,"ConsoleLogView");


    #ifndef Q_OS_WINDOWS //minGW doesn't support web view
        QtWebView::initialize();
    #endif

    QGuiApplication app(argc, argv); //QApplication if widgets are used

    app.setApplicationName("diaryth");
    app.setOrganizationName("accumerite");
    app.setOrganizationDomain("accumerite.ru");

    Q_INIT_RESOURCE(fonts);
    Q_INIT_RESOURCE(cards);
    Q_INIT_RESOURCE(tests);

    qDebug() << "Current working path "<<QDir::currentPath();
    int fontId = QFontDatabase::addApplicationFont(":/fonts/prefont.ttf");

    QStringList famList = QFontDatabase::applicationFontFamilies(fontId) ;
    qDebug() << famList << " font families for id "<<fontId;
    if (famList.isEmpty() == false)
        app.setFont(QFont(famList[0], 11, QFont::Normal, false));
    else
        qWarning() << "Failed to load font";

    Config::getInst().checkConfig();

    qmlRegisterType<diaryth::ConfigTableModel>("diaryth", 1, 0, "ConfigTableModel");
    qmlRegisterType<diaryth::VisualReport>("diaryth", 1, 0, "VisualReport");
    qmlRegisterType<diaryth::JsonReport>("diaryth", 1, 0, "JsonReport");

#ifdef AuralsLegacy
    qmlRegisterType<diaryth::WaveshapeQML>("diaryth", 1, 0, "Waveshape");
    qmlRegisterType<diaryth::SpectrographQML>("diaryth", 1, 0,"Spectrograph");
    qmlRegisterType<diaryth::ACGraphQML>("diaryth", 1, 0,"ACgraph");
    qmlRegisterType<diaryth::StretchImageQML>("diaryth", 1, 0,"StretchImage");
    qmlRegisterType<diaryth::CepstrumgraphQML>("diaryth", 1, 0,"Cepstrumgraph");
#endif

    qmlRegisterUncreatableMetaObject(diaryth::staticMetaObject,
    "diaryth", 1, 0, "VisualTypes", "Error: object creation for enum not supported");

    QDir dir;
    if (dir.exists("records") == false) { //AudioHandler records, later they would be removed from current project
        if (dir.mkdir("records") == false)
            qDebug() << "Failed to create records directory";
    }
    if (dir.exists("recorder") == false) {
        if (dir.mkdir("recorder") == false)
            qDebug() << "Failed to create records directory";
    }

    //Настройки //KOI8-R //ISO 8859-5 //UTF-8 //Windows-1251 //UTF-8
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    QQmlApplicationEngine engine;

#ifdef AuralsLegacy
    diaryth::AudioHandler audio;
    engine.rootContext()->setContextProperty("audio", &audio);
#endif

    diaryth::ConfigQML config;
    diaryth::SQLBase sqlBase;
    diaryth::DiaryCardEngine cardEngine;
    diaryth::TestsEngine testsEngine;
    diaryth::Recorder recorder(sqlBase);
    diaryth::RequestClient requestClient;

    registerTestsAndCards(sqlBase);

    engine.rootContext()->setContextProperty("recorder", &recorder);
    engine.rootContext()->setContextProperty("aconfig", &config);
    engine.rootContext()->setContextProperty("sqlBase", &sqlBase);
    engine.rootContext()->setContextProperty("cardEngine", &cardEngine);
    engine.rootContext()->setContextProperty("testsEngine", &testsEngine);
    engine.rootContext()->setContextProperty("requestClient", &requestClient);

    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    int res = 0;
    try {
        res = app.exec();
    }
    catch(std::exception& e) {
        qDebug() << "Catched exception " << e.what();
        res = -1;
    }

    return res;
}
