#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QMap>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QPushButton>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QLabel>
#include <QScrollArea>
#include <QMessageBox>
#include <QSettings>
#include <QFileDialog>
struct ParameterDefinition {
    QString section;   // e.g., "BASIC PRINTER SETTINGS"  
   
    QString key; // "nozzle_diameter", "layer_height", etc.
    QString aliase; // "beam_width", "layer_height", etc.
    QString type; // "double", "unsigned int", "bool", "string", "options"
    QString default_value;
    QString comment;
    QStringList options; // only used if type == "options"
    QVariant value; // to store the current value of the parameter
};
class ConfigDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConfigDialog(QWidget *parent = nullptr);
    ~ConfigDialog();
    /// @brief 
    void fillParameters();

private slots:
    void saveConfig();

private:
    void createForm(QVBoxLayout *mainLayout);
    void writeConfigToFile(const QString &filename);
    /// @brief 
    /// @return 
    void readConfigValues();
    

    QMap<QString, QWidget*> inputFields;  // Store created widgets by key
    QFormLayout *formLayout;
    QList<ParameterDefinition> allParams;
};

#endif // CONFIGDIALOG_H
