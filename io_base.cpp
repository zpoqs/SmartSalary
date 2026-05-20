#include "io_base.h"
#include <QFile>
#include <QFileInfo>
#include <QCoreApplication>
#include <QDebug>
#include <QDateTime>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QSettings>
#include "FindUser.h"
#include "check.h"
#include <QDir>
#include <QSet>

// ============================================================
// 薪智汇控系统数据持久层 (SQLite Data Layer)
// 架构设计: 基于轻量级本地嵌入式数据库 SQLite 进行持久化存储
// ============================================================


QString findCsvFile(const QString& filename) {
    if (QFile::exists(filename)) return filename;
    QString path1 = QCoreApplication::applicationDirPath() + "/" + filename;
    if (QFile::exists(path1)) return path1;
    QString parent1 = QFileInfo(QCoreApplication::applicationDirPath()).absolutePath();
    QString path2 = parent1 + "/" + filename;
    if (QFile::exists(path2)) return path2;
    QString parent2 = QFileInfo(parent1).absolutePath();
    QString path3 = parent2 + "/" + filename;
    if (QFile::exists(path3)) return path3;
    QString path4 = "E:/project_team_softeg/SmartSalary/" + filename;
    if (QFile::exists(path4)) return path4;
    return filename;
}


static QString findConfigFile() {
    QString filename = "config.ini";
    if (QFile::exists(filename)) return filename;
    QString path1 = QCoreApplication::applicationDirPath() + "/" + filename;
    if (QFile::exists(path1)) return path1;
    QString parent1 = QFileInfo(QCoreApplication::applicationDirPath()).absolutePath();
    QString path2 = parent1 + "/" + filename;
    if (QFile::exists(path2)) return path2;
    QString parent2 = QFileInfo(parent1).absolutePath();
    QString path3 = parent2 + "/" + filename;
    if (QFile::exists(path3)) return path3;
    QString path4 = "E:/project_team_softeg/SmartSalary/" + filename;
    if (QFile::exists(path4)) return path4;
    return filename;
}

bool InitDatabase()
{
    QSqlDatabase db;
    if (QSqlDatabase::contains("qt_sql_default_connection")) {
        db = QSqlDatabase::database("qt_sql_default_connection");
    } else {
        QSettings settings(findConfigFile(), QSettings::IniFormat);
        QString engine = settings.value("Database/Engine", "SQLite").toString().trimmed();
        bool useMysql = (engine.compare("MySQL", Qt::CaseInsensitive) == 0);
        bool mysqlConnected = false;

        if (useMysql) {
            // 尝试加载 MySQL 驱动并连接
            if (QSqlDatabase::isDriverAvailable("QMYSQL")) {
                db = QSqlDatabase::addDatabase("QMYSQL");
                db.setHostName(settings.value("Database/MysqlHost", "localhost").toString());
                db.setPort(settings.value("Database/MysqlPort", 3306).toInt());
                db.setDatabaseName(settings.value("Database/MysqlDbName", "salary_system").toString());
                db.setUserName(settings.value("Database/MysqlUser", "root").toString());
                db.setPassword(settings.value("Database/MysqlPassword", "").toString());
                
                if (db.open()) {
                    mysqlConnected = true;
                    qDebug() << "[Database Engine] ✅ MySQL 数据库主引擎连接成功！";
                } else {
                    qWarning() << "[Database Engine] ⚠️ MySQL 握手失败 (" << db.lastError().text() 
                               << ")。可能原因：服务未开启、密码错误或库不存在。";
                }
            } else {
                qWarning() << "[Database Engine] ⚠️ Qt 环境中缺失 QMYSQL 驱动！无法连接到 MySQL 服务。";
            }

            if (!mysqlConnected) {
                qWarning() << "[Database Engine] ⚠️ 触发灾备降级策略 (Failover Fallback) -> 切换至本地 SQLite 引擎。";
                // 清理并移除原有的连接，以便重新建立 SQLite 连接
                db = QSqlDatabase();
                QSqlDatabase::removeDatabase(QSqlDatabase::defaultConnection);
            }
        }

        // 如果不使用 MySQL 或者 MySQL 连接失败，降级为 SQLite
        if (!mysqlConnected) {
            db = QSqlDatabase::addDatabase("QSQLITE");
            QString sqlitePath = settings.value("Database/SqliteDbName", "salary_system.db").toString();
            if (QFileInfo(sqlitePath).isRelative()) {
                QString srcDir = "E:/project_team_softeg/SmartSalary";
                if (QDir(srcDir).exists()) {
                    sqlitePath = srcDir + "/" + sqlitePath;
                } else {
                    sqlitePath = QCoreApplication::applicationDirPath() + "/" + sqlitePath;
                }
            }
            db.setDatabaseName(sqlitePath);
            if (!db.open()) {
                qCritical() << "[Database Engine] ❌ SQLite 降级容灾连接失败:" << db.lastError().text();
                return false;
            }
            qDebug() << "[Database Engine] ✅ SQLite 容灾备份引擎启动成功！系统具备 100% 可用性。本地路径:" << sqlitePath;
        }
    }

    QSqlQuery query;
    
    // 兼容两端引擎的标准 SQL 建表语句 (使用 VARCHAR 和 DOUBLE)
    query.exec("CREATE TABLE IF NOT EXISTS employee ("
               "id VARCHAR(50) PRIMARY KEY, "
               "name VARCHAR(100), "
               "sex VARCHAR(10), "
               "dept VARCHAR(100), "
               "position INT, "
               "address VARCHAR(255), "
               "date_birth VARCHAR(50), "
               "phone VARCHAR(50), "
               "email VARCHAR(100), "
               "password VARCHAR(100), "
               "status INT)");

    // 2. 创建工资表
    query.exec("CREATE TABLE IF NOT EXISTS salary ("
               "id VARCHAR(50), "
               "year VARCHAR(10), "
               "mon VARCHAR(10), "
               "basic_salary DOUBLE, "
               "salary_position DOUBLE, "
               "salary_performance DOUBLE, "
               "house DOUBLE, "
               "reward DOUBLE, "
               "pay_salary DOUBLE, "
               "fund DOUBLE, "
               "tax DOUBLE, "
               "attend_redu DOUBLE, "
               "real_salary DOUBLE, "
               "PRIMARY KEY(id, year, mon))");

    // 3. 创建考勤表
    query.exec("CREATE TABLE IF NOT EXISTS attendance ("
               "id VARCHAR(50), "
               "late INT, "
               "exit_early INT, "
               "year VARCHAR(10), "
               "mon VARCHAR(10), "
               "overtime INT, "
               "PRIMARY KEY(id, year, mon))");

    // 4. 创建岗位工资表
    query.exec("CREATE TABLE IF NOT EXISTS position_salary ("
               "dept VARCHAR(100), "
               "position INT, "
               "salary_position DOUBLE, "
               "department_coefficient DOUBLE, "
               "PRIMARY KEY(dept, position))");

    // 检查是否为空，如果为空则进行迁移或灌入初始演示数据
    query.exec("SELECT COUNT(*) FROM employee");
    if (query.next() && query.value(0).toInt() == 0) {
        qDebug() << "[Database] 检测到数据库表为空，正在尝试从本地 CSV 数据进行首次导入/迁移...";
        
        bool employeeLoaded = false;
        
        // 1. 尝试导入 员工信息表.csv
        QFile empFile(findCsvFile("员工信息表.csv"));
        if (empFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&empFile);
            in.setCodec("GBK");
            in.setAutoDetectUnicode(true);
            if (!in.atEnd()) in.readLine(); // 跳过标题行
            
            QSqlDatabase::database().transaction();
            while (!in.atEnd()) {
                QString line = in.readLine().trimmed();
                if (line.isEmpty()) continue;
                QStringList parts = line.split(",");
                if (parts.size() >= 11) {
                    QSqlQuery ins;
                    ins.prepare("INSERT INTO employee (id, name, sex, dept, position, address, date_birth, phone, email, password, status) "
                                "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
                    ins.addBindValue(parts[0].trimmed());
                    ins.addBindValue(parts[1].trimmed());
                    ins.addBindValue(parts[2].trimmed());
                    ins.addBindValue(parts[3].trimmed());
                    ins.addBindValue(parts[4].toInt());
                    ins.addBindValue(parts[5].trimmed());
                    ins.addBindValue(parts[6].trimmed());
                    ins.addBindValue(parts[7].trimmed());
                    ins.addBindValue(parts[8].trimmed());
                    ins.addBindValue(parts[9].trimmed());
                    ins.addBindValue(parts[10].toInt());
                    ins.exec();
                }
            }
            QSqlDatabase::database().commit();
            empFile.close();
            employeeLoaded = true;
            qDebug() << "[Database] 员工信息表.csv 迁移成功！";
        }
        
        // 2. 尝试导入 岗位工资表.csv
        QFile posFile(findCsvFile("岗位工资表.csv"));
        if (posFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&posFile);
            in.setCodec("GBK");
            in.setAutoDetectUnicode(true);
            if (!in.atEnd()) in.readLine(); // 跳过标题行
            
            QSqlDatabase::database().transaction();
            while (!in.atEnd()) {
                QString line = in.readLine().trimmed();
                if (line.isEmpty()) continue;
                QStringList parts = line.split(",");
                if (parts.size() >= 4) {
                    QSqlQuery ins;
                    ins.prepare("INSERT INTO position_salary (dept, position, salary_position, department_coefficient) VALUES (?, ?, ?, ?)");
                    ins.addBindValue(parts[0].trimmed());
                    ins.addBindValue(parts[1].toInt());
                    ins.addBindValue(parts[2].toDouble());
                    ins.addBindValue(parts[3].toDouble());
                    ins.exec();
                }
            }
            QSqlDatabase::database().commit();
            posFile.close();
            qDebug() << "[Database] 岗位工资表.csv 迁移成功！";
        }
        
        // 3. 尝试导入 员工考勤表.csv
        QFile attFile(findCsvFile("员工考勤表.csv"));
        if (attFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&attFile);
            in.setCodec("GBK");
            in.setAutoDetectUnicode(true);
            if (!in.atEnd()) in.readLine(); // 跳过标题行
            
            QSqlDatabase::database().transaction();
            while (!in.atEnd()) {
                QString line = in.readLine().trimmed();
                if (line.isEmpty()) continue;
                QStringList parts = line.split(",");
                if (parts.size() >= 6) {
                    QSqlQuery ins;
                    ins.prepare("INSERT INTO attendance (id, late, exit_early, year, mon, overtime) VALUES (?, ?, ?, ?, ?, ?)");
                    ins.addBindValue(parts[0].trimmed());
                    ins.addBindValue(parts[1].toInt());
                    ins.addBindValue(parts[2].toInt());
                    ins.addBindValue(parts[3].trimmed());
                    ins.addBindValue(parts[4].trimmed());
                    ins.addBindValue(parts[5].toInt());
                    ins.exec();
                }
            }
            QSqlDatabase::database().commit();
            attFile.close();
            qDebug() << "[Database] 员工考勤表.csv 迁移成功！";
        }
        
        // 4. 尝试导入 工资表.csv
        QFile salFile(findCsvFile("工资表.csv"));
        if (salFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&salFile);
            in.setCodec("GBK");
            in.setAutoDetectUnicode(true);
            if (!in.atEnd()) in.readLine(); // 跳过标题行
            
            QSqlDatabase::database().transaction();
            while (!in.atEnd()) {
                QString line = in.readLine().trimmed();
                if (line.isEmpty()) continue;
                QStringList parts = line.split(",");
                if (parts.size() >= 13) {
                    QSqlQuery ins;
                    ins.prepare("INSERT INTO salary (id, year, mon, basic_salary, salary_position, salary_performance, house, reward, pay_salary, fund, tax, attend_redu, real_salary) "
                                "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
                    ins.addBindValue(parts[0].trimmed());
                    ins.addBindValue(parts[1].trimmed());
                    ins.addBindValue(parts[2].trimmed());
                    ins.addBindValue(parts[3].toDouble());
                    ins.addBindValue(parts[4].toDouble());
                    ins.addBindValue(parts[5].toDouble());
                    ins.addBindValue(parts[6].toDouble());
                    ins.addBindValue(parts[7].toDouble());
                    ins.addBindValue(parts[8].toDouble());
                    ins.addBindValue(parts[9].toDouble());
                    ins.addBindValue(parts[10].toDouble());
                    ins.addBindValue(parts[11].toDouble());
                    ins.addBindValue(parts[12].toDouble());
                    ins.exec();
                }
            }
            QSqlDatabase::database().commit();
            salFile.close();
            qDebug() << "[Database] 工资表.csv 迁移成功！";
        }
        
        // 如果本地没有 CSV 员工数据，则使用原演示数据保底
        if (!employeeLoaded) {
            qDebug() << "[Database] 未找到本地员工 CSV 数据，正在生成系统默认演示数据...";
            
            // 插入岗位信息
            query.exec("INSERT INTO position_salary VALUES('技术部', 2, 8500.0, 1.2)");
            query.exec("INSERT INTO position_salary VALUES('技术部', 1, 5500.0, 1.2)");
            query.exec("INSERT INTO position_salary VALUES('市场部', 2, 7500.0, 1.1)");
            query.exec("INSERT INTO position_salary VALUES('市场部', 1, 4500.0, 1.1)");
            query.exec("INSERT INTO position_salary VALUES('人事部', 2, 6500.0, 1.0)");
            query.exec("INSERT INTO position_salary VALUES('人事部', 1, 4000.0, 1.0)");
            query.exec("INSERT INTO position_salary VALUES('财务部', 2, 7000.0, 1.0)");
            query.exec("INSERT INTO position_salary VALUES('财务部', 1, 4200.0, 1.0)");

            // 插入员工信息（含管理员和普通员工）
            query.exec("INSERT INTO employee VALUES('E001', '张三', '男', '技术部', 2, '北京市朝阳区', '1995-03-12', '13811112222', 'zsq@company.com', '123456', 1)");
            query.exec("INSERT INTO employee VALUES('E002', '李四', '女', '财务部', 2, '上海市徐汇区', '1996-07-24', '13922223333', 'liuke@company.com', '123456', 1)");
            query.exec("INSERT INTO employee VALUES('E003', '王五', '男', '技术部', 1, '广州市天河区', '1998-11-05', '13733334444', 'wgs@company.com', '123456', 1)");
            query.exec("INSERT INTO employee VALUES('E004', '赵六', '男', '市场部', 1, '深圳市南山区', '1997-04-18', '13644445555', 'chengfei@company.com', '123456', 1)");
            query.exec("INSERT INTO employee VALUES('E005', '钱七', '女', '人事部', 1, '杭州市西湖区', '1999-09-30', '13555556666', 'yanjie@company.com', '123456', 1)");
            query.exec("INSERT INTO employee VALUES('E006', '孙八', '男', '市场部', 2, '南京市玄武区', '1994-01-15', '13466667777', 'gaohang@company.com', '123456', 1)");

            // 插入考勤信息 (2026年5月)
            query.exec("INSERT INTO attendance VALUES('E001', 0, 0, '2026', '5', 12)");
            query.exec("INSERT INTO attendance VALUES('E002', 1, 0, '2026', '5', 2)");
            query.exec("INSERT INTO attendance VALUES('E003', 2, 1, '2026', '5', 8)");
            query.exec("INSERT INTO attendance VALUES('E004', 0, 3, '2026', '5', 15)");
            query.exec("INSERT INTO attendance VALUES('E005', 0, 0, '2026', '5', 0)");
            query.exec("INSERT INTO attendance VALUES('E006', 1, 1, '2026', '5', 5)");

            // 插入上月工资记录 (2026年4月)
            query.exec("INSERT INTO salary VALUES('E001', '2026', '4', 6000, 8500, 2040, 500, 300, 17340, 2080, 2150, 0, 13110)");
            query.exec("INSERT INTO salary VALUES('E002', '2026', '4', 5500, 7000, 1400, 500, 200, 14600, 1752, 1344, 50, 11454)");
            query.exec("INSERT INTO salary VALUES('E003', '2026', '4', 4500, 5500, 1320, 400, 100, 11820, 1418, 770, 150, 9482)");
            query.exec("INSERT INTO salary VALUES('E004', '2026', '4', 4000, 4500, 990, 400, 150, 10040, 1204, 560, 200, 8076)");
            query.exec("INSERT INTO salary VALUES('E005', '2026', '4', 3500, 4000, 800, 300, 0, 8600, 1032, 350, 0, 7218)");
            query.exec("INSERT INTO salary VALUES('E006', '2026', '4', 5000, 7500, 1650, 500, 200, 14850, 1782, 1410, 80, 11578)");

            // 插入当月初始数据 (2026年5月)
            query.exec("INSERT INTO salary VALUES('E001', '2026', '5', 6000, 8500, 2040, 500, 400, 17440, 2092, 2180, 0, 13168)");
            query.exec("INSERT INTO salary VALUES('E002', '2026', '5', 5500, 7000, 1400, 500, 100, 14500, 1740, 1314, 50, 11396)");
            query.exec("INSERT INTO salary VALUES('E003', '2026', '5', 4500, 5500, 1320, 400, 200, 11920, 1430, 790, 180, 9520)");
            query.exec("INSERT INTO salary VALUES('E004', '2026', '5', 4000, 4500, 990, 400, 300, 10190, 1222, 590, 150, 8228)");
            query.exec("INSERT INTO salary VALUES('E005', '2026', '5', 3500, 4000, 800, 300, 100, 8700, 1044, 360, 0, 7296)");
            query.exec("INSERT INTO salary VALUES('E006', '2026', '5', 5000, 7500, 1650, 500, 150, 14800, 1776, 1390, 90, 11544)");
        }
    }
    return true;
}

// ============================================================
// 员工数据读写
// ============================================================

Employee GetEmployee(QString line)
{
    // 保留接口兼容，实际走数据库后此函数不再被外部调用
    Q_UNUSED(line);
    return Employee();
}

QVector<Employee> ReadEmployee()
{
    QVector<Employee> employees;
    QSqlQuery query("SELECT id, name, sex, dept, position, address, date_birth, phone, email, password, status FROM employee");
    while (query.next()) {
        Employee emp(
            query.value(0).toString(),
            query.value(1).toString(),
            query.value(2).toString(),
            query.value(3).toString(),
            query.value(4).toInt(),
            query.value(5).toString(),
            query.value(6).toString(),
            query.value(7).toString(),
            query.value(8).toString(),
            query.value(9).toString(),
            query.value(10).toInt()
        );
        employees.push_back(emp);
    }
    return employees;
}

int WriteEmployeeToFile(QVector<Employee> user)
{
    // Check for ID uniqueness in the input list first
    QSet<QString> seenIds;
    for (const Employee& emp : user) {
        if (seenIds.contains(emp.id)) {
            qWarning() << "[Database Error] Duplicate ID in list:" << emp.id;
            return -2; // duplicate ID error
        }
        seenIds.insert(emp.id);
    }

    QSqlDatabase::database().transaction();
    QSqlQuery query;
    query.exec("DELETE FROM employee");
    for (const Employee& emp : user) {
        query.prepare("INSERT INTO employee (id, name, sex, dept, position, address, date_birth, phone, email, password, status) "
                      "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
        query.addBindValue(emp.id);
        query.addBindValue(emp.name);
        query.addBindValue(emp.sex);
        query.addBindValue(emp.Dept);
        query.addBindValue(emp.Position);
        query.addBindValue(emp.Address);
        query.addBindValue(emp.Date_birth);
        query.addBindValue(emp.Phone);
        query.addBindValue(emp.Email);
        query.addBindValue(emp.Password);
        query.addBindValue(emp.Status);
        if (!query.exec()) {
            qDebug() << "[Database Error] WriteEmployee:" << query.lastError().text();
            QSqlDatabase::database().rollback();
            return 0; // write failed
        }
    }
    QSqlDatabase::database().commit();
    return 1;
}

bool AddInfoToFile(const QString& path_from, const QString& path_to)
{
    // 保留接口兼容性
    Q_UNUSED(path_from);
    Q_UNUSED(path_to);
    return true;
}

// ============================================================
// 工资数据读写
// ============================================================

Salary GetSalary(QString line)
{
    Q_UNUSED(line);
    return Salary();
}

QVector<Salary> ReadSalary()
{
    QVector<Salary> salaries;
    QSqlQuery query("SELECT id, year, mon, basic_salary, salary_position, salary_performance, "
                    "house, reward, pay_salary, fund, tax, attend_redu, real_salary FROM salary");
    while (query.next()) {
        Salary sal(
            query.value(0).toString(),
            query.value(1).toString(),
            query.value(2).toString(),
            query.value(3).toDouble(),
            query.value(4).toDouble(),
            query.value(5).toDouble(),
            query.value(6).toDouble(),
            query.value(7).toDouble(),
            query.value(9).toDouble(), // fund
            query.value(8).toDouble(), // pay_salary
            query.value(10).toDouble(), // tax
            query.value(11).toDouble(), // attend_redu
            query.value(12).toDouble()  // real_salary
        );
        salaries.push_back(sal);
    }
    return salaries;
}

int WriteSalaryToFile(QVector<Salary> salary)
{
    QSqlDatabase::database().transaction();
    QSqlQuery query;
    query.exec("DELETE FROM salary");
    for (const Salary& sal : salary) {
        query.prepare("INSERT INTO salary (id, year, mon, basic_salary, salary_position, salary_performance, "
                      "house, reward, pay_salary, fund, tax, attend_redu, real_salary) "
                      "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
        query.addBindValue(sal.id);
        query.addBindValue(sal.time.year);
        query.addBindValue(sal.time.mon);
        query.addBindValue(sal.basic_salary);
        query.addBindValue(sal.salary_position);
        query.addBindValue(sal.salary_Performance);
        query.addBindValue(sal.House);
        query.addBindValue(sal.reward);
        query.addBindValue(sal.pay_salary);
        query.addBindValue(sal.fund);
        query.addBindValue(sal.tax);
        query.addBindValue(sal.attend_redu);
        query.addBindValue(sal.real_salary);
        query.exec();
    }
    QSqlDatabase::database().commit();
    return 1;
}

void AddSalaryToFile()
{
    QDateTime currentTime = QDateTime::currentDateTime();
    QDate currentDate = currentTime.date();
    int day = currentDate.day();
    QString mon_now = QString::number(currentDate.month());
    int result = Check_Mon_Once(mon_now);
    
    if (day == 12 && result == 0) {
        QVector<Salary> sal_this_mon = Salary_New_Mon();
        AddNewPerSalary(sal_this_mon);
    }
}

QString SalaryToString(Salary& salary)
{
    return QString("%1,%2,%3,%4,%5,%6,%7,%8,%9,%10,%11,%12,%13")
        .arg(salary.id)
        .arg(salary.time.year)
        .arg(salary.time.mon)
        .arg(salary.basic_salary)
        .arg(salary.salary_position)
        .arg(salary.salary_Performance)
        .arg(salary.House)
        .arg(salary.reward)
        .arg(salary.pay_salary)
        .arg(salary.fund)
        .arg(salary.tax)
        .arg(salary.attend_redu)
        .arg(salary.real_salary);
}

QVector<Salary> Salary_New_Mon()
{
    QDateTime currentTime = QDateTime::currentDateTime();
    QDate currentDate = currentTime.date();
    QString year = QString::number(currentDate.year());
    QString mon_pre = QString::number(currentDate.month() - 1);
    QString mon_now = QString::number(currentDate.month());
    
    QVector<Salary> sal_premon;
    if (mon_now == "1") {
        sal_premon = FindSalary_Mon(QString::number(currentDate.year() - 1), "12");
    } else {
        sal_premon = FindSalary_Mon(year, mon_pre);
    }
    
    QVector<Salary> sal_nowmon;
    for (int i = 0; i < sal_premon.size(); i++) {
        const Salary& sa = sal_premon[i];
        Employee em = FindUserByidInEmp(sa.id);
        if (em.Status == 1) {
            Salary sa_now(sa.id, year, mon_now, sa.basic_salary, sa.salary_position);
            sa_now.getReward();
            sa_now.getHouse();
            sa_now.getAttend_redu();
            sa_now.getPay_salary();
            sa_now.getFund();
            sa_now.getSalary_Performance();
            sa_now.getTax();
            sa_now.getReal_salary();

            sal_nowmon.push_back(sa_now);
        } else if (em.id == "") {
            break;
        }
    }
    return sal_nowmon;
}

void AddNewPerSalary(QVector<Salary> sa_user)
{
    QSqlQuery query;
    for (const Salary& sal : sa_user) {
        query.prepare("INSERT OR REPLACE INTO salary (id, year, mon, basic_salary, salary_position, salary_performance, "
                      "house, reward, pay_salary, fund, tax, attend_redu, real_salary) "
                      "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
        query.addBindValue(sal.id);
        query.addBindValue(sal.time.year);
        query.addBindValue(sal.time.mon);
        query.addBindValue(sal.basic_salary);
        query.addBindValue(sal.salary_position);
        query.addBindValue(sal.salary_Performance);
        query.addBindValue(sal.House);
        query.addBindValue(sal.reward);
        query.addBindValue(sal.pay_salary);
        query.addBindValue(sal.fund);
        query.addBindValue(sal.tax);
        query.addBindValue(sal.attend_redu);
        query.addBindValue(sal.real_salary);
        query.exec();
    }
}

QVector<Salary> Per_Sal_New()
{
    QDateTime currentTime = QDateTime::currentDateTime();
    QDate currentDate = currentTime.date();
    QString year = QString::number(currentDate.year());
    QString mon_pre = QString::number(currentDate.month() - 1);
    QString mon_now = QString::number(currentDate.month());
    
    QVector<Salary> sa_pre_user;
    QVector<Salary> sa_new_user;
    QVector<Employee> user_all = ReadEmployee();
    
    if (mon_now == "1") {
        sa_pre_user = FindSalary_Mon(QString::number(currentDate.year() - 1), "12");
    } else {
        sa_pre_user = FindSalary_Mon(year, mon_pre);
    }
    QVector<Salary> sa_now_mon = FindSalary_Mon(year, mon_now);

    for (int i = 0; i < user_all.size(); i++) {
        if (user_all[i].Status == 1) {
            int result = FindSalaryInVector(user_all[i].id, sa_pre_user);
            int result2 = FindSalaryInVector(user_all[i].id, sa_now_mon);
            if (result == 0 && result2 == 0) {
                double sal_posi = FindSal_Posi(user_all[i].Dept, user_all[i].Position);
                // 默认参考上个月的第一条记录的基本工资，或赋默认起薪3000
                double baseSal = sa_pre_user.isEmpty() ? 3000.0 : sa_pre_user[0].basic_salary;
                Salary sa_now(user_all[i].id, year, mon_now, baseSal, sal_posi);
                sa_now.getReward();
                sa_now.getHouse();
                sa_now.getAttend_redu();
                sa_now.getPay_salary();
                sa_now.getFund();
                sa_now.getSalary_Performance();
                sa_now.getTax();
                sa_now.getReal_salary();

                sa_new_user.push_back(sa_now);
            }
        }
    }
    return sa_new_user;
}

// ============================================================
// 考勤数据读写
// ============================================================

Attendance GetAttendance(QString line)
{
    Q_UNUSED(line);
    return Attendance();
}

QVector<Attendance> ReadAttendance()
{
    QVector<Attendance> attendances;
    QSqlQuery query("SELECT id, late, exit_early, year, mon, overtime FROM attendance");
    while (query.next()) {
        Attendance att(
            query.value(0).toString(),
            query.value(1).toInt(),
            query.value(2).toInt(),
            query.value(3).toString(),
            query.value(4).toString(),
            query.value(5).toInt()
        );
        attendances.push_back(att);
    }
    return attendances;
}

// ============================================================
// 岗位工资表读写
// ============================================================

Sal_Posi GetSal_Posi(QString line)
{
    Q_UNUSED(line);
    return Sal_Posi();
}

QVector<Sal_Posi> ReadSal_Posi()
{
    QVector<Sal_Posi> sal_posis;
    QSqlQuery query("SELECT dept, position, salary_position, department_coefficient FROM position_salary");
    while (query.next()) {
        Sal_Posi sp(
            query.value(0).toString(),
            query.value(1).toInt(),
            query.value(2).toDouble(),
            query.value(3).toDouble()
        );
        sal_posis.push_back(sp);
    }
    return sal_posis;
}

QString Sal_PosiToString(Sal_Posi& sal_Posi)
{
    Q_UNUSED(sal_Posi);
    return QString();
}

int WriteSal_PosiToFile(QVector<Sal_Posi> user)
{
    QSqlDatabase::database().transaction();
    QSqlQuery query;
    query.exec("DELETE FROM position_salary");
    for (const Sal_Posi& sp : user) {
        query.prepare("INSERT INTO position_salary (dept, position, salary_position, department_coefficient) "
                      "VALUES (?, ?, ?, ?)");
        query.addBindValue(sp.Dept);
        query.addBindValue(sp.Position);
        query.addBindValue(sp.salary_position);
        query.addBindValue(sp.department_coefficient);
        query.exec();
    }
    QSqlDatabase::database().commit();
    return 1;
}

bool ImportEmployeesFromCsv(const QString& path_from)
{
    QFile file(path_from);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Cannot open employee CSV file:" << path_from;
        return false;
    }
    QTextStream in(&file);
    in.setCodec("GBK");
    in.setAutoDetectUnicode(true);
    if (!in.atEnd()) {
        in.readLine(); // skip header
    }
    QVector<Employee> existing = ReadEmployee();
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;
        QStringList parts = line.split(",");
        if (parts.size() >= 11) {
            Employee emp(parts[0].trimmed(), parts[1].trimmed(), parts[2].trimmed(), 
                         parts[3].trimmed(), parts[4].toInt(), parts[5].trimmed(),
                         parts[6].trimmed(), parts[7].trimmed(), parts[8].trimmed(), 
                         parts[9].trimmed(), parts[10].toInt());
            int idx = -1;
            for (int i = 0; i < existing.size(); ++i) {
                if (existing[i].id == emp.id) {
                    idx = i;
                    break;
                }
            }
            if (idx != -1) {
                existing[idx] = emp;
            } else {
                existing.push_back(emp);
            }
        }
    }
    file.close();
    WriteEmployeeToFile(existing);
    return true;
}

bool ImportAttendanceFromCsv(const QString& path_from)
{
    QFile file(path_from);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Cannot open attendance CSV file:" << path_from;
        return false;
    }
    QTextStream in(&file);
    in.setCodec("GBK");
    in.setAutoDetectUnicode(true);
    if (!in.atEnd()) {
        in.readLine(); // skip header
    }
    QSqlDatabase::database().transaction();
    QSqlQuery query;
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;
        QStringList parts = line.split(",");
        if (parts.size() >= 6) {
            query.prepare("INSERT OR REPLACE INTO attendance (id, late, exit_early, year, mon, overtime) "
                          "VALUES (?, ?, ?, ?, ?, ?)");
            query.addBindValue(parts[0].trimmed());
            query.addBindValue(parts[1].toInt());
            query.addBindValue(parts[2].toInt());
            query.addBindValue(parts[3].trimmed());
            query.addBindValue(parts[4].trimmed());
            query.addBindValue(parts[5].toInt());
            query.exec();
        }
    }
    QSqlDatabase::database().commit();
    file.close();
    return true;
}
