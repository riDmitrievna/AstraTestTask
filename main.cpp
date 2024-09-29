/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QApplication>
#include <QFileSystemModel>
#include <QFileIconProvider>
#include <QScreen>
#include <QScroller>
#include <QTreeView>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QMessageBox>
#include <QLineEdit>
#include <QWidget>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QStandardItem>


int calculateFolderSize(const QString& path) {
    QDir dir(path);
    qint64 totalSize = 0;

    foreach (QString file, dir.entryList(QDir::Files | QDir::NoDotAndDotDot | QDir::Hidden)) {
        totalSize += QFileInfo(dir, file).size();
    }

    foreach (QString subDir, dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden)) {
        totalSize += calculateFolderSize(dir.filePath(subDir));
    }

    return totalSize;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QCoreApplication::setApplicationVersion(QT_VERSION_STR);
    QCommandLineParser parser;
    parser.setApplicationDescription("Qt Dir View Example");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption dontUseCustomDirectoryIconsOption("c", "Set QFileSystemModel::DontUseCustomDirectoryIcons");
    parser.addOption(dontUseCustomDirectoryIconsOption);
    QCommandLineOption dontWatchOption("w", "Set QFileSystemModel::DontWatch");
    parser.addOption(dontWatchOption);
    parser.addPositionalArgument("directory", "The directory to start in.");
    parser.process(app);

    QFileSystemModel model;

    QString homeDir = QDir::homePath();
    model.setRootPath(homeDir);


    model.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden);


    //model.setRootPath("");
    if (parser.isSet(dontUseCustomDirectoryIconsOption))
        model.setOption(QFileSystemModel::DontUseCustomDirectoryIcons);
    if (parser.isSet(dontWatchOption))
        model.setOption(QFileSystemModel::DontWatchForChanges);
    //
    QWidget mainWidget;
        QVBoxLayout* layout = new QVBoxLayout(&mainWidget);

        QLineEdit* filterEdit = new QLineEdit(&mainWidget);
        filterEdit->setPlaceholderText("Фильтр по имени...");
        layout->addWidget(filterEdit);
    //

    QTreeView tree;
    tree.setModel(&model);


   tree.setRootIndex(model.index(QDir::homePath()));



    // Demonstrating look and feel features
    tree.setAnimated(false);
    tree.setIndentation(20);
    tree.setSortingEnabled(true);

    // Обработка фильтрации
        QObject::connect(filterEdit, &QLineEdit::textChanged, [&model](const QString& text) {
            model.setNameFilters(QStringList() << "*" + text + "*");
            model.setNameFilterDisables(false);
        });


        // Добавление кнопки "Обновить" и метки для размера папки
             QObject::connect(&tree, &QTreeView::clicked, [&](const QModelIndex& index) {
                if (model.isDir(index)) {


                    QString folderPath = model.filePath(index);
                    QPushButton* updateButton = new QPushButton("Обновить");
                    //layout->removeWidget(updateButton);
                    QLabel* sizeLabel = new QLabel("Размер: " + QString::number(calculateFolderSize(folderPath)) + " байт");

                    QObject::connect(updateButton, &QPushButton::clicked, [folderPath, sizeLabel]() {
                                    sizeLabel->setText("Размер: " + QString::number(calculateFolderSize(folderPath)) + " байт");
                                });
                    QObject::connect(&tree, &QTreeView::clicked, [updateButton, sizeLabel]() {

                            //прячем лейбл и кнопку
                            sizeLabel->hide();
                            updateButton->hide();

                        });

                    layout->addWidget(updateButton);
                    layout->addWidget(sizeLabel);



                }
            });
            layout->addWidget(&tree);


    const QSize availableSize = tree.screen()->availableGeometry().size();
    tree.resize(availableSize / 2);
    tree.setColumnWidth(0, tree.width() / 3);

    // Make it flickable on touchscreens
    QScroller::grabGesture(&tree, QScroller::TouchGesture);

    tree.setWindowTitle(QObject::tr("Dir View"));
    //tree.show();
    mainWidget.show();

    return app.exec();
}
