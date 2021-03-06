/* ============================================================
* Crusta - Qt5 webengine browser
* Copyright (C) 2017-2018 Anmol Gautam <anmol@crustabrowser.com>
*
* THIS FILE IS A PART OF CRUSTA
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
* ============================================================ */

#include "bookmarkmanager.h"

#include <QDir>
#include <QHeaderView>
#include <iostream>


BookmarkManager::BookmarkManager(MainView *m)
{
    mview = m;
    setLayout(vbox);
    auto *hbox = new QHBoxLayout();
    hbox->addWidget(new QLabel());
    hbox->addWidget(search);
    hbox->addWidget(sbtn);
    search->setPlaceholderText(tr("Search..."));
    search->setFixedWidth(200);
    sbtn->setFixedWidth(50);
    vbox->addLayout(hbox);
    vbox->addWidget(display);
    vbox->addWidget(info_lbl);
    info_lbl->setMinimumHeight(200);
    auto *lbl_vbox = new QVBoxLayout();
    info_lbl->setLayout(lbl_vbox);
    lbl_vbox->addWidget(info_lbl_1);
    lbl_vbox->addWidget(info_lbl_2);
    lbl_vbox->addWidget(info_lbl_3);
    lbl_vbox->addWidget(new QLabel());
    connect(display, &QTreeWidget::itemClicked, this, &BookmarkManager::displayInfo);
    QStringList header;
    header.append(tr("Title"));
    header.append(tr("URL"));
    header.append(tr("Description"));
    display->setHeaderLabels(header);
    setWindowTitle(tr("Crusta : Bookmarks Manager"));
    loadBookmarks();
    display->setContextMenuPolicy(Qt::CustomContextMenu);
    display->header()->resizeSection(0, 200);
    display->header()->resizeSection(1, 300);
    display->header()->resizeSection(2, 200);
    connect(display, &QTreeWidget::customContextMenuRequested, this, &BookmarkManager::showContextMenu);
    connect(open, &QAction::triggered, this, &BookmarkManager::openUrl);
    connect(del, &QAction::triggered, this, &BookmarkManager::clearEntry);
    auto *h1box = new QHBoxLayout();
    QLabel *editdescription = new QLabel(tr("Edit Desription : "));
    h1box->addWidget(editdescription);
    editdescription->setStyleSheet("color: #00b0e3");
    h1box->addWidget(description);
    h1box->addWidget(save);
    vbox->addLayout(h1box);
    connect(save, &QPushButton::clicked, this, &BookmarkManager::saveDescription);
    connect(sbtn, &QPushButton::clicked, this, &BookmarkManager::searchBookmark);
    setMinimumWidth(300);
    setMaximumWidth(395);
    connect(display, &QTreeWidget::itemDoubleClicked, this, &BookmarkManager::openUrl);
}

void BookmarkManager::loadBookmarks()
{
    QFile inputFile(QDir::homePath() + "/.crusta_db/bookmarks.txt");

    if (inputFile.open(QIODevice::ReadOnly)) {
        QTextStream in(&inputFile);
        in.setCodec("UTF-8");

        while (!in.atEnd()) {
            QString line = in.readLine();
            QStringList data = line.split(">>>>>");

            if (data.count() == 1) {
                continue;
            }

            if (data.count() == 2) {
                data.append("");
            }

            if (!(data[0] == "" || data[1] == "")) {
                auto *item = new QTreeWidgetItem(data);
                display->insertTopLevelItem(0, item);

            }
        }

        inputFile.close();
    }
}

void BookmarkManager::showContextMenu(const QPoint &pos)
{
    if (display->itemAt(pos) == nullptr) {
        return;
    }

    auto *cmenu = new QMenu();
    cmenu->addAction(open);
    cmenu->addAction(del);
    cmenu->exec(display->mapToGlobal(pos));
}

void BookmarkManager::openUrl()
{
    QTreeWidgetItem *item = display->currentItem();
    QUrl url = QUrl(item->text(1));
    mview->addNormalTab();
    int index = mview->tabWindow->count() - 1;
    mview->tabWindow->setCurrentIndex(index);
    QWidget *widget = mview->tabWindow->widget(index);
    QLayout *layout = widget->layout();
    auto *webview = (WebView *)layout->itemAt(1)->widget();
    webview->load(url);
}

void BookmarkManager::clearEntry()
{
    QTreeWidgetItem *item = display->currentItem();
    QString forbidden = item->text(0).toUtf8() + ">>>>>" + item->text(1).toUtf8() + ">>>>>" + item->text(2).toUtf8();
    QFile f(QDir::homePath() + "/.crusta_db/bookmarks.txt");

    if (f.open(QIODevice::ReadWrite | QIODevice::Text)) {
        QString s;
        QTextStream t(&f);
        t.setCodec("UTF-8");

        while (!t.atEnd()) {
            QString line = t.readLine();

            if (line != forbidden) {
                s.append(line + "\n");
            }
        }

        f.resize(0);
        t << s;
        f.close();
    }

    delete item;
}

void BookmarkManager::saveDescription()
{
    if (display->currentItem() == nullptr) {
        return;
    }

    QTreeWidgetItem *item = display->currentItem();
    QString forbidden = item->text(0).toUtf8() + ">>>>>" + item->text(1).toUtf8() + ">>>>>" + item->text(2).toUtf8();
    item->setText(2, description->text());
    description->setText("");
    QString newline = item->text(0).toUtf8() + ">>>>>" + item->text(1).toUtf8() + ">>>>>" + item->text(2).toUtf8();
    QFile f(QDir::homePath() + "/.crusta_db/bookmarks.txt");

    if (f.open(QIODevice::ReadWrite | QIODevice::Text)) {
        QString s;
        QTextStream t(&f);
        t.setCodec("UTF-8");

        while (!t.atEnd()) {
            QString line = t.readLine();

            if (line != forbidden) {
                s.append(line + "\n");
            } else {
                s.append(newline + "\n");
            }
        }

        f.resize(0);
        t << s;
        f.close();
    }
}

void BookmarkManager::searchBookmark()
{
    QString key = search->text();
    display->clear();
    QFile inputFile(QDir::homePath() + "/.crusta_db/bookmarks.txt");

    if (inputFile.open(QIODevice::ReadOnly)) {
        QTextStream in(&inputFile);
        in.setCodec("UTF-8");

        while (!in.atEnd()) {
            QString line = in.readLine();
            QStringList data = line.split(">>>>>");

            if (data.count() == 1) {
                continue;
            }

            if (data.count() == 2) {
                data.append("");
            }

            QString master = data[2];

            if (master.contains(key)) {
                auto *item = new QTreeWidgetItem(data);
                display->insertTopLevelItem(0, item);

            }
        }

        inputFile.close();
    }
}

void BookmarkManager::displayInfo(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);

    QString title = item->text(0);
    QString url = item->text(1);
    QString info = item->text(2);
    info_lbl_1->setText(title);
    info_lbl_1->setWordWrap(true);
    info_lbl_1->setStyleSheet("font-size: 15px; color: #00b0e3");
    info_lbl_2->setText(url);
    info_lbl_2->setWordWrap(true);
    info_lbl_2->setStyleSheet("font-size: 11px; color: #00b0e3");
    info_lbl_3->setText(info);
    info_lbl_3->setWordWrap(true);
    info_lbl_3->setStyleSheet("font-size: 13px; color: #00b0e3");
}



PrivateBookmarkManager::PrivateBookmarkManager(PrivateMainView *m)
{
    pmview = m;
    setLayout(vbox);
    auto *hbox = new QHBoxLayout();
    hbox->addWidget(new QLabel());
    hbox->addWidget(search);
    hbox->addWidget(sbtn);
    search->setPlaceholderText(tr("Search..."));
    search->setFixedWidth(200);
    sbtn->setFixedWidth(50);
    vbox->addLayout(hbox);
    vbox->addWidget(pdisplay);
    vbox->addWidget(info_lbl);
    info_lbl->setMinimumHeight(200);
    auto *lbl_vbox = new QVBoxLayout();
    info_lbl->setLayout(lbl_vbox);
    lbl_vbox->addWidget(info_lbl_1);
    lbl_vbox->addWidget(info_lbl_2);
    lbl_vbox->addWidget(info_lbl_3);
    lbl_vbox->addWidget(new QLabel());
    connect(pdisplay, &QTreeWidget::itemClicked, this, &PrivateBookmarkManager::pdisplayInfo);
    QStringList header;
    header.append(tr("Title"));
    header.append(tr("URL"));
    header.append(tr("Description"));
    pdisplay->setHeaderLabels(header);
    setWindowTitle(tr("Crusta : Bookmarks Manager"));
    loadBookmarks();
    pdisplay->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(pdisplay, &QTreeWidget::customContextMenuRequested, this, &PrivateBookmarkManager::showContextMenu);
    connect(open, &QAction::triggered, this, &PrivateBookmarkManager::openUrl);
    connect(del, &QAction::triggered, this, &PrivateBookmarkManager::clearEntry);
    auto *h1box = new QHBoxLayout();
    h1box->addWidget(new QLabel(tr("Edit Desription : ")));
    h1box->addWidget(description);
    h1box->addWidget(save);
    vbox->addLayout(h1box);
    connect(save, &QPushButton::clicked, this, &PrivateBookmarkManager::saveDescription);
    connect(sbtn, &QPushButton::clicked, this, &PrivateBookmarkManager::searchBookmark);
    connect(pdisplay, &QTreeWidget::itemDoubleClicked, this, &PrivateBookmarkManager::openUrl);
}

void PrivateBookmarkManager::loadBookmarks()
{
    QFile inputFile(QDir::homePath() + "/.crusta_db/bookmarks.txt");

    if (inputFile.open(QIODevice::ReadOnly)) {
        QTextStream in(&inputFile);
        in.setCodec("UTF-8");

        while (!in.atEnd()) {
            QString line = in.readLine();
            QStringList data = line.split(">>>>>");

            if (data.count() == 1) {
                continue;
            }

            if (data.count() == 2) {
                data.append("");
            }

            if (!(data[0] == "" || data[1] == "")) {
                auto item = new QTreeWidgetItem(data);
                pdisplay->insertTopLevelItem(0, item);

            }
        }

        inputFile.close();
    }
}

void PrivateBookmarkManager::showContextMenu(const QPoint &pos)
{
    if (pdisplay->itemAt(pos) == nullptr) {
        return;
    }

    auto *cmenu = new QMenu();
    cmenu->addAction(open);
    cmenu->addAction(del);
    //cmenu->setStyleSheet("QMenu{background-color:white;color:black} QMenu::selected{color:white;background-color:black}");
    cmenu->exec(pdisplay->mapToGlobal(pos));
}

void PrivateBookmarkManager::openUrl()
{
    std::cout << "Reached Bkmrk\n";
    QTreeWidgetItem *item = pdisplay->currentItem();
    QUrl url = QUrl(item->text(1));
    pmview->addNormalTab();
    int index = pmview->tabWindow->count() - 1;
    pmview->tabWindow->setCurrentIndex(index);
    QWidget *widget = pmview->tabWindow->widget(index);
    QLayout *layout = widget->layout();
    auto *webview = (PrivateWebView *)layout->itemAt(1)->widget();
    webview->load(url);
}

void PrivateBookmarkManager::clearEntry()
{
    QTreeWidgetItem *item = pdisplay->currentItem();
    QString forbidden = item->text(0).toUtf8() + ">>>>>" + item->text(1).toUtf8() + ">>>>>" + item->text(2).toUtf8();
    QFile f(QDir::homePath() + "/.crusta_db/bookmarks.txt");

    if (f.open(QIODevice::ReadWrite | QIODevice::Text)) {
        QString s;
        QTextStream t(&f);
        t.setCodec("UTF-8");

        while (!t.atEnd()) {
            QString line = t.readLine();

            if (line != forbidden) {
                s.append(line + "\n");
            }
        }

        f.resize(0);
        t << s;
        f.close();
    }

    delete item;
}

void PrivateBookmarkManager::saveDescription()
{
    if (pdisplay->currentItem() == nullptr) {
        return;
    }

    QTreeWidgetItem *item = pdisplay->currentItem();
    QString forbidden = item->text(0).toUtf8() + ">>>>>" + item->text(1).toUtf8() + ">>>>>" + item->text(2).toUtf8();
    item->setText(2, description->text());
    description->setText("");
    QString newline = item->text(0).toUtf8() + ">>>>>" + item->text(1).toUtf8() + ">>>>>" + item->text(2).toUtf8();
    QFile f(QDir::homePath() + "/.crusta_db/bookmarks.txt");

    if (f.open(QIODevice::ReadWrite | QIODevice::Text)) {
        QString s;
        QTextStream t(&f);
        t.setCodec("UTF-8");

        while (!t.atEnd()) {
            QString line = t.readLine();

            if (line != forbidden) {
                s.append(line + "\n");
            } else {
                s.append(newline + "\n");
            }
        }

        f.resize(0);
        t << s;
        f.close();
    }
}

void PrivateBookmarkManager::searchBookmark()
{
    QString key = search->text();
    pdisplay->clear();
    QFile inputFile(QDir::homePath() + "/.crusta_db/bookmarks.txt");

    if (inputFile.open(QIODevice::ReadOnly)) {
        QTextStream in(&inputFile);
        in.setCodec("UTF-8");

        while (!in.atEnd()) {
            QString line = in.readLine();
            QStringList data = line.split(">>>>>");

            if (data.count() == 1) {
                continue;
            }

            if (data.count() == 2) {
                data.append("");
            }

            QString master = data[2];

            if (master.contains(key)) {
                auto *item = new QTreeWidgetItem(data);
                pdisplay->insertTopLevelItem(0, item);

            }
        }

        inputFile.close();
    }
}

void PrivateBookmarkManager::pdisplayInfo(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);

    QString title = item->text(0);
    QString url = item->text(1);
    QString info = item->text(2);
    info_lbl_1->setText(title);
    info_lbl_1->setWordWrap(true);
    info_lbl_1->setStyleSheet("font-size: 15px; color: #00b0e3");
    info_lbl_2->setText(url);
    info_lbl_2->setWordWrap(true);
    info_lbl_2->setStyleSheet("font-size: 11px; color: #00b0e3");
    info_lbl_3->setText(info);
    info_lbl_3->setWordWrap(true);
    info_lbl_3->setStyleSheet("font-size: 13px; color: #00b0e3");
}
