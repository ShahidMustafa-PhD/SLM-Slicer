#ifndef READSVG_H
#define READSVG_H

#include <QFileDialog>
//#include <QDebug>
#include <QXmlStreamReader>
#include <QMessageBox>
#include <QGraphicsScene>
#include <iostream>
#include <fstream>
#include <string.h>
#include <vector>
#include <QTimer>
#include <windows.h>

#include <QString>
#include <QFile>

#include <QPlainTextEdit>
#include <QtXml>
#include <QDomDocument>
//#include "../src/libslic3r/Layer.hpp"
//#include "../src/libslic3r/Model.hpp"
//#include "../src/libslic3r/Print.hpp"
//#include "../src/libslic3r/libslic3r.h"

using namespace std;
//using namespace ClipperLib;

namespace Slic3r {
    class ReadSvg
    {
        //Q_OBJECT  // Required for meta-object system


    public:
        //ReadSvg();
       // ~ReadSvg();
        //void read(QIODevice* device);

    private:
       // void _readSVG();
        
       // Slic3r::Points parseSVGNumbers(QString cmd);





    private:

        QXmlStreamReader xml;
        QGraphicsScene* scene;


        ofstream  myfile;
        ofstream clipfile;
        ofstream offsetfile;

        QString gcodefileName;
        QString  LaserVector_fileName;

        QPlainTextEdit* textEdit;
    };

}
#endif // MAINWINDOW_H
