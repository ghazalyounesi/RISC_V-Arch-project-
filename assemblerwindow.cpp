#include "assemblerwindow.h"
#include <QLabel>
#include <QGroupBox>
#include "simulatorwindow.h"
// سازنده کلاس پنجره اسمبلر
AssemblerWindow::AssemblerWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUI(); // تابع کمکی برای ساخت و چیدمان تمام ویجت‌ها
}

AssemblerWindow::~AssemblerWindow()
{
    // در اینجا می‌توانید حافظه تخصیص داده شده را آزاد کنید اگر لازم باشد
}

// تابع اصلی برای ساخت رابط کاربری
void AssemblerWindow::setupUI()
{
    // 1. ساخت ویجت‌ها
    assemblyCodeEditor = new QPlainTextEdit();
    assemblyCodeEditor->setPlaceholderText("Enter your assembly code here..."); // متن پیش‌فرض

    outputBox = new QTextBrowser();
    warningsBox = new QTextBrowser();

    submitButton = new QPushButton("Submit Assembly Code");
    firstPassButton = new QPushButton("Run First Pass");
    secondPassButton = new QPushButton("Run Second Pass");
    nextButton = new QPushButton("Save and Proceed to Simulator");

    // 2. چیدمان ویجت‌ها
    QVBoxLayout *mainLayout = new QVBoxLayout();
    QHBoxLayout *editorLayout = new QHBoxLayout();
    QHBoxLayout *buttonLayout = new QHBoxLayout();

    // بخش ویرایشگر و خروجی‌ها
    QVBoxLayout *leftLayout = new QVBoxLayout();
    leftLayout->addWidget(new QLabel("Assembly Code Input:"));
    leftLayout->addWidget(assemblyCodeEditor, 2); // 2/3 فضا برای ویرایشگر

    QVBoxLayout *rightLayout = new QVBoxLayout();
    rightLayout->addWidget(new QLabel("Output:"));
    rightLayout->addWidget(outputBox, 1); // 1/2 فضا برای خروجی
    rightLayout->addWidget(new QLabel("Warnings/Errors:"));
    rightLayout->addWidget(warningsBox, 1); // 1/2 فضا برای هشدار

    editorLayout->addLayout(leftLayout, 2); // بخش چپ (ویرایشگر) 2 برابر بخش راست فضا می‌گیرد
    editorLayout->addLayout(rightLayout, 1);

    // بخش دکمه‌ها
    buttonLayout->addWidget(submitButton);
    buttonLayout->addWidget(firstPassButton);
    buttonLayout->addWidget(secondPassButton);
    buttonLayout->addWidget(nextButton);

    mainLayout->addLayout(editorLayout);
    mainLayout->addLayout(buttonLayout);

    // قرار دادن چیدمان اصلی در یک ویجت مرکزی
    QWidget *centralWidget = new QWidget();
    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);

    QString styleSheet = R"(
        QMainWindow, QWidget {
            background-color: #F4F6FC; /* پس‌زمینه اصلی بسیار روشن */
            font-family: Segoe UI, Arial;
            font-size: 10pt;
            color: #333;
        }
        QLabel {
            font-weight: bold;
            color: #5D3587; /* بنفش تیره برای عنوان‌ها */
            padding-top: 5px;
        }
        QPushButton {
            background-color: #6A5ACD; /* آبی-بنفش برای دکمه‌ها */
            color: white;
            border: none;
            padding: 8px 16px;
            border-radius: 5px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #836FFF; /* روشن‌تر هنگام هاور */
        }
        QPushButton:disabled {
            background-color: #C9C5E3; /* بنفش بسیار کم‌رنگ */
            color: #888;
        }
        QTextBrowser, QPlainTextEdit {
            background-color: #FFFFFF;
            border: 1px solid #D1D9E6;
            border-radius: 5px;
            font-family: Consolas, "Courier New", monospace;
        }
    )";
    this->setStyleSheet(styleSheet);

    // 3. اتصال سیگنال‌ها به اسلات‌ها
    connect(assemblyCodeEditor, &QPlainTextEdit::textChanged, this, &AssemblerWindow::onAssemblyCodeChanged);
    connect(submitButton, &QPushButton::clicked, this, &AssemblerWindow::submitCode);
    connect(firstPassButton, &QPushButton::clicked, this, &AssemblerWindow::runFirstPass);
    connect(secondPassButton, &QPushButton::clicked, this, &AssemblerWindow::runSecondPass);
    connect(nextButton, &QPushButton::clicked, this, &AssemblerWindow::saveAndProceed);

    // 4. تنظیم وضعیت اولیه UI
    resetToInitialState();
    setWindowTitle("RISC-V Assembler");
}

// تنظیم UI به حالت اولیه
void AssemblerWindow::resetToInitialState()
{
    submitButton->setEnabled(false); // در ابتدا غیرفعال است
    firstPassButton->setEnabled(false);
    secondPassButton->setEnabled(false);
    nextButton->setEnabled(false);
    assemblyCodeEditor->setReadOnly(false);
    outputBox->clear();
    warningsBox->clear();
}

// وقتی متن داخل ویرایشگر تغییر می‌کند
void AssemblerWindow::onAssemblyCodeChanged()
{
    // اگر متنی وجود داشته باشد، دکمه submit فعال می‌شود
    submitButton->setEnabled(!assemblyCodeEditor->toPlainText().isEmpty());
}

// وقتی دکمه "Submit" کلیک می‌شود
void AssemblerWindow::submitCode()
{
    submittedCode = assemblyCodeEditor->toPlainText();
    if (submittedCode.isEmpty()) {
        QMessageBox::warning(this, "Empty Code", "Cannot submit empty code.");
        return;
    }

    // قفل کردن ویرایشگر و فعال کردن pass 1
    assemblyCodeEditor->setReadOnly(true);
    submitButton->setEnabled(false);
    firstPassButton->setEnabled(true);
    secondPassButton->setEnabled(false);
    nextButton->setEnabled(false);

    QMessageBox::information(this, "Code Submitted", "Assembly code has been submitted. You can now run the first pass.");
}

// وقتی دکمه "First Pass" کلیک می‌شود
void AssemblerWindow::runFirstPass()
{
    // اجرای منطق Pass 1 از کلاس Parser
    FirstPassResult result = assembler.runFirstPass(submittedCode.toStdString());

    // نمایش خروجی و هشدارها
    outputBox->setText(QString::fromStdString(result.symbol_table_output));

    warningsBox->clear();
    if (!result.warnings.empty()) {
        QString warningsText;
        for (const auto& warning : result.warnings) {
            warningsText += QString::fromStdString(warning) + "\n";
        }
        warningsBox->setText(warningsText);
    } else {
        warningsBox->setText("No warnings or errors.");
    }

    if (result.success) {
        QMessageBox::information(this, "First Pass Successful", "First pass completed successfully. You can now run the second pass.");
        firstPassButton->setEnabled(false);
        secondPassButton->setEnabled(true); // فعال کردن دکمه Pass 2
    } else {
        QMessageBox::critical(this, "First Pass Failed", "First pass failed due to fatal errors. Please correct the code and submit again.");
        // بازگرداندن UI به حالتی که کاربر بتواند کد را ویرایش کند
        assemblyCodeEditor->setReadOnly(false);
        submitButton->setEnabled(true);
        firstPassButton->setEnabled(false);
    }
}

// وقتی دکمه "Second Pass" کلیک می‌شود
void AssemblerWindow::runSecondPass()
{
    // اجرای منطق Pass 2
    secondPassResult = assembler.runSecondPass(submittedCode.toStdString());

    // نمایش خروجی باینری
    outputBox->setText(QString::fromStdString(secondPassResult.binary_hex_output));

    if (secondPassResult.success) {
        QMessageBox::information(this, "Second Pass Successful", "Second pass completed successfully. You can now save the binary file and proceed.");
        secondPassButton->setEnabled(false);
        nextButton->setEnabled(true); // فعال کردن دکمه "Next"
    } else {
        QMessageBox::critical(this, "Second Pass Failed", "A fatal error occurred during the second pass. Please review your code.");
        // بازگرداندن UI برای ویرایش مجدد
        assemblyCodeEditor->setReadOnly(false);
        submitButton->setEnabled(true);
        firstPassButton->setEnabled(false);
        secondPassButton->setEnabled(false);
    }
}


void AssemblerWindow::saveAndProceed()
{
    // باز کردن پنجره ذخیره فایل
    QString fileName = QFileDialog::getSaveFileName(this, "Save Binary File", "", "Binary Files (*.bin);;All Files (*)");

    if (fileName.isEmpty()) {
        // اگر کاربر پنجره را ببندد یا کنسل کند، کاری انجام نده
        return;
    }

    // نوشتن داده‌های باینری در فایل
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::critical(this, "Save Error", "Could not open file for writing: " + file.errorString());
        return;
    }

    const auto& binaryData = secondPassResult.binary_data;
    file.write(reinterpret_cast<const char*>(binaryData.data()), binaryData.size());
    file.close();

    QMessageBox::information(this, "File Saved", "Binary file has been saved successfully to:\n" + fileName);

    // ۲. این پنجره (اسمبلر) را مخفی کن
    this->hide();

    // ۳. یک نمونه از پنجره شبیه‌ساز بساز و مسیر فایل باینری را به آن بده
    SimulatorWindow *simWindow = new SimulatorWindow(fileName, nullptr);

    // ۴. اطمینان حاصل کن که با بسته شدن پنجره شبیه‌ساز، حافظه آن آزاد می‌شود
    simWindow->setAttribute(Qt::WA_DeleteOnClose);

    // ۵. پنجره شبیه‌ساز را نمایش بده
    simWindow->show();
}
