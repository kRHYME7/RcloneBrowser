#include "job_widget.h"
#include "utils.h"
#include <QRegularExpressionMatch>

JobWidget::JobWidget(QProcess *process, const QString &info,
                     const QStringList &args, const QString &source,
                     const QString &dest, QWidget *parent)
    : QWidget(parent), mProcess(process) {
  ui.setupUi(this);

  mArgs.append(QDir::toNativeSeparators(GetRclone()));
  mArgs.append(GetRcloneConf());
  mArgs.append(args);

  ui.source->setText(source);
  ui.dest->setText(dest);
  ui.info->setText(info);

  ui.details->setVisible(false);

  ui.output->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
  ui.output->setVisible(false);

  QObject::connect(
      ui.showDetails, &QToolButton::toggled, this, [=](bool checked) {
        ui.details->setVisible(checked);
        ui.showDetails->setArrowType(checked ? Qt::DownArrow : Qt::RightArrow);
      });

  QObject::connect(
      ui.showOutput, &QToolButton::toggled, this, [=](bool checked) {
        ui.output->setVisible(checked);
        ui.showOutput->setArrowType(checked ? Qt::DownArrow : Qt::RightArrow);
      });

  ui.cancel->setIcon(
      QApplication::style()->standardIcon(QStyle::SP_DialogCloseButton));

  QObject::connect(ui.cancel, &QToolButton::clicked, this, [=]() {
    if (mRunning) {
      int button = QMessageBox::question(
          this, "Transfer",
          QString("rclone process is still running. Do you want to cancel it?"),
          QMessageBox::Yes | QMessageBox::No);
      if (button == QMessageBox::Yes) {
        cancel();
      }
    } else {
      emit closed();
    }
  });

  ui.copy->setIcon(
      QApplication::style()->standardIcon(QStyle::SP_FileLinkIcon));

  QObject::connect(ui.copy, &QToolButton::clicked, this, [=]() {
    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setText(mArgs.join(" "));
  });

  QObject::connect(mProcess, &QProcess::readyRead, this, [=]() {
    QRegularExpression rxSize(
        R"(^Transferred:\s+(\S+ \S+) \(([^)]+)\)$)"); // Until rclone 1.42
    QRegularExpression rxSize2(
        R"(^Transferred:\s+([0-9.]+)(\S)? \/ (\S+) (\S+), ([0-9%-]+), (\S+ \S+), (\S+) (\S+)$)"); // Starting with rclone 1.43
    QRegularExpression rxErrors(R"(^Errors:\s+(\S+)$)");
    QRegularExpression rxChecks(R"(^Checks:\s+(\S+)$)"); // Until rclone 1.42
    QRegularExpression rxChecks2(
        R"(^Checks:\s+(\S+) \/ (\S+), ([0-9%-]+)$)");   // Starting with
                                                        // rclone 1.43
    QRegularExpression rxTransferred(R"(^Transferred:\s+(\S+)$)"); // Until rclone 1.42
    QRegularExpression rxTransferred2(
        R"(^Transferred:\s+(\S+) \/ (\S+), ([0-9%-]+)$)"); // Starting with
                                                           // rclone 1.43
    QRegularExpression rxTime(R"(^Elapsed time:\s+(\S+)$)");
    QRegularExpression rxProgress(
        R"(^\*([^:]+):\s*([^%]+)% done.+(ETA: [^)]+)$)"); // Until rclone 1.38
    QRegularExpression rxProgress2(
        R"(\*([^:]+):\s*([^%]+)% \/[a-zA-z0-9.]+, [a-zA-z0-9.]+\/s, (\w+)$)"); // Starting with rclone 1.39

    while (mProcess->canReadLine()) {
      QString line = mProcess->readLine().trimmed();
      if (++mLines == 10000) {
        ui.output->clear();
        mLines = 1;
      }
      ui.output->appendPlainText(line);

      if (line.isEmpty()) {
        for (auto it = mActive.begin(), eit = mActive.end(); it != eit;
             /* empty */) {
          auto label = it.value();
          if (mUpdated.contains(label)) {
            ++it;
          } else {
            it = mActive.erase(it);
            ui.progress->removeWidget(label->buddy());
            ui.progress->removeWidget(label);
            delete label->buddy();
            delete label;
          }
        }
        mUpdated.clear();
        continue;
      }

      QRegularExpressionMatch match = rxSize.match(line);
      if (match.hasMatch()) {
        ui.size->setText(match.captured(1));
        ui.bandwidth->setText(match.captured(2));
      } else {
        match = rxSize2.match(line);
        if (match.hasMatch()) {
          ui.size->setText(match.captured(1) + " " + match.captured(2) + "B" +
                           ", " + match.captured(5));
          ui.bandwidth->setText(match.captured(6));
          ui.eta->setText(match.captured(8));
          ui.totalsize->setText(match.captured(3) + " " + match.captured(4));
        } else {
          match = rxErrors.match(line);
          if (match.hasMatch()) {
            ui.errors->setText(match.captured(1));
          } else {
            match = rxChecks.match(line);
            if (match.hasMatch()) {
              ui.checks->setText(match.captured(1));
            } else {
              match = rxChecks2.match(line);
              if (match.hasMatch()) {
                ui.checks->setText(match.captured(1) + " / " +
                                   match.captured(2) + ", " +
                                   match.captured(3));
              } else {
                match = rxTransferred.match(line);
                if (match.hasMatch()) {
                  ui.transferred->setText(match.captured(1));
                } else {
                  match = rxTransferred2.match(line);
                  if (match.hasMatch()) {
                    ui.transferred->setText(match.captured(1) + " / " +
                                            match.captured(2) + ", " +
                                            match.captured(3));
                  } else {
                    match = rxTime.match(line);
                    if (match.hasMatch()) {
                      ui.elapsed->setText(match.captured(1));
                    } else {
                      match = rxProgress.match(line);
                      if (match.hasMatch()) {
                        QString name = match.captured(1).trimmed();

                        auto it = mActive.find(name);

                        QLabel *label;
                        QProgressBar *bar;
                        if (it == mActive.end()) {
                          label = new QLabel();
                          label->setText(name);

                          bar = new QProgressBar();
                          bar->setMinimum(0);
                          bar->setMaximum(100);
                          bar->setTextVisible(true);

                          label->setBuddy(bar);

                          ui.progress->addRow(label, bar);

                          mActive.insert(name, label);
                        } else {
                          label = it.value();
                          bar = static_cast<QProgressBar *>(label->buddy());
                        }

                        bar->setValue(match.captured(2).toInt());
                        bar->setToolTip(match.captured(3));

                        mUpdated.insert(label);
                      } else {
                        match = rxProgress2.match(line);
                        if (match.hasMatch()) {
                          QString name = match.captured(1).trimmed();

                          auto it = mActive.find(name);

                          QLabel *label;
                          QProgressBar *bar;
                          if (it == mActive.end()) {
                            label = new QLabel();

                            QString nameTrimmed;

                            if (name.length() > 47) {
                              nameTrimmed =
                                  name.left(25) + "..." + name.right(19);
                            } else {
                              nameTrimmed = name;
                            }

                            label->setText(nameTrimmed);

                            bar = new QProgressBar();
                            bar->setMinimum(0);
                            bar->setMaximum(100);
                            bar->setTextVisible(true);

                            label->setBuddy(bar);

                            ui.progress->addRow(label, bar);

                            mActive.insert(name, label);
                          } else {
                            label = it.value();
                            bar = static_cast<QProgressBar *>(label->buddy());
                          }

                          bar->setValue(match.captured(2).toInt());
                          bar->setToolTip("File name: " + name +
                                          "\nFile stats" +
                                          match.captured(0).mid(
                                              match.captured(0).indexOf(':')));

                          mUpdated.insert(label);
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  });

  QObject::connect(mProcess,
                   static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(
                       &QProcess::finished),
                   this, [=](int status, QProcess::ExitStatus) {
                     mProcess->deleteLater();
                     for (auto label : mActive) {
                       ui.progress->removeWidget(label->buddy());
                       ui.progress->removeWidget(label);
                       delete label->buddy();
                       delete label;
                     }

                     mRunning = false;
                     if (status == 0) {
                       ui.showDetails->setStyleSheet(
                           "QToolButton { border: 0; color: black; }");
                       ui.showDetails->setText("Finished");
                     } else {
                       ui.showDetails->setStyleSheet(
                           "QToolButton { border: 0; color: red; }");
                       ui.showDetails->setText("Error");
                     }

                     ui.cancel->setToolTip("Close");

                     emit finished(ui.info->text());
                   });

  ui.showDetails->setStyleSheet("QToolButton { border: 0; color: green; }");
  ui.showDetails->setText("Running");
}

JobWidget::~JobWidget() {}

void JobWidget::showDetails() { ui.showDetails->setChecked(true); }

void JobWidget::cancel() {
  if (!mRunning) {
    return;
  }

  mProcess->kill();
  mProcess->waitForFinished();

  emit closed();
}
