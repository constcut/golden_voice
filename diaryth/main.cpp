﻿#include "app/Init.hpp"
#include "app/LogHandler.hpp"


using namespace diaryth;

int main(int argc, char *argv[])
{
    LogHandler::getInstance().setFilename("log.txt");
    qDebug() << "Starting application";
    return mainInit(argc,argv);
}
