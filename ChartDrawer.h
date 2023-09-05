#ifndef CHARTDRAWER_H
#define CHARTDRAWER_H

#include <QChartView>
#include <memory>
#include <string>
#include <QPieSeries>
#include <QPieSlice>
#include <QBarSeries>
#include <QBarSet>
#include <QLineSeries>
#include <QHorizontalBarSeries>
#include <QFileDialog>
#include <QMessageBox>
#include <QPdfWriter>

using namespace QtCharts;

class AbstractChartRenderer {
public:
    virtual ~AbstractChartRenderer() {}

    void renderChart(QList<QPair<QString, QString>> extractedData, std::unique_ptr<QChartView> &chartView) {
        chartView->chart()->removeAllSeries();
        setupChartTitle(chartView);
        createSeries(extractedData, chartView);
        setupChartOptions(chartView);
        chartView->setRenderHint(QPainter::Antialiasing);
        chartView->update();
    }

protected:
    void setupChartOptions(std::unique_ptr<QChartView> &chartView) {
        chartView->chart()->setAnimationOptions(QChart::SeriesAnimations);
    };

    virtual void setupChartTitle(std::unique_ptr<QChartView> &chartView) = 0;

    virtual void createSeries(QList<QPair<QString, QString>> extractedData, std::unique_ptr<QChartView> &chartView) = 0;
};

class PieChartRenderer : public AbstractChartRenderer {
protected:
    void setupChartTitle(std::unique_ptr<QChartView> &chartView) override {
        chartView->chart()->setTitle("Круговая диаграмма");
    }

    void createSeries(QList<QPair<QString, QString>> extractedData, std::unique_ptr<QChartView> &chartView) override {
        std::unique_ptr<QPieSeries> series = std::make_unique<QPieSeries>();
        for (const QPair<QString, QString> &pair: extractedData) {
            QString key = pair.first;
            qreal value = pair.second.toDouble();
            series->append(key, value);
        }
        // Освобождаем указатель
        chartView->chart()->addSeries(series.release());
    }
};

class BarChartRenderer : public AbstractChartRenderer {
protected:
    void setupChartTitle(std::unique_ptr<QChartView> &chartView) override {
        chartView->chart()->setTitle("Столбчатая диаграмма");
    }

    void createSeries(QList<QPair<QString, QString>> extractedData, std::unique_ptr<QChartView> &chartView) override {
        std::unique_ptr<QBarSeries> series(new QBarSeries());
        for (const QPair<QString, QString> &pair: extractedData) {
            QString key = pair.first;
            qreal value = pair.second.toDouble();

            std::unique_ptr<QBarSet> barSet(new QBarSet(key));
            *barSet << value;
            series->append(barSet.release());
        }
        // Освобождаем указатель
        chartView->chart()->addSeries(series.release());
    }
};

class HorizontalBarChartRenderer : public AbstractChartRenderer {
protected:
    void setupChartTitle(std::unique_ptr<QChartView> &chartView) override {
        chartView->chart()->setTitle("Столбчатая горизонтальная диаграмма");
    }

    void createSeries(QList<QPair<QString, QString>> extractedData, std::unique_ptr<QChartView> &chartView) override {
        std::unique_ptr<QHorizontalBarSeries> series(new QHorizontalBarSeries());
        for (const QPair<QString, QString> &pair: extractedData) {
            QString key = pair.first;
            qreal value = pair.second.toDouble();

            std::unique_ptr<QBarSet> barSet(new QBarSet(key));
            *barSet << value;
            series->append(barSet.release());
        }
        // Освобождаем указатель
        chartView->chart()->addSeries(series.release());
    }
};

#endif // CHARTDRAWER_H
