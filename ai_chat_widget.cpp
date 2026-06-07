#include "ai_chat_widget.h"
#include <QScrollBar>
#include <QTextBrowser>
#include <QFrame>
#include <QDateTime>
#include<QTimer>


// ============================================================
// AiChatWidget 实现
// 气泡式AI对话界面，嵌入管理员主面板
// ============================================================

AiChatWidget::AiChatWidget(QWidget *parent)
    : QWidget(parent)
    , m_assistant(new AiAssistant(this))
{
    setupUi();
    applyStyle();

    // 连接AI后端信号
    connect(m_assistant, &AiAssistant::responseReceived,
            this, &AiChatWidget::onAiResponseReceived);
    connect(m_assistant, &AiAssistant::errorOccurred,
            this, &AiChatWidget::onAiError);
    connect(m_assistant, &AiAssistant::thinkingStarted,
            this, &AiChatWidget::onAiThinking);

    // 发送按钮
    connect(m_sendBtn, &QPushButton::clicked, this, &AiChatWidget::onSendClicked);
    connect(m_inputEdit, &QLineEdit::returnPressed, this, &AiChatWidget::onSendClicked);

    // 快捷功能按钮
    connect(m_reportBtn, &QPushButton::clicked, this, &AiChatWidget::onAnalyzeReportClicked);
    connect(m_trendBtn,  &QPushButton::clicked, this, &AiChatWidget::onAnalyzeTrendClicked);

    // 欢迎消息
    appendSystemMessage("🤖 你好！我是薪智汇控AI助手。\n"
                        "你可以问我：\n"
                        "• 「分析本月工资分布情况」\n"
                        "• 「哪些员工本月有异常扣款？」\n"
                        "• 「帮我解释个税计算规则」\n"
                        "或点击下方快捷按钮一键生成报告。");
}

AiChatWidget::~AiChatWidget() {}

void AiChatWidget::setSalaryData(const QVector<Salary>& salaries,
                                  const QVector<Employee>& employees)
{
    m_salaries  = salaries;
    m_employees = employees;
    m_assistant->setContext(salaries, employees);
}

// ----------------------------------------------------------------
// 槽: 发送用户消息
// ----------------------------------------------------------------
void AiChatWidget::onSendClicked()
{
    QString text = m_inputEdit->text().trimmed();
    if (text.isEmpty()) return;

    appendUserBubble(text);
    m_inputEdit->clear();
    m_sendBtn->setEnabled(false);

    m_assistant->chat(text);
}

// ----------------------------------------------------------------
// 槽: 一键生成月度薪资分析报告
// ----------------------------------------------------------------
void AiChatWidget::onAnalyzeReportClicked()
{
    if (m_salaries.isEmpty()) {
        appendSystemMessage("⚠ 暂无工资数据，请先在工资管理界面加载数据。");
        return;
    }
    appendUserBubble("请帮我分析本月全体员工的工资情况，生成月度薪资报告。");
    m_reportBtn->setEnabled(false);
    m_sendBtn->setEnabled(false);

    m_assistant->analyzeSalaryReport(m_salaries, m_employees);
}

// ----------------------------------------------------------------
// 槽: 分析工资趋势
// ----------------------------------------------------------------
void AiChatWidget::onAnalyzeTrendClicked()
{
    appendUserBubble("请分析当前薪资数据中是否存在异常情况和优化建议。");
    m_sendBtn->setEnabled(false);

    // 用通用chat接口，上下文已设置
    m_assistant->chat("请分析当前工资数据中的异常情况，"
                      "包括：实发工资为0的记录、考勤扣除异常高的员工、"
                      "以及整体薪资结构的优化建议。");
}

// ----------------------------------------------------------------
// 槽: AI响应到达
// ----------------------------------------------------------------
void AiChatWidget::onAiResponseReceived(const QString& text)
{
    m_thinkingLabel->setVisible(false);
    appendAiBubble(text);
    m_sendBtn->setEnabled(true);
    m_reportBtn->setEnabled(true);
}

// ----------------------------------------------------------------
// 槽: AI出错
// ----------------------------------------------------------------
void AiChatWidget::onAiError(const QString& error)
{
    m_thinkingLabel->setVisible(false);
    appendSystemMessage("❌ " + error);
    m_sendBtn->setEnabled(true);
    m_reportBtn->setEnabled(true);
}

// ----------------------------------------------------------------
// 槽: AI开始思考
// ----------------------------------------------------------------
void AiChatWidget::onAiThinking()
{
    m_thinkingLabel->setVisible(true);
    scrollToBottom();
}

// ----------------------------------------------------------------
// 私有: 添加用户消息气泡
// ----------------------------------------------------------------
void AiChatWidget::appendUserBubble(const QString& text)
{
    QWidget* row = new QWidget();
    QHBoxLayout* rowLayout = new QHBoxLayout(row);
    rowLayout->setContentsMargins(8, 4, 8, 4);

    QLabel* bubble = new QLabel(text);
    bubble->setWordWrap(true);
    bubble->setMaximumWidth(360);
    bubble->setObjectName("userBubble");
    bubble->setTextInteractionFlags(Qt::TextSelectableByMouse);

    rowLayout->addStretch();
    rowLayout->addWidget(bubble);

    m_chatLayout->addWidget(row);
    scrollToBottom();
}

// ----------------------------------------------------------------
// 私有: 添加AI回复气泡（支持Markdown简单渲染）
// ----------------------------------------------------------------
void AiChatWidget::appendAiBubble(const QString& text)
{
    QWidget* row = new QWidget();
    QHBoxLayout* rowLayout = new QHBoxLayout(row);
    rowLayout->setContentsMargins(8, 4, 8, 4);

    // 用QTextBrowser支持富文本/Markdown
    QTextBrowser* bubble = new QTextBrowser();
    bubble->setMarkdown(text);
    bubble->setObjectName("aiBubble");
    bubble->setMaximumWidth(400);
    bubble->setMinimumHeight(60);
    // 动态调整高度
    bubble->document()->setPageSize(QSizeF(380, 99999));
    int docHeight = static_cast<int>(bubble->document()->size().height()) + 20;
    bubble->setFixedHeight(qMin(docHeight, 400));
    bubble->setReadOnly(true);
    bubble->setOpenExternalLinks(true);

    rowLayout->addWidget(bubble);
    rowLayout->addStretch();

    m_chatLayout->addWidget(row);
    scrollToBottom();
}

// ----------------------------------------------------------------
// 私有: 添加系统提示消息
// ----------------------------------------------------------------
void AiChatWidget::appendSystemMessage(const QString& text)
{
    QLabel* lbl = new QLabel(text);
    lbl->setWordWrap(true);
    lbl->setObjectName("systemMessage");
    lbl->setAlignment(Qt::AlignCenter);
    lbl->setContentsMargins(16, 8, 16, 8);

    m_chatLayout->addWidget(lbl);
    scrollToBottom();
}

// ----------------------------------------------------------------
// 私有: 滚动到最底部
// ----------------------------------------------------------------
void AiChatWidget::scrollToBottom()
{
    QScrollBar* sb = m_scrollArea->verticalScrollBar();
    // 用 QTimer 延迟一帧，确保布局已更新
    QTimer::singleShot(50, [sb]() {
        sb->setValue(sb->maximum());
    });
}

// ----------------------------------------------------------------
// 私有: 构建UI布局
// ----------------------------------------------------------------
void AiChatWidget::setupUi()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // 标题栏
    QLabel* titleLabel = new QLabel("🤖  薪智汇控 AI 助手");
    titleLabel->setObjectName("aiTitle");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setFixedHeight(44);
    mainLayout->addWidget(titleLabel);

    // 聊天区域（滚动）
    m_scrollArea    = new QScrollArea();
    m_chatContainer = new QWidget();
    m_chatLayout    = new QVBoxLayout(m_chatContainer);
    m_chatLayout->setAlignment(Qt::AlignTop);
    m_chatLayout->setSpacing(6);
    m_chatLayout->setContentsMargins(4, 8, 4, 8);

    m_scrollArea->setWidget(m_chatContainer);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setObjectName("chatArea");
    mainLayout->addWidget(m_scrollArea, 1);

    // "思考中" 指示
    m_thinkingLabel = new QLabel("  ⏳ AI 正在分析，请稍候...");
    m_thinkingLabel->setObjectName("thinkingLabel");
    m_thinkingLabel->setVisible(false);
    m_thinkingLabel->setFixedHeight(30);
    mainLayout->addWidget(m_thinkingLabel);

    // 快捷按钮行
    QHBoxLayout* quickBtnLayout = new QHBoxLayout();
    quickBtnLayout->setContentsMargins(8, 4, 8, 4);
    m_reportBtn = new QPushButton("📊 生成月度报告");
    m_trendBtn  = new QPushButton("🔍 异常分析");
    m_reportBtn->setObjectName("quickBtn");
    m_trendBtn->setObjectName("quickBtn");
    quickBtnLayout->addWidget(m_reportBtn);
    quickBtnLayout->addWidget(m_trendBtn);
    mainLayout->addLayout(quickBtnLayout);

    // 输入行
    QHBoxLayout* inputLayout = new QHBoxLayout();
    inputLayout->setContentsMargins(8, 4, 8, 8);
    m_inputEdit = new QLineEdit();
    m_inputEdit->setPlaceholderText("输入问题，按 Enter 发送...");
    m_inputEdit->setObjectName("chatInput");
    m_sendBtn   = new QPushButton("发送");
    m_sendBtn->setObjectName("sendBtn");
    m_sendBtn->setFixedWidth(64);
    inputLayout->addWidget(m_inputEdit);
    inputLayout->addWidget(m_sendBtn);
    mainLayout->addLayout(inputLayout);
}

// ----------------------------------------------------------------
// 私有: 应用样式表
void AiChatWidget::applyStyle()
{
    this->setStyleSheet(R"(
        QWidget {
            background-color: rgba(255, 255, 255, 0.75);
            color: #2c3e50;
            font-family: "Microsoft YaHei", "PingFang SC", sans-serif;
            border-radius: 12px;
            font-size: 14px;
        }
        QLabel#aiTitle {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #ffffff, stop:1 #eef6fc);
            color: #0f4c81;
            border: 1.5px solid #bcd6ed;
            font-size: 15px;
            font-weight: bold;
            border-top-left-radius: 12px;
            border-top-right-radius: 12px;
            border-bottom-left-radius: 0px;
            border-bottom-right-radius: 0px;
        }
        QScrollArea#chatArea {
            border: none;
            background-color: transparent;
        }
        QLabel#userBubble {
            background-color: #e0effa;
            color: #0f4c81;
            border: 1.5px solid #bcd6ed;
            border-radius: 12px;
            padding: 10px 14px;
            font-size: 14px;
        }
        QTextBrowser#aiBubble {
            background-color: rgba(255, 255, 255, 0.9);
            color: #2c3e50;
            border-radius: 12px;
            padding: 10px 14px;
            border: 1px solid #bcd6ed;
            font-size: 14px;
        }
        QLabel#systemMessage {
            background-color: rgba(255, 255, 255, 0.85);
            color: #555555;
            border-radius: 8px;
            font-size: 13px;
            border: 1px solid #bcd6ed;
        }
        QLabel#thinkingLabel {
            background-color: rgba(255, 255, 255, 0.8);
            color: #0f4c81;
            font-size: 13px;
        }
        QPushButton#quickBtn {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #ffffff, stop:1 #eef6fc);
            color: #0f4c81;
            border: 1.5px solid #bcd6ed;
            border-radius: 8px;
            padding: 6px 12px;
            font-size: 13px;
            font-weight: bold;
        }
        QPushButton#quickBtn:hover {
            background: #e0effa;
            border-color: #0f4c81;
        }
        QLineEdit#chatInput {
            background-color: #ffffff;
            border: 1.5px solid #bcd6ed;
            border-radius: 8px;
            padding: 8px 12px;
            color: #2c3e50;
            font-size: 14px;
        }
        QLineEdit#chatInput:focus {
            border-color: #0f4c81;
        }
        QPushButton#sendBtn {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #ffffff, stop:1 #eef6fc);
            color: #0f4c81;
            border: 1.5px solid #bcd6ed;
            border-radius: 8px;
            font-size: 14px;
            font-weight: bold;
        }
        QPushButton#sendBtn:hover {
            background: #e0effa;
            border-color: #0f4c81;
        }
        QPushButton#sendBtn:disabled {
            background: #f5f5f5;
            border-color: #e0e0e0;
            color: #b0bec5;
        }
    )");
}
