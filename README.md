# chart-drawer
Лабораторная работа #3 по предмету "Технология разработки программного обеспечения" 

# ТЗ
Разработка приложения печати графиков.

Дано: предложен начальный вариант архитектуры ПО, в которую требуется внести изменения с целью снижения связности архитектуры. Используется принцип внедрения зависимости. Реализация внедрения зависимости с помощью IOC контейнера.

При разработке архитектуры учесть

Возможность добавления новых графиков (графики отличаются видом и данными Изменение визуального стиля графиков (цветной, черно белый). Общие требования к GUI

Загружаем данные, путем выбора нужного файла. Данные в ПО не отображаем, отображаем только график, построенный относительно считанных данных. При печати в pdf выбираем место сохранения графика.

Часть №1 Рассмотрение формата исходный данных данных. Варианты представления исходных входных данных.

Исходные данные для печати соответствуют некоторому типу, который определятся пользователем.

Данные определенного типа могут отображаться конкретным графиком, который ориентирован на этот тип данных.

Примеры данных.
1) Данные характеризуются парой [дата,значение], хранятся в БД SQLite (архив с данными прилагается).
2) Данные представлены JSON файлом( Примеры сгенерировать самостоятельно). Формат данных [дата,значение].

Общий функционал такой – пользователь выбирает папку для просмотра в левой части, а в правой отображается ее содержимое. Данное приложение реализовано с помощью концепции MVC. В предложенной реализации используется готовая модель файловой системы(QFileSystemModel). Отображать модель будем с помощью двух представлений: QTreeView (левая часть) и QTableView (правая часть). Для реализации реакции отображения содержимого папки относительно выбранной папки используем метод selectionModel (treeView->selectionModel()) рассматриваемого представления, также используем слот on_selectionChangedSlot для обработки выбора элемента в TreeView. Данный слот связываем с сигналом selectionChanged(const QItemSelection &, const QItemSelection &), который вызывается тогда, когда осуществляется выбор элемента в TreeView.
