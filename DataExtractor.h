#ifndef DATAEXTRACTOR_H
#define DATAEXTRACTOR_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QVariant>
#include <QString>
#include <QMap>
#include <QFile>

class AbstractDataExtractor
{
public:
    virtual ~AbstractDataExtractor() {}
    virtual bool checkFile(const QString &filePath) = 0;
    virtual QList<QPair<QString, QString>> extractData(const QString& filePath) = 0;
};

class SqlDataExtractor : public AbstractDataExtractor
{
public:
    bool checkFile(const QString& filePath)
    {
        if (!QFile::exists(filePath)) {
            return false;
        }

        QSqlDatabase database = QSqlDatabase::addDatabase("QSQLITE");
        database.setDatabaseName(filePath);
        if (!database.open()) {
            return false;
        }

        QStringList tables = database.tables();
        database.close();

        return !tables.isEmpty();
    };

    QList<QPair<QString, QString>> extractData(const QString& filePath)
    {
        QList<QPair<QString, QString>> extractedData;

        QSqlDatabase database = QSqlDatabase::addDatabase("QSQLITE");
        database.setDatabaseName(filePath);
        database.open();

        QStringList tables = database.tables();
        QString tableName = tables.first();

        QSqlQuery query("SELECT * FROM " + tableName + " ");
        query.exec();

        QMap<QString, QPair<double, int>> groupedData;

        // Группируем данные по ключу и вычисляем сумму и количество значений
        while (query.next()) {
            QString unpreparedKey = query.value(0).toString();
            QString preparedKey = unpreparedKey.split(' ').first();
            double value = query.value(1).toDouble();

            if (groupedData.contains(preparedKey)) {
                QPair<double, int> pair = groupedData.value(preparedKey);
                double sum = pair.first;
                int count = pair.second;
                groupedData[preparedKey] = qMakePair(sum + value, count + 1);
            } else {
                groupedData[preparedKey] = qMakePair(value, 1);
            }
        }

        // Вычисляем среднее значение для каждого ключа
        for (const QString& date : groupedData.keys()) {
            QPair<double, int> pair = groupedData.value(date);
            double sum = pair.first;
            int count = pair.second;
            double average = sum / count;
            extractedData.append(qMakePair(date, QString::number(average)));
        }

        // Сортируем ключи по возрастанию для того, чтобы корректно построить диаграмму
        std::sort(extractedData.begin(), extractedData.end(), [](const QPair<QString, QString>& pair1, const QPair<QString, QString>& pair2) {
            QDate date1 = QDate::fromString(pair1.first, "dd.MM.yyyy");
            QDate date2 = QDate::fromString(pair2.first, "dd.MM.yyyy");

            return date1 < date2;
        });

        database.close();

        return extractedData;
    }
};

// Конкретная реализация DataExtractor для формата JSON
class JsonDataExtractor : public AbstractDataExtractor
{
public:
    bool checkFile(const QString& filePath)
    {

        QFile file(filePath);
        // Проверяем, существует ли файл и может ли он быть открыт для чтения
        if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
            return false;
        }

        // Читаем только необходимый минимум данных, чтобы проверить файл на валидность
        QByteArray jsonData = file.read(1024);
        file.close();

        QJsonParseError jsonError;
        // Пытаемся распарсить JSON
        QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData, &jsonError);

        if (jsonError.error != QJsonParseError::NoError) {
            return false;
        }

        // Получаем объект QJsonObject из jsonDoc
        QJsonObject jsonObj = jsonDoc.object();

        // Проверяем наличие ключа "data" и его типа
        if (!jsonObj.contains("data") || !jsonObj.value("data").isArray()) {
            return false;
        }

        return true;
    };

    QList<QPair<QString, QString>> extractData(const QString& filePath)
    {
        QList<QPair<QString, QString>> extractedData;
        // Открытие файла для чтения
        QFile file(filePath);
        file.open(QIODevice::ReadOnly);
        // Чтение содержимого файла в виде JSON-данных
        QByteArray jsonData = file.readAll();
        // Закрытие файла
        file.close();
        // Создание JSON-документа из прочитанных данных
        QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData);
        // Получение корневого объекта JSON
        QJsonObject jsonObj = jsonDoc.object();
        // Получение значения "data" из корневого объекта
        QJsonValue dataValue = jsonObj.value("data");
        // Преобразование значения "data" в массив JSON
        QJsonArray dataArray = dataValue.toArray();
        // Итерация по элементам массива
        for (const QJsonValue& itemValue : dataArray) {
            // Проверка, является ли элемент объектом
            if (itemValue.isObject()) {
                // Преобразование элемента в объект JSON
                QJsonObject itemObj = itemValue.toObject();
                // Проверка наличия ключа "key" и значения "value" в объекте
                if (itemObj.contains("key") && itemObj.contains("value")) {
                    // Извлечение значения ключа и значения из объекта
                    QString key = itemObj.value("key").toString();
                    QString value = itemObj.value("value").toVariant().toString();
                    // Добавляем пары ключ-значение в список extractedData
                    extractedData.append(qMakePair(key, value));
                }
            }
        }

        return extractedData;
    }
};

// Конкретная реализация DataExtractor для формата CSV
class CsvDataExtractor : public AbstractDataExtractor
{
public:
    bool checkFile(const QString& filePath)
    {
        // Создаем объект QFile для работы с файлом по указанному пути
        QFile file(filePath);
        // Открываем файл в режиме чтения и текстовом режиме
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            // Если не удалось открыть файл, возвращаем false
            return false;
        }
        // Создаем объект QTextStream для чтения данных из файла
        QTextStream in(&file);
        // Считываем первую строку файла, содержащую заголовки столбцов
        QString headerLine = in.readLine();
        file.close();

        // Разбиваем строку на отдельные заголовки с помощью разделителя ','
        QStringList headers = headerLine.split(',');

        // Проверяем наличие требуемых заголовков
        return (headers.contains("Key") && headers.contains("Value"));
    };

    QList<QPair<QString, QString>> extractData(const QString& filePath)
    {
        QList<QPair<QString, QString>> extractedData;
        // Создаем объект QFile для работы с файлом по указанному пути
        QFile file(filePath);
        // Открываем файл в режиме чтения и текстовом режиме
        file.open(QIODevice::ReadOnly | QIODevice::Text);
        // Создаем объект QTextStream для чтения данных из файла
        QTextStream in(&file);
        // Считываем первую строку файла, содержащую заголовки столбцов
        QString headerLine = in.readLine();
        // Разбиваем строку на отдельные заголовки с помощью разделителя ','
        QStringList headers = headerLine.split(',');
        // Находим индекс столбца "Key" в списке заголовков
        int keyIndex = headers.indexOf("Key");
        // Находим индекс столбца "Value" в списке заголовков
        int valueIndex = headers.indexOf("Value");
        // Пока не достигнут конец файла
        while (!in.atEnd()) {
            // Считываем строку из файла
            QString line = in.readLine();
            // Разбиваем строку на отдельные значения с помощью разделителя ','
            QStringList values = line.split(',');
            // Проверяем, что в строке достаточно значений для столбцов "Key" и "Value"
            if (values.size() > keyIndex && values.size() > valueIndex) {
                // Получаем значение столбца "Key" и удаляем лишние пробелы в начале и конце
                QString key = values[keyIndex].trimmed();
                // Получаем значение столбца "Value" и удаляем лишние пробелы в начале и конце
                QString value = values[valueIndex].trimmed();
                // Добавляем пары ключ-значение в список extractedData
                extractedData.append(qMakePair(key, value));
            }
        }

        file.close();
        return extractedData;
    }
};

#endif // DATAEXTRACTOR_H
