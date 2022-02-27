#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <math.h>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include <QFile>
#include <QDataStream>
#include <QDebug>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts>
#include <QGridLayout>
#include <QStackedWidget>
#include <QTableWidget>

using namespace QtCharts;


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    serial = new QSerialPort(this);
    serial->setPortName("COM12");
    serial->setBaudRate(9600);
    ui->Current_lineEdit->setValidator( new QDoubleValidator(-1.00,1.00,2,this) );
    ui->Current_lineEdit_2->setValidator( new QDoubleValidator(-1.00,1.00,2,this) );
    ui->List_current_lineEdit->setValidator( new QDoubleValidator(-1.00,1.00,2,this) );
    ui->List_current_lineEdit_2->setValidator( new QDoubleValidator(-1.00,1.00,2,this) );
    ui->List_time_lineEdit->setValidator( new QIntValidator(1,2147483647,this));
    ui->List_time_lineEdit_2->setValidator( new QIntValidator(1,2147483647,this));
}



MainWindow::~MainWindow()
{
    if (serial->isOpen())
    {
        serial->close();
    }
    delete ui;
}

/*/////  ABOUT //////*/
void MainWindow::on_pushButton_clicked()
{
   QMessageBox::information(this, tr("About"), tr("Magnetic Tweezers Driver GUI \nVersion 1.6 alpha  \nCristian Castillo, 2020 ") );
}


/*///////////////////*/
/*     CONFIG        */
/*///////////////////*/

void MainWindow::on_refresh_pushButton_clicked()
{
    QList<QSerialPortInfo>com_ports = QSerialPortInfo::availablePorts();
    QSerialPortInfo port;
    ui->port_comboBox->clear();
    foreach(port, com_ports)
    {
        ui->port_comboBox->addItem(port.portName());
    }
}

void MainWindow::on_port_comboBox_currentIndexChanged(const QString &arg1)
{
    serial->close();
    serial->setPortName(arg1);
    serial->setBaudRate(QSerialPort::Baud9600);
    serial->setDataBits(QSerialPort::Data8);
    serial->setParity(QSerialPort::NoParity);
    serial->setStopBits(QSerialPort::OneStop);
    serial->setFlowControl(QSerialPort::NoFlowControl);
    serial->open(QIODevice::ReadWrite);
}

void MainWindow::on_Enable_pushButton_clicked()
{
    if(serial->isOpen())
    {
        if(enable_output==false)
        {
            serial->write("E\r\n");
            ui->Enable_pushButton->setText("DISABLE");
            enable_output=true;
        }
        else
        {
            serial->write("D\r\n");
            ui->Enable_pushButton->setText("ENABLE");
            enable_output=false;
        }
    }
    else
    {
        QMessageBox::warning(this, tr("SERIAL"), tr("Select a COM port") );

    }
}


/*//////////////////////////*/
/*          CH1             */
/*//////////////////////////*/


/*/////  CURRENT [I]  //////*/
void MainWindow::on_Current_run_pushButton_clicked()
{
    if(serial->isOpen())
    {
        QString setcurrent = ui->Current_lineEdit->text();
        if(setcurrent.toDouble()<=1.0 && setcurrent.toDouble()>=-1.0)
        {
            QString frame = "I 1 " + setcurrent + "\r\n";
            serial->write(frame.toUtf8());
        }
        else
        {
            QMessageBox::warning(this, tr("ERROR"), tr("Parameters out of bounds [Imax=+-1.0A]") );
        }
    }
    else
    {
        QMessageBox::warning(this, tr("SERIAL"), tr("Select a COM port") );

    }
}
void MainWindow::on_Current_stop_pushButton_clicked()
{
    if(serial->isOpen())
    {
        QString frame = "I 1 0\r\n";
        serial->write(frame.toUtf8());

    }
    else
    {
        QMessageBox::warning(this, tr("SERIAL"), tr("Select a COM port") );
    }
}



/*/////  LIST [L]  //////*/

void MainWindow::on_List_add_pushButton_clicked()
{
    if(serial->isOpen())
    {
        QString setcurrent = ui->List_current_lineEdit->text();
        QString settime = ui->List_time_lineEdit->text();
        if(setcurrent.toDouble()<=1.0 && setcurrent.toDouble()>=-1.0)
        {
            QString frame = "L " + setcurrent +" " + settime + "\r\n";
            serial->write(frame.toUtf8());
            list_puntos1.append(setcurrent+","+settime);
        }
        else
        {
            QMessageBox::warning(this, tr("ERROR"), tr("Parameters out of bounds [Imax=+-1.0A]") );
        }
    }
    else
    {
        QMessageBox::warning(this, tr("SERIAL"), tr("Select a COM port") );

    }
}

void MainWindow::on_List_clear_pushButton_clicked()
{
    if(serial->isOpen())
    {
        serial->write("L 0 0\r\n");
        list_puntos1.clear();
    }
    else
    {
        QMessageBox::warning(this, tr("SERIAL"), tr("Select a COM port") );
    }

}

void MainWindow::on_List_run_pushButton_clicked()
{
    if(serial->isOpen())
    {
        if(list_output==false)
        {
            if(ui->checkBox_L1->isChecked())
            {
                serial->write("L T 1\r\n");
            }
            else
            {
                serial->write("L 1\r\n");
            }
            ui->List_run_pushButton->setText("STOP");
            list_output=true;
        }
        else
        {
            serial->write("L 0\r\n");
            QThread::msleep(200);
            serial->write("L T 0\r\n");
            ui->List_run_pushButton->setText("RUN");
            list_output=false;
        }
    }
    else
    {
        QMessageBox::warning(this, tr("SERIAL"), tr("Select a COM port") );

    }
}

void MainWindow::on_List_graph_pushButton_clicked()
{


    // SERIE
    QLineSeries *series = new QtCharts::QLineSeries();
    QScatterSeries *scatter = new QScatterSeries();
    scatter->setName("scatter1");
    scatter->setMarkerShape(QScatterSeries::MarkerShapeCircle);
    scatter->setMarkerSize(10.0);
    int time=0;
    foreach(auto &x,list_puntos1)
    {
        const QStringList parts = x.split(",");
        series->append(time,parts.at(0).toFloat());
        series->append(parts.at(1).toInt()+time,parts.at(0).toFloat());
        scatter->append(time,parts.at(0).toFloat());
        scatter->append(parts.at(1).toInt()+time,parts.at(0).toFloat());
        time=time+parts.at(1).toInt();
    }

    // CHART
    chart  = new QChart;
    chart->legend()->hide();
    chart->addSeries(series);
    chart->addSeries(scatter);
    chart->setTitle("CH1 CURRENT PROTOCOL");
    QFont titleFont("Times", 12, QFont::Bold);
    QFont axisFont("Times", 10, QFont::Bold);
    chart->setTitleFont(titleFont);

    QValueAxis *axisX = new QValueAxis();
    axisX->setTitleText("Time [ms]");
    axisX->setLabelFormat("%i");
    axisX->setTitleFont(axisFont);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText("Current [A]");
    axisY->setLabelFormat("%g");
    axisY->setTitleFont(axisFont);
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->resize(640,480);
    chartView->setWindowTitle("CH1 Current protocol");

    // TABLE
    tableWidget = new QTableWidget(this);
    tableWidget->setRowCount(list_puntos1.length());
    tableWidget->setColumnCount(2);
    int a = 0;
    foreach(auto &x,list_puntos1)
    {
        const QStringList parts = x.split(",");
        tableWidget->setItem(a,0,new QTableWidgetItem(parts.at(0)));
        tableWidget->setItem(a,1,new QTableWidgetItem(parts.at(1)));
        tableWidget->item(a,0)->setTextAlignment( Qt::AlignCenter);
        tableWidget->item(a,1)->setTextAlignment( Qt::AlignCenter);
        a++;
    }
    tableWidget->setHorizontalHeaderItem(0, new QTableWidgetItem("Current [A]"));
    tableWidget->setHorizontalHeaderItem(1, new QTableWidgetItem("Time [ms]"));
    tableWidget->setColumnWidth(0,110);
    tableWidget->setColumnWidth(1,110);

    //STACK
    QWidget *wdg = new QWidget;
    QGridLayout *mainLayout = new QGridLayout(wdg);
    mainLayout->addWidget(tableWidget, 1, 0);
    mainLayout->addWidget(chartView, 1, 1);
    mainLayout->setColumnStretch(1, 1);
    mainLayout->setColumnStretch(0, 0);
    wdg->resize(800,600);
    wdg->setWindowTitle("Chart");
    wdg->show();


}


void MainWindow::on_List_save_graph_pushButton_clicked()
{

    // SERIE
    QLineSeries *series = new QtCharts::QLineSeries();
    QScatterSeries *scatter = new QScatterSeries();
    scatter->setName("scatter1");
    scatter->setMarkerShape(QScatterSeries::MarkerShapeCircle);
    scatter->setMarkerSize(10.0);
    int time=0;
    foreach(auto &x,list_puntos1)
    {
        const QStringList parts = x.split(",");
        series->append(time,parts.at(0).toFloat());
        series->append(parts.at(1).toInt()+time,parts.at(0).toFloat());
        scatter->append(time,parts.at(0).toFloat());
        scatter->append(parts.at(1).toInt()+time,parts.at(0).toFloat());
        time=time+parts.at(1).toInt();
    }

    // CHART
    chart  = new QChart;
    chart->legend()->hide();
    chart->addSeries(series);
    chart->addSeries(scatter);
    chart->setTitle("CH1 CURRENT PROTOCOL");
    QFont titleFont("Times", 12, QFont::Bold);
    QFont axisFont("Times", 10, QFont::Bold);
    chart->setTitleFont(titleFont);

    QValueAxis *axisX = new QValueAxis();
    axisX->setTitleText("Time [ms]");
    axisX->setLabelFormat("%i");
    axisX->setTitleFont(axisFont);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText("Current [A]");
    axisY->setLabelFormat("%g");
    axisY->setTitleFont(axisFont);
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->resize(640,480);
    chartView->setWindowTitle("CH1 Current protocol");

    // TABLE
    tableWidget = new QTableWidget(this);
    tableWidget->setRowCount(list_puntos1.length());
    tableWidget->setColumnCount(2);
    int a = 0;
    foreach(auto &x,list_puntos1)
    {
        const QStringList parts = x.split(",");
        tableWidget->setItem(a,0,new QTableWidgetItem(parts.at(0)));
        tableWidget->setItem(a,1,new QTableWidgetItem(parts.at(1)));
        tableWidget->item(a,0)->setTextAlignment( Qt::AlignCenter);
        tableWidget->item(a,1)->setTextAlignment( Qt::AlignCenter);
        a++;
    }
    tableWidget->setHorizontalHeaderItem(0, new QTableWidgetItem("Current [A]"));
    tableWidget->setHorizontalHeaderItem(1, new QTableWidgetItem("Time [ms]"));
    tableWidget->setColumnWidth(0,110);
    tableWidget->setColumnWidth(1,110);
    // STACK
    QWidget *wdg = new QWidget;
    QGridLayout *mainLayout = new QGridLayout(wdg);
    mainLayout->addWidget(tableWidget, 1, 0);
    mainLayout->addWidget(chartView, 1, 1);
    mainLayout->setColumnStretch(1, 1);
    mainLayout->setColumnStretch(0, 0);
    wdg->resize(800,600);
    wdg->setWindowTitle("Chart");

    QString filename = QFileDialog::getSaveFileName(this, "DialogTitle", "chart.png", "PNG files (.png, *.png)", 0, 0); // getting the filename (full path)
    QFile data(filename);
    if(data.open(QFile::WriteOnly |QFile::Truncate))
    {
        wdg->grab().save(filename);
    }

}


void MainWindow::on_List_csv_pushButton_clicked()
{
    QString filename = QFileDialog::getSaveFileName(this, "DialogTitle", "filename.csv", "CSV files (.csv);;Zip files (.zip, *.7z)", 0, 0); // getting the filename (full path)
    QFile data(filename);
    if(data.open(QFile::WriteOnly |QFile::Truncate))
    {
    QTextStream output(&data);
    foreach(auto &x,list_puntos1)
        output<<x + "\r\n";
    }
}


/*/////  SINEWAVE [S]  //////*/


void MainWindow::on_SW_run_pushButton_clicked()
{
    if(serial->isOpen())
    {
        if(sw_output==false)
        {
            QString frequency= ui->SW_frequency_lineEdit->text();
            QString amplitude= ui->SW_amplitude_lineEdit->text();
            QString offset= ui->SW_offset_lineEdit->text();
            if(frequency.toDouble()<=100.0 && frequency.toDouble()>=0.1)
            {
                if (amplitude.toDouble()<=1.0 && amplitude.toDouble()>=0.0)
                {
                    if (amplitude.toDouble()+offset.toDouble()<=1.0 && -amplitude.toDouble()+offset.toDouble()>=-1.0)
                    {
                        QString frame="S " + frequency + " " + amplitude + " " + offset + "\r\n";
                        serial->write(frame.toUtf8());
                        ui->SW_run_pushButton->setText("STOP");
                        if(ui->checkBox_SW1->isChecked())
                        {
                            serial->write("S T 1\r\n");
                        }
                        else
                        {
                            serial->write("S 1\r\n");
                        }
                        sw_output=true;
                    }
                    else
                    {
                        QMessageBox::warning(this, tr("Sine Wave"), tr("Parameters out of bounds [Imax+offset<+-1.0]") );
                    }
                }
                else
                {
                    QMessageBox::warning(this, tr("Sine Wave"), tr("Parameters out of bounds [Imax<+-1.0]") );
                }
            }
            else
            {
                QMessageBox::warning(this, tr("Sine Wave"), tr("Parameters out of bounds [0.1<Fmax<100.0]") );
            }

        }
        else
        {
            serial->write("S 0\r\n");
            QThread::msleep(200);
            serial->write("S T 0\r\n");
            ui->SW_run_pushButton->setText("RUN");
            sw_output=false;
        }

    }
    else
    {
        QMessageBox::warning(this, tr("SERIAL"), tr("Select a COM port") );

    }
}


/*///// RAMP ///////*/


void MainWindow::on_R_add_pushButton_clicked()
{
    if(serial->isOpen())
    {
        QString setcurrent = ui->R_current_lineEdit->text();
        QString settime = ui->R_time_lineEdit->text();
        if(setcurrent.toDouble()<=1.0 && setcurrent.toDouble()>=-1.0)
        {
            QString frame = "R " + setcurrent +" " + settime + "\r\n";
            serial->write(frame.toUtf8());
            ramp_puntos1.append(setcurrent+","+settime);
        }
        else
        {
            QMessageBox::warning(this, tr("ERROR"), tr("Parameters out of bounds [Imax=+-1.0A]") );
        }
    }
    else
    {
        QMessageBox::warning(this, tr("SERIAL"), tr("Select a COM port") );

    }
}

void MainWindow::on_R_clear_pushButton_clicked()
{
    if(serial->isOpen())
    {
        serial->write("R 0 0\r\n");
        ramp_puntos1.clear();
    }
    else
    {
        QMessageBox::warning(this, tr("SERIAL"), tr("Select a COM port") );
    }
}

void MainWindow::on_R_csv_pushButton_clicked()
{
    QString filename = QFileDialog::getSaveFileName(this, "DialogTitle", "filename.csv", "CSV files (.csv);;Zip files (.zip, *.7z)", 0, 0); // getting the filename (full path)
    QFile data(filename);
    if(data.open(QFile::WriteOnly |QFile::Truncate))
    {
    QTextStream output(&data);
    foreach(auto &x,ramp_puntos1)
        output<<x + "\r\n";
    }
}

void MainWindow::on_R_graph_pushButton_clicked()
{

    // SERIE
    QLineSeries *series = new QtCharts::QLineSeries();
    QScatterSeries *scatter = new QScatterSeries();
    scatter->setName("scatter1");
    scatter->setMarkerShape(QScatterSeries::MarkerShapeCircle);
    scatter->setMarkerSize(10.0);
    int time=0;
    series->append(0,0);
    scatter->append(0,0);
    foreach(auto &x,ramp_puntos1)
    {
        const QStringList parts = x.split(",");
        series->append(parts.at(1).toInt()+time,parts.at(0).toFloat());
        scatter->append(parts.at(1).toInt()+time,parts.at(0).toFloat());
        time=time+parts.at(1).toInt();
    }

    // CHART
    chart  = new QChart;
    chart->legend()->hide();
    chart->addSeries(series);
    chart->addSeries(scatter);
    chart->setTitle("CH1 CURRENT RAMP PROTOCOL");
    QFont titleFont("Times", 12, QFont::Bold);
    QFont axisFont("Times", 10, QFont::Bold);
    chart->setTitleFont(titleFont);

    QValueAxis *axisX = new QValueAxis();
    axisX->setTitleText("Time [ms]");
    axisX->setLabelFormat("%i");
    axisX->setTitleFont(axisFont);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText("Current [A]");
    axisY->setLabelFormat("%g");
    axisY->setTitleFont(axisFont);
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->resize(640,480);
    chartView->setWindowTitle("CH1 CURRENT RAMP PROTOCO");

    //TABLE
    tableWidget = new QTableWidget(this);
    tableWidget->setRowCount(ramp_puntos1.length());
    tableWidget->setColumnCount(2);
    int a = 0;
    foreach(auto &x,ramp_puntos1)
    {
        const QStringList parts = x.split(",");
        tableWidget->setItem(a,0,new QTableWidgetItem(parts.at(0)));
        tableWidget->setItem(a,1,new QTableWidgetItem(parts.at(1)));
        tableWidget->item(a,0)->setTextAlignment( Qt::AlignCenter);
        tableWidget->item(a,1)->setTextAlignment( Qt::AlignCenter);
        a++;
    }
    tableWidget->setHorizontalHeaderItem(0, new QTableWidgetItem("Current [A]"));
    tableWidget->setHorizontalHeaderItem(1, new QTableWidgetItem("Time [ms]"));
    tableWidget->setColumnWidth(0,110);
    tableWidget->setColumnWidth(1,110);
    // STACK
    QWidget *wdg = new QWidget;
    QGridLayout *mainLayout = new QGridLayout(wdg);
    mainLayout->addWidget(tableWidget, 1, 0);
    mainLayout->addWidget(chartView, 1, 1);
    mainLayout->setColumnStretch(1, 1);
    mainLayout->setColumnStretch(0, 0);
    wdg->resize(800,600);
    wdg->setWindowTitle("Chart");
    wdg->show();


}
void MainWindow::on_R_save_graph_pushButton_clicked()
{

    // SERIE
    QLineSeries *series = new QtCharts::QLineSeries();
    QScatterSeries *scatter = new QScatterSeries();
    scatter->setName("scatter1");
    scatter->setMarkerShape(QScatterSeries::MarkerShapeCircle);
    scatter->setMarkerSize(10.0);
    int time=0;
    series->append(0,0);
    scatter->append(0,0);
    foreach(auto &x,ramp_puntos1)
    {
        const QStringList parts = x.split(",");
        series->append(parts.at(1).toInt()+time,parts.at(0).toFloat());
        scatter->append(parts.at(1).toInt()+time,parts.at(0).toFloat());
        time=time+parts.at(1).toInt();
    }

    // CHART
    chart  = new QChart;
    chart->legend()->hide();
    chart->addSeries(series);
    chart->addSeries(scatter);
    chart->setTitle("CH1 CURRENT RAMP PROTOCOL");
    QFont titleFont("Times", 12, QFont::Bold);
    QFont axisFont("Times", 10, QFont::Bold);
    chart->setTitleFont(titleFont);

    QValueAxis *axisX = new QValueAxis();
    axisX->setTitleText("Time [ms]");
    axisX->setLabelFormat("%i");
    axisX->setTitleFont(axisFont);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText("Current [A]");
    axisY->setLabelFormat("%g");
    axisY->setTitleFont(axisFont);
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->resize(640,480);
    chartView->setWindowTitle("CH1 CURRENT RAMP PROTOCO");

    //TABLE
    tableWidget = new QTableWidget(this);
    tableWidget->setRowCount(ramp_puntos1.length());
    tableWidget->setColumnCount(2);
    int a = 0;
    foreach(auto &x,ramp_puntos1)
    {
        const QStringList parts = x.split(",");
        tableWidget->setItem(a,0,new QTableWidgetItem(parts.at(0)));
        tableWidget->setItem(a,1,new QTableWidgetItem(parts.at(1)));
        tableWidget->item(a,0)->setTextAlignment( Qt::AlignCenter);
        tableWidget->item(a,1)->setTextAlignment( Qt::AlignCenter);
        a++;
    }
    tableWidget->setHorizontalHeaderItem(0, new QTableWidgetItem("Current [A]"));
    tableWidget->setHorizontalHeaderItem(1, new QTableWidgetItem("Time [ms]"));
    tableWidget->setColumnWidth(0,110);
    tableWidget->setColumnWidth(1,110);
    // STACK
    QWidget *wdg = new QWidget;
    QGridLayout *mainLayout = new QGridLayout(wdg);
    mainLayout->addWidget(tableWidget, 1, 0);
    mainLayout->addWidget(chartView, 1, 1);
    mainLayout->setColumnStretch(1, 1);
    mainLayout->setColumnStretch(0, 0);
    wdg->resize(800,600);
    wdg->setWindowTitle("Chart");
    QString filename = QFileDialog::getSaveFileName(this, "DialogTitle", "chart.png", "PNG files (.png, *.png)", 0, 0); // getting the filename (full path)
    QFile data(filename);
    if(data.open(QFile::WriteOnly |QFile::Truncate))
    {
        wdg->grab().save(filename);
    }


}

void MainWindow::on_R_RUN_pushButton_clicked()
{
    if(serial->isOpen())
    {
        if(ramp_output==false)
        {
            if(ui->checkBox_R1->isChecked())
            {
                serial->write("R T 1\r\n");
            }
            else
            {
                serial->write("R 1\r\n");
            }
            ui->R_RUN_pushButton->setText("STOP");
            ramp_output=true;
            foreach(auto &x,ramp_puntos1)
                qDebug()<<x;
        }
        else
        {
            serial->write("R 0\r\n");
            QThread::msleep(200);
            serial->write("R T 0\r\n");
            ui->R_RUN_pushButton->setText("RUN");
            ramp_output=false;
        }
    }
    else
    {
        QMessageBox::warning(this, tr("SERIAL"), tr("Select a COM port") );

    }
}



/*///////////////////*/
/*        CH2       */
/*//////////////////*/


/*/////  CURRENT CH2 [I]  //////*/
void MainWindow::on_Current_run_pushButton_2_clicked()
{
    if(serial->isOpen())
    {
        QString setcurrent = ui->Current_lineEdit_2->text();
        if(setcurrent.toDouble()<=1.0 && setcurrent.toDouble()>=-1.0)
        {
            QString frame = "I 2 " + setcurrent + "\r\n";
            serial->write(frame.toUtf8());
        }
        else
        {
            QMessageBox::warning(this, tr("ERROR"), tr("Parameters out of bounds [Imax=+-1.0A]") );
        }
    }
    else
    {
        QMessageBox::warning(this, tr("SERIAL"), tr("Select a COM port") );

    }
}
void MainWindow::on_Current_stop_pushButton_2_clicked()
{
    if(serial->isOpen())
    {
        QString frame = "I 2 0\r\n";
        serial->write(frame.toUtf8());

    }
    else
    {
        QMessageBox::warning(this, tr("SERIAL"), tr("Select a COM port") );

    }
}


/*/////  LIST CH2 [L]  //////*/
void MainWindow::on_List_add_pushButton_2_clicked()
{
    if(serial->isOpen())
    {
        QString setcurrent = ui->List_current_lineEdit_2->text();
        QString settime = ui->List_time_lineEdit_2->text();
        if(setcurrent.toDouble()<=1.0 && setcurrent.toDouble()>=-1.0)
        {
            QString frame = "L " + setcurrent +" " + settime + "\r\n";
            serial->write(frame.toUtf8());
            list_puntos2.append(setcurrent+","+settime);
        }
        else
        {
            QMessageBox::warning(this, tr("ERROR"), tr("Parameters out of bounds [Imax=+-1.0A]") );
        }
    }
    else
    {
        QMessageBox::warning(this, tr("SERIAL"), tr("Select a COM port") );

    }
}

void MainWindow::on_List_clear_pushButton_2_clicked()
{
    if(serial->isOpen())
    {
        serial->write("L 0 0\r\n");
        list_puntos2.clear();
    }
    else
    {
        QMessageBox::warning(this, tr("SERIAL"), tr("Select a COM port") );
    }
}

void MainWindow::on_List_run_pushButton_2_clicked()
{
    if(serial->isOpen())
    {
        if(list_output_2==false)
        {
            if(ui->checkBox_L2->isChecked())
            {
                serial->write("L T 2\r\n");
            }
            else
            {
                serial->write("L 2\r\n");
            }
            ui->List_run_pushButton_2->setText("STOP");
            list_output_2=true;
            foreach(auto &x,list_puntos2)
                qDebug()<<x;
        }
        else
        {
            serial->write("L 0\r\n");
            QThread::msleep(200);
            serial->write("L T 0\r\n");
            ui->List_run_pushButton_2->setText("RUN");
            list_output_2=false;
        }
    }
    else
    {
        QMessageBox::warning(this, tr("SERIAL"), tr("Select a COM port") );

    }
}

void MainWindow::on_List_csv_pushButton_2_clicked()
{
    QString filename = QFileDialog::getSaveFileName(this, "DialogTitle", "filename.csv", "CSV files (.csv);;Zip files (.zip, *.7z)", 0, 0); // getting the filename (full path)
    QFile data(filename);
    if(data.open(QFile::WriteOnly |QFile::Truncate))
    {
    QTextStream output(&data);
    foreach(auto &x,list_puntos2)
        output<<x + "\r\n";
    }
}

/*/  SHOW GRAPH /*/
void MainWindow::on_List_graph_pushButton_2_clicked()
{



    //SERIE
    QLineSeries *series2 = new QtCharts::QLineSeries();
    QScatterSeries *scatter2 = new QScatterSeries();
    scatter2->setName("scatter2");
    scatter2->setMarkerShape(QScatterSeries::MarkerShapeCircle);
    scatter2->setMarkerSize(10.0);
    int time=0;
    foreach(auto &x,list_puntos2)
    {
        const QStringList parts = x.split(",");
        series2->append(time,parts.at(0).toFloat());
        series2->append(parts.at(1).toInt()+time,parts.at(0).toFloat());
        scatter2->append(time,parts.at(0).toFloat());
        scatter2->append(parts.at(1).toInt()+time,parts.at(0).toFloat());
        time=time+parts.at(1).toInt();
    }

    // CHART
    chart  = new QChart;
    chart->legend()->hide();
    chart->addSeries(series2);
    chart->addSeries(scatter2);
    chart->setTitle("CH2 CURRENT PROTOCOL");
    QFont titleFont("Times", 12, QFont::Bold);
    QFont axisFont("Times", 10, QFont::Bold);
    chart->setTitleFont(titleFont);

    QValueAxis *axisX = new QValueAxis();
    axisX->setTitleText("Time [ms]");
    axisX->setLabelFormat("%i");
    axisX->setTitleFont(axisFont);
    chart->addAxis(axisX, Qt::AlignBottom);
    series2->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText("Current [A]");
    axisY->setLabelFormat("%g");
    axisY->setTitleFont(axisFont);
    chart->addAxis(axisY, Qt::AlignLeft);
    series2->attachAxis(axisY);

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->resize(640,480);
    chartView->setWindowTitle("CH2 Current protocol");

    // TABLE
    tableWidget = new QTableWidget(this);
    tableWidget->setRowCount(list_puntos2.length());
    tableWidget->setColumnCount(2);
    int a = 0;
    foreach(auto &x,list_puntos2)
    {
        const QStringList parts = x.split(",");
        tableWidget->setItem(a,0,new QTableWidgetItem(parts.at(0)));
        tableWidget->setItem(a,1,new QTableWidgetItem(parts.at(1)));
        tableWidget->item(a,0)->setTextAlignment( Qt::AlignCenter);
        tableWidget->item(a,1)->setTextAlignment( Qt::AlignCenter);
        a++;
    }
    tableWidget->setHorizontalHeaderItem(0, new QTableWidgetItem("Current [A]"));
    tableWidget->setHorizontalHeaderItem(1, new QTableWidgetItem("Time [ms]"));
    tableWidget->setColumnWidth(0,110);
    tableWidget->setColumnWidth(1,110);
    // STACK
    QWidget *wdg = new QWidget;
    QGridLayout *mainLayout = new QGridLayout(wdg);
    mainLayout->addWidget(tableWidget, 1, 0);
    mainLayout->addWidget(chartView, 1, 1);
    mainLayout->setColumnStretch(1, 1);
    mainLayout->setColumnStretch(0, 0);
    wdg->resize(800,600);
    wdg->setWindowTitle("Chart");
    wdg->show();

}


/*/  SAVE GRAPH /*/
void MainWindow::on_List_save_graph_pushButton_2_clicked()
{


    // SERIE
    QLineSeries *series2 = new QtCharts::QLineSeries();
    QScatterSeries *scatter2 = new QScatterSeries();
    scatter2->setName("scatter2");
    scatter2->setMarkerShape(QScatterSeries::MarkerShapeCircle);
    scatter2->setMarkerSize(10.0);
    int time=0;
    foreach(auto &x,list_puntos2)
    {
        const QStringList parts = x.split(",");
        series2->append(time,parts.at(0).toFloat());
        series2->append(parts.at(1).toInt()+time,parts.at(0).toFloat());
        scatter2->append(time,parts.at(0).toFloat());
        scatter2->append(parts.at(1).toInt()+time,parts.at(0).toFloat());
        time=time+parts.at(1).toInt();
    }

    // CHART
    chart  = new QChart;
    chart->legend()->hide();
    chart->addSeries(series2);
    chart->addSeries(scatter2);
    chart->setTitle("CH2 CURRENT PROTOCOL");
    QFont titleFont("Times", 12, QFont::Bold);
    QFont axisFont("Times", 10, QFont::Bold);
    chart->setTitleFont(titleFont);

    QValueAxis *axisX = new QValueAxis();
    axisX->setTitleText("Time [ms]");
    axisX->setLabelFormat("%i");
    axisX->setTitleFont(axisFont);
    chart->addAxis(axisX, Qt::AlignBottom);
    series2->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText("Current [A]");
    axisY->setLabelFormat("%g");
    axisY->setTitleFont(axisFont);
    chart->addAxis(axisY, Qt::AlignLeft);
    series2->attachAxis(axisY);

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->resize(640,480);
    chartView->setWindowTitle("CH2 Current protocol");

    // TABLE
    tableWidget = new QTableWidget(this);
    tableWidget->setRowCount(list_puntos2.length());
    tableWidget->setColumnCount(2);
    int a = 0;
    foreach(auto &x,list_puntos2)
    {
        const QStringList parts = x.split(",");
        tableWidget->setItem(a,0,new QTableWidgetItem(parts.at(0)));
        tableWidget->setItem(a,1,new QTableWidgetItem(parts.at(1)));
        tableWidget->item(a,0)->setTextAlignment( Qt::AlignCenter);
        tableWidget->item(a,1)->setTextAlignment( Qt::AlignCenter);
        a++;
    }
    tableWidget->setHorizontalHeaderItem(0, new QTableWidgetItem("Current [A]"));
    tableWidget->setHorizontalHeaderItem(1, new QTableWidgetItem("Time [ms]"));
    tableWidget->setColumnWidth(0,110);
    tableWidget->setColumnWidth(1,110);
    // STACK
    QWidget *wdg = new QWidget;
    QGridLayout *mainLayout = new QGridLayout(wdg);
    mainLayout->addWidget(tableWidget, 1, 0);
    mainLayout->addWidget(chartView, 1, 1);
    mainLayout->setColumnStretch(1, 1);
    mainLayout->setColumnStretch(0, 0);
    wdg->resize(800,600);
    wdg->setWindowTitle("Chart");

    // SAVE FIGURE
    QString filename = QFileDialog::getSaveFileName(this, "DialogTitle", "chart.png", "PNG files (.png, *.png)", 0, 0); // getting the filename (full path)
    QFile data(filename);
    if(data.open(QFile::WriteOnly |QFile::Truncate))
    {
        wdg->grab().save(filename);
    }

}


/*/////  SINEWAVE CH2 [S]  //////*/
void MainWindow::on_SW_run_pushButton_2_clicked()
{
    if(serial->isOpen())
    {
        if(sw_output_2==false)
        {
            QString frequency= ui->SW_frequency_lineEdit_2->text();
            QString amplitude= ui->SW_amplitude_lineEdit_2->text();
            QString offset= ui->SW_offset_lineEdit_2->text();
            if(frequency.toDouble()<=100.0 && frequency.toDouble()>=0.1)
            {
                if (amplitude.toDouble()<=1.0 && amplitude.toDouble()>=0.0)
                {
                    if (amplitude.toDouble()+offset.toDouble()<=1.0 && -amplitude.toDouble()+offset.toDouble()>=-1.0)
                    {
                        QString frame="S " + frequency + " " + amplitude + " " + offset + "\r\n";
                        serial->write(frame.toUtf8());
                        ui->SW_run_pushButton_2->setText("STOP");
                        if(ui->checkBox_SW2->isChecked())
                        {
                            serial->write("S T 2\r\n");
                        }
                        else
                        {
                            serial->write("S 2\r\n");
                        }
                        sw_output_2=true;
                    }
                    else
                    {
                        QMessageBox::warning(this, tr("Sine Wave"), tr("Parameters out of bounds [Imax+offset<+-1.0]") );
                    }
                }
                else
                {
                    QMessageBox::warning(this, tr("Sine Wave"), tr("Parameters out of bounds [Imax<+-1.0]") );
                }
            }
            else
            {
                QMessageBox::warning(this, tr("Sine Wave"), tr("Parameters out of bounds [0.1<Fmax<100.0]") );
            }

        }
        else
        {
            serial->write("S 0\r\n");
            QThread::msleep(200);
            serial->write("S T 0\r\n");
            ui->SW_run_pushButton_2->setText("RUN");
            sw_output_2=false;
        }

    }
    else
    {
        QMessageBox::warning(this, tr("SERIAL"), tr("Select a COM port") );

    }
}




/*///// RAMP ///////*/
void MainWindow::on_R_add_pushButton_2_clicked()
{
    if(serial->isOpen())
    {
        QString setcurrent = ui->R_current_lineEdit_2->text();
        QString settime = ui->R_time_lineEdit_2->text();
        if(setcurrent.toDouble()<=1.0 && setcurrent.toDouble()>=-1.0)
        {
            QString frame = "R " + setcurrent +" " + settime + "\r\n";
            serial->write(frame.toUtf8());
            ramp_puntos2.append(setcurrent+","+settime);
        }
        else
        {
            QMessageBox::warning(this, tr("ERROR"), tr("Parameters out of bounds [Imax=+-1.0A]") );
        }
    }
    else
    {
        QMessageBox::warning(this, tr("SERIAL"), tr("Select a COM port") );

    }
}

void MainWindow::on_R_clear_pushButton_2_clicked()
{
    if(serial->isOpen())
    {
        serial->write("R 0 0 \r\n");
        ramp_puntos2.clear();
    }
    else
    {
        QMessageBox::warning(this, tr("SERIAL"), tr("Select a COM port") );
    }
}

void MainWindow::on_R_csv_pushButton_2_clicked()
{
    QString filename = QFileDialog::getSaveFileName(this, "DialogTitle", "filename.csv", "CSV files (.csv);;Zip files (.zip, *.7z)", 0, 0); // getting the filename (full path)
    QFile data(filename);
    if(data.open(QFile::WriteOnly |QFile::Truncate))
    {
    QTextStream output(&data);
    foreach(auto &x,ramp_puntos2)
        output<<x + "\r\n";
    }
}

void MainWindow::on_R_graph_pushButton_2_clicked()
{

    // SERIE
    QLineSeries *series = new QtCharts::QLineSeries();
    QScatterSeries *scatter = new QScatterSeries();
    scatter->setName("scatter1");
    scatter->setMarkerShape(QScatterSeries::MarkerShapeCircle);
    scatter->setMarkerSize(10.0);
    int time=0;
    series->append(0,0);
    scatter->append(0,0);
    foreach(auto &x,ramp_puntos2)
    {
        const QStringList parts = x.split(",");
        series->append(parts.at(1).toInt()+time,parts.at(0).toFloat());
        scatter->append(parts.at(1).toInt()+time,parts.at(0).toFloat());
        time=time+parts.at(1).toInt();
    }

    // CHART
    chart  = new QChart;
    chart->legend()->hide();
    chart->addSeries(series);
    chart->addSeries(scatter);
    chart->setTitle("CH2 CURRENT RAMP PROTOCOL");
    QFont titleFont("Times", 12, QFont::Bold);
    QFont axisFont("Times", 10, QFont::Bold);
    chart->setTitleFont(titleFont);

    QValueAxis *axisX = new QValueAxis();
    axisX->setTitleText("Time [ms]");
    axisX->setLabelFormat("%i");
    axisX->setTitleFont(axisFont);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText("Current [A]");
    axisY->setLabelFormat("%g");
    axisY->setTitleFont(axisFont);
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->resize(640,480);
    chartView->setWindowTitle("CH2 CURRENT RAMP PROTOCO");

    // TABLE
    tableWidget = new QTableWidget(this);
    tableWidget->setRowCount(ramp_puntos2.length());
    tableWidget->setColumnCount(2);
    int a = 0;
    foreach(auto &x,ramp_puntos2)
    {
        const QStringList parts = x.split(",");
        tableWidget->setItem(a,0,new QTableWidgetItem(parts.at(0)));
        tableWidget->setItem(a,1,new QTableWidgetItem(parts.at(1)));
        tableWidget->item(a,0)->setTextAlignment( Qt::AlignCenter);
        tableWidget->item(a,1)->setTextAlignment( Qt::AlignCenter);
        a++;
    }
    tableWidget->setHorizontalHeaderItem(0, new QTableWidgetItem("Current [A]"));
    tableWidget->setHorizontalHeaderItem(1, new QTableWidgetItem("Time [ms]"));
    tableWidget->setColumnWidth(0,110);
    tableWidget->setColumnWidth(1,110);
    // STACK
    QWidget *wdg = new QWidget;
    QGridLayout *mainLayout = new QGridLayout(wdg);
    mainLayout->addWidget(tableWidget, 1, 0);
    mainLayout->addWidget(chartView, 1, 1);
    mainLayout->setColumnStretch(1, 1);
    mainLayout->setColumnStretch(0, 0);
    wdg->resize(800,600);
    wdg->setWindowTitle("Chart");
    wdg->show();


}
void MainWindow::on_R_save_graph_pushButton_2_clicked()
{

    // SERIE
    QLineSeries *series = new QtCharts::QLineSeries();
    QScatterSeries *scatter = new QScatterSeries();
    scatter->setName("scatter1");
    scatter->setMarkerShape(QScatterSeries::MarkerShapeCircle);
    scatter->setMarkerSize(10.0);
    int time=0;
    series->append(0,0);
    scatter->append(0,0);
    foreach(auto &x,ramp_puntos2)
    {
        const QStringList parts = x.split(",");
        series->append(parts.at(1).toInt()+time,parts.at(0).toFloat());
        scatter->append(parts.at(1).toInt()+time,parts.at(0).toFloat());
        time=time+parts.at(1).toInt();
    }

    //CHART
    chart  = new QChart;
    chart->legend()->hide();
    chart->addSeries(series);
    chart->addSeries(scatter);
    chart->setTitle("CH2 CURRENT RAMP PROTOCOL");
    QFont titleFont("Times", 12, QFont::Bold);
    QFont axisFont("Times", 10, QFont::Bold);
    chart->setTitleFont(titleFont);

    QValueAxis *axisX = new QValueAxis();
    axisX->setTitleText("Time [ms]");
    axisX->setLabelFormat("%i");
    axisX->setTitleFont(axisFont);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText("Current [A]");
    axisY->setLabelFormat("%g");
    axisY->setTitleFont(axisFont);
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->resize(640,480);
    chartView->setWindowTitle("CH2 CURRENT RAMP PROTOCO");

    // TABLE
    tableWidget = new QTableWidget(this);
    tableWidget->setRowCount(ramp_puntos2.length());
    tableWidget->setColumnCount(2);
    int a = 0;
    foreach(auto &x,ramp_puntos2)
    {
        const QStringList parts = x.split(",");
        tableWidget->setItem(a,0,new QTableWidgetItem(parts.at(0)));
        tableWidget->setItem(a,1,new QTableWidgetItem(parts.at(1)));
        tableWidget->item(a,0)->setTextAlignment( Qt::AlignCenter);
        tableWidget->item(a,1)->setTextAlignment( Qt::AlignCenter);
        a++;
    }
    tableWidget->setHorizontalHeaderItem(0, new QTableWidgetItem("Current [A]"));
    tableWidget->setHorizontalHeaderItem(1, new QTableWidgetItem("Time [ms]"));
    tableWidget->setColumnWidth(0,110);
    tableWidget->setColumnWidth(1,110);
    // STACK
    QWidget *wdg = new QWidget;
    QGridLayout *mainLayout = new QGridLayout(wdg);
    mainLayout->addWidget(tableWidget, 1, 0);
    mainLayout->addWidget(chartView, 1, 1);
    mainLayout->setColumnStretch(1, 1);
    mainLayout->setColumnStretch(0, 0);
    wdg->resize(800,600);
    wdg->setWindowTitle("Chart");


    QString filename = QFileDialog::getSaveFileName(this, "DialogTitle", "chart.png", "PNG files (.png, *.png)", 0, 0); // getting the filename (full path)
    QFile data(filename);
    if(data.open(QFile::WriteOnly |QFile::Truncate))
    {
        wdg->grab().save(filename);
    }

}

void MainWindow::on_R_RUN_pushButton_2_clicked()
{
    if(serial->isOpen())
    {
        if(ramp_output_2==false)
        {
            if(ui->checkBox_R2->isChecked())
            {
                serial->write("R T 2\r\n");
            }
            else
            {
                serial->write("R 2\r\n");
            }
            ui->R_RUN_pushButton_2->setText("STOP");
            ramp_output_2=true;
            foreach(auto &x,ramp_puntos2)
                qDebug()<<x;
        }
        else
        {
            serial->write("R 0\r\n");
            QThread::msleep(200);
            serial->write("R T 0\r\n");
            ui->R_RUN_pushButton_2->setText("RUN");
            ramp_output_2=false;
        }
    }
    else
    {
        QMessageBox::warning(this, tr("SERIAL"), tr("Select a COM port") );

    }
}















/*////////////////////////*/







