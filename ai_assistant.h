#ifndef AI_ASSISTANT_H
#define AI_ASSISTANT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QString>
#include <QVector>
#include "base_lei.h"

// ============================================================
// AiAssistant - 薪智汇控AI助手后端核心类
// 负责与阿里云通义千问 API 通信
// ============================================================
class AiAssistant : public QObject
{
    Q_OBJECT

public:
    explicit AiAssistant(QObject *parent = nullptr);
    ~AiAssistant();

    // --- 三大核心AI功能 ---

    // 功能1: 分析当月工资数据，生成异常提醒和摘要报告
    void analyzeSalaryReport(const QVector<Salary>& salaries, const QVector<Employee>& employees);

    // 功能2: 针对单个员工，生成个人薪资分析（含趋势判断）
    void analyzePersonalSalary(const Employee& emp, const QVector<Salary>& history);

    // 功能3: 通用对话接口，管理员可用自然语言提问
    void chat(const QString& userMessage);

    // 设置对话上下文（把当前数据传给AI参考）
    void setContext(const QVector<Salary>& salaries, const QVector<Employee>& employees);

signals:
    // AI开始思考
    void thinkingStarted();
    // AI回复到达（完整文本）
    void responseReceived(const QString& replyText);
    // 请求出错
    void errorOccurred(const QString& errorMsg);

private slots:
    void onReplyFinished(QNetworkReply* reply);

private:
    QNetworkAccessManager* m_manager;
    QString m_apiKey;
    QString m_model;
    bool m_enableThinking;
    QString m_contextSummary; // 当前数据上下文摘要

    // 构建薪资分析Prompt
    QString buildSalaryAnalysisPrompt(const QVector<Salary>& salaries,
                                       const QVector<Employee>& employees);
    // 构建个人薪资Prompt
    QString buildPersonalSalaryPrompt(const Employee& emp,
                                       const QVector<Salary>& history);
    // 发送HTTP请求到API
    void sendToApi(const QString& systemPrompt, const QString& userMessage);

    // 从CSV格式的工资数据构建文本摘要
    QString salaryDataToText(const QVector<Salary>& salaries,
                              const QVector<Employee>& employees);
};

#endif // AI_ASSISTANT_H
