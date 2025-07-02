#ifndef SIMULATORWINDOW_H
#define SIMULATORWINDOW_H

#include <QMainWindow>
#include <array>
#include "cpu.h" 


class QTableWidget;
class QPushButton;
class QTextBrowser;
class QTimer;
class QTableWidgetItem;

class SimulatorWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit SimulatorWindow(const QString &binaryFilePath, QWidget *parent = nullptr);
    ~SimulatorWindow();

private slots:
    void onClockClicked();
    void onRunStopClicked();
    void autoClockTick();

private:
    
    void setupUI();
    bool loadProgram(const QString &filePath);
    void updateUI(const CycleChanges &changes);

    
    void highlightRegister(int regIndex);
    void updateAllRegisterTables();

    
    void populateMemoryTable();
    void updateMemoryTable(uint32_t address, const std::string& mnemonic);
    void highlightMemoryCell(int row);

    
    QString formatCycleState(const CycleChanges &changes);
    QString getStageName(PipelineStage stage);

    
    QTextBrowser *cycleStateBox;
    std::array<QTableWidget*, 4> registerTables;
    std::array<QTableWidgetItem*, 32> registerItems;
    QPushButton *clockButton;
    QPushButton *runStopButton;
    QTableWidget *memoryTable; 

    
    CPU cpu;
    QTimer *runTimer;
    bool isRunning;
};

#endif // SIMULATORWINDOW_H
