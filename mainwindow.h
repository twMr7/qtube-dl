#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProcess>
#include <QString>
#include <QStringList>
#include <QImage>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    const QString YOUTUBE_DL;
    QProcess *process;
    QString outputText;
    QString thumbnailFile;
    QImage thumbnail;

protected:
    void toggleControls(bool enable);
    QString runYoutubeDl(QStringList &arguments);
    QString findThumbnailFileName(QString &rawoutput);

private slots:
    void on_checkExtractAudio_toggled(bool checked);
    void on_btnGetInfo_clicked();
    void on_btnDownload_clicked();
    void on_btnOpenFolder_clicked();
    void on_youtubedl_started();
    void on_youtubedl_ready2read();
    void on_youtubedl_finished(int exitCode, QProcess::ExitStatus exitStatus);

    void on_actionAbout_triggered();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
