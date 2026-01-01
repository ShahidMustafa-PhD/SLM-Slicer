// SvgLoaderWorker.h
#pragma once

#include <QObject>
#include <QThread>
#include <QStringList>
#include <QGraphicsSvgItem>

class SvgLoaderWorker : public QObject
{
    Q_OBJECT

public:
explicit SvgLoaderWorker(const QString& folderPath, const QStringList& svgFiles, QObject* parent = nullptr);

signals:
    void svgItemLoaded(QGraphicsSvgItem* item);
    void finished();

public slots:
    void process();  // The main function that runs in the thread

private:
    QString folderPath;
    QStringList svgFiles;
};
