#include "mainwindow.h"
#include <QApplication>
#include <crtdbg.h>
#include <QIcon>
#include <QFileInfo>
#include <QDebug>

#ifdef _WIN32
#include <QDir>
#include <QStandardPaths>
#include <windows.h>

/**
 * @brief Setup library paths for the installed application
 * 
 * Installation structure (created by CMake installer):
 * install-debug/
 * ??? bin/
 * ?   ??? MarcSLM.exe              (executable - runs here)
 * ?   ??? Qt6Core.dll              (Qt libraries - all in bin/)
 * ?   ??? Qt6Gui.dll
 * ?   ??? Qt6Widgets.dll
 * ?   ??? platforms/qwindows.dll   (Qt platform plugins - in bin/platforms/)
 * ?   ??? imageformats/            (Image plugins - in bin/imageformats/)
 * ?   ??? styles/                  (Style plugins - in bin/styles/)
 * ?   ??? [other DLLs]
 * ??? lib/
 *     ??? [optional libraries]
 * 
 * The exe runs from: install-debug/bin/
 * So QCoreApplication::applicationDirPath() returns: install-debug/bin/
 * All DLLs and plugins are in the same directory or subdirectories of bin/
 */
void setupLibraryPaths()
{
    QString exeDir = QCoreApplication::applicationDirPath();
    qDebug() << "Executable directory:" << exeDir;

    // Use correct wchar conversion
    std::wstring wExeDir = exeDir.toStdWString();
    SetDllDirectoryW(wExeDir.c_str());
    qDebug() << "SetDllDirectoryW:" << exeDir;

    QString platformsDir = exeDir + "/platforms";
    QCoreApplication::addLibraryPath(exeDir);
    QCoreApplication::addLibraryPath(platformsDir);
    qDebug() << "Added to Qt library paths:" << exeDir << "," << platformsDir;

    // Optional: set QT_PLUGIN_PATH to bin/
    qputenv("QT_PLUGIN_PATH", exeDir.toLocal8Bit());
    qDebug() << "QT_PLUGIN_PATH set to:" << exeDir;

    QFileInfo qwindowsDll(platformsDir + "/qwindowsd.dll");
    if (qwindowsDll.exists())
        qDebug() << "? Found qwindowsd.dll at:" << qwindowsDll.absoluteFilePath();
    else
        qWarning() << "? qwindowsd.dll NOT found at:" << platformsDir + "/qwindowsd.dll";

    QFileInfo qt6Core(exeDir + "/Qt6Cored.dll");
    if (qt6Core.exists())
        qDebug() << "? Found Qt6Cored.dll at:" << qt6Core.absoluteFilePath();
    else
        qWarning() << "? Qt6Cored.dll NOT found at:" << exeDir + "/Qt6Cored.dll";
}

#endif // _WIN32

int main(int argc, char **argv)
{
    // Enable memory leak detection in debug builds (MSVC only)
    // Uncomment this line if you want to track memory leaks
    //_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

#ifdef _WIN32
    // Setup DLL search paths BEFORE creating QApplication
    // This is critical because QApplication initialization loads platform plugins
    // and other Qt libraries, so the search paths must be configured first
   setupLibraryPaths();
#endif

    // Create QApplication - at this point, all DLLs should be found
    QApplication app(argc, argv);

    // Create and display main window
    MainWindow w;
    w.setWindowTitle("MarcSLM");
    w.setWindowIcon(QIcon(":/icons/icon3.png"));
    w.show();

    return app.exec();
}
