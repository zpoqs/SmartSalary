#include "User.h"
#include "ui_user.h"
#include <QDebug>
#include "io_base.h"
#include <QMessageBox>
#include <QPushButton>

QString Id_initial;//用户登录时的账号
User::User(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::User)
{
    ui->setupUi(this);
    setWindowTitle("薪智汇控 - 员工个人中心");
    qDebug()<<Id_initial;
    connect(&user_per_mag,SIGNAL(Show_User_Widget()),this,SLOT(show_User_again()));
    connect(&user_sal,SIGNAL(Show_User_Widget()),this,SLOT(show_User_again()));
    
    // 隐藏主屏占位的“小提醒”文本框与标签
    ui->help->hide();
    ui->title->hide();

    // 在左侧菜单上方动态创建“帮助说明”按钮
    QPushButton* btn_help = new QPushButton("系统使用帮助", this);
    btn_help->setGeometry(40, 145, 300, 45);
    btn_help->setStyleSheet(
        "QPushButton {"
        "    font: 12pt 'Agency FB';"
        "    color: #2c3e50;"
        "    background-color: #eef2f7;"
        "    border: 1px solid #dcdfe6;"
        "    border-radius: 6px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #e0e6ed;"
        "    border-color: #b3d8ff;"
        "}"
    );
    connect(btn_help, &QPushButton::clicked, this, [this]() {
        QMessageBox::information(this, "系统使用帮助", 
            "1. 个人档案管理：您可以安全修改自己的手机号、邮箱、登录密码等个人关键档案信息。\n"
            "2. 我的薪资账单：支持一键查询您历史每月的各项应发、实发工资以及公积金扣税详情，同时支持查看配套的月度考勤明细。\n"
            "3. 薪智汇控 AI 助手：搭载通义千问最新大模型，支持自然语言对话，您可以点击“月度报告”让 AI 为您解读本月实发工资分布与异常扣款原因。");
    });

    // 初始化员工 AI 助手，铺满中右侧整个大面板
    m_aiChatWidget = new AiChatWidget(this);
    m_aiChatWidget->setGeometry(380, 200, 1080, 740);
}

User::~User()
{
    delete ui;
}

void User::on_btn_back_login_clicked()
{
    emit Show_login_Widget();
    this->close();
}


void User::on_btn_quit_clicked()
{
    qApp->quit();
}

void User::on_btn_user_mag_clicked()
{
    SetIdFromUserToMag(Id_initial);
    user_per_mag.show();
    user_per_mag.Show_info();
    this->hide();
}
void Set_Id(const QString& id)
{
    Id_initial = id;
}
void User::show_User_again()
{
    this->show();
}

void User::on_btn_sly_mag_clicked()
{
    SetIdFromUserToSal(Id_initial);
    user_sal.show();
    user_sal.DisplayUserSalaryData();
    this->hide();
}

void User::initAIForUser(const QString& id)
{
    QVector<Employee> all_emp = ReadEmployee();
    QVector<Employee> filtered_emp;
    for (const Employee& emp : all_emp) {
        if (emp.id == id) {
            filtered_emp.push_back(emp);
            break;
        }
    }
    
    QDateTime currentTime = QDateTime::currentDateTime();
    QDate currentDate = currentTime.date();
    QVector<Salary> all_sal = ReadSalary();
    QVector<Salary> filtered_sal;
    for (const Salary& sal : all_sal) {
        if (sal.id == id) {
            if (sal.time.year.toInt() < currentDate.year() || 
                (sal.time.year.toInt() == currentDate.year() && sal.time.mon.toInt() < currentDate.month())) {
                filtered_sal.push_back(sal);
            }
        }
    }
    m_aiChatWidget->setSalaryData(filtered_sal, filtered_emp);
}
