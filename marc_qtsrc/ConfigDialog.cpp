#include "ConfigDialog.h"



ConfigDialog::ConfigDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowFlags(windowFlags() | Qt::Window | Qt::WindowMinMaxButtonsHint);
    setWindowTitle("Edit Configuration");
    resize(1000, 600);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    this->setStyleSheet("background-color: #1B2A49;");

    fillParameters();
    createForm(mainLayout);

    QPushButton *saveButton = new QPushButton("📂 Save to config.ini");
    saveButton->setStyleSheet("QPushButton { padding: 5px 15px; font-size: 14px; background-color: transparent; color: white; border: none; } QPushButton:hover { background-color: #4C566A; }");
 



    connect(saveButton, &QPushButton::clicked, this, &ConfigDialog::saveConfig);
    mainLayout->addStretch();
    mainLayout->addWidget(saveButton);
}

ConfigDialog::~ConfigDialog() {}




void ConfigDialog::createForm(QVBoxLayout *outerLayout)
{
   
    //qDebug() << "Total parameters:" << allParams.size();
    //QMessageBox::information(this, "allParams.size()", "Total parameters size: " + QString::number(allParams.size()));
    if (allParams.isEmpty()) {
        QMessageBox::warning(this, "Warning", "No parameters available.");
        return;
    }
    QMap<QString, QFormLayout*> sectionLayouts;
    
    // Create section layouts for each parameter
    for (const ParameterDefinition &param : allParams) {
        QWidget *inputWidget = nullptr;
        
        if (param.type == "double") {
            QDoubleSpinBox *spinBox = new QDoubleSpinBox;
            spinBox->setRange(0.0, 10000.0);
            spinBox->setDecimals(3);
            spinBox->setSingleStep(0.01);
            spinBox->setValue(param.default_value.toDouble());
            inputWidget = spinBox;

        } else if (param.type == "unsigned int") {
            QSpinBox *spinBox = new QSpinBox;
            spinBox->setRange(0, 100000);
            spinBox->setValue(param.default_value.toUInt());
            inputWidget = spinBox;

        } else if (param.type == "bool") {
            QCheckBox *checkBox = new QCheckBox;
            QString val = param.default_value.trimmed().toLower();
            checkBox->setChecked(val == "1" || val == "true");
            inputWidget = checkBox;

        } else if (param.type == "string") {
            QLineEdit *lineEdit = new QLineEdit(param.default_value);
            inputWidget = lineEdit;

        } else if (param.type == "options") {
            QComboBox *comboBox = new QComboBox;
            comboBox->addItems(param.options);
            int defaultIndex = param.options.indexOf(param.default_value);
            if (defaultIndex >= 0)
                comboBox->setCurrentIndex(defaultIndex);
            inputWidget = comboBox;
        }

        if (inputWidget) {
            if (!sectionLayouts.contains(param.section)) {
                sectionLayouts[param.section] = new QFormLayout;
            }
            sectionLayouts[param.section]->addRow(param.key + ":", inputWidget);
            inputFields[param.key] = inputWidget;
        }
    }
   
    //QMessageBox::information(this, "sectionLayouts", "sectionLayouts: " + QString::number(sectionLayouts.keys().size()));
    // Organize sections in a grid layout
    QGridLayout *mainGridLayout = new QGridLayout;
  
this->setLayout(outerLayout);  // If inside a QWidget or QDialog subclass
    int row = 0, col = 0;
    int maxColumns =3;

    for (const QString &section : sectionLayouts.keys()) {
        if (section=="DISABLED") continue;  // Skip "DISABLED" sections
        if (section=="CONSTANT") continue;  // Skip "CONSTANT" sections
        QLabel *sectionLabel = new QLabel("<b>" + section + "</b>");
        sectionLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        
        QWidget *sectionWidget = new QWidget;
        sectionWidget->setLayout(sectionLayouts[section]);

        QVBoxLayout *sectionLayout = new QVBoxLayout;
        sectionLayout->addWidget(sectionLabel);
        sectionLayout->addWidget(sectionWidget);
       
      
        QWidget *sectionContainer = new QWidget;
        sectionContainer->setLayout(sectionLayout);
        sectionContainer->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sectionContainer->setFixedHeight(450);  // Set the height you want
       
        mainGridLayout->addWidget(sectionContainer, row, col);
        
        col++;
        if (col >= maxColumns) {
            col = 0;
            row++;
        }
    }
    
    QWidget *centralWidget = new QWidget;
    centralWidget->setStyleSheet("background-color: #ECEFF4;");
    centralWidget->setLayout(mainGridLayout);
   
    outerLayout->addWidget(centralWidget); // or wrap in QScrollArea if needed

   QScrollArea *scrollArea = new QScrollArea;
   scrollArea->setStyleSheet("background-color: #1B2A49;");
   scrollArea->setWidget(centralWidget);
   scrollArea->setWidgetResizable(true);
   //scrollArea->setMinimumSize(600, 400);
   //scrollArea->setMaximumSize(1000, 800);
    outerLayout->addWidget(scrollArea);

    this->setLayout(outerLayout);
    this->setMinimumSize(900, 600);  // Or whatever size you need

   this->show();  // Make sure the dialog is shown  
   
}



void ConfigDialog::saveConfig() {
    // Save the input fields' data to the config file
    //fillParameters(); already didi in constrctor
    readConfigValues();// Update the values in allParams from the input fields
    writeConfigToFile("config.ini");
    QMessageBox::information(this, "Saved", "Configuration saved to config.ini");
}

void ConfigDialog::writeConfigToFile(const QString &filePath) {
    if (allParams.isEmpty()) {
        qWarning() << "allParams is empty. Nothing to write.";
        return;
    }

    QMap<QString, QList<ParameterDefinition>> sections;

    if (allParams.isEmpty()) {
        qWarning() << "allParams is empty. Nothing to write.";
        return;
    }

    

    // Group parameters by section
    for (const auto& param : allParams) {
        sections[param.section].append(param);
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Could not open file for writing:" << file.errorString();
        return;
    }

    QTextStream out(&file);
   // out.setCodec("UTF-8");  // Optional: ensures Unicode compatibility

    for (auto it = sections.constBegin(); it != sections.constEnd(); ++it) {
        const QString& sectionName = it.key();
        const QList<ParameterDefinition>& params = it.value();

        // Write section header
        out << ";=========================\n";
        out << "; " << sectionName << "\n";
        out << ";=========================\n";

        for (const auto& p : params) {
           
            
            out << p.aliase << " = " << p.value.toString() ;
            if (!p.comment.isEmpty())
                out << "  ; " << p.comment;
            out << "\n";
        }
        out << "\n";
    }

    out.flush();  // <-- Ensure all text is written to file before closing
    file.close();

    qDebug() << "INI file written to:" << filePath;
}

void ConfigDialog::fillParameters() 
{
  
        // Assuming the ParameterDefinition struct is defined as:


// ========================
// BASIC PRINTER SETTINGS
// ========================
allParams.append({"SLICER SETTINGS", "beam_width (mm)", "nozzle_diameter", "double", "0.08", "Laser spot or nozzle width in mm", {}});
allParams.append({"SLICER SETTINGS", "layer_height (mm)", "layer_height", "double", "0.08", "Layer thickness (Z resolution)", {}});
allParams.append({"SLICER SETTINGS", "first_layer_height (mm)", "first_layer_height", "double", "0.08", "First layer height, can help with adhesion", {}});
allParams.append({"SLICER SETTINGS", "xy_size_compensation (mm)", "xy_size_compensation", "double", "0.1", "Shrink/enlarge XY dimensions (mm)", {}});
allParams.append({"SLICER SETTINGS", "fill_gaps", "fill_gaps", "bool", "1", "Enable gap fill between shells", {}});
// MOTION & QUALITY
allParams.append({"SLICER SETTINGS", "resolution", "resolution (mm)", "double", "0.005", "Model resolution (XY tolerance)", {}});
//allParams.append({"DISABLED", "pressure_advance", "pressure_advance", "double", "0", "Not used in laser printing", {}});
allParams.append({"SLICER SETTINGS", "adaptive_slicing", "adaptive_slicing", "bool", "0", "Turn off for now (experimental feature)", {}});
allParams.append({"SLICER SETTINGS", "adaptive_slicing_quality (%)", "adaptive_slicing_quality", "double", "75.", "If adaptive slicing is used", {}});
allParams.append({"SLICER SETTINGS", "match_horizontal_surfaces", "match_horizontal_surfaces", "bool", "1", "Improve visual quality of horizontal faces", {}});
allParams.append({"SLICER SETTINGS", "interface_shells", "interface_shells", "unsigned int", "1", "Use additional interface shell layers", {}});
allParams.append({"DISABLED", "avoid_crossing_perimeters", "avoid_crossing_perimeters", "bool", "1", "Avoid travel over outermost shells", {}});
allParams.append({"DISABLED", "only_retract_when_crossing_perimeters", "only_retract_when_crossing_perimeters", "bool", "1", "Retract only when needed", {}});
allParams.append({"SLICER SETTINGS", "threads", "threads", "unsigned int", "6", "Number of CPU threads to use for slicing", {}});
// ========================
// FILAMENT SETTINGS
// ========================
allParams.append({"CONSTANT", "filament_diameter", "filament_diameter", "double", "0.08", "Set to non-zero to avoid slicing errors (placeholder for laser)", {}});
allParams.append({"DISABLED", "extrusion_multiplier", "extrusion_multiplier", "double", "1", "1 = default; adjust for over/under extrusion (not very relevant for laser)", {}});

// ========================
// SHELL SETTINGS
// ========================
allParams.append({"CONSTANT", "extrusion_width", "extrusion_width", "double", "0.08", "Overall extrusion width (slightly > nozzle for better bonding)", {}});
allParams.append({"CONSTANT", "perimeter_extrusion_width", "perimeter_extrusion_width", "double", "0.08", "Width of each perimeter line", {}});
allParams.append({"CONSTANT", "external_perimeter_extrusion_width", "external_perimeter_extrusion_width", "double", "0.08", "Width of outermost perimeter", {}});
allParams.append({"CONSTANT", "top_infill_extrusion_width", "top_infill_extrusion_width", "double", "0.08", "Width for top surface infill", {}});
allParams.append({"CONSTANT", "infill_extrusion_width", "infill_extrusion_width", "double", "0.08", "Width for infill lines", {}});
allParams.append({"CONSTANT", "solid_infill_extrusion_width", "solid_infill_extrusion_width", "double", "0.08", "Width for solid infill areas", {}});
allParams.append({"CONSTANT", "support_material_extrusion_width", "support_material_extrusion_width", "double", "0.08", "Width for support material", {}});
allParams.append({"CONSTANT", "support_material_interface_extrusion_width", "support_material_interface_extrusion_width", "double", "0.08", "Width for interface support material", {}});

allParams.append({"SHELL SETTINGS", "perimeters", "perimeters", "unsigned int", "2", "Number of vertical shells: 8 × 0.125 = 1 mm", {}});
allParams.append({"SHELL SETTINGS", "min_shell_thickness (mm)", "min_shell_thickness", "double", "0.16", "Minimum wall thickness required (in mm)", {}});
allParams.append({"SHELL SETTINGS", "min_top_bottom_shell_thickness (mm)", "min_top_bottom_shell_thickness", "double", "2", "Controls minimum solid top/bottom thickness in mm", {}});
allParams.append({"SHELL SETTINGS", "top_solid_layers", "top_solid_layers", "unsigned int", "8", "8 × 0.1 mm layer = 0.8 mm top shell", {}});
allParams.append({"SHELL SETTINGS", "bottom_solid_layers", "bottom_solid_layers", "unsigned int", "8", "8 × 0.1 mm layer = 0.8 mm bottom shell", {}});

allParams.append({"SHELL SETTINGS", "extra_perimeters", "extra_perimeters", "bool", "1", "Adds more perimeters where needed for strength", {}});
allParams.append({"SHELL SETTINGS", "thin_walls", "thin_walls", "bool", "1", "Enables detection of narrow features for wall filling", {}});

// INFILL SETTINGS
allParams.append({"FILL SETTINGS", "fill_density (%)", "fill_density", "double", "100", "0–100%: how solid the inside of the model is", {}});
allParams.append({"FILL SETTINGS", "fill_pattern", "fill_pattern", "options", "triangular", "Options: honeycomb, rectilinear, grid, gyroid, etc.", {"honeycomb", "rectilinear", "grid", "gyroid", "triangular"}});
allParams.append({"FILL SETTINGS", "bottom_infill_pattern", "bottom_infill_pattern", "options", "gyroid", "", {"honeycomb", "rectilinear", "grid", "gyroid", "triangular"}});
allParams.append({"FILL SETTINGS", "top_infill_pattern", "top_infill_pattern", "options", "gyroid", "Options: honeycomb, rectilinear, grid, gyroid, etc.", {"honeycomb", "rectilinear", "grid", "gyroid", "triangular"}});
allParams.append({"FILL SETTINGS", "infill_overlap (%)", "infill_overlap", "double", "10", "Overlap between infill and perimeters (0–100%)", {}});
allParams.append({"FILL SETTINGS", "infill_only_where_needed", "infill_only_where_needed", "bool", "1", "Only generate infill where it supports top layers", {}});
allParams.append({"FILL SETTINGS", "infill_first", "infill_first", "bool", "1", "Print infill before perimeters (for better bonding)", {}});
allParams.append({"FILL SETTINGS", "infill_every_layers", "infill_every_layers", "bool", "1", "Infill every layer (can be >1 for sparse infill)", {}});
allParams.append({"DISABLED", "infill_acceleration", "infill_acceleration", "double", "1", "Acceleration for infill moves", {}});
allParams.append({"DISABLED", "infill_extruder", "infill_extruder", "unsigned int", "1", "Tool/extruder used for infill", {}});
allParams.append({"DISABLED", "solid_infill_extruder", "solid_infill_extruder", "unsigned int", "1", "Tool/extruder used for solid infill", {}});
allParams.append({"FILL SETTINGS", "solid_infill_every_layers", "solid_infill_every_layers", "bool", "1", "Add solid infill at every layer", {}});
allParams.append({"DISABLED", "solid_infill_below_area", "solid_infill_below_area", "double", "10", "Fill solid infill if top surface is smaller than this area (mm²)", {}});

// SUPPORT SETTINGS
allParams.append({"SUPPORT SETTINGS", "support", "support", "bool", "1", "Enable support generation", {}});
allParams.append({"SUPPORT SETTINGS", "support_pattern", "support_pattern", "options", "gyroid", "Options: grid, lines, honeycomb, gyroid, etc.", {"grid", "lines", "honeycomb", "gyroid"}});
allParams.append({"SUPPORT SETTINGS", "support_spacing (mm)", "support_spacing", "double", "2.5", "Space between support lines", {}});
allParams.append({"SUPPORT SETTINGS", "support_angle (deg)", "support_angle", "double", "45", "Angle of support pattern", {}});
allParams.append({"SUPPORT SETTINGS", "support_threshold", "support_threshold", "double", "60", "Overhang threshold (degrees)", {}});
allParams.append({"SUPPORT SETTINGS", "support_contact_distance (mm)", "support_contact_distance", "double", "0.1", "Vertical gap between support and model", {}});
allParams.append({"SUPPORT SETTINGS", "support_buildplate_only", "support_buildplate_only", "unsigned int", "0", "1 = only supports from bed; 0 = everywhere", {}});
allParams.append({"SUPPORT SETTINGS", "support_interface_layers", "support_interface_layers", "unsigned int", "4", "Layers of dense interface support", {}});
allParams.append({"SUPPORT SETTINGS", "support_interface_spacing (mm)", "support_interface_spacing", "double", "1", "Spacing between interface layers", {}});
allParams.append({"DISABLED", "support_material_interface_extruder", "support_interface_extruder", "unsigned int", "1", "Tool used for interface support", {}});
allParams.append({"SUPPORT SETTINGS", "support_max_layers", "support_max_layers", "unsigned int", "5", "Max layers of support (optional)", {}});
allParams.append({"SUPPORT SETTINGS", "support_enforce_layers", "support_enforce_layers", "unsigned int", "10", "Always insert at least this many support layers", {}});
allParams.append({"SUPPORT SETTINGS", "support_pillar_size (mm)", "support_pillar_size", "double", "2.5", "Width of support pillar", {}});
allParams.append({"SUPPORT SETTINGS", "support_pillar_spacing (mm)", "support_pillar_spacing", "double", "3", "Spacing between pillars", {}});
allParams.append({"DISABLED", "support_material_extruder", "support_extruder", "unsigned int", "1", "Tool used for generating support", {}});
allParams.append({"DISABLED", "support_material_speed", "support_speed", "double", "60", "Speed (mm/s) for printing support", {}});

// SKIRT AND BRIM
allParams.append({"DISABLED", "skirt_distance", "skirt_distance", "double", "6", "Distance between model and skirt", {}});
allParams.append({"DISABLED", "skirt_height", "skirt_height", "unsigned int", "1", "Number of layers for skirt", {}});
allParams.append({"DISABLED", "skirts", "skirts", "unsigned int", "1", "Number of skirt loops", {}});
allParams.append({"DISABLED", "brim_width", "brim_width", "double", "2", "Width of brim around object base", {}});
allParams.append({"DISABLED", "brim_connections_width", "brim_connections_width", "double", "0.1", "Width of connections between brim and part", {}});

// RETRACTION SETTINGS
allParams.append({"DISABLED", "retract_before_travel", "retract_before_travel", "unsigned int", "1", "Enable retraction before travel moves", {}});
allParams.append({"DISABLED", "retract_length", "retract_length", "unsigned int", "20", "Retraction length (mm)", {}});
allParams.append({"DISABLED", "retract_length_toolchange", "retract_length_toolchange", "unsigned int", "10", "Retraction during tool change", {}});
allParams.append({"DISABLED", "retract_lift", "retract_lift", "unsigned int", "0", "Lift head during retraction (Z-hop)", {}});
allParams.append({"DISABLED", "retract_layer_change", "retract_layer_change", "unsigned int", "0", "Retract on layer change", {}});


// BED AND OBJECT HANDLING
allParams.append({"DISABLED", "complete_objects", "complete_objects", "unsigned int", "1", "Print complete object before moving to next (if multiple)", {}});
allParams.append({"DISABLED", "duplicate_distance", "duplicate_distance", "unsigned int", "6", "Distance between duplicated parts", {}});


// MISC

 
return;
      
}
    
void ConfigDialog::readConfigValues()
{
   
    QVariant nozzle_diameter;
    for (int i = 0; i < allParams.size(); ++i) {
        ParameterDefinition &param = allParams[i];  // Non-const reference so we can modify it
        const QString &key = param.key;
        QWidget *widget = inputFields.value(key, nullptr);

        if (!widget) continue;

        QVariant value;

        if (param.type == "string") {
            if (QLineEdit *lineEdit = qobject_cast<QLineEdit*>(widget)) {
                value = lineEdit->text();
            }
        } else if (param.type == "double") {
            if (QDoubleSpinBox *doubleSpin = qobject_cast<QDoubleSpinBox*>(widget)) {
                value = doubleSpin->value();
            }

            if(param.aliase == "nozzle_diameter") {  // Special case for nozzle_diameter
                nozzle_diameter = value;
            }
        } else if (param.type == "unsigned int") {
            if (QSpinBox *Spin = qobject_cast<QSpinBox*>(widget)) {
                value = Spin->value();
            }
        } else if (param.type == "bool") {
            if (QCheckBox *checkBox = qobject_cast<QCheckBox*>(widget)) {
                if(checkBox->isChecked()) value = 1; else value = 0;
            }
        } else if (param.type == "options") {
            if (QComboBox *comboBox = qobject_cast<QComboBox*>(widget)) {
                value = comboBox->currentText();
            }
        } else {
            qWarning("Unknown parameter type for key: %s", qUtf8Printable(key));
        }

        if (value.isValid()) {
            //values.insert(key, value);
            param.value = value;  // Update the value inside ParameterDefinition
        }
    }
  // Fill in the CONSTANT and DISABLED values
  for (int i = 0; i < allParams.size(); ++i) {
    ParameterDefinition &param = allParams[i];  // Non-const reference so we can modify it
    const QString &section = param.section;

    if (section == "CONSTANT") {
    param.value = nozzle_diameter;  // Use the value from nozzle_diameter
    
    }
       }








    return ;
}

 



