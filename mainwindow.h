#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "IOCContainer.h"
#include "DataExtractor.h"
#include "ChartDrawer.h"
#include <QMainWindow>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>
#include <QListView>
#include <QItemSelectionModel>
#include <QSplitter>
#include <QFileSystemModel>
#include <QChartView>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QList>
#include <QGraphicsColorizeEffect>
#include <QPdfWriter>
#include <QPainter>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void errorMessageReceived(QString text);

public slots:
    void openFolder();
    void handleFileSelectionChanged(const QItemSelection&);
    void changeChartType(const QString&);
    void printErrorLabel(QString);
    void updateChartColorMode(bool);
    void exportChart();

private:
    std::unique_ptr<QPushButton> openFolderButton;
    std::unique_ptr<QLabel> chartTypeLabel;
    std::unique_ptr<QLabel> errorLabel;
    std::unique_ptr<QChartView> chartView;
    std::unique_ptr<QComboBox> chartTypeComboBox;        // Список диаграмм
    std::unique_ptr<QCheckBox> BWCheckbox;               // Black-white вид
    std::unique_ptr<QPushButton> exportButton;
    std::unique_ptr<QListView> fileListView;
    std::unique_ptr<QWidget> chartViewWidget;
    std::shared_ptr<QFileSystemModel> fileSystemModel;   // Модель файловой системы для QListView
    std::unique_ptr<QVBoxLayout> layout;                 // Обертка для QLabel и QChartView
    std::unique_ptr<QSplitter> splitter;                // Разделитель
    std::unique_ptr<DataExtractorInterface> dataExtractor;
    std::shared_ptr<AbstractChartRenderer> chartRenderer;
    QList<QPair<QString, QString>> extractedData;
    QString selectedFilePath;
    QItemSelectionModel* ListSelectionModel;
    bool isChartRendered;
    IOCContainer container;
};

#endif // MAINWINDOW_H
