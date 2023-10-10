#include "mainwindow.h"

int IOCContainer::s_nextTypeId = 1;

MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent) {
    selectedFilePath = "";
    isChartRendered = false;

    openFolderButton = std::make_unique<QPushButton>("Открыть папку", this);
    openFolderButton->setStyleSheet("border: 1px solid black; border-radius: 5px; padding: 5px;");

    chartTypeLabel = std::make_unique<QLabel>("Тип диаграммы:", this);
    chartTypeLabel->setAlignment(Qt::AlignHCenter);;
    chartTypeLabel->setStyleSheet("border: 1px solid black; border-radius: 5px; padding: 5px;");

    chartTypeComboBox = std::make_unique<QComboBox>(this);
    chartTypeComboBox->addItem("Столбчатая диаграмма");
    chartTypeComboBox->addItem("Круговая диаграмма");
    chartTypeComboBox->addItem("Горизонтальная столбчатая диаграмма");
    chartTypeComboBox->setStyleSheet("border: 1px solid black; border-radius: 5px; padding: 5px;");

    BWCheckbox = std::make_unique<QCheckBox>("Черно-белая диаграмма", this);
    BWCheckbox->setStyleSheet("border: 1px solid black; border-radius: 5px; padding: 5px;");

    exportButton = std::make_unique<QPushButton>("Экспорт", this);
    exportButton->setStyleSheet("border: 1px solid black; border-radius: 5px; padding: 5px;");

    // Список файлов
    fileListView = std::make_unique<QListView>(this);
    fileListView->setMinimumWidth(100);
    fileListView->resize(350, 0);
    fileListView->setStyleSheet("border: 1px solid black; border-radius: 5px; padding: 5px;");

    // Создаем виджет, в который вкладывается QVBoxLayout, уже в который будут вкладываться отображение диаграмм
    chartViewWidget = std::make_unique<QWidget>(this);
    chartViewWidget->setMinimumWidth(100);
    chartViewWidget->resize(674, 0);

    // Модель файловой системы для QListView
    fileSystemModel = std::make_shared<QFileSystemModel>(this);

    // Разделитель
    splitter = std::make_unique<QSplitter>(Qt::Horizontal, this);
    splitter->setChildrenCollapsible(false);
    splitter->addWidget(fileListView.get());
    splitter->addWidget(chartViewWidget.get());
    splitter->setHandleWidth(1);

    // Layout, в котором будут отображаться QChartView и QLabel
    layout = std::make_unique<QVBoxLayout>();
    chartView = std::make_unique<QChartView>(this);
    errorLabel = std::make_unique<QLabel>(this);
    errorLabel->setAlignment(Qt::AlignHCenter | Qt::AlignCenter);
    errorLabel->setVisible(false);
    layout->addWidget(errorLabel.get());
    layout->addWidget(chartView.get());
    chartViewWidget->setLayout(layout.get());

    // QHBoxLayout и элементы управления
    std::unique_ptr<QHBoxLayout> topLayout = std::make_unique<QHBoxLayout>();
    topLayout->addWidget(openFolderButton.get());
    topLayout->addWidget(chartTypeLabel.get());
    topLayout->addWidget(chartTypeComboBox.get());
    topLayout->addWidget(BWCheckbox.get());
    topLayout->addWidget(exportButton.get());

    // QVBoxLayout и добавляем QHBoxLayout и splitter
    std::unique_ptr<QVBoxLayout> mainLayout = std::make_unique<QVBoxLayout>();
    mainLayout->addLayout(topLayout.release());
    mainLayout->addWidget(splitter.release());

    // QWidget в качестве центрального виджета и устанавливаем QVBoxLayout в него
    std::unique_ptr<QWidget> centralWidget = std::make_unique<QWidget>(this);
    centralWidget->setLayout(mainLayout.release());
    setCentralWidget(centralWidget.release());

    setMinimumSize(800, 600);
    resize(1024, 768);


    connect(openFolderButton.get(), &QPushButton::clicked, this, &MainWindow::openFolder);
    connect(this, SIGNAL(errorMessageReceived(QString)), this, SLOT(printErrorLabel(QString)));
    connect(chartTypeComboBox.get(), SIGNAL(currentTextChanged(const QString&)), this,
            SLOT(changeChartType(const QString&)));
    connect(BWCheckbox.get(), &QCheckBox::stateChanged, this, &MainWindow::updateChartColorMode);
    connect(exportButton.get(), &QPushButton::clicked, this, &MainWindow::exportChart);
}

MainWindow::~MainWindow() {}

void MainWindow::openFolder() {
    QString folderPath = QFileDialog::getExistingDirectory(this, "Выберите папку", QDir::homePath());
    QDir folderDir(folderPath);
    if (folderDir.exists()) {
        if (!folderDir.isEmpty()) {
            fileSystemModel->setFilter(QDir::NoDotAndDotDot | QDir::Files);
            fileSystemModel->setRootPath(folderPath);

            fileListView->setModel(fileSystemModel.get());
            fileListView->setRootIndex(fileSystemModel->index(folderPath));

            ListSelectionModel = fileListView->selectionModel();
            connect(ListSelectionModel, &QItemSelectionModel::selectionChanged, this,
                    &MainWindow::handleFileSelectionChanged);
        } else {
            emit errorMessageReceived("Указанная папка пуста");
        }
    } else {
        emit errorMessageReceived("Указанная папка не существует");
    }
}

void MainWindow::handleFileSelectionChanged(const QItemSelection &selected) {
    if (!selected.isEmpty()) {
        QModelIndex selectedIndex = selected.indexes().first();

        selectedFilePath = fileSystemModel->filePath(selectedIndex);

        QFileInfo fileInfo(selectedFilePath);
        QString fileExtension = fileInfo.suffix();

        if (fileExtension == "sqlite") {
            dataExtractor = std::make_unique<SqlDataExtractor>();
        } else if (fileExtension == "json") {
            dataExtractor = std::make_unique<JsonDataExtractor>();
        } else if (fileExtension == "csv") {
            dataExtractor = std::make_unique<CsvDataExtractor>();
        } else {
            dataExtractor = nullptr;
            emit errorMessageReceived("Неподдерживаемый тип файла");
            return;
        }

        if (dataExtractor->checkFile(selectedFilePath)) {
            extractedData = dataExtractor->extractData(selectedFilePath);
            // Мгновенная отрисовка диаграммы выбранного типа при выборе файла
            changeChartType(chartTypeComboBox->currentText());
        } else {
            emit errorMessageReceived("Произошла ошибка при проверке файла");
        }
    }
}

void MainWindow::changeChartType(const QString &type) {
    if (selectedFilePath.isEmpty()) {
        return;
    }
    if (type == "Столбчатая диаграмма") {
        container.RegisterFactory<AbstractChartRenderer, BarChartRenderer>();
    } else if (type == "Круговая диаграмма") {
        container.RegisterFactory<AbstractChartRenderer, PieChartRenderer>();
    } else if (type == "Горизонтальная столбчатая диаграмма") {
        container.RegisterFactory<AbstractChartRenderer, HorizontalBarChartRenderer>();
    }
    chartRenderer = container.GetObject<AbstractChartRenderer>();

    if (chartRenderer) {
        if (errorLabel) {
            errorLabel->setVisible(false);
        }
        chartView->setVisible(true);
        chartRenderer->renderChart(extractedData, chartView);
        isChartRendered = true;
    } else {
        emit errorMessageReceived("Невозможно создать объект диаграммы");
        isChartRendered = false;
    }
}

void MainWindow::printErrorLabel(QString text) {
    if (chartView) {
        chartView->setVisible(false);
    }
    errorLabel->setVisible(true);
    errorLabel->setText(text);
}

void MainWindow::updateChartColorMode(bool isChecked) {
    if (chartView) {
        if (isChecked) {
            std::unique_ptr<QGraphicsColorizeEffect> effect = std::make_unique<QGraphicsColorizeEffect>();
            effect->setColor(Qt::black);
            chartView->chart()->setGraphicsEffect(effect.get());
            // Освобождаем умный указатель
            effect.release();
        } else {
            chartView->chart()->setGraphicsEffect(nullptr);
        }
    }
}


void MainWindow::exportChart() {
    if (!chartView || !isChartRendered) {
        return;
    }

    QString filePath = QFileDialog::getSaveFileName(nullptr, "Экспорт диаграммы", "", "PDF (*.pdf)");
    if (filePath.isEmpty()) {
        return;
    }

    QPdfWriter pdfWriter(filePath);
    QPainter painter(&pdfWriter);

    chartView->render(&painter);
    painter.end();
}
