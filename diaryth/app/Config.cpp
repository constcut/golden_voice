#include "Config.hpp"

#include <string>
#include <fstream>
#include <QDebug>
#include <iostream>


using namespace diaryth;


void diaryth::initGlobals()
{
    Config& conf = Config::getInst();

    conf.invertedLocation = "";
    conf.testsLocation = "";
    conf.theUserId = "";

    conf.screenSize.height=0;
    conf.screenSize.width=0;
    conf.screenSize.dpi=0;

    conf.isMobile = false; //desktop actually is default

#ifdef __ANDROID_API__
    conf.isMobile = true;
#endif
    conf.platform = "other";
#ifdef linux
    conf.platform = "linux";  //not very sure
#endif
#ifdef WIN32
   conf.platform = "windows";
#endif
#ifdef __ANDROID_API__
   conf.platform = "android";
#endif

}


void diaryth::setTestLocation(std::string newTL)
{
    std::string invertedLocation="";

    std::cout << "setCurrent location to "<<newTL.c_str()<<std::endl;
    Config& conf = Config::getInst();

    conf.testsLocation = newTL;

    //invert
    invertedLocation.clear();

    for (size_t i = 0; i < newTL.length(); ++i)
        if (newTL[i]=='/')
            invertedLocation.push_back('\\');
        else
            invertedLocation.push_back(newTL[i]);

    conf.invertedLocation = invertedLocation;
}


void Config::connectLog(bool *ptrValue, int index,std::string logName)
{
    if (index==-1)
        index = topIndex+1;

    logs[index] = ptrValue;
    logsNames[index] = logName;

    if (index>topIndex)
        topIndex=index;
}



//log Name 1/0
//v Name 1290391
void Config::load(std::ifstream &file)
{
    std::string lastLine = "firstOne";

    int count = 0;
    while (!lastLine.empty())
    {
        ++count;
        std::string line;
        std::getline(file, line);


        if (line != lastLine)
        {
            addLine(line);
            lastLine = line;
        }
        else
            break;
        //hotfix
        if (line.substr(0,4)=="zend")
            break;
    }

    std::cout <<"Total count "<<count;


}

std::string repairString(std::string str)
{
    std::string response = str;
    //first 0
    while (response[0]==' ')
        response.erase(0,1);



    if (response[0]=='"')
    {
        int clIndex = 0;
        for (size_t i = 1; i < response.size(); ++i)
            if (response[i]=='"')
            {
                clIndex = i;
                break;
            }

        if (clIndex)
        response = response.substr(1,clIndex-1);
    }


    return response;
}

void Config::addLine(std::string anotherLine)
{
            //std::cout <<anotherLine.c_str()<<std::endl;
            size_t eqSign = anotherLine.find("=");

            if (eqSign != std::string::npos)
            {
                std::string left = anotherLine.substr(0,eqSign-1);
                std::string right = anotherLine.substr(eqSign+1);
                right = repairString(right);
                values[left]=right;
                std::cout <<left.c_str()<<"="<<right.c_str()<<std::endl;
            }
}

//should connect some values to be used
//first
void Config::save(std::ofstream &file) const
{
     //file.opened();
     for (auto it = values.begin();
          it!= values.end(); ++it)
     {
         std::string curLine = (*it).first + " = " + (*it).second + "\n";
         file.write(curLine.c_str(), curLine.size());
     }
     file.close();
}

void Config::printValues() const
{
    qDebug()<<"Configuration parameters";
    for (auto it = values.begin();
         it!= values.end(); ++it)
    {
        qDebug()<< (*it).first.c_str() <<" = "<<(*it).second.c_str();
    }
    qDebug()<<"Config printed";
}

void Config::addValue(std::string name, std::string val)
{
    if (values.count(name) == 0)
    {
        std::pair<std::string,std::string> newLine;
        newLine.first = name;
        newLine.second = val;
        values.insert(newLine);
       // std::map<std::string,std::string>:
    }
}


void Config::checkConfig()
{
    addValue("field","value");
}


