#include "mainwindow.h"
#include <QApplication>
#include <QIcon>
#include <QDir>

#ifdef _WIN32
#include <windows.h>
#endif

// --------------------
// Windows DLL handling
// --------------------
#ifdef _WIN32
static void setupWindowsDllSearchPath()
{
    wchar_t exePath[MAX_PATH];
    GetModuleFileNameW(nullptr, exePath, MAX_PATH);

    std::wstring exeDir(exePath);
    exeDir = exeDir.substr(0, exeDir.find_last_of(L"\\/"));

    std::wstring binDir = exeDir + L"\\bin";

    // Secure modern DLL loading
    SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
    AddDllDirectory(binDir.c_str());
}
#endif

int main(int argc, char* argv[])
{
#ifdef _WIN32
    // ✔ NO Qt calls here
    setupWindowsDllSearchPath();
#endif

    // ✔ FIRST Qt object
    QApplication app(argc, argv);

    // --------------------
    // Qt plugin paths
    // --------------------
    QString exeDir = QCoreApplication::applicationDirPath();
    QString binDir = exeDir + "/bin";

    QCoreApplication::addLibraryPath(binDir);
    QCoreApplication::addLibraryPath(binDir + "/platforms");
    QCoreApplication::addLibraryPath(binDir + "/imageformats");
    QCoreApplication::addLibraryPath(binDir + "/styles");

    // --------------------
    // Main window
    // --------------------
    MainWindow w;
    w.setWindowTitle("MarcSLM");
    w.setWindowIcon(QIcon(":/icons/icon3.png"));
    w.show();

    return app.exec();
}
