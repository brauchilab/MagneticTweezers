#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSerialPort/QSerialPort>
#include <QThread>

#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts>
#include <QTableWidget>



namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    bool enable_output = false;
    bool list_output = false;
    bool list_output_2 = false;
    bool list_TTL_output = false;
    bool list_TTL_output_2 = false;
    bool sw_output = false;
    bool sw_output_2 = false;
    bool sw_TTL_output = false;
    bool sw_TTL_output_2 = false;
    bool ramp_output = false;
    bool ramp_output_2 = false;
    bool ramp_TTL_output = false;
    bool ramp_TTL_output_2 = false;
    QList<QString> list_puntos1;
    QList<QString> list_puntos2;
    QList<QString> ramp_puntos1;
    QList<QString> ramp_puntos2;


private slots:
    void on_refresh_pushButton_clicked();
    void on_port_comboBox_currentIndexChanged(const QString &arg1);
    void on_Enable_pushButton_clicked();
    void on_Current_run_pushButton_clicked();
    void on_Current_run_pushButton_2_clicked();

    void on_List_add_pushButton_clicked();

    void on_List_add_pushButton_2_clicked();

    void on_List_clear_pushButton_clicked();

    void on_List_clear_pushButton_2_clicked();

    void on_List_run_pushButton_clicked();

    void on_List_run_pushButton_2_clicked();

    void on_SW_run_pushButton_clicked();

    void on_SW_run_pushButton_2_clicked();


    void on_pushButton_clicked();

    void on_Current_stop_pushButton_clicked();

    void on_Current_stop_pushButton_2_clicked();


    void on_List_csv_pushButton_clicked();

    void on_List_graph_pushButton_clicked();

    void on_List_csv_pushButton_2_clicked();

    void on_List_graph_pushButton_2_clicked();

    void on_List_save_graph_pushButton_clicked();
    void on_List_save_graph_pushButton_2_clicked();

    void on_R_add_pushButton_clicked();

    void on_R_clear_pushButton_clicked();

    void on_R_csv_pushButton_clicked();

    void on_R_graph_pushButton_clicked();

    void on_R_RUN_pushButton_2_clicked();

    void on_R_save_graph_pushButton_clicked();

    void on_R_RUN_pushButton_clicked();

    void on_R_add_pushButton_2_clicked();

    void on_R_clear_pushButton_2_clicked();

    void on_R_csv_pushButton_2_clicked();

    void on_R_save_graph_pushButton_2_clicked();

    void on_R_graph_pushButton_2_clicked();


private:
    Ui::MainWindow *ui;
    QString clickedAnchor;
    QSerialPort *serial;
    QChart *chart;
    QChartView *chartView;
    QTableWidget *tableWidget;

};

#endif // MAINWINDOW_H

