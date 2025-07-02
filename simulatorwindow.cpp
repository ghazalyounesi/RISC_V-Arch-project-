#include "simulatorwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QSplitter>
#include <QTableWidget>
#include <QLabel>
#include <QMessageBox>
#include <QHeaderView>
#include <QApplication>
#include <QTextStream>
#include <QTextBrowser> 
#include <QPushButton>  
#include <QTimer>       

SimulatorWindow::SimulatorWindow(const QString &binaryFilePath, QWidget *parent)
    : QMainWindow(parent), isRunning(false)
{
    setupUI();
    if (!loadProgram(binaryFilePath)) {
        QMessageBox::critical(this, "Error", "Failed to load program: " + binaryFilePath);
        QTimer::singleShot(0, this, &SimulatorWindow::close);
    } else {
        updateAllRegisterTables();
        populateMemoryTable(); 
    }
}

SimulatorWindow::~SimulatorWindow()
{
    delete runTimer;
}

void SimulatorWindow::setupUI()
{
   
    cycleStateBox = new QTextBrowser();
    cycleStateBox->setFont(QFont("Monospace", 10));
    clockButton = new QPushButton("Clock");
    runStopButton = new QPushButton("Run");
    runTimer = new QTimer(this);

    
    memoryTable = new QTableWidget(this);
    memoryTable->setColumnCount(2);
    memoryTable->setHorizontalHeaderLabels({"Address", "Value (Hex)"});
    memoryTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    memoryTable->verticalHeader()->setVisible(false);
    memoryTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

   
    QWidget *centralWidget = new QWidget;
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    QSplitter *topSplitter = new QSplitter(Qt::Horizontal);

    QWidget *registersWidget = new QWidget;
    QGridLayout *registersLayout = new QGridLayout(registersWidget);
    for (int i = 0; i < 4; ++i) {
        registerTables[i] = new QTableWidget(1, 8, this);
        registerTables[i]->verticalHeader()->setVisible(false);
        QStringList headers;
        for (int j = 0; j < 8; ++j) {
            int regIndex = i * 8 + j;
            headers << QString("x%1").arg(regIndex);
            registerItems[regIndex] = new QTableWidgetItem("0x00000000");
            registerItems[regIndex]->setTextAlignment(Qt::AlignCenter);
            registerItems[regIndex]->setFlags(registerItems[regIndex]->flags() & ~Qt::ItemIsEditable);
            registerTables[i]->setItem(0, j, registerItems[regIndex]);
        }
        registerTables[i]->setHorizontalHeaderLabels(headers);
        registerTables[i]->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        registerTables[i]->setFixedHeight(registerTables[i]->rowHeight(0) + registerTables[i]->horizontalHeader()->height() + 5);
        registersLayout->addWidget(registerTables[i], i, 0);
    }
    topSplitter->addWidget(registersWidget);

    
    QSplitter *rightSplitter = new QSplitter(Qt::Vertical);

    QWidget* cycleWidget = new QWidget;
    QVBoxLayout* cycleLayout = new QVBoxLayout(cycleWidget);
    cycleLayout->addWidget(new QLabel("Cycle State & Details:"));
    cycleLayout->addWidget(cycleStateBox);
    rightSplitter->addWidget(cycleWidget);

    QWidget* memoryWidget = new QWidget;
    QVBoxLayout* memoryLayout = new QVBoxLayout(memoryWidget);
    memoryLayout->addWidget(new QLabel("Memory View:"));
    memoryLayout->addWidget(memoryTable);
    rightSplitter->addWidget(memoryWidget);

    topSplitter->addWidget(rightSplitter);
    topSplitter->setStretchFactor(0, 2); 
    topSplitter->setStretchFactor(1, 1); 

    
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(clockButton);
    buttonLayout->addWidget(runStopButton);
    buttonLayout->addStretch();

    mainLayout->addWidget(topSplitter);
    mainLayout->addLayout(buttonLayout);
    setCentralWidget(centralWidget);

    QString styleSheet = R"(
        QMainWindow, QWidget {
            background-color: #F4F6FC; 
            font-family: Segoe UI, Arial;
            font-size: 10pt;
            color: #333;
        }
        QLabel {
            font-weight: bold;
            color: #5D3587; 
            padding-top: 5px;
        }
        QPushButton {
            background-color: #6A5ACD; 
            color: white;
            border: none;
            padding: 8px 16px;
            border-radius: 5px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #836FFF; 
        }
        QPushButton:disabled {
            background-color: #C9C5E3;
            color: #888;
        }
        QTextBrowser, QTableWidget {
            background-color: #FFFFFF;
            border: 1px solid #D1D9E6;
            border-radius: 5px;
        }
        QTableWidget {
            font-family: Consolas, "Courier New", monospace;
        }
        QHeaderView::section {
            background-color: #A084E8; 
            color: white;
            padding: 4px;
            border: none;
            font-weight: bold;
        }
        QTableWidget {
            gridline-color: #EAEBFC; 
        }
        QSplitter::handle {
            background-color: #D1D9E6;
        }
        QSplitter::handle:hover {
            background-color: #A084E8;
        }
        QSplitter::handle:vertical {
            height: 5px;
        }
        QSplitter::handle:horizontal {
            width: 5px;
        }
    )";
    this->setStyleSheet(styleSheet);


    connect(clockButton, &QPushButton::clicked, this, &SimulatorWindow::onClockClicked);
    connect(runStopButton, &QPushButton::clicked, this, &SimulatorWindow::onRunStopClicked);
    connect(runTimer, &QTimer::timeout, this, &SimulatorWindow::autoClockTick);

    setWindowTitle("RISC-V Simulator");
    resize(1400, 800);
}

void SimulatorWindow::populateMemoryTable()
{
    const auto& memData = cpu.get_memory().get_all_data();
    memoryTable->setRowCount(memData.size());

    for (size_t i = 0; i < memData.size(); ++i) {
        QTableWidgetItem *addressItem = new QTableWidgetItem(QString("0x%1").arg(i, 4, 16, QChar('0')));
        memoryTable->setItem(i, 0, addressItem);

        QTableWidgetItem *valueItem = new QTableWidgetItem(QString("0x%1").arg(memData[i], 2, 16, QChar('0')));
        valueItem->setTextAlignment(Qt::AlignCenter);
        memoryTable->setItem(i, 1, valueItem);
    }
}

void SimulatorWindow::updateMemoryTable(uint32_t address, const std::string& mnemonic)
{
    int bytes_to_update = 0;
    if (mnemonic == "sw") bytes_to_update = 4;
    else if (mnemonic == "sh") bytes_to_update = 2;
    else if (mnemonic == "sb") bytes_to_update = 1;

    if (bytes_to_update == 0) return;

    if (address < (uint32_t)memoryTable->rowCount()) {
        memoryTable->scrollToItem(memoryTable->item(address, 0), QAbstractItemView::PositionAtTop);
    }

    for (int i = 0; i < bytes_to_update; ++i) {
        uint32_t current_addr = address + i;
        if (current_addr < MEMORY_SIZE) {
            uint8_t newValue = cpu.get_memory().read_byte(current_addr);
            memoryTable->item(current_addr, 1)->setText(QString("0x%1").arg(newValue, 2, 16, QChar('0')));
            highlightMemoryCell(current_addr);
        }
    }
}

void SimulatorWindow::highlightMemoryCell(int row)
{
    if (row < 0 || row >= memoryTable->rowCount()) return;

    QTableWidgetItem *item = memoryTable->item(row, 1);
    if(item) {
        item->setBackground(QColor(205, 220, 255)); 
        QTimer::singleShot(500, this, [item](){
            if (item) item->setBackground(Qt::white);
        });
    }
}

void SimulatorWindow::updateUI(const CycleChanges &changes)
{
    cycleStateBox->setText(formatCycleState(changes));

    if (changes.register_write) {
        highlightRegister(changes.register_write->first);
    }

    if (changes.instruction_finished) {
        updateAllRegisterTables();
    }

    if (changes.memory_write) {
        updateMemoryTable(changes.memory_write->first, cpu.get_current_instruction().mnemonic);
    }
}

bool SimulatorWindow::loadProgram(const QString &filePath)
{
    try {
        if (!cpu.load_program(filePath.toStdString())) {
            return false;
        }
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Runtime Error", e.what());
        return false;
    }
    cycleStateBox->setText("Program loaded successfully.\nPC: 0x" + QString::number(cpu.get_pc(), 16));
    return true;
}

void SimulatorWindow::onClockClicked()
{
    if (cpu.is_halted()) {
        QMessageBox::information(this, "Halted", "CPU is halted. Cannot execute further.");
        if(isRunning) onRunStopClicked();
        clockButton->setEnabled(false);
        runStopButton->setEnabled(false);
        return;
    }

    try {
        CycleChanges changes = cpu.clock_tick();
        updateUI(changes);
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Runtime Error", e.what());
        if(isRunning) onRunStopClicked();
        clockButton->setEnabled(false);
        runStopButton->setEnabled(false);
    }
}

void SimulatorWindow::onRunStopClicked()
{
    isRunning = !isRunning;
    if (isRunning) {
        runStopButton->setText("Stop");
        clockButton->setEnabled(false);
        runTimer->start(500);
    } else {
        runStopButton->setText("Run");
        clockButton->setEnabled(true);
        runTimer->stop();
    }
}

void SimulatorWindow::autoClockTick()
{
    onClockClicked();
    QApplication::processEvents();
}

void SimulatorWindow::highlightRegister(int regIndex)
{
    if (regIndex > 0 && regIndex < 32) {
        QTableWidgetItem *item = registerItems[regIndex];

        uint32_t newValue = cpu.get_registers().read(regIndex);
        item->setText("0x" + QString("%1").arg(newValue, 8, 16, QChar('0')));

        item->setBackground(QColor(255, 250, 205));

        QTimer::singleShot(500, this, [item](){
            if (item) item->setBackground(Qt::white);
        });
    }
}

void SimulatorWindow::updateAllRegisterTables()
{
    auto regs = cpu.get_registers().get_all_registers();
    for (int i = 0; i < 32; ++i) {
        registerItems[i]->setText("0x" + QString("%1").arg(regs[i], 8, 16, QChar('0')));
    }
}

QString SimulatorWindow::formatCycleState(const CycleChanges &changes)
{
    QString state;
    QTextStream stream(&state);

    stream << "============================================\n"
           << "PC: 0x" << QString("%1").arg(changes.current_pc, 8, 16, QChar('0'))
           << " | Stage: " << getStageName(changes.stage)
           << " | Instruction: " << QString::fromStdString(cpu.get_current_instruction().mnemonic) << "\n"
           << "RTL: " << QString::fromStdString(changes.rtl_description) << "\n";

    if (changes.register_write) {
        stream << ">> REGISTER WRITE: x" << changes.register_write->first
               << " = 0x" << QString("%1").arg(changes.register_write->second, 8, 16, QChar('0')) << "\n";
    }
    if (changes.memory_write) {
        stream << ">> MEMORY WRITE: M[0x" << QString("%1").arg(changes.memory_write->first, 8, 16, QChar('0'))
        << "] = 0x" << QString("%1").arg(changes.memory_write->second, 8, 16, QChar('0')) << "\n";
    }
    if (changes.memory_read_addr) {
        stream << ">> MEMORY READ: Addr=0x" << QString("%1").arg(*changes.memory_read_addr, 8, 16, QChar('0')) << "\n";
    }
    if (changes.instruction_finished) {
        stream << "--- Instruction Finished ---\n";
    }
    stream << "============================================\n";

    return state;
}

QString SimulatorWindow::getStageName(PipelineStage stage) {
    switch(stage) {
    case PipelineStage::FETCH:      return "Instruction Fetch";
    case PipelineStage::DECODE:     return "Instruction Decode";
    case PipelineStage::EXECUTE:    return "Execute";
    case PipelineStage::MEMORY:     return "Memory Access";
    case PipelineStage::WRITE_BACK: return "Write Back";
    case PipelineStage::IDLE:       return "Idle";
    default:                        return "Unknown";
    }
}
