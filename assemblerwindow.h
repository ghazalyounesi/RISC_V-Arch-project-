#ifndef ASSEMBLERWINDOW_H
#define ASSEMBLERWINDOW_H

#include <QMainWindow>
#include "parser.h" // فایل هدر اصلی منطق اسمبلر شما
#include <QPlainTextEdit>
#include <QTextBrowser>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
// Forward declarations for Qt classes to speed up compilation
class QPlainTextEdit;
class QPushButton;
class QTextBrowser;

// تعریف کلاس برای پنجره اصلی اسمبلر
class AssemblerWindow : public QMainWindow
{
    Q_OBJECT

public:
    AssemblerWindow(QWidget *parent = nullptr);
    ~AssemblerWindow();

private slots:
    // این اسلات مسئول باز کردن پنجره بعدی است
    void saveAndProceed();

    // سایر اسلات‌ها
    void onAssemblyCodeChanged();
    void submitCode();
    void runFirstPass();
    void runSecondPass();

private:
    void setupUI();
    void resetToInitialState();

    // ویجت‌های اصلی در UI
    QPlainTextEdit *assemblyCodeEditor;
    QTextBrowser *outputBox;
    QTextBrowser *warningsBox;
    QPushButton *submitButton;
    QPushButton *firstPassButton;
    QPushButton *secondPassButton;
    QPushButton *nextButton;

    // نمونه‌ای از کلاس Parser شما
    Parser assembler;

    // متغیرهایی برای نگهداری وضعیت
    QString submittedCode;
    SecondPassResult secondPassResult;
};

#endif // ASSEMBLERWINDOW_H
