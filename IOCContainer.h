#include <functional>
#include <iostream>
#include <memory>
#include <map>
#include <string>
using namespace std;

class IOCContainer
{
    static int s_nextTypeId;
    template<typename T>
    static int GetTypeID() {
        static int typeId = s_nextTypeId++;
        return typeId;
    }

public:
    //Создание typeid для типа
    /*
     * В предлагаемой реализации контейнера IOC  есть статическая целочисленная переменная,
     * указывающая идентификатор следующего типа, который будет выделен,
     * и экземпляр статической локальной переменной для каждого типа,
     * доступ к которому можно получить, вызвав метод GetTypeID.
    */

    /*
     * Получение экземпляров объекта
     * Теперь, когда у нас есть идентификатор типа,
     * мы должны иметь возможность хранить какой-то фабричный объект,
     * чтобы представить тот факт, что мы не знаем, как создать этот объект.
     * Поскольку я хочу хранить все фабрики в одной коллекции,
     * я выбираю абстрактный базовый класс, от которого будут производными фабрики,
     * и реализацию, которая фиксирует функтор для последующего вызова.
     * Для краткости я использовал std::map для хранения фабрик, однако
     * вы можете рассмотреть и другие варианты для повышения эффективности.
    */

    class FactoryRoot
    {
    public:
        virtual ~FactoryRoot() {}
    };


    std::map<int, std::shared_ptr<FactoryRoot>> m_factories;

    //Получить экземпляр объекта

    /*
     * Класс CFactory<T> представляет фабрику для типа T.
     * Он хранит функтор m_functor, который может создавать экземпляры объектов типа T.
     * Метод GetObject() вызывает функтор и возвращает созданный объект.
    */

    template<typename T>
    class CFactory : public FactoryRoot
    {
        std::function<std::shared_ptr<T>()> m_functor;

    public:
        ~CFactory() {}

        CFactory(std::function<std::shared_ptr<T>()> functor)
            : m_functor(functor)
        {}

        std::shared_ptr<T> GetObject() {
            return m_functor();
        }
    };

    template<typename T>
    std::shared_ptr<T> GetObject() {
        auto typeId = GetTypeID<T>();
        auto factoryBase = m_factories[typeId];
        auto factory = std::static_pointer_cast<CFactory<T>>(factoryBase);
        return factory->GetObject();
    }

    //Регистрация экземпляров

    /*
     * Метод RegisterFunctor() регистрирует функтор для создания объекта типа TInterface.
     * Функтор принимает параметры TS... и возвращает std::shared_ptr<TInterface>.
     * Создается экземпляр CFactory<TInterface>, который использует переданный функтор для создания объекта.
    */

    template<typename TInterface, typename... TS>
    void RegisterFunctor(
        std::function<std::shared_ptr<TInterface>(std::shared_ptr<TS>... ts)> functor) {
        m_factories[GetTypeID<TInterface>()] = std::make_shared<CFactory<TInterface>>(
            [ = ] { return functor(GetObject<TS>()...); });
    }

    //Регистрация одного экземпляра объекта

    /*
     * Метод RegisterInstance() регистрирует существующий экземпляр объекта типа TInterface в контейнере.
     * Создается экземпляр CFactory<TInterface>, который всегда возвращает переданный экземпляр объекта.
    */

    template<typename TInterface>
    void RegisterInstance(std::shared_ptr<TInterface> t) {
        m_factories[GetTypeID<TInterface>()] = std::make_shared<CFactory<TInterface>>(
            [ = ] { return t; });
    }

    //Подаем указатель на функцию

    /*
     * Метод RegisterFunctor() регистрирует функцию, возвращающую std::shared_ptr<TInterface>,
     * как функтор для создания объекта типа TInterface.
    */

    template<typename TInterface, typename... TS>
    void RegisterFunctor(std::shared_ptr<TInterface> (*functor)(std::shared_ptr<TS>... ts)) {
        RegisterFunctor(
            std::function<std::shared_ptr<TInterface>(std::shared_ptr<TS>... ts)>(functor));
    }

    //Фабрика, которая будет вызывать конструктор, для каждого экземпляра

    /*
     * Метод RegisterFactory() регистрирует фабрику, которая будет вызывать конструктор объекта типа TConcrete.
     * Фабрика принимает параметры TArguments... и создает объект типа TConcrete с помощью std::make_shared<TConcrete>().
    */

    template<typename TInterface, typename TConcrete, typename... TArguments>
    void RegisterFactory() {
        RegisterFunctor(
            std::function<std::shared_ptr<TInterface>(std::shared_ptr<TArguments>... ts)>(
                [](std::shared_ptr<TArguments>... arguments) -> std::shared_ptr<TInterface> {
                    return std::make_shared<TConcrete>(
                        std::forward<std::shared_ptr<TArguments>>(arguments)...);
                }));
    }

    //Фабрика, которая будет возвращать один экземпляр

    /*
     * Метод RegisterInstance() регистрирует существующий экземпляр объекта типа TConcrete,
     * созданный с помощью std::make_shared<TConcrete>(GetObject<TArguments>()...),
     * как фабрику для типа TInterface.
     * Этот метод используется для регистрации единственного экземпляра объекта.
    */

    template<typename TInterface, typename TConcrete, typename... TArguments>
    void RegisterInstance() {
        RegisterInstance<TInterface>(std::make_shared<TConcrete>(GetObject<TArguments>()...));
    }
};
