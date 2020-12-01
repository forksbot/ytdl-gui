#include "mainactions.h"
#include <string>
#include <iostream>
#include <QPushButton>
#include <QMessageBox>
#include <cstdlib>
#include <unistd.h>
#include <QThread>

std::string whitespace = " ";
std::string quote = "'";

mainActions::mainActions(QObject *parent) : QObject(parent)
{
        ytdl* window = ytdl::getWinInstance();
        Ui::ytdl* ui = ytdl::getUiInstance();

        connect(ui->buttonDownload, SIGNAL(released()), window, SLOT(downloadAction()));

}

std::string QString_to_str(QString input) {
        std::string output = input.toUtf8().constData();
        return output;
}


//Begin ytdl namespace

void ytdl::run_ytdl(std::string input) {
    //initialize thread
    QThread* downloadThread = new QThread;
    mainCommand* download_instance = new mainCommand(input);
    download_instance->moveToThread(downloadThread);

    //start thread
    connect(downloadThread, SIGNAL(started()), this, SLOT(messageDownload()));
    connect(downloadThread, SIGNAL(started()), download_instance, SLOT(download()));
    connect(download_instance, SIGNAL(returnFinished(int)), this, SLOT(printResult(int)));

    //delete thread
    connect(download_instance, SIGNAL(finished()), downloadThread, SLOT(quit()));
    connect(this, SIGNAL(userAccepted()), download_instance, SLOT(deleteLater()));
    connect(this, SIGNAL(userAccepted()), downloadThread, SLOT(deleteLater()));

    //execute
    downloadThread->start();

}

void ytdl::messageDownload() {
    downloading = new QMessageBox;
    downloading->setStandardButtons(QMessageBox::NoButton);
    downloading->setText("Downloading...");
    downloading->exec();
}

void ytdl::printResult(int result_num) {
        delete downloading;

        if (result_num == 0) {
                QMessageBox success;
                success.setIcon(QMessageBox::Information);
                success.setText("Download Succeeded");
                success.exec();
                emit userAccepted();
        }
        else {
                QMessageBox fail;
                fail.setIcon(QMessageBox::Critical);
                fail.setText("Failed! Recheck input for errors.");
                fail.exec();
                emit userAccepted();
        }
}

void ytdl::downloadAction() {
    std::string ytdl_prog = "youtube-dl";
    std::string url_str = quote + QString_to_str(ui->lineURL->text()) + quote;
    std::string directory_str = quote + QString_to_str(ui->lineBrowse->text()) + "/%(title)s.%(ext)s" + quote;

    //Music selected
    if (ui->Tabs->currentIndex() == 0) {
        std::string audio_format;
        switch(ui->MFormatGroup->checkedId()) {
                case 0:
                        audio_format = "aac";
                        break;
                case 1:
                        audio_format = "flac";
                        break;
                case 2:
                        audio_format = "mp3";
                        break;
                case 3:
                        audio_format = "opus";
                        break;
                case 4:
                        audio_format = "wav";
                        break;
        }

        std::string audio_quality;
        switch(ui->MQualityGroup->checkedId()) {
                case 0:
                        audio_quality = "0";
                        break;
                case 1:
                        audio_quality = "2";
                        break;
                case 2:
                        audio_quality = "5";
                        break;
                case 3:
                        audio_quality = "8";
        }

        std::string command = ytdl_prog + " -x " + url_str + " -o " + directory_str \
                + " --audio-format " + audio_format \
                + " --audio-quality " + audio_quality \
                + " --ignore-config " + "--no-playlist";

        this->run_ytdl(command);
    }

    //Video selected
    else {
        std::string audio_format = "bestaudio";
        std::string video_format;
        switch(ui->VFormatGroup->checkedId()) {
                case 0:
                        video_format = "3gp";
                        break;
                case 1:
                        video_format = "flv";
                        break;
                case 2:
                        video_format = "mp4";
                        audio_format = "m4a";
                        break;
                case 3:
                        video_format = "webm";
                        break;
        }

        std::string video_res;
        switch(ui->VResGroup->checkedId()) {
                case 0:
                        video_res = "2160";
                        break;
                case 1:
                        video_res = "1440";
                        break;
                case 2:
                        video_res = "1080";
                        break;
                case 3:
                        video_res = "720";
                        break;
                case 4:
                        video_res = "480";
                        break;
        }

        std::string format_options = quote + video_format + "[height=" + video_res \
                + "]+" + audio_format + "/bestvideo[height<=" + video_res + "]+bestaudio" + quote;

        std::string command = ytdl_prog + whitespace + url_str + " -o " + directory_str \
                + " -f " + format_options \
                + " --ignore-config " + "--no-playlist";

        this->run_ytdl(command);
    }

}
