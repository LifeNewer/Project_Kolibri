#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void startProcessing();

private:
    Ui::MainWindow *ui;

    QByteArray modifyData(const QByteArray& data, const QByteArray& key);
};

#endif // MAINWINDOW_H
