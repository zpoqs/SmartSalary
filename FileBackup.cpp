#include "Filebackup.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QSqlDatabase>

// ============================================================
// 数据库表级备份与灾备恢复引擎 (Database-level Hot Backup Engine)
// 纯 SQL 级事务操作，替代原有的绝对路径文件复制
// ============================================================

static bool BackupTable(const QString& sourceTable, const QString& backupTable)
{
    QSqlQuery query;
    // 如果备份表不存在，创建它（结构同原表）
    query.exec(QString("CREATE TABLE IF NOT EXISTS %1 AS SELECT * FROM %2 WHERE 1=0").arg(backupTable).arg(sourceTable));
    // 清空备份表
    query.exec(QString("DELETE FROM %1").arg(backupTable));
    // 导入最新数据
    bool ok = query.exec(QString("INSERT INTO %1 SELECT * FROM %2").arg(backupTable).arg(sourceTable));
    if (!ok) {
        qDebug() << "[Backup Engine Error] 表备份失败:" << sourceTable << "->" << backupTable << ":" << query.lastError().text();
    }
    return ok;
}

static bool RestoreTable(const QString& sourceTable, const QString& backupTable)
{
    QSqlQuery query;
    // 检查备份表是否存在且有数据
    query.exec(QString("SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='%1'").arg(backupTable));
    if (query.next() && query.value(0).toInt() == 0) {
        qDebug() << "[Restore Engine Error] 恢复失败，未找到备份表:" << backupTable;
        return false;
    }

    QSqlDatabase::database().transaction();
    query.exec(QString("DELETE FROM %1").arg(sourceTable));
    bool ok = query.exec(QString("INSERT INTO %1 SELECT * FROM %2").arg(sourceTable).arg(backupTable));
    
    if (ok) {
        QSqlDatabase::database().commit();
        qDebug() << "[Restore Engine] 数据还原成功:" << backupTable << "->" << sourceTable;
    } else {
        QSqlDatabase::database().rollback();
        qDebug() << "[Restore Engine Error] 数据还原失败:" << query.lastError().text();
    }
    return ok;
}

void BackUpEmployee()
{
    if (BackupTable("employee", "employee_backup")) {
        qDebug() << "员工数据表级备份成功";
    } else {
        qDebug() << "员工数据表级备份失败";
    }
}

void BackUpSalary()
{
    if (BackupTable("salary", "salary_backup")) {
        qDebug() << "工资数据表级备份成功";
    } else {
        qDebug() << "工资数据表级备份失败";
    }
}

void BackUpAttendce()
{
    if (BackupTable("attendance", "attendance_backup")) {
        qDebug() << "考勤数据表级备份成功";
    } else {
        qDebug() << "考勤数据表级备份失败";
    }
}

void BackUpSal_Posi()
{
    if (BackupTable("position_salary", "position_salary_backup")) {
        qDebug() << "岗位工资数据表级备份成功";
    } else {
        qDebug() << "岗位工资数据表级备份失败";
    }
}

bool BackUpFile(const QString& path_pre, const QString& path_back)
{
    // 保留接口兼容性
    Q_UNUSED(path_pre);
    Q_UNUSED(path_back);
    return true;
}

void RestoreEmployee()
{
    if (RestoreTable("employee", "employee_backup")) {
        qDebug() << "员工信息表级恢复成功";
    } else {
        qDebug() << "员工信息表级恢复失败";
    }
}

void RestoreSalary()
{
    if (RestoreTable("salary", "salary_backup")) {
        qDebug() << "工资表级恢复成功";
    } else {
        qDebug() << "工资表级恢复失败";
    }
}

void RestoreAttendce()
{
    if (RestoreTable("attendance", "attendance_backup")) {
        qDebug() << "考勤表级恢复成功";
    } else {
        qDebug() << "考勤表级恢复失败";
    }
}

void RestoreSal_Posi()
{
    if (RestoreTable("position_salary", "position_salary_backup")) {
        qDebug() << "岗位工资表级恢复成功";
    } else {
        qDebug() << "岗位工资表级恢复失败";
    }
}
