#include "ai_assistant.h"
#include <QFileInfo>
#include <QCoreApplication>
#include <QFile>
#include <QSettings>
#include <QDebug>
#include <QDateTime>

// ============================================================
// AiAssistant 实现
// 对接阿里云 DashScope OpenAI 兼容接口 (qwen-max)
// ============================================================


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

AiAssistant::AiAssistant(QObject *parent)
    : QObject(parent)
    , m_manager(new QNetworkAccessManager(this))
    , m_model("qwen3.7-max")
    , m_enableThinking(true)
{
    // 从配置文件读取API Key和模型配置，避免硬编码在源代码中
    QSettings settings(findConfigFile(), QSettings::IniFormat);
    m_apiKey = settings.value("AI/ApiKey", "").toString();
    m_model = settings.value("AI/Model", "qwen3.7-max").toString().trimmed();
    m_enableThinking = settings.value("AI/EnableThinking", true).toBool();

    if (m_apiKey.isEmpty()) {
        qWarning() << "[AiAssistant] 警告: 未找到 API Key，请在 config.ini 中配置 [AI] ApiKey";
    }

    connect(m_manager, &QNetworkAccessManager::finished,
            this, &AiAssistant::onReplyFinished);
}

AiAssistant::~AiAssistant()
{
}

// ----------------------------------------------------------------
// 功能1: 分析当月全体员工工资报告
// ----------------------------------------------------------------
void AiAssistant::analyzeSalaryReport(const QVector<Salary>& salaries,
                                       const QVector<Employee>& employees)
{
    if (salaries.isEmpty()) {
        emit errorOccurred("暂无工资数据可供分析。");
        return;
    }

    QString systemPrompt =
        "你是一位专业的人力资源财务分析师，精通中国企业薪酬管理。"
        "请基于用户提供的员工工资数据，生成一份简洁、专业的薪资分析报告，使用中文回复。"
        "报告应包含：1) 整体薪资水平概述，2) 异常情况提醒（如实发为0、扣税偏高等），"
        "3) 各部门薪资分布简述，4) 优化建议（1-2条）。"
        "回复格式使用Markdown，不超过400字。";

    QString dataText = buildSalaryAnalysisPrompt(salaries, employees);
    sendToApi(systemPrompt, dataText);
}

// ----------------------------------------------------------------
// 功能2: 针对单个员工的个人薪资分析
// ----------------------------------------------------------------
void AiAssistant::analyzePersonalSalary(const Employee& emp,
                                         const QVector<Salary>& history)
{
    QString systemPrompt =
        "你是一位薪酬顾问，请根据用户提供的员工薪资历史数据，"
        "分析其薪资变化趋势，指出是否存在异常月份，并给出简短的职业发展建议。"
        "使用中文，语气友好，不超过200字。";

    QString userMsg = QString("员工信息：工号=%1，姓名=%2，部门=%3，职位=%4\n\n薪资历史（近%5个月）：\n")
                          .arg(emp.id).arg(emp.name).arg(emp.Dept)
                          .arg(emp.Position == 1 ? "普通员工" : "主管")
                          .arg(history.size());

    for (const Salary& s : history) {
        userMsg += QString("  %1年%2月: 基本工资=%3, 实发=%4, 考勤扣除=%5\n")
                       .arg(s.time.year).arg(s.time.mon)
                       .arg(s.basic_salary, 0, 'f', 2).arg(s.real_salary, 0, 'f', 2).arg(s.attend_redu, 0, 'f', 2);
    }

    sendToApi(systemPrompt, userMsg);
}

// ----------------------------------------------------------------
// 功能3: 通用自然语言对话
// ----------------------------------------------------------------
void AiAssistant::chat(const QString& userMessage)
{
    QString systemPrompt =
        "你是薪智汇控系统的AI助手，专门辅助管理员处理员工工资管理相关问题。"
        "你可以回答关于工资计算、考勤政策、个税规则等问题。"
        "当前系统数据摘要：" + m_contextSummary + "\n"
        "请用简洁、专业的中文回答，不超过300字。";

    sendToApi(systemPrompt, userMessage);
}

// ----------------------------------------------------------------
// 设置上下文（用于通用对话时提供数据背景）
// ----------------------------------------------------------------
void AiAssistant::setContext(const QVector<Salary>& salaries,
                              const QVector<Employee>& employees)
{
    if (salaries.isEmpty()) {
        m_contextSummary = "当前无工资数据。";
        return;
    }

    // 找出最新的发薪年份和月份
    int latestYear = 0;
    int latestMonth = 0;
    for (const Salary& s : salaries) {
        int y = s.time.year.toInt();
        int m = s.time.mon.toInt();
        if (y > latestYear || (y == latestYear && m > latestMonth)) {
            latestYear = y;
            latestMonth = m;
        }
    }

    QVector<Salary> currentSalaries;
    for (const Salary& s : salaries) {
        if (s.time.year == QString::number(latestYear) && s.time.mon == QString::number(latestMonth)) {
            currentSalaries.push_back(s);
        }
    }

    if (currentSalaries.isEmpty()) {
        m_contextSummary = "当前无有效的最新月份工资数据。";
        return;
    }

    // 计算简单统计值用于上下文
    double totalReal = 0.0, maxReal = 0.0, minReal = 1e9;
    for (const Salary& s : currentSalaries) {
        totalReal += s.real_salary;
        if (s.real_salary > maxReal) maxReal = s.real_salary;
        if (s.real_salary > 0 && s.real_salary < minReal) minReal = s.real_salary;
    }
    double avgReal = totalReal / currentSalaries.size();

    // 统计在职员工人数
    int activeEmployeeCount = 0;
    for (const Employee& e : employees) {
        if (e.Status == 1) {
            activeEmployeeCount++;
        }
    }

    m_contextSummary = QString("最新发薪月（%1年%2月）共%3名发薪员工，该月平均实发工资%4元，"
                               "最高实发%5元，最低实发%6元，"
                               "系统内在职员工%7人。")
                           .arg(latestYear)
                           .arg(latestMonth)
                           .arg(currentSalaries.size())
                           .arg(QString::number(avgReal, 'f', 0))
                           .arg(QString::number(maxReal, 'f', 0))
                           .arg(QString::number(minReal, 'f', 0))
                           .arg(activeEmployeeCount);
}

// ----------------------------------------------------------------
// 私有：发送API请求
// ----------------------------------------------------------------
void AiAssistant::sendToApi(const QString& systemPrompt, const QString& userMessage)
{
    if (m_apiKey.isEmpty()) {
        emit errorOccurred("API Key 未配置，请在 config.ini 中添加 [AI] ApiKey=your_key");
        return;
    }

    emit thinkingStarted();

    // 构建请求URL（阿里云 DashScope OpenAI 兼容接口）
    QUrl url("https://dashscope.aliyuncs.com/compatible-mode/v1/chat/completions");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", "Bearer " + m_apiKey.toUtf8());

    // 构建 messages 数组
    QJsonArray messages;

    QJsonObject sysMsg;
    sysMsg["role"] = "system";
    sysMsg["content"] = systemPrompt;
    messages.append(sysMsg);

    QJsonObject userMsg;
    userMsg["role"] = "user";
    userMsg["content"] = userMessage;
    messages.append(userMsg);

    // 构建请求体
    QJsonObject body;
    body["model"] = m_model;
    body["messages"] = messages;
    body["stream"] = false;
    body["max_tokens"] = 4000;
    if (m_enableThinking) {
        body["enable_thinking"] = true;
    }

    QByteArray postData = QJsonDocument(body).toJson(QJsonDocument::Compact);
    m_manager->post(request, postData);
}

// ----------------------------------------------------------------
// 私有：网络请求完成回调
// ----------------------------------------------------------------
void AiAssistant::onReplyFinished(QNetworkReply* reply)
{
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        QString errDetail = QString::fromUtf8(reply->readAll());
        emit errorOccurred(QString("网络请求失败: %1\n详情: %2")
                               .arg(reply->errorString()).arg(errDetail));
        return;
    }

    QByteArray responseData = reply->readAll();
    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        emit errorOccurred(QString("响应解析失败: %1").arg(parseError.errorString()));
        return;
    }

    QJsonObject jsonObj = jsonDoc.object();

    // 检查API错误
    if (jsonObj.contains("error")) {
        QString apiErr = jsonObj["error"].toObject()["message"].toString();
        emit errorOccurred(QString("API错误: %1").arg(apiErr));
        return;
    }

    // 提取回复内容
    if (jsonObj.contains("choices")) {
        QJsonArray choices = jsonObj["choices"].toArray();
        if (!choices.isEmpty()) {
            QJsonObject messageObj = choices[0].toObject()["message"].toObject();
            QString content = messageObj["content"].toString();
            QString reasoning = messageObj["reasoning_content"].toString();

            QString finalResponse;
            if (!reasoning.isEmpty()) {
                finalResponse += "> **🤔 思考过程：**\n";
                for (const QString& line : reasoning.split('\n')) {
                    finalResponse += "> " + line + "\n";
                }
                finalResponse += "\n---\n\n";
            }
            finalResponse += content;

            emit responseReceived(finalResponse);
            return;
        }
    }

    emit errorOccurred("响应格式异常，未能解析AI回复内容。");
}

// ----------------------------------------------------------------
// 私有：构建工资分析Prompt文本
// ----------------------------------------------------------------
QString AiAssistant::buildSalaryAnalysisPrompt(const QVector<Salary>& salaries,
                                                const QVector<Employee>& employees)
{
    if (salaries.isEmpty()) {
        return "当前无工资数据。";
    }

    // 找出最新的发薪年份和月份
    int latestYear = 0;
    int latestMonth = 0;
    for (const Salary& s : salaries) {
        int y = s.time.year.toInt();
        int m = s.time.mon.toInt();
        if (y > latestYear || (y == latestYear && m > latestMonth)) {
            latestYear = y;
            latestMonth = m;
        }
    }

    QVector<Salary> currentSalaries;
    for (const Salary& s : salaries) {
        if (s.time.year == QString::number(latestYear) && s.time.mon == QString::number(latestMonth)) {
            currentSalaries.push_back(s);
        }
    }

    if (currentSalaries.isEmpty()) {
        return "当前无有效的最新月份工资数据。";
    }

    // 计算统计值
    double totalReal = 0, maxReal = 0, minReal = 1e9;
    double totalBasic = 0, totalPerf = 0, totalHouse = 0, totalReward = 0;
    double totalFund = 0, totalTax = 0, totalRedu = 0;
    int anomalyCount = 0;
    QMap<QString, QPair<double, int>> deptStats; // dept -> (totalReal, count)

    for (const Salary& s : currentSalaries) {
        totalReal += s.real_salary;
        totalBasic += s.basic_salary;
        totalPerf += s.salary_Performance;
        totalHouse += s.House;
        totalReward += s.reward;
        totalFund += s.fund;
        totalTax += s.tax;
        totalRedu += s.attend_redu;

        if (s.real_salary > maxReal) maxReal = s.real_salary;
        if (s.real_salary > 0 && s.real_salary < minReal) minReal = s.real_salary;
        if (s.real_salary <= 0 || s.attend_redu > 500) anomalyCount++;

        // 找对应员工的部门
        for (const Employee& e : employees) {
            if (e.id == s.id) {
                deptStats[e.Dept].first += s.real_salary;
                deptStats[e.Dept].second++;
                break;
            }
        }
    }
    double avgReal = totalReal / currentSalaries.size();

    QString yearMon = QString("%1年%2月").arg(latestYear).arg(latestMonth);

    QString prompt = QString("以下是薪智汇控系统%1的工资数据统计：\n\n").arg(yearMon);
    prompt += QString("【总体概况】\n发薪员工总数: %1人\n平均实发工资: %2元\n最高实发: %3元\n最低实发: %4元\n疑似异常记录: %5条\n\n")
                  .arg(currentSalaries.size()).arg(avgReal, 0, 'f', 2).arg(maxReal, 0, 'f', 2).arg(minReal, 0, 'f', 2).arg(anomalyCount);

    prompt += QString("【薪酬构成明细汇总】\n基本工资总额: %1元\n绩效工资总额: %2元\n房改补贴总额: %3元\n奖励金总额: %4元\n公积金扣除总额: %5元\n个人所得税总额: %6元\n考勤扣款总额: %7元\n\n")
                  .arg(totalBasic, 0, 'f', 2).arg(totalPerf, 0, 'f', 2).arg(totalHouse, 0, 'f', 2)
                  .arg(totalReward, 0, 'f', 2).arg(totalFund, 0, 'f', 2).arg(totalTax, 0, 'f', 2)
                  .arg(totalRedu, 0, 'f', 2);

    prompt += "【各部门平均实发工资】\n";
    for (auto it = deptStats.begin(); it != deptStats.end(); ++it) {
        double deptAvg = it.value().second > 0 ? it.value().first / it.value().second : 0;
        prompt += QString("  %1: %2元 (%3人)\n")
                      .arg(it.key()).arg(deptAvg, 0, 'f', 2).arg(it.value().second);
    }

    prompt += "\n【前10条工资明细】\n";
    int showCount = qMin(10, currentSalaries.size());
    for (int i = 0; i < showCount; i++) {
        const Salary& s = currentSalaries[i];
        prompt += QString("  工号%1: 基本=%2, 岗位=%3, 绩效=%4, "
                          "考勤扣=%5, 个税=%6, 实发=%7\n")
                      .arg(s.id)
                      .arg(s.basic_salary, 0, 'f', 2)
                      .arg(s.salary_position, 0, 'f', 2)
                      .arg(s.salary_Performance, 0, 'f', 2)
                      .arg(s.attend_redu, 0, 'f', 2)
                      .arg(s.tax, 0, 'f', 2)
                      .arg(s.real_salary, 0, 'f', 2);
    }

    prompt += "\n请根据以上详细构成数据，进行薪酬结构合理性评估、部门成本差异分析并给出管理优化建议，生成一份高度专业、排版清晰的薪资分析报告。";
    return prompt;
}
