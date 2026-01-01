// SvgLoaderWorker.cpp
#include "SvgLoaderWorker.h"
#include <QGraphicsSvgItem>

SvgLoaderWorker::SvgLoaderWorker(const QString& folderPath, const QStringList& svgFiles, QObject* parent)
    : QObject(parent), folderPath(folderPath), svgFiles(svgFiles)
{
    // You can add any additional initialization or setup here if needed
}

void SvgLoaderWorker::process()
{
    double zSpacing = 10.0;

    for (int i = 0; i < svgFiles.size(); ++i) {
        if (QThread::currentThread()->isInterruptionRequested()) {
            break;  // Stop if cancel is requested
        }
        QString filePath = folderPath + "/" + svgFiles[i];
        QGraphicsSvgItem* svgItem = new QGraphicsSvgItem(filePath);
        svgItem->setZValue(i * zSpacing);
        svgItem->setPos(0, 0);

        emit svgItemLoaded(svgItem);  // Send each loaded item back to main thread
    }

    emit finished();
    
}
