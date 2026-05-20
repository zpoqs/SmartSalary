#include <QApplication>
#include <QFile>
#include "login.h"
#include "io_base.h"

// ============================================================
// main.cpp - 薪智汇控系统程序入口
// ============================================================

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 初始化SQLite数据库系统
    if (!InitDatabase()) {
        qWarning() << "[Main] 警告: 关系型数据库初始化失败，这可能会影响数据持久化。";
    }

    // 设置应用元信息
    QApplication::setApplicationName("薪智汇控系统");
    QApplication::setApplicationVersion("2.0");
    QApplication::setOrganizationName("SoftEng-Team");

    // 加载全局QSS样式表（深色主题）
    QFile styleFile(":/resources/style.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        QString style = QLatin1String(styleFile.readAll());
        a.setStyleSheet(style);
        styleFile.close();
    }

    // 启动登录窗口
    login w;
    w.show();

    return a.exec();
}
