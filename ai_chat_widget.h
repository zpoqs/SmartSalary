#ifndef AI_CHAT_WIDGET_H
#define AI_CHAT_WIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QMovie>
#include <QPropertyAnimation>
#include "ai_assistant.h"

// ============================================================
// AiChatWidget - AI对话界面组件
// 可嵌入到管理员主界面，提供气泡式AI对话体验
// ============================================================
class AiChatWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AiChatWidget(QWidget *parent = nullptr);
    ~AiChatWidget();

    // 设置当前工资和员工数据（用于AI分析时的数据上下文）
    void setSalaryData(const QVector<Salary>& salaries,
                       const QVector<Employee>& employees);

private slots:
    void onSendClicked();
    void onAiResponseReceived(const QString& text);
    void onAiError(const QString& error);
    void onAiThinking();
    void onAnalyzeReportClicked();  // 一键生成月度报告
    void onAnalyzeTrendClicked();   // 一键分析工资趋势

private:
    // UI组件
    QScrollArea*  m_scrollArea;
    QWidget*      m_chatContainer;
    QVBoxLayout*  m_chatLayout;
    QLineEdit*    m_inputEdit;
    QPushButton*  m_sendBtn;
    QPushButton*  m_reportBtn;      // 一键分析月度报告
    QPushButton*  m_trendBtn;       // 工资趋势分析
    QLabel*       m_thinkingLabel;  // "思考中..." 指示

    // AI后端
    AiAssistant*  m_assistant;

    // 数据
    QVector<Salary>   m_salaries;
    QVector<Employee> m_employees;

    // 辅助函数
    void appendUserBubble(const QString& text);
    void appendAiBubble(const QString& text);
    void appendSystemMessage(const QString& text);
    void scrollToBottom();
    void setupUi();
    void applyStyle();
};

#endif // AI_CHAT_WIDGET_H
