#ifndef ENTITY_SALARY_H
#define ENTITY_SALARY_H

#include <QString>
#include "entity_common.h"

class Salary_base {
public:
    QString id;
    date time;
    double basic_salary;
    
    Salary_base(const QString& id, const QString& year, const QString& mon, double basic_salary)
        : id(id), time(year, mon), basic_salary(basic_salary) {}
    Salary_base() = default;
    virtual ~Salary_base() = default;
};

class Salary : public Salary_base {
public:
    double salary_position;
    double salary_Performance;
    double House;
    double reward;
    double pay_salary;
    double fund;
    double tax;
    double attend_redu;
    double real_salary;

    Salary(const QString& id, const QString& year, const QString& mon, double basic_salary,
           double salary_pos, double salary_perf, double house, double rew,
           double fund_val, double pay_salary, double tax_val, double attend_red, double real_sal)
        : Salary_base(id, year, mon, basic_salary)
        , salary_position(salary_pos), salary_Performance(salary_perf), House(house)
        , reward(rew), pay_salary(pay_salary), fund(fund_val), tax(tax_val)
        , attend_redu(attend_red), real_salary(real_sal) {}

    Salary(const QString& id, const QString& year, const QString& mon, double basic_salary, double salary_position)
        : Salary_base(id, year, mon, basic_salary), salary_position(salary_position)
    {
        salary_Performance = House = reward = pay_salary = fund = tax = attend_redu = real_salary = 0.0;
    }
    
    Salary() = default;
    ~Salary() override = default;

    double getSalary_Performance();
    double getHouse();
    double getReward();
    double getPay_salary();
    double getFund();
    double getTax();
    double getAttend_redu();
    double getReal_salary();
};

class Sal_Posi {
public:
    QString Dept;
    int Position;
    double salary_position;
    double department_coefficient;

    Sal_Posi(const QString& dept, int posi, double sal_pos, double dept_co)
        : Dept(dept), Position(posi), salary_position(sal_pos), department_coefficient(dept_co) {}
    Sal_Posi() = default;
    ~Sal_Posi() = default;
};

#endif // ENTITY_SALARY_H
