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
#pragma once

#include <QTabBar>
#include <QPoint>

class AddTabButton;
class TabWidget;

class TabBar : public QTabBar
{
    Q_OBJECT
    enum State {
        CLOSING,
        NORMAL
    };
public:
    explicit TabBar(QWidget *parent = nullptr);
    QSize tabSizeHint(int index) const;

    void addNewTab();
protected:
    void tabInserted(int index);
    void mouseMoveEvent(QMouseEvent *event);
private:
    AddTabButton *m_addTabButton = nullptr;
    TabWidget *m_tabWidget = nullptr;

    State m_state = NORMAL;
    int m_previousWidth = 0;
    QPoint m_previousMousePos = QPoint(-1, -1);

    void updateAddTabButton(int tabWidth) const;
    void addTabCloseButton(int index);
    void showContextMenu(const QPoint &pos);
};
