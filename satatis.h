#ifndef SATATIS_H
#define SATATIS_H
#include<QString>
#include<QWidget>
#include<unordered_map>
#include"io_base.h"
void drawGenderPieChart(QWidget *parent);
void drawAttendanceChart(QWidget *parent);
void drawBarChart(const QMap<QString, int> &departmentCount, QWidget *parent);
void createPieAgeChart(QVector<int>& differences, QWidget *parent);
void createPieChart(const std::unordered_map<int, int> &statuses, QWidget *parent);
void readRewardsAndCreateBarChartFromDb(QWidget *parent);
void createSalaryBarChart(const QMap<QString, int> &totalSalaries, QWidget *parent);
void createSalaryLastBarChart(QVector<Salary> &lastMonthSalaries,QWidget *parent);
void createOvertimeChart(const QMap<QString, int> &monthlySums, QWidget *parent);
QMap<QString, int> countEmployeesByDepartmentFromDb() ;
QVector<int> extractAndSubtractFrom2026() ;
std::unordered_map<int, int> countEmployeeStatusesFromDb();
QMap<QString, int> calculateTotalSalaryPerEmployeeFromDb();
QMap<QString, int> calculateMonthlyOvertimeSumsFromDb();
#endif // SATATIS_H
