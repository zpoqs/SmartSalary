#include "Admin.h"
#include "ui_admin.h"
#include"io_base.h"
#include"Adm_user_manage.h"
#include<QString>
#include"Filebackup.h"
#include"satatis.h"
#include <QMessageBox>
#include <QPushButton>

Admin::Admin(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Admin)
{

    ui->setupUi(this);
    setWindowTitle("薪智汇控 - 管理员后台");
    connect(&adm_user_mag,SIGNAL(Show_Admin_Widget()),this,SLOT(show_Adm_again()));
    connect(&adm_sal_mag,SIGNAL(Show_Admin_Widget()),this,SLOT(show_Adm_again()));
    connect(&adm_user_satis,SIGNAL(Show_Admin_Widget()),this,SLOT(show_Adm_again()));
    connect(&adm_sal_satis,SIGNAL(Show_Admin_Widget()),this,SLOT(show_Adm_again()));
    
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
            "1. 员工个人信息管理：支持单个手动录入员工、删除员工或保存修改。若有编号冲突将触发强校验提醒。\n"
            "2. 员工工资信息管理：根据当月考勤计算工资，并支持批量导入 CSV 考勤报表计算实发工资。\n"
            "3. 员工工资/个人信息统计：通过折线图、扇形图、柱状图等多色图表直观展示薪酬与人资结构。\n"
            "4. 恢复原来文件信息：用于将数据库或备份的 CSV 考勤/员工数据进行一键灾备恢复。\n"
            "5. 薪智汇控 AI 助手：搭载通义千问大模型，输入自然语言即可一键为您智能分析生成当月薪资报告、异常排查等。");
    });

    // 初始化 AI 助手界面，铺满中右侧整个大面板
    m_aiChatWidget = new AiChatWidget(this);
    m_aiChatWidget->setGeometry(380, 140, 1080, 800);
    m_aiChatWidget->setSalaryData(ReadSalary(), ReadEmployee());
}

Admin::~Admin()
{
    delete ui;
}

void Admin::on_btn_user_mag_clicked()
{
    adm_user_mag.show();
    QTableWidget* table = adm_user_mag.GetTable();
    adm_user_mag.DisplayEmployeeData(table);
    this->hide();
}

void Admin::on_btn_back_login_clicked()
{
    emit Show_login_Widget();
    this->close();
}
void Admin::show_Adm_again()
{
    this->show();
    // 重新加载数据，保证 AI 拥有最新上下文
    if (m_aiChatWidget) {
        m_aiChatWidget->setSalaryData(ReadSalary(), ReadEmployee());
    }
}



void Admin::on_btn_quit_clicked()
{
    qApp->quit();
}

void Admin::on_btn_sal_mag_clicked()
{
    adm_sal_mag.show();
    QTableWidget* table_now = adm_sal_mag.GetNowTable();
    table_now->blockSignals(true);//非常重要，否则一直出现消息框
    adm_sal_mag.DisplaySalaryNowData(table_now);
    table_now->blockSignals(false);

    this->hide();
}

void Admin::on_pushButton_clicked()
{
    RestoreEmployee();
    RestoreSalary();
    RestoreSal_Posi();
    RestoreAttendce();
}

void Admin::on_btn_sala_satis_clicked()
{
    adm_sal_satis.show();
    adm_sal_satis.DisplayResult_Statis();
    this->hide();
}

void Admin::on_btn_user_sas_clicked()
{
    adm_user_satis.show();
    adm_user_satis.ShowResult_StaUser();
    this->hide();
}
