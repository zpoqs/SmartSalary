#include "Adm_user_satis.h"
#include "ui_adm_user_satis.h"
#include"satatis.h"
#include"QDebug"
Adm_user_satis::Adm_user_satis(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Adm_user_satis)
{
    ui->setupUi(this);
    setWindowTitle("薪智汇控 - 员工数据统计");
    ShowResult_StaUser();
}
void Adm_user_satis::ShowResult_StaUser()
{
    drawGenderPieChart(this);
    QMap<QString, int> dept = countEmployeesByDepartmentFromDb();
    drawBarChart(dept, this);
    QVector<int> peo = extractAndSubtractFrom2026();

    createPieAgeChart(peo,this);
    std::unordered_map<int, int> peo_statu = countEmployeeStatusesFromDb();
    createPieChart(peo_statu, this);
}

Adm_user_satis::~Adm_user_satis()
{
    delete ui;
}

void Adm_user_satis::on_btn_back_clicked()//返回
{
    emit Show_Admin_Widget();
    this->close();
}

void Adm_user_satis::on_pushButton_2_clicked()
{
    qApp->quit();
}
