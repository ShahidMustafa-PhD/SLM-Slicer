
#pragma once

#include <QVTKOpenGLNativeWidget.h>
#include <QCoreApplication> // ✅ Required to fix your error
#include <QDragEnterEvent>
#include <QDropEvent>

class CustomVTKWidget : public QVTKOpenGLNativeWidget
{
public:
    CustomVTKWidget(QWidget* parent = nullptr) : QVTKOpenGLNativeWidget(parent) {
        setAcceptDrops(false); // Forward drops to parent instead
    }
protected:
    void dragEnterEvent(QDragEnterEvent* event) override {
        if (parentWidget())
            QCoreApplication::sendEvent(parentWidget(), event);
    }
    void dropEvent(QDropEvent* event) override {
        if (parentWidget())
            QCoreApplication::sendEvent(parentWidget(), event);
    }
};
