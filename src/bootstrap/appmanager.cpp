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
#include "appmanager.h"
#include "window.h"
#include "settings.h"
#include "database.h"
#include "scripts.h"
#include "tab.h"
#include "tabwidget.h"
#include "crustascheme.h"
#include "downloadmanager.h"

AppManager::AppManager(QObject *parent)
    : QObject(parent)
{
    m_application.organisationName = QStringLiteral("crusta");
    m_application.organisationDomain = QStringLiteral("crusta.io");
    m_application.name = QStringLiteral("Crusta - Fast, Secure & Modern Web-browser");
    m_application.majorVersion = 2;
    m_application.minorVersion = 0;
    m_application.patchVersion = 0;
    // set developmentBuild to false when releasing
    m_application.mode = Development;

    setUpWebEngineProfile();

    m_database = new Database(this);
    m_database->createDatabases();
}

AppManager::~AppManager()
{
    for (Window *window : m_windowList) {
        window->close();
    }
}

AppManager *AppManager::instance()
{
    static AppManager *applicationManager = new AppManager();
    return applicationManager;
}

QString AppManager::organisationName()
{
    return m_application.organisationName;
}

QString AppManager::organisationDomain()
{
    return m_application.organisationDomain;
}

QString AppManager::applicationName()
{
    return m_application.name;
}

int AppManager::majorVersion()
{
    return m_application.majorVersion;
}

int AppManager::minorVersion()
{
    return m_application.minorVersion;
}

int AppManager::patchVersion()
{
    return m_application.patchVersion;
}

QString AppManager::applicationVersionString()
{
    QString versionString = m_application.majorVersion + QStringLiteral(".") + m_application.minorVersion + QStringLiteral(".") + m_application.patchVersion;
    if (m_application.mode == Development) {
        versionString += QStringLiteral("+dev");
    } else if (m_application.mode == Alpha) {
        versionString += QStringLiteral("+alpha");
    } else if (m_application.mode == Beta) {
        versionString += QStringLiteral("+beta");
    }
    return versionString;
}


void AppManager::bootstrap()
{
    Window *window = new Window();
    window->show();

    m_windowList.append(window);
}

Settings *AppManager::settings()
{
    if (!m_settings) {
        m_settings = new Settings(this);
    }
    return m_settings;
}

QWebEngineProfile *AppManager::webEngineProfile() const
{
    return m_webEngineProfile;
}

Database *AppManager::database() const
{
    return m_database;
}

int AppManager::addTab(Tab *tab, int type)
{
    int index = -1;
    for (Window *window : qAsConst(m_windowList)) {
        if (window->isActiveWindow()) {
            index = window->tabWidget()->addTab(tab, type);
            break;
        }
    }
    return index;
}

TabWidget *AppManager::getTabWidget(Tab *tab)
{
    TabWidget *tabWidget = nullptr;
    for (Window *window : qAsConst(m_windowList)) {
        int index = window->tabWidget()->indexOf(tab);
        if (index != -1) {
            tabWidget = window->tabWidget();
            break;
        }
    }
    return tabWidget;
}

Tab *AppManager::getCurrentTab()
{
    for (Window *window : qAsConst(m_windowList)) {
        if (window->isActiveWindow()) {
            return window->tabWidget()->getTab(window->tabWidget()->currentIndex());
        }
    }
    return nullptr;
}

int AppManager::indexOfTab(Tab *tab)
{
    int tabIndex = -1;
    for (Window *window : qAsConst(m_windowList)) {
        int index = window->tabWidget()->indexOf(tab);
        if (index != -1) {
            tabIndex = index;
            break;
        }
    }
    return tabIndex;
}

void AppManager::setUpWebEngineProfile()
{
    m_webEngineProfile = new QWebEngineProfile(this);

    QWebEngineScript webChannelScript;
    webChannelScript.setInjectionPoint(QWebEngineScript::DocumentCreation);
    webChannelScript.setName("webChannelScript");
    webChannelScript.setRunsOnSubFrames(false);
    webChannelScript.setSourceCode(Scripts::webChannelScript());
    webChannelScript.setWorldId(QWebEngineScript::ApplicationWorld);
    m_webEngineProfile->scripts()->insert(webChannelScript);

    QWebEngineScript speeddialScript;
    speeddialScript.setInjectionPoint(QWebEngineScript::DocumentReady);
    speeddialScript.setName("speeddialScript");
    speeddialScript.setRunsOnSubFrames(false);
    speeddialScript.setSourceCode(Scripts::speeddialScript());
    speeddialScript.setWorldId(QWebEngineScript::ApplicationWorld);
    m_webEngineProfile->scripts()->insert(speeddialScript);

    CrustaScheme *crustaScheme = new CrustaScheme(this);
    m_webEngineProfile->installUrlSchemeHandler("crusta", crustaScheme);

    connect(m_webEngineProfile, &QWebEngineProfile::downloadRequested, this, [](QWebEngineDownloadItem *download) {
        DownloadManager::showPreDownloadDialog(download);
    });
}
