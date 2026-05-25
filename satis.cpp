#include <QApplication>
#include <QDebug>
#include "FindUser.h"
#include "inquiry_base_information.h"
#include "Filebackup.h"
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QChart>
#include <QtCharts/QPieSeries>
#include <satatis.h>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QBarSet>
#include <QtCharts/QHorizontalBarSeries>
#include <QLayout>
#include <iostream>
#include <QtCharts/QBarSeries>
#include <QtCharts/QValueAxis>
#include <QMap>
#include "io_base.h"
#include <QtMath>

QT_CHARTS_USE_NAMESPACE

// 1. 男女比例折线/扇形图
void drawGenderPieChart(QWidget *parent) {
    int maleCount = 0;
    int femaleCount = 0;

    QVector<Employee> employ = ReadEmployee();
    for (int i = 0; i < employ.size(); i++) {
        if (employ[i].sex == "男" && employ[i].Status == 1) {
            maleCount++;
        }
        if (employ[i].sex == "女" && employ[i].Status == 1) {
            femaleCount++;
        }
    }

    QPieSeries *series = new QPieSeries();
    QPieSlice *sliceMale = series->append("男性", maleCount);
    QPieSlice *sliceFemale = series->append("女性", femaleCount);
    sliceMale->setBrush(QColor("#3498db")); // 柔和蓝
    sliceFemale->setBrush(QColor("#e74c3c")); // 柔和红

    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("男女比例扇形图");
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);

    QChartView *chartView = new QChartView(chart, parent);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setGeometry(100, 80, 500, 400);
    chartView->show();
}

// 2. 总体考勤折线图
QList<QPair<int, QPair<int, int>>> readAttendanceDataFromDb() {
    QVector<Attendance> attends = ReadAttendance();
    QList<QPair<int, QPair<int, int>>> data;
    for (const Attendance& att : attends) {
        int month = att.time.mon.toInt();
        data.append(qMakePair(month, qMakePair(att.late, att.exit_early)));
    }
    return data;
}

void drawAttendanceChart(QWidget *parent) {
    QList<QPair<int, QPair<int, int>>> data = readAttendanceDataFromDb();
    if (data.isEmpty())
        return;

    QLineSeries *lateSeries = new QLineSeries();
    QLineSeries *leaveEarlySeries = new QLineSeries();

    for (const auto &pair : data) {
        lateSeries->append(pair.first, pair.second.first);
        leaveEarlySeries->append(pair.first, pair.second.second);
    }

    QChart *chart = new QChart();
    chart->addSeries(lateSeries);
    chart->addSeries(leaveEarlySeries);
    chart->setTitle("员工考勤折线图");
    chart->createDefaultAxes();

    lateSeries->setName("迟到次数");
    leaveEarlySeries->setName("早退次数");

    QStringList months;
    for (int i = 1; i <= 12; ++i)
        months << QString::number(i);

    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(months);
    chart->setAxisX(axisX, lateSeries);

    QChartView *chartView = new QChartView(chart, parent);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setGeometry(800, 900, 1000, 400);
    chartView->show();
}

// 3. 各部门人数
QMap<QString, int> countEmployeesByDepartmentFromDb() {
    QMap<QString, int> departmentCount;
    QVector<Employee> employees = ReadEmployee();
    for (const Employee& emp : employees) {
        if (emp.Status == 1) {
            departmentCount[emp.Dept]++;
        }
    }
    return departmentCount;
}

void drawBarChart(const QMap<QString, int> &departmentCount, QWidget *parent) {
    QtCharts::QHorizontalBarSeries *series = new QtCharts::QHorizontalBarSeries();
    QStringList colors = {"#3498db", "#2ecc71", "#e74c3c", "#f1c40f", "#9b59b6"};
    QStringList keys = departmentCount.keys();

    for (int i = 0; i < keys.size(); ++i) {
        QString dept = keys[i];
        QtCharts::QBarSet *set = new QtCharts::QBarSet(dept);
        for (int j = 0; j < keys.size(); ++j) {
            if (i == j) {
                *set << departmentCount[dept];
            } else {
                *set << 0;
            }
        }
        set->setColor(QColor(colors[i % colors.size()]));
        series->append(set);
    }

    QtCharts::QChart *chart = new QtCharts::QChart();
    chart->addSeries(series);
    chart->setTitle("各部门员工人数");
    chart->setAnimationOptions(QtCharts::QChart::SeriesAnimations);
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);

    QtCharts::QBarCategoryAxis *axisY = new QtCharts::QBarCategoryAxis();
    axisY->append(keys);
    chart->createDefaultAxes();
    chart->setAxisY(axisY, series);

    QtCharts::QChartView *chartView = new QtCharts::QChartView(chart, parent);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setGeometry(600, 0, 900, 500);
    chartView->show();
}

// 4. 年龄分布图 (基准年份 2026年)
QVector<int> extractAndSubtractFrom2026() {
    QVector<Employee> em = ReadEmployee();
    QVector<int> diff;
    for (int i = 0; i < em.size(); i++) {
        if (em[i].Status == 1) {
            QString first = em[i].Date_birth.left(4);
            int year = first.toUInt();
            int result = 2026 - year; // 统一升级为 2026
            diff.push_back(result);
        }
    }
    return diff;
}

void createPieAgeChart(QVector<int>& differences, QWidget* parent) {
    QtCharts::QPieSeries *series = new QtCharts::QPieSeries();
    QVector<int> intervals(5, 0); // 5个区间
    for (int diff : differences) {
        if (diff >= 0 && diff <= 45) {
            int index = diff / 9;
            if (index < intervals.size()) {
                intervals[index]++;
            }
        }
    }

    QStringList ageColors = {"#3498db", "#2ecc71", "#f1c40f", "#e67e22", "#9b59b6"};
    for (int i = 0; i < intervals.size(); ++i) {
        // 使用 2026年 作为基准进行区间文字展示
        QString label = QString("%1 - %2").arg(2026 - i * 9).arg(2026 - (i + 1) * 9 + 1);
        QPieSlice *slice = series->append(label, intervals[i]);
        if (i < ageColors.size()) {
            slice->setBrush(QColor(ageColors[i]));
        }
    }

    QtCharts::QChart *chart = new QtCharts::QChart();
    chart->addSeries(series);
    chart->setTitle("员工年龄分布（按九年区间）");
    chart->setAnimationOptions(QtCharts::QChart::SeriesAnimations);
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);

    QtCharts::QChartView *chartView = new QtCharts::QChartView(chart, parent);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setGeometry(600, 500, 900, 500);
    chartView->show();
}

// 5. 在职状态分布
std::unordered_map<int, int> countEmployeeStatusesFromDb() {
    std::unordered_map<int, int> statuses;
    statuses[1] = 0; // 在职
    statuses[0] = 0; // 不在职

    QVector<Employee> employees = ReadEmployee();
    for (const Employee& emp : employees) {
        if (emp.Status == 1 || emp.Status == 0) {
            statuses[emp.Status]++;
        }
    }
    return statuses;
}

void createPieChart(const std::unordered_map<int, int> &statuses, QWidget *parent) {
    QtCharts::QPieSeries *series = new QtCharts::QPieSeries();

    int inOffice = statuses.count(1) ? statuses.at(1) : 0;
    int notInOffice = statuses.count(0) ? statuses.at(0) : 0;
    QPieSlice *sliceActive = series->append("在职", inOffice);
    QPieSlice *sliceInactive = series->append("不在职", notInOffice);
    
    sliceActive->setBrush(QColor("#2ecc71")); // 翠绿色
    sliceInactive->setBrush(QColor("#95a5a6")); // 柔和灰

    QtCharts::QChart *chart = new QtCharts::QChart();
    chart->addSeries(series);
    chart->setTitle("员工在职状态");
    chart->setAnimationOptions(QtCharts::QChart::SeriesAnimations);
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);

    QtCharts::QChartView *chartView = new QtCharts::QChartView(chart, parent);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setGeometry(100, 500, 500, 400);
    chartView->show();
}

// 6. 薪酬奖励统计
void readRewardsAndCreateBarChartFromDb(QWidget* parent) {
    std::vector<int> rewardCounts(4, 0); // 4个区间: 0-499, 500-999, 1000-1499, 1500-2000

    QVector<Salary> salaries = ReadSalary();
    for (const Salary& sal : salaries) {
        // 过滤 4 月的奖励信息且数值在 0 - 10000 之间
        if (sal.time.mon == "4" && sal.reward >= 0 && sal.reward <= 10000) {
            int index = (int)sal.reward / 500;
            if (index < rewardCounts.size()) {
                rewardCounts[index]++;
            }
        }
    }

    QtCharts::QBarSeries *series = new QtCharts::QBarSeries();
    QStringList colors = {"#e74c3c", "#f1c40f", "#2ecc71", "#3498db"};
    QStringList categories;

    for (int i = 0; i < rewardCounts.size(); ++i) {
        QString category = QString("%1-%2").arg(i * 500).arg((i + 1) * 500 - 1);
        categories.append(category);
        
        QtCharts::QBarSet *barSet = new QtCharts::QBarSet(category);
        for (int j = 0; j < rewardCounts.size(); ++j) {
            if (i == j) {
                *barSet << rewardCounts[i];
            } else {
                *barSet << 0;
            }
        }
        barSet->setColor(QColor(colors[i % colors.size()]));
        series->append(barSet);
    }

    QtCharts::QChart *chart = new QtCharts::QChart();
    chart->addSeries(series);
    chart->setTitle("员工奖励区间人数 (4月份)");
    chart->setAnimationOptions(QtCharts::QChart::SeriesAnimations);

    QtCharts::QBarCategoryAxis *axisX = new QtCharts::QBarCategoryAxis();
    axisX->append(categories);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    QtCharts::QValueAxis *axisY = new QtCharts::QValueAxis();
    axisY->setTitleText("人数");
    int maxCount = *std::max_element(rewardCounts.begin(), rewardCounts.end());
    axisY->setRange(0, maxCount > 0 ? maxCount : 5);
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    QtCharts::QChartView *chartView = new QtCharts::QChartView(chart, parent);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setGeometry(0, 700, 800, 600);
    chartView->show();
}

// 7. 员工总薪资累计统计
QMap<QString, int> calculateTotalSalaryPerEmployeeFromDb() {
    QMap<QString, int> totalSalaries;
    QVector<Salary> salaries = ReadSalary();
    for (const Salary& sal : salaries) {
        totalSalaries[sal.id] += (int)sal.real_salary;
    }
    return totalSalaries;
}

void createSalaryBarChart(const QMap<QString, int> &totalSalaries, QWidget* parent) {
    std::vector<int> salaryCounts(6, 0); // 0-9999, 10000-19999, ..., 50000-59999

    for (QMap<QString, int>::const_iterator it = totalSalaries.cbegin(); it != totalSalaries.cend(); ++it) {
        int salary = it.value();
        if (salary >= 0 && salary < 60000) {
            int index = salary / 10000;
            if (index < salaryCounts.size()) {
                salaryCounts[index]++;
            }
        }
    }

    QtCharts::QBarSeries *series = new QtCharts::QBarSeries();
    QStringList colors = {"#2ecc71", "#3498db", "#9b59b6", "#f1c40f", "#e67e22", "#e74c3c"};
    QStringList categories;

    for (int i = 0; i < salaryCounts.size(); ++i) {
        QString category = QString("%1-%2").arg(i * 10000).arg((i + 1) * 10000 - 1);
        categories.append(category);

        QtCharts::QBarSet *barSet = new QtCharts::QBarSet(category);
        for (int j = 0; j < salaryCounts.size(); ++j) {
            if (i == j) {
                *barSet << salaryCounts[i];
            } else {
                *barSet << 0;
            }
        }
        barSet->setColor(QColor(colors[i % colors.size()]));
        series->append(barSet);
    }

    QtCharts::QChart *chart = new QtCharts::QChart();
    chart->addSeries(series);
    chart->setTitle("员工工资总额区间人数");
    chart->setAnimationOptions(QtCharts::QChart::SeriesAnimations);

    QtCharts::QBarCategoryAxis *axisX = new QtCharts::QBarCategoryAxis();
    axisX->append(categories);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    QtCharts::QValueAxis *axisY = new QtCharts::QValueAxis();
    axisY->setTitleText("人数");
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    QtCharts::QChartView *chartView = new QtCharts::QChartView(chart, parent);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setGeometry(700, 0, 1300, 400);
    chartView->show();
}

// 8. 上月工资区间人数统计
void createSalaryLastBarChart(QVector<Salary>& lastMonthSalaries, QWidget* parent) {
    int minSalary = INT_MAX;
    int maxSalary = INT_MIN;
    for (const Salary& salary : lastMonthSalaries) {
        if (salary.real_salary < minSalary)
            minSalary = salary.real_salary;
        if (salary.real_salary > maxSalary)
            maxSalary = salary.real_salary;
    }

    if (lastMonthSalaries.isEmpty()) {
        minSalary = 0;
        maxSalary = 10000;
    }

    int interval = 1500;
    int numIntervals = (maxSalary - minSalary) / interval + 1;
    if (numIntervals <= 0) numIntervals = 1;

    std::vector<int> salaryCounts(numIntervals, 0);
    for (const Salary& salary : lastMonthSalaries) {
        int index = (salary.real_salary - minSalary) / interval;
        if (index >= 0 && index < numIntervals) {
            salaryCounts[index]++;
        }
    }

    QtCharts::QBarSeries *series = new QtCharts::QBarSeries();
    QStringList colors = {"#3498db", "#2ecc71", "#e74c3c", "#f1c40f", "#9b59b6", "#e67e22"};
    QStringList categories;

    for (int i = 0; i < salaryCounts.size(); ++i) {
        QString category = QString("%1-%2").arg(minSalary + i * interval).arg(minSalary + (i + 1) * interval - 1);
        categories.append(category);

        QtCharts::QBarSet *barSet = new QtCharts::QBarSet(category);
        for (int j = 0; j < salaryCounts.size(); ++j) {
            if (i == j) {
                *barSet << salaryCounts[i];
            } else {
                *barSet << 0;
            }
        }
        barSet->setColor(QColor(colors[i % colors.size()]));
        series->append(barSet);
    }

    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("员工上月工资区间人数");
    chart->setAnimationOptions(QChart::SeriesAnimations);

    QtCharts::QBarCategoryAxis *axisX = new QtCharts::QBarCategoryAxis();
    axisX->append(categories);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    QtCharts::QValueAxis *axisY = new QtCharts::QValueAxis();
    axisY->setTitleText("人数");
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    QtCharts::QChartView *chartView = new QtCharts::QChartView(chart, parent);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setGeometry(800, 500, 1000, 400);
    chartView->show();
}

// 9. 月度加班时长统计
QMap<QString, int> calculateMonthlyOvertimeSumsFromDb() {
    QMap<QString, int> monthlySums;
    QVector<Attendance> attends = ReadAttendance();
    for (const Attendance& att : attends) {
        monthlySums[att.time.mon] += att.overtime;
    }
    return monthlySums;
}

void createOvertimeChart(const QMap<QString, int> &monthlySums, QWidget* parent) {
    QtCharts::QBarSeries *series = new QtCharts::QBarSeries();
    QStringList colors = {"#1abc9c", "#3498db", "#9b59b6", "#e67e22", "#e74c3c", "#2ecc71", "#f1c40f"};
    QStringList categories;
    
    int i = 0;
    for (auto it = monthlySums.constBegin(); it != monthlySums.constEnd(); ++it) {
        categories << it.key();
    }

    for (auto it = monthlySums.constBegin(); it != monthlySums.constEnd(); ++it) {
        QString month = it.key();
        int overtimeSum = it.value();
        
        QtCharts::QBarSet *barSet = new QtCharts::QBarSet(month);
        for (int j = 0; j < monthlySums.size(); ++j) {
            if (i == j) {
                *barSet << overtimeSum;
            } else {
                *barSet << 0;
            }
        }
        barSet->setColor(QColor(colors[i % colors.size()]));
        series->append(barSet);
        i++;
    }

    QtCharts::QChart *chart = new QtCharts::QChart();
    chart->addSeries(series);
    chart->setTitle("员工每月加班总时长");
    chart->setAnimationOptions(QtCharts::QChart::SeriesAnimations);

    QtCharts::QBarCategoryAxis *axisX = new QtCharts::QBarCategoryAxis();
    axisX->append(categories);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    QtCharts::QValueAxis *axisY = new QtCharts::QValueAxis();
    axisY->setTitleText("加班总时长");
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    QtCharts::QChartView *chartView = new QtCharts::QChartView(chart, parent);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setGeometry(0, 80, 700, 600);
    chartView->show();
}
