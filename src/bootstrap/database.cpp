/* ============================================================
* Crusta - Qt5 webengine browser
* Copyright (C) 2017-2018 Anmol Gautam <tarptaeya@gmail.com>
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
#include "database.h"
#include "historyitem.h"
#include "speeddialitem.h"
#include "bookmarksitem.h"
#include "panelitem.h"
#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>

Database::Database(QObject *parent)
    : QObject(parent)
{
}

void Database::createDatabases()
{
    m_db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"));
    m_db.setDatabaseName(QStringLiteral("crusta.db.sqlite"));
    if (!m_db.open()) {
        qWarning() << "ERROR OPENING DATABASE:" << m_db.lastError();
        return;
    }
    createHistoryDatabase();
    createSpeeddialDatabase();
    createBookmarksDatabase();
    createCompleterDatabase();
    createCategoryDatabase();
    createPanelsDatabase();
}

bool Database::addHistoryEntry(HistoryItem item)
{
    QSqlQuery query;
    query.prepare("SELECT * FROM history WHERE url = ?");
    query.addBindValue(item.url());
    if (query.exec() && query.next()) {
        query.prepare("UPDATE history SET timestamp = ?, favicon = ?, title = ?, visitCount = visitCount + 1, loadingTime = loadingTime + ? WHERE url = ?");
    } else {
        query.prepare("INSERT INTO history (timestamp, favicon, title, visitCount, loadingTime, url) VALUES (?, ?, ?, 1, ?, ?)");
    }
    query.addBindValue(item.timestamp());
    query.addBindValue(item.favicon());
    query.addBindValue(item.title());
    query.addBindValue(item.loadingTime());
    query.addBindValue(item.url());
    return query.exec();
}

bool Database::removeHistoryEntry(const QString &urlString)
{
    QSqlQuery query;
    query.prepare("DELETE FROM history WHERE url = ?");
    query.addBindValue(urlString);
    return query.exec();
}

QList<HistoryItem> Database::loadHistory()
{
    QList<HistoryItem> historyItems;
    QSqlQuery query;
    query.prepare("SELECT * FROM history ORDER BY timestamp DESC");
    query.exec();
    while (query.next()) {
        long long timestamp = query.value(0).toLongLong();
        QByteArray image = query.value(1).toByteArray();
        QString title = query.value(2).toString();
        QString url = query.value(3).toString();
        HistoryItem item;
        item.setTimestamp(timestamp);
        item.setFavicon(image);
        item.setTitle(title);
        item.setUrl(url);
        historyItems.append(item);
    }
    return historyItems;
}

bool Database::addSpeeddialEntry(SpeeddialItem item)
{
    QSqlQuery query;
    query.prepare("SELECT * FROM speeddial WHERE url = ?");
    query.addBindValue(item.url());
    if (query.exec() && query.next()) {
        if (!item.title().isEmpty() && !item.image().isEmpty()) {
            query.prepare("UPDATE speeddial SET image = ?  WHERE url = ?");
            query.addBindValue(item.image());
            query.addBindValue(item.url());
        } else {
            return false;
        }
    } else {
        query.prepare("INSERT INTO speeddial (image, title, url) VALUES (?, ?, ?)");
        query.addBindValue(item.image());
        query.addBindValue(item.title());
        query.addBindValue(item.url());
    }
    return query.exec();
}

bool Database::removeSpeeddialEntry(const QString &url)
{
    QSqlQuery query;
    query.prepare("DELETE FROM speeddial WHERE url = ?");
    query.addBindValue(url);
    return query.exec();
}

QList<SpeeddialItem> Database::loadSpeeddialEntries()
{
    QList<SpeeddialItem> speeddialItems;
    QSqlQuery query;
    query.prepare("SELECT * FROM speeddial");
    query.exec();
    while (query.next()) {
        QByteArray image = query.value(0).toByteArray();
        QString title = query.value(1).toString();
        QString url = query.value(2).toString();
        SpeeddialItem item;
        item.setImage(image);
        item.setTitle(title);
        item.setUrl(url);
        speeddialItems.append(item);
    }
    return speeddialItems;
}

bool Database::addBookmarksEntry(BookmarksItem item)
{
    QSqlQuery query;
    query.prepare("SELECT * FROM bookmarks WHERE url = ?");
    query.addBindValue(item.url());
    if (query.exec() && query.next()) {
        query.prepare("UPDATE bookmarks SET title = ?, folder = ?, favicon = ? WHERE url = ?");
        query.addBindValue(item.title());
        query.addBindValue(item.category());
        query.addBindValue(item.favicon());
        query.addBindValue(item.url());
    } else {
        query.prepare("INSERT INTO bookmarks (favicon, title, url, folder) VALUES (?, ?, ?, ?)");
        query.addBindValue(item.favicon());
        query.addBindValue(item.title());
        query.addBindValue(item.url());
        query.addBindValue(item.category());
    }
    return query.exec();

}

BookmarksItem Database::isBookmarked(const QString &urlString)
{
    QSqlQuery query;
    query.prepare("SELECT * FROM bookmarks WHERE url = ?");
    query.addBindValue(urlString);
    if (query.exec() && query.next()) {
        QString title = query.value(1).toString();
        QString url = query.value(2).toString();
        BookmarksItem item;
        item.setTitle(title);
        item.setUrl(url);
        return item;
    }
    return BookmarksItem();
}

bool Database::removeBookmarksEntry(const QString &url)
{
    QSqlQuery query;
    query.prepare("DELETE FROM bookmarks WHERE url = ?");
    query.addBindValue(url);
    return query.exec();
}

QList<BookmarksItem> Database::loadBookmarks()
{
    QList<BookmarksItem> bookmarksItems;
    QSqlQuery query;
    query.prepare("SELECT * FROM bookmarks");
    query.exec();
    while (query.next()) {
        QByteArray image = query.value(0).toByteArray();
        QString title = query.value(1).toString();
        QString url = query.value(2).toString();
        QString category = query.value(3).toString();
        BookmarksItem item;
        item.setFavicon(image);
        item.setTitle(title);
        item.setUrl(url);
        item.setCategory(category);
        bookmarksItems.append(item);
    }
    return bookmarksItems;
}

bool Database::addCompleterEntry(const QString &entry)
{
    QSqlQuery query;
    query.prepare("INSERT INTO completer VALUES (?)");
    query.addBindValue(entry);
    return query.exec();
}

QStringList Database::loadCompleterEntries()
{
    QStringList entries;
    QSqlQuery query;
    query.prepare("SELECT * FROM completer");
    query.exec();
    while (query.next()) {
        entries.append(query.value(0).toString());
    }
    return entries;
}

bool Database::addCategory(const QString &category)
{
    QSqlQuery query;
    query.prepare("INSERT INTO category VALUES (?)");
    query.addBindValue(category);
    return query.exec();
}

QStringList Database::loadCategories()
{
    QStringList categories;
    QSqlQuery query;
    query.prepare("SELECT * FROM category");
    query.exec();
    while (query.next()) {
        categories.append(query.value(0).toString());
    }
    return categories;
}

bool Database::addPanel(PanelItem item)
{
    QSqlQuery query;
    query.prepare("INSERT OR REPLACE INTO panels VALUES (?, ?)");
    query.addBindValue(item.url());
    query.addBindValue(item.favicon());
    return query.exec();
}

bool Database::removePanel(const QString &urlString)
{
    QSqlQuery query;
    query.prepare("DELETE FROM panels WHERE url = ?");
    query.addBindValue(urlString);
    return query.exec();
}

QList<PanelItem> Database::loadPanels()
{
    QList<PanelItem> panelItems;
    QSqlQuery query;
    query.prepare("SELECT * FROM panels");
    query.exec();
    while (query.next()) {
        PanelItem item;
        item.setUrl(query.value(0).toString());
        item.setFavicon(query.value(1).toByteArray());
        panelItems.append(item);
    }
    return panelItems;
}

void Database::createHistoryDatabase()
{
    QSqlQuery query("CREATE TABLE IF NOT EXISTS history (timestamp INTEGER, favicon BLOB, title TEXT, url TEXT PRIMARY KEY, visitCount INTEGER, loadingTime INTEGER)");
    query.exec();
}

void Database::createSpeeddialDatabase()
{
    QSqlQuery query("CREATE TABLE IF NOT EXISTS speeddial (image BLOB, title TEXT, url TEXT PRIMARY KEY)");
    query.exec();
}

void Database::createBookmarksDatabase()
{
    QSqlQuery query("CREATE TABLE IF NOT EXISTS bookmarks (favicon BLOB, title TEXT, url TEXT PRIMARY KEY, category TEXT)");
    query.exec();
}

void Database::createCompleterDatabase()
{
    QSqlQuery query("CREATE TABLE IF NOT EXISTS completer (entry TEXT UNIQUE)");
    query.exec();
}

void Database::createCategoryDatabase()
{
    QSqlQuery query("CREATE TABLE IF NOT EXISTS category (name TEXT UNIQUE)");
    query.exec();
}

void Database::createPanelsDatabase()
{
    QSqlQuery query("CREATE TABLE IF NOT EXISTS panels (url TEXT UNIQUE, favicon BLOB)");
    query.exec();
}
