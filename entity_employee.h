#ifndef ENTITY_EMPLOYEE_H
#define ENTITY_EMPLOYEE_H

#include <QString>

enum class PositionLevel {
    Staff = 1,
    Manager = 2
};

enum class EmployeeStatus {
    Resigned = 0,
    Active = 1
};

class People {
public:
    QString id;
    QString name;
    QString sex;

    People() : id("0"), name("0"), sex("0") {}
    People(const QString& id, const QString& name, const QString& sex)
        : id(id), name(name), sex(sex) {}
        
    virtual ~People() = default;

    [[nodiscard]] virtual QString getEntityDescription() const = 0;
};

class Employee : public People {
public:
    QString Dept;
    int Position;
    QString Address;
    QString Date_birth;
    QString Phone;
    QString Email;
    QString Password;
    int Status;

    Employee(const QString& id, const QString& name, const QString& sex, const QString& dept,
             const int position, const QString& address, const QString& date_birth, 
             const QString& phone, const QString& email, const QString& password, int status)
        : People(id, name, sex), Dept(dept), Position(position), Address(address), 
          Date_birth(date_birth), Phone(phone), Email(email), Password(password), Status(status) {}
          
    Employee() = default;
    ~Employee() override = default;

    [[nodiscard]] QString getEntityDescription() const override {
        return QString("Employee [ID: %1, Name: %2, Dept: %3]").arg(id, name, Dept);
    }
    
    [[nodiscard]] bool isActive() const { return Status == static_cast<int>(EmployeeStatus::Active); }
    [[nodiscard]] bool isManager() const { return Position == static_cast<int>(PositionLevel::Manager); }
};

#endif // ENTITY_EMPLOYEE_H
