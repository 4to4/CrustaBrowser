#include "tabwidget.h"
#include "tabbar.h"
#include "tab.h"
#include "stackedwidget.h"
#include "../webview/webview.h"
#include <QVBoxLayout>
#include <QUrl>

TabWidget::TabWidget(QWidget *parent)
    : QWidget(parent)
{
    m_tabBar = new TabBar(this);
    QVBoxLayout *vBoxLayout = new QVBoxLayout(this);
    setLayout(vBoxLayout);
    vBoxLayout->setContentsMargins(0, 0, 0, 0);
    vBoxLayout->setSpacing(0);
    vBoxLayout->addWidget(m_tabBar);

    m_stakedWidget = new StackedWidget(this);
    vBoxLayout->addWidget(m_stakedWidget);

    m_tabBar->setVirtualTab(m_stakedWidget->currentWidget());
    connect(m_stakedWidget, &StackedWidget::currentChanged, this, [this]{
        m_tabBar->setVirtualTab(m_stakedWidget->currentWidget());
    });

    addTab();
}

TabWidget::~TabWidget()
{
}

void TabWidget::addTab()
{
    addTab(QUrl(""));
}

void TabWidget::addTab(const QUrl &url)
{
    Tab *tab = new Tab(m_stakedWidget);
    if (!url.isEmpty()) {
        tab->webview()->load(url);
    } else {
        tab->webview()->loadStartupUrl();
    }
    int index = m_stakedWidget->addWidget(tab);
    m_stakedWidget->setCurrentIndex(index);
}
