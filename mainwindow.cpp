#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Устанавливаем значения по умолчанию
    ui->deleteInputCheckBox->setChecked(false);
    ui->overwriteRadioButton->setChecked(true);
    ui->timerIntervalSpinBox->setValue(1000);

    // Соединяем сигнал нажатия кнопки "Старт" со слотом startProcessing()
    connect(ui->startButton, &QPushButton::clicked, this, &MainWindow::startProcessing);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::startProcessing()
{
    QString inputPath = ui->inputPathLineEdit->text();
    QString outputPath = ui->outputPathLineEdit->text();
    QString fileMask = ui->fileMaskLineEdit->text();
    QString key = ui->keyLineEdit->text();
    bool deleteInputFile = ui->deleteInputCheckBox->isChecked();
    int timerInterval = ui->timerIntervalSpinBox->value();

    QDir inputDir(inputPath);
    QDir outputDir(outputPath);

    if (!inputDir.exists() || !outputDir.exists()) {
        QMessageBox::critical(this, "Ошибка", "Некорректный путь к директории");
        return;
    }

    QStringList filters;
    filters << fileMask;

    inputDir.setNameFilters(filters);

    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, [this, &inputDir, &outputDir, &key, deleteInputFile]() {
        QStringList inputFileList = inputDir.entryList();

        if (inputFileList.isEmpty()) {
            return;
        }

        foreach (QString fileName, inputFileList) {
            QString inputFilePath = inputDir.absoluteFilePath(fileName);
            QString outputFilePath = outputDir.absoluteFilePath(fileName);

            // Защита от "дурака"
            QFile inputFile(inputFilePath);
            if (!inputFile.open(QIODevice::ReadWrite)) {
                continue;
            }
            inputFile.close();

            QFile outputFile(outputFilePath);
            if (ui->overwriteRadioButton->isChecked() || !outputFile.exists()) {
                // Перезапись или файл не существует - производим модификацию
                if (inputFile.open(QIODevice::ReadOnly) && outputFile.open(QIODevice::WriteOnly)) {
                    QByteArray inputData = inputFile.readAll();
                    QByteArray keyData = key.toLatin1();
                    QByteArray modifiedData = modifyData(inputData, keyData);
                    outputFile.write(modifiedData);
                    inputFile.close();
                    outputFile.close();

                    if (deleteInputFile) {
                        inputDir.remove(fileName);
                    }
                } else {
                    QMessageBox::critical(this, "Ошибка", "Не удалось открыть файлы для чтения/записи");
                }
            } else {
                // Модификация имени файла
                int counter = 1;
                QString baseName = outputDir.absoluteFilePath(QFileInfo(outputFilePath).baseName());
                QString suffix = QFileInfo(outputFilePath).suffix();

                while (outputFile.exists()) {
                    outputFilePath = baseName + "_" + QString::number(counter) + "." + suffix;
                    counter++;
                }

                if (inputFile.open(QIODevice::ReadOnly) && outputFile.open(QIODevice::WriteOnly)) {
                    QByteArray inputData = inputFile.readAll();
                    QByteArray keyData = key.toLatin1();
                    QByteArray modifiedData = modifyData(inputData, keyData);
                    outputFile.write(modifiedData);
                    inputFile.close();
                    outputFile.close();

                    if (deleteInputFile) {
                        inputDir.remove(fileName);
                    }
                } else {
                    QMessageBox::critical(this, "Ошибка", "Не удалось открыть файлы для чтения/записи");
                }
            }
        }
    });

    timer->start(timerInterval);
}

QByteArray MainWindow::modifyData(const QByteArray& data, const QByteArray& key)
{
    QByteArray modifiedData;
    modifiedData.resize(data.size());

    for (int i = 0; i < data.size(); ++i) {
        modifiedData[i] = data[i] ^ key[i % key.size()];
    }

    return modifiedData;
}
