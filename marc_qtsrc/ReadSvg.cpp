//#include "readsvg.h"
//#include "readsvg.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ReadSvg.h"




/*ReadSvg::ReadSvg()
{
}

/// @brief 
ReadSvg::~ReadSvg()
{
    // delete this;
}*/



/*void ReadSvg::read(QIODevice *device)
{  //Sleep(100);
    xml.setDevice( device );
    if (xml.readNextStartElement())
    {
        if(xml.name() == "svg")
        {
           // _readSVG();
        }
        else
        {

            QMessageBox msgBox;
            msgBox.setText("Not a valid SVG file");
            msgBox.exec();
           
        }
    }
}*/

/*void ReadSvg::_readSVG()
{  int Layer_Number=0; //
    //ui->textEdit->append(QString("Starts Reading SVG File Now"));
    QString  hole=QString("hole");
    QString  contour = QString("contour");
    Polygons holes;
    Polygon m_contour;
    Polygon m_hole;
  
    Slic3r::ExPolygons expolygons;
    std::vector<Slic3r::ExPolygons> expolygons_by_layer;
    coord_t layer_hieght;
    std::vector<coord_t> layer_hieghts;
    while (!xml.atEnd() && !xml.hasError())
    {

        QString name = xml.name().toString();

        if (xml.isStartElement())
        {            
            
            if(name == "g")
            {
                //ui->textEdit->append(QString("New Layer"));
                Layer_Number++;
			   //Generate layer here...
               
				 
                QXmlStreamAttributes attrib = xml.attributes();
				
				if(attrib.hasAttribute("slic3r:z"))
                { 
			    QString pointsStr = attrib.value("slic3r:z").toString();
                pointsStr.replace(",", " ");
                layer_hieght = (coord_t)(pointsStr.toDouble() * 1000000);//parseSVGNumbers(pointsStr);
                layer_hieghts.push_back(layer_hieght);
                expolygons.clear();// New Layer..so clear previous....
                //ui->textEdit->append(QString("Layer Hieght Z= ")+QString::number(layer_hieght *1000000));
                //LayerHieght=Z.at(0)*1000000;
                }
               


            }
            else if (name == "polygon")
            {
                QXmlStreamAttributes attrib = xml.attributes();


                if (attrib.value("slic3r:type").toString() == contour)

                {
                    // ui->textEdit->append(QString("-Contour"));
                    QString pointsStr = attrib.value("points").toString();
                    pointsStr.replace(",", " ");
                    Slic3r::ExPolygon exp;
                    Slic3r::Polygon pp= Polygon(parseSVGNumbers(pointsStr));
                    if (pp.is_counter_clockwise());else pp.make_counter_clockwise();
                    exp.contour = pp;
                    //1-Instantiate Expolygon here...
                    //2-Fill --Polygon contour;
                    //exp.contour = Polygon();//reseting exp.
                    //= m_contour;//Slic3r::ExPolygon(m_contour);
                    expolygons.push_back(exp);//

                    //exp.holes.clear();// every contour may or may not have holes.. reset...
                   // Make layer hee...

                }
                if (attrib.value("slic3r:type").toString() == hole)

                {
                    //ui->textEdit->append(QString("--Hole"));
                    QString pointsStr = attrib.value("points").toString();
                    pointsStr.replace(",", " ");
                    m_hole = Polygon(parseSVGNumbers(pointsStr));
                    expolygons.back().holes.push_back(m_hole);
                   // exp.holes.push_back(m_hole);
                    //3-Fill --Polygon holes;


                }



            }
               
                
        }
           
        
        else if( xml.isEndElement() )
        {
            if(name == "g")  // -end of a layer...
            {    // Make Layer here completes here...
                 //
                //ExPolygonCollection slices;
                expolygons_by_layer.push_back(expolygons);
                //ExPolygonCollection(const ExPolygon & expolygon);
                expolygons.clear();
                //exp.holes.clear();// clear holes at the end of layer...
                //qDebug() << "God dammit, an END of group - ";
            // ui->textEdit->append(QString("An END of group. "));
             
            }
            if (name == "polygon")
            {    // unlikely to enter here..


            }
        }

        //qDebug() << "ACTUAL XML ELEMENT: " << xml.name();
        xml.readNext();
    }

    myfile.close();

    if (xml.hasError())
    {
        //qDebug() << "XML error: " << xml.errorString() << endl;
      //ui->textEdit->append(QString("XML error:"));
    }
    else if (xml.atEnd())
    {
        //Debug() << "Reached end, done" << endl;
       //ui->textEdit->append(QString("Reached end, done"));

       
    }

    //qDebug() << "Size of hights:" << layer_hieghts.size(); 
    //qDebug() << "Size of Layers:" << expolygons_by_layer.size();

    //Launching the printObject

    Model  slm_model;
    Print* slm_print = new Print();
    ModelObject* modobj = slm_model.add_object();
    slm_model.repair();
    ModelVolume* stlvolumeptr;
    TriangleMesh modmesh;
    stlvolumeptr = modobj->add_volume(modmesh);
    stlvolumeptr->modifier = false;
    DynamicPrintConfig print_config;
    //DynamicPrintConfig c = volumeconfig(config_file);
    //print_config.apply(c);
    
    print_config.normalize();
    stlvolumeptr->config = print_config;
    slm_print->apply_config(print_config);
    slm_model.add_default_instances();

    modobj->arrange_volumes();
    modobj->update_bounding_box();
    modobj->center_around_origin();
    slm_print->add_model_object(modobj);
    //std::cout  <<".....svg now:...... " << std::endl;
    slm_print->objects.front()->_slicesvg(expolygons_by_layer,  layer_hieghts);
    //std::cout << ".......slice done........: " << std::endl;
    slm_print->objects.front()->detect_surfaces_type();//slm_model.bounding_box()
    slm_print->objects.front()->_infill();//slm_model.bounding_box()
    //slm_print->objects.front()->Support_pillers();
    slm_print->objects.front()->write_svg();




}*/



/*Slic3r::Points ReadSvg::parseSVGNumbers(QString cmd)
{   
    cmd.replace(QRegExp("[a-df-zA-DF-Z]"), "");
    cmd.replace("-", " -");
    cmd.replace( "e -", "e-" );
    //qDebug( ) << "COMMAND " << cmd;
    cmd = cmd.trimmed();
    cmd = cmd.simplified();
    QStringList numStr = cmd.split(" ");
    std::vector<coord_t> num;
    foreach(QString str, numStr)
    {
        //num.push_back( qAbs(str.toDouble()) ); // <-- honestly that was stupido.
            num.push_back(static_cast<coord_t>(scale_(str.toDouble())) );
    }
    // Lets make Points
    Slic3r::Points points;
    if (num.size() % 2 == 0) //Is the number of cooridnates even?
    {   //  necessary condition  to have close polygones...


      

        for (size_t i = 0; i < num.size(); i += 2)
        {
            points.push_back(Slic3r::Point(num[i], num[i + 1]));
        }


    }
    else
    {
       // qDebug() << "Error, odd number of coordinates for polygon: " ;
        //ui->textEdit->append(QString("Error, odd number of coordinates for polygon: "));
    }


 return points;
}*/



	
	
    





