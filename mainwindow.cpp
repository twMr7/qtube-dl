#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QScrollBar>
#include <QImageReader>
#include <QPixmap>
#include <QSize>
#include <QTextStream>
#include <Qdir>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    YOUTUBE_DL{ "youtube-dl.exe" }
{
    ui->setupUi(this);

    // setup process and signal-slot
    process = new QProcess(this);
    connect(process, &QProcess::started,
            this, &MainWindow::on_youtubedl_started);
    connect(process, &QProcess::readyReadStandardOutput,
            this, &MainWindow::on_youtubedl_ready2read);
    connect(process, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            this, &MainWindow::on_youtubedl_finished);

    // check version of youtube-dl
    QString getVersionResult = runYoutubeDl(QStringList() << "--version");
    statusBar()->showMessage(YOUTUBE_DL + " " + getVersionResult);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::toggleControls(bool enable)
{
    ui->editUrl->setEnabled(enable);
    ui->btnDownload->setEnabled(enable);
    ui->btnGetInfo->setEnabled(enable);
    ui->groupOptions->setEnabled(enable);
}

QString MainWindow::runYoutubeDl(QStringList &arguments)
{
    toggleControls(false);
    // run synchronous youtube-dl process
    QProcess syncProcess;
    syncProcess.setProcessChannelMode(QProcess::MergedChannels);
    syncProcess.start(YOUTUBE_DL, arguments);
    QString strResult;
    if (syncProcess.waitForStarted() && syncProcess.waitForReadyRead())
    {
        QTextStream stream(syncProcess.readAllStandardOutput());
        strResult = stream.readAll();
    }
    else
    {
        switch (syncProcess.error())
        {
        case QProcess::FailedToStart:
            strResult ="Failed to start.";
            break;
        case QProcess::Crashed:
            strResult = "crashed!";
            break;
        case QProcess::Timedout:
            strResult = "Executing timeout.";
            break;
        case QProcess::ReadError:
        case QProcess::WriteError:
        case QProcess::UnknownError:
        default:
            strResult = "Unknown error.";
            break;
        }
    }
    syncProcess.waitForFinished();
    toggleControls(true);
    return strResult;
}

QString MainWindow::findThumbnailFileName(QString &rawoutput)
{
    QTextStream rawstream(&rawoutput);
    QString aline;
    QString strTofind = "Writing thumbnail to: ";
    QString filename;
    while (rawstream.readLineInto(&aline))
    {
        int pos = aline.indexOf(strTofind);
        if (pos != -1)
        {
            filename = aline.right(aline.length() - pos - strTofind.length());
            break;
        }
    }
    return filename;
}

void MainWindow::on_checkExtractAudio_toggled(bool checked)
{
    ui->checkKeepVideo->setEnabled(checked);
    ui->labelAudioFormat->setEnabled(checked);
    ui->selectAudioFormat->setEnabled(checked);
}

void MainWindow::on_btnGetInfo_clicked()
{
    if (ui->editUrl->text().isEmpty())
    {
        QMessageBox::information(this, tr("Missing URL"), tr("Video URL is necessary for download."));
        return;
    }

    // get description
    outputText = runYoutubeDl(QStringList()
                              << "--no-playlist"
                              << "--get-title"
                              << "--get-description"
                              << ui->editUrl->text());
    outputText += "\r\n----------------\r\n";

    // then get thumbnail image
    QStringList arguments;
    arguments << "-o" << "./download/%(title)s.%(ext)s"
              << "--no-playlist"
              << "--write-thumbnail"
              << "--skip-download"
              << ui->editUrl->text();
    // start asynchronous youtube-dl process
    process->setProcessChannelMode(QProcess::MergedChannels);
    process->start(YOUTUBE_DL, arguments);
}

void MainWindow::on_btnDownload_clicked()
{
    if (ui->editUrl->text().isEmpty())
    {
        QMessageBox::information(this, tr("Missing URL"), tr("Video URL is necessary for download."));
        return;
    }

    QStringList arguments;
    arguments << "-o" << "./download/%(title)s.%(ext)s"
              << "--no-playlist"
              << "--write-thumbnail";
    if (ui->checkExtractAudio->isChecked()) {
        arguments << "-x"
                  << "--audio-format" << ui->selectAudioFormat->currentText()
                  << "--audio-quality" << "0"
                  << "--ffmpeg-location" << "./ffmpeg/bin";
        if (ui->checkKeepVideo->isChecked())
            arguments << "--keep-video";
    }
    arguments << ui->editUrl->text();
    // start asynchronous youtube-dl process
    process->setProcessChannelMode(QProcess::MergedChannels);
    process->start(YOUTUBE_DL, arguments);
}

void MainWindow::on_btnOpenFolder_clicked()
{
    const QString explorer = "explorer.exe";
    // open the default download folder
    QString param = "\"" + QDir::toNativeSeparators(QApplication::applicationDirPath() + "/download/\"");
    QString command = explorer + " " + param;
    QProcess::startDetached(command);
}

void MainWindow::on_youtubedl_started()
{
    toggleControls(false);
}

void MainWindow::on_youtubedl_ready2read()
{
    QTextStream stream(process->readAllStandardOutput());
    QString strResult = stream.readAll();
    outputText += strResult;
    ui->editProcessOutput->setText(outputText);
    ui->editProcessOutput->verticalScrollBar()->setSliderPosition(
        ui->editProcessOutput->verticalScrollBar()->maximum());
}

void MainWindow::on_youtubedl_finished(int exitCode, QProcess::ExitStatus exitStatus)
{
    // try to find thumbnail file name from output text
    thumbnailFile = findThumbnailFileName(outputText);
    if (!thumbnailFile.isEmpty())
    {
        QImageReader imgreader(thumbnailFile);
        imgreader.setAutoTransform(true);
        QImage newimage = imgreader.read();
        if (!newimage.isNull()) {
            float scaleFactor = (float)ui->editProcessOutput->height() / newimage.size().height();
            QSize newsize = newimage.size() * scaleFactor;
            ui->labelThumbnail->setPixmap(QPixmap::fromImage(newimage.scaled(newsize, Qt::KeepAspectRatio)));
        }
        else
        {
            ui->labelThumbnail->setPixmap(QPixmap());
        }
    }

    statusBar()->showMessage(QString("%1 is done, code: %2, status: %3").arg(YOUTUBE_DL).arg(exitCode).arg(exitStatus));

    // clear the output text buffer
    outputText.clear();
    // reset control status
    toggleControls(true);
}

void MainWindow::on_actionAbout_triggered()
{
   QMessageBox::about(this, "About qtube-dl",
                      "<p><center><b>qtube-dl 1.0.0</b></center><p>"
                      "<p><a href='https://github.com/twMr7/qtube-dl'>https://github.com/twMr7/qtube-dl</a>  <br/>"
                      "James Chang &lt;twmr7@outlook.com&gt;  <br/></p>");
}
