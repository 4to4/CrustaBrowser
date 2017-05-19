/* ============================================================
* Crusta - Qt5 webengine browser
* Copyright (C) 2017 Anmol Gautam <tarptaeya@gmail.com>
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

#include "webview.h"
#include "tabwindow.h"
#include "fullscreennotifier.h"
#include "timenotifier.h"
#include "popup.h"
#include "downloaditemwidget.h"
#include "mainview.h"
#include "downloadmanager.h"

#include <QWebEngineView>
#include <QWebEnginePage>
#include <QWebEngineProfile>
#include <QWebEngineSettings>
#include <QWebEngineFullScreenRequest>
#include <QString>
#include <QStringList>
#include <QLayout>
#include <QAction>
#include <QStackedWidget>
#include <QTabWidget>
#include <QWidget>
#include <QLabel>
#include <QMovie>
#include <QIcon>
#include <QTabBar>
#include <QPixmap>
#include <QUrl>
#include <QDir>
#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <QIODevice>
#include <QRadioButton>
#include <QFileIconProvider>
#include <QGroupBox>
#include <QDesktopServices>
#include <QDialog>
#include <QFileDialog>

#include <iostream>




WebView::WebView(){
    exitFullScreen->setShortcut(Qt::Key_Escape);
    createWebView();
    settings()->setAttribute(QWebEngineSettings::FullScreenSupportEnabled,true);
    settings()->setAttribute(QWebEngineSettings::PluginsEnabled,true);
    settings()->setAttribute(QWebEngineSettings::JavascriptCanOpenWindows,true);
    connect(this->page(),&QWebEnginePage::fullScreenRequested,this,&WebView::acceptFullScreen);
    connect(this->page(),&QWebEnginePage::loadStarted,this,&WebView::spinnerStarted);
    connect(this->page(),&QWebEnginePage::loadFinished,this,&WebView::loadFinished);
    connect(this->page(),&QWebEnginePage::iconChanged,this,&WebView::faviconChanged);
    connect(this->page(),&QWebEnginePage::titleChanged,this,&WebView::pageTitleChanged);
    connect(this->page(),&QWebEnginePage::featurePermissionRequested,this,&WebView::permissionHandler);
    connect(exitFullScreen,&QAction::triggered,this,&WebView::ExitAction);
    connect(page()->profile(),&QWebEngineProfile::downloadRequested,this,&WebView::download);
    connect(page(),&QWebEnginePage::linkHovered,this,&WebView::showLinkHovered);
    connect(page(),&QWebEnginePage::windowCloseRequested,this,&WebView::closeTab);
    //connect(page(),&QWebEnginePage::recentlyAudibleChanged,this,&WebView::audioInfo);
    connect(page(),&QWebEnginePage::authenticationRequired,this,&WebView::authenticate);

    //context-menu
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this,&WebView::customContextMenuRequested,this,&WebView::showContextMenu);
}

void WebView::createWebView(){
    setPage(webpage);
    load(QUrl(this->home_page));
}

QWebEngineView* WebView::returnView(){
    return this;
}

void WebView::home(){
    load(QUrl(this->home_page));
}

void WebView::spinnerStarted(){
    if(parent()==NULL)return;
    try{
        QWidget* widget=(QWidget*)this->parent();
        QStackedWidget* stackedwidget=(QStackedWidget*)widget->parent();
        QTabWidget* tabwidget=(QTabWidget*)stackedwidget->parent();
        int count=tabwidget->count();
        for(int i=0;i<count;i++){
            QWidget* w=tabwidget->widget(i);
            QLayout* l=w->layout();
            WebView* v=(WebView*)l->itemAt(1)->widget();
            if(v==this){
                QLabel* spinner=new QLabel();
                QMovie* loader=new QMovie(":/res/videos/loader.gif");
                loader->start();
                spinner->setMovie(loader);
                tabwidget->tabBar()->setTabButton(i,QTabBar::LeftSide,spinner);
            }
        }
    }
    catch(...){
        return;
    }
}

void WebView::faviconChanged(QIcon fav){
    if(parent()==NULL)return;
    try{
        QWidget* widget=(QWidget*)this->parent();
        QStackedWidget* stackedwidget=(QStackedWidget*)widget->parent();
        QTabWidget* tabwidget=(QTabWidget*)stackedwidget->parent();
        int count=tabwidget->count();
        for(int i=0;i<count;i++){
            QWidget* w=tabwidget->widget(i);
            QLayout* l=w->layout();
            QWebEngineView* v=(QWebEngineView*)l->itemAt(1)->widget();
            if(v==this){
                QPixmap pix=fav.pixmap(16,16);
                QLabel* lab=new QLabel();
                lab->setPixmap(pix);
                if(!page()->recentlyAudible())
                    tabwidget->tabBar()->setTabButton(i,QTabBar::LeftSide,lab);
            }
        }
    }
    catch(...){
        return;
    }
}

void WebView::pageTitleChanged(){
    if(parent()==NULL)return;
    try{
        QWidget* widget=(QWidget*)this->parent();
        QStackedWidget* stackedwidget=(QStackedWidget*)widget->parent();
        QTabWidget* tabwidget=(QTabWidget*)stackedwidget->parent();
        int count=tabwidget->count();
        for(int i=0;i<count;i++){
            QWidget* w=tabwidget->widget(i);
            QLayout* l=w->layout();
            QWebEngineView* v=(QWebEngineView*)l->itemAt(1)->widget();
            if(v==this){
                QString title=this->page()->title();
                tabwidget->setTabText(i,title);
            }
        }
    }
    catch(...){
        return;
    }
}

void WebView::acceptFullScreen(QWebEngineFullScreenRequest request){
    if(request.toggleOn()){
        request.accept();
        wasFullScreened=true;
        widget=(QWidget*)this->parent();
        layout=widget->layout();
        layout->removeWidget(this);
        addAction(exitFullScreen);
        setParent(0);
        showFullScreen();
        notifier->setViewParent(this);
        notifier->showNotifier();
        timeNotifier->setViewParent(this);
        timeAction->setShortcut(Qt::Key_T);
        connect(timeAction,&QAction::triggered,timeNotifier,&TimeNotifier::showNotifier);
        addAction(timeAction);
    }
    else{
        request.accept();
        notifier->setParent(0);
        timeNotifier->setParent(0);
        setParent(widget);
        layout->addWidget(this);
        removeAction(exitFullScreen);
        removeAction(timeAction);
    }
}

void WebView::ExitAction(){
    page()->triggerAction(QWebEnginePage::ExitFullScreen);
}

QWebEngineView* WebView::createWindow(QWebEnginePage::WebWindowType type){
    switch(type){
    case QWebEnginePage::WebBrowserTab: {
        WebView* view=new WebView();
        view->raise();
        TabWindow* tabwin=new TabWindow();
        tabwin->vbox->setContentsMargins(0,0,0,0);
        tabwin->setWebView(view);
        tabwin->createControls();
        QWidget* widget=(QWidget*)this->parent();
        QStackedWidget* stackedwidget=(QStackedWidget*)widget->parent();
        QTabWidget* tabwidget=(QTabWidget*)stackedwidget->parent();
        tabwidget->insertTab(tabwidget->currentIndex()+1,tabwin->tab,tr("New Tab"));
        tabwidget->setCurrentIndex(tabwidget->currentIndex()+1);
        return view;
    }
    case QWebEnginePage::WebBrowserBackgroundTab: {
        std::cout<<"Background Tab Requested"<<std::endl;
        return new QWebEngineView();
    }
    case QWebEnginePage::WebBrowserWindow: {
        std::cout<<"window Requested"<<std::endl;
        QWebEngineView* view=new QWebEngineView();
        connect(view->page(),&QWebEnginePage::windowCloseRequested,view,&QWebEngineView::close);
        view->setWindowTitle(view->title());
        view->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        view->show();
        view->move((this->width()-view->width())/2,(this->height()-view->height())/2);
        connect(view,SIGNAL(titleChanged(QString)),view,SLOT(setWindowTitle(QString)));
        return view;
    }
    case QWebEnginePage::WebDialog: {
        PopUp* popup=new PopUp();
        QWebEngineView* view=new QWebEngineView();
        popup->setWebView(view);
        popup->createView();
        popup->widget->show();
        connect(view->page(),&QWebEnginePage::windowCloseRequested,popup,&PopUp::close);
        popup->widget->move((this->width()-view->width())/2,(this->height()-view->height())/2);
        return view;
    }
    }
    return nullptr;
}

void WebView::permissionHandler(const QUrl &securityOrigin, QWebEnginePage::Feature feature){
    QDialog* dg=new QDialog();
    QVBoxLayout* vl=new QVBoxLayout();
    dg->setLayout(vl);
    dg->setModal(true);
    dg->setWindowFlags(Qt::FramelessWindowHint);
    dg->setFixedWidth(400);
    QHBoxLayout* h0=new QHBoxLayout();
    QLabel* permission=new QLabel();
    h0->addWidget(permission);
    vl->addLayout(h0);
    QHBoxLayout* h1=new QHBoxLayout();
    h1->addWidget(new QLabel());
    QPushButton* cncl=new QPushButton();
    QPushButton* ok=new QPushButton();
    h1->addWidget(cncl);
    h1->addWidget(ok);
    vl->addLayout(h1);
    cncl->setText(tr("No"));
    ok->setText(tr("Yes"));
    dg->setStyleSheet("QWidget{color:white;background-color:blueviolet} QPushButton{border:0.5px solid;background-color:crimson;color:white;border-radius:0px;border-color:crimson;padding:2px 4px;} QPushButton:hover{background-color:white;color:crimson}");
    connect(cncl,&QPushButton::clicked,dg,&QDialog::reject);
    connect(ok,&QPushButton::clicked,dg,&QDialog::accept);
    switch (feature) {
    case QWebEnginePage::Notifications:{
        permission->setText(tr("Allow Notifications from this website"));
        if(dg->exec()!=QDialog::Accepted){
            page()->setFeaturePermission(securityOrigin,feature,QWebEnginePage::PermissionDeniedByUser);
            return;
        }
        page()->setFeaturePermission(securityOrigin,feature,QWebEnginePage::PermissionGrantedByUser);
        break;
    }
    case QWebEnginePage::MouseLock:{
        permission->setText(tr("Allow Pointer Lock for this website"));
        if(dg->exec()!=QDialog::Accepted){
            page()->setFeaturePermission(securityOrigin,feature,QWebEnginePage::PermissionDeniedByUser);
            return;
        }
        page()->setFeaturePermission(securityOrigin,feature,QWebEnginePage::PermissionGrantedByUser);
        break;
    }
    case QWebEnginePage::Geolocation:{
        permission->setText(tr("Allow Geolocation from current website"));
        if(dg->exec()!=QDialog::Accepted){
            page()->setFeaturePermission(securityOrigin,feature,QWebEnginePage::PermissionDeniedByUser);
            return;
        }
        page()->setFeaturePermission(securityOrigin,feature,QWebEnginePage::PermissionGrantedByUser);
        break;
    }
    case QWebEnginePage::MediaAudioCapture:{
        permission->setText(tr("Allow Audio Capture for this site"));
        if(dg->exec()!=QDialog::Accepted){
            page()->setFeaturePermission(securityOrigin,feature,QWebEnginePage::PermissionDeniedByUser);
            return;
        }
        page()->setFeaturePermission(securityOrigin,feature,QWebEnginePage::PermissionGrantedByUser);
        break;
    }
    case QWebEnginePage::MediaVideoCapture:{
        permission->setText(tr("Allow Video Capture for this site"));
        if(dg->exec()!=QDialog::Accepted){
            page()->setFeaturePermission(securityOrigin,feature,QWebEnginePage::PermissionDeniedByUser);
            return;
        }
        page()->setFeaturePermission(securityOrigin,feature,QWebEnginePage::PermissionGrantedByUser);
        break;
    }
    case QWebEnginePage::MediaAudioVideoCapture:{
        permission->setText(tr("Allow Audio/Video Capture for this site"));
        if(dg->exec()!=QDialog::Accepted){
            page()->setFeaturePermission(securityOrigin,feature,QWebEnginePage::PermissionDeniedByUser);
            return;
        }
        page()->setFeaturePermission(securityOrigin,feature,QWebEnginePage::PermissionGrantedByUser);
        break;
    }
    }
}

void WebView::download(QWebEngineDownloadItem *download_item){
    QString path=download_item->path();
    QDialog* w=new QDialog();
    w->setWindowFlags(Qt::FramelessWindowHint);
    QVBoxLayout* box=new QVBoxLayout();
    w->setLayout(box);
    QLabel* yhcto=new QLabel(tr("You have choosen to open"));
    yhcto->setFont(QFont("Ariel",-1,-1,true));
    box->addWidget(yhcto);
    QHBoxLayout* hbx=new QHBoxLayout();
    QFileIconProvider* fip=new QFileIconProvider();
    QIcon icon=fip->icon(QFileInfo(path));
    QLabel* ilabel=new QLabel();
    ilabel->setPixmap(icon.pixmap(64,64));
    hbx->addWidget(ilabel);
    hbx->addWidget(new QLabel(path.split('/')[path.split('/').length()-1]+"\n"+tr("which is : ")+download_item->mimeType()));
    box->addLayout(hbx);
    QRadioButton* ropen=new QRadioButton();
    QRadioButton* rsave=new QRadioButton();
    ropen->setText(tr("Open..."));
    rsave->setText(tr("Save"));
    QGroupBox* gb=new QGroupBox();
    QVBoxLayout* vgb=new QVBoxLayout();
    vgb->addWidget(ropen);
    vgb->addWidget(rsave);
    rsave->setChecked(true);
    gb->setLayout(vgb);
    gb->setFlat(true);
    gb->setTitle(tr("What should Crusta do with this file?"));
    box->addWidget(gb);
    QPushButton* cncl_btn=new QPushButton();
    cncl_btn->setFixedWidth(75);
    cncl_btn->setText(tr("Cancel"));
    QPushButton* ok_btn=new QPushButton();
    ok_btn->setFixedWidth(75);
    ok_btn->setText(tr("Ok"));
    QHBoxLayout* h1bx=new QHBoxLayout();
    h1bx->addWidget(new QLabel());
    h1bx->addWidget(cncl_btn);
    h1bx->addWidget(ok_btn);
    box->addLayout(h1bx);
    w->setStyleSheet("QDialog{background-color:deepskyblue;color:blueviolet} QLabel{color:blueviolet} QGroupBox{color:blueviolet;} QRadioButton{color:blueviolet;} QPushButton{border:0.5px solid blueviolet;padding:4px 8px;color:white;background-color:blueviolet} QPushButton:hover{background-color:white;color:blueviolet}");
    connect(cncl_btn,&QPushButton::clicked,w,&QDialog::reject);
    connect(ok_btn,&QPushButton::clicked,w,&QDialog::accept);
    if(w->exec()!=QDialog::Accepted){
        download_item->cancel();
        return;
    }
    if(ropen->isChecked()){
        QString cpath=QDir::tempPath();
        cpath+="/"+path.split('/')[path.split('/').length()-1];
        download_item->setPath(cpath);
        download_item->accept();
        connect(download_item,&QWebEngineDownloadItem::finished,this,[this,cpath]{downloadFinished(cpath);});
    }
    else{
        QFileDialog f;
        f.setOption(QFileDialog::DontUseNativeDialog,true);
        QString fname=f.getSaveFileName(this,tr("Download File"),QDir::homePath(),QString(),nullptr,f.options());
        if(fname=="")fname=path;
        download_item->setPath(fname);
        download_item->accept();
    }
}

void WebView::downloadFinished(QString path){
    QDesktopServices::openUrl(path);
}

void WebView::showLinkHovered(QString url){
    link=url;
    //TODO : make a link hovered showing QLabel;
}

void WebView::closeTab(){
    QWidget* widget=(QWidget*)this->parent();
    QStackedWidget* stackedwidget=(QStackedWidget*)widget->parent();
    QTabWidget* tabwidget=(QTabWidget*)stackedwidget->parent();
    for(int i=0;i<tabwidget->count();i++){
        QWidget* w=tabwidget->widget(i);
        QLayout* l=w->layout();
        QWebEngineView* v=(QWebEngineView*)l->itemAt(1)->widget();
        if(v==this){
            v->load(QUrl("http://"));
            v->disconnect();
            v->deleteLater();
            tabwidget->removeTab(i);
            break;
        }
    }
}

void WebView::audioInfo(){
    if(this->parent()==NULL)return;
    QWidget* widget=(QWidget*)this->parent();
    QStackedWidget* stackedwidget=(QStackedWidget*)widget->parent();
    QTabWidget* tabwidget=(QTabWidget*)stackedwidget->parent();
    for(int i=0;i<tabwidget->count();i++){
        QWidget* w=tabwidget->widget(i);
        QLayout* l=w->layout();
        QWebEngineView* v=(QWebEngineView*)l->itemAt(1)->widget();
        if(v==this){
            QIcon ipix=QIcon(":/res/drawables/volume_max.png");
            QPixmap pix=ipix.pixmap(16,16);
            QLabel* lab=new QLabel();
            lab->setPixmap(pix);
            tabwidget->tabBar()->setTabButton(i,QTabBar::LeftSide,lab);
            break;
        }
    }
}

void WebView::authenticate(const QUrl u,QAuthenticator *authenticator){
    std::cout<<"yes"<<std::endl;
}

void WebView::showContextMenu(const QPoint& pos){
    QMenu* contextMenu=new QMenu();
    if(link!=""){
        QAction* open_link_in_new_tab=new QAction(tr("Open Link In New Tab"));
        connect(open_link_in_new_tab,&QAction::triggered,this,[this]{triggerPageAction(QWebEnginePage::OpenLinkInNewTab);});
        contextMenu->addAction(open_link_in_new_tab);
        QAction* open_link_in_new_window=new QAction(tr("Open Link In New Window"));
        connect(open_link_in_new_window,&QAction::triggered,this,[this]{triggerPageAction(QWebEnginePage::OpenLinkInNewWindow);});
        contextMenu->addAction(open_link_in_new_window);
        contextMenu->addSeparator();
        QAction* follow_link=new QAction(tr("Follow Link"));
        connect(follow_link,&QAction::triggered,this,[this]{triggerPageAction(QWebEnginePage::OpenLinkInThisWindow);});
        contextMenu->addAction(follow_link);
        QAction* copy_link=new QAction(tr("Copy Link Address"));
        connect(copy_link,&QAction::triggered,this,[this]{triggerPageAction(QWebEnginePage::CopyLinkToClipboard);});
        contextMenu->addAction(copy_link);
        contextMenu->addSeparator();
    }
    else if(this->selectedText()!=""){
        QString text=this->selectedText();
        QAction* a_cut=new QAction(tr("Cut"));
        connect(a_cut,&QAction::triggered,this,[this]{triggerPageAction(QWebEnginePage::Cut);});
        contextMenu->addAction(a_cut);
        QAction* a_copy=new QAction(tr("Copy"));
        connect(a_copy,&QAction::triggered,this,[this]{triggerPageAction(QWebEnginePage::Copy);});
        contextMenu->addAction(a_copy);
        QAction* a_paste=new QAction(tr("Paste"));
        connect(a_paste,&QAction::triggered,this,[this]{triggerPageAction(QWebEnginePage::Paste);});
        contextMenu->addAction(a_paste);
        contextMenu->addSeparator();
    }
    QAction* back_page=new QAction(QIcon(":/res/drawables/back.svg"),tr("Back"));
    connect(back_page,&QAction::triggered,this,&WebView::back);
    contextMenu->addAction(back_page);
    QAction* forward_page=new QAction(QIcon(":/res/drawables/forward.svg"),tr("Forward"));
    connect(forward_page,&QAction::triggered,this,&WebView::forward);
    contextMenu->addAction(forward_page);
    QAction* reload_page=new QAction(QIcon(":/res/drawables/reload.svg"),tr("Reload"));
    connect(reload_page,&QAction::triggered,this,&WebView::reload);
    contextMenu->addAction(reload_page);
    QAction* reload_and_bypass_cache=new QAction(tr("Reload And Bypass Cache"));
    connect(reload_and_bypass_cache,&QAction::triggered,this,[this]{triggerPageAction(QWebEnginePage::ReloadAndBypassCache);});
    contextMenu->addAction(reload_and_bypass_cache);
    contextMenu->addSeparator();
    QAction* view_page_source=new QAction(tr("View Page Source"));
    connect(view_page_source,&QAction::triggered,this,[this]{triggerPageAction(QWebEnginePage::ViewSource);});
    contextMenu->addAction(view_page_source);
    contextMenu->setStyleSheet("QMenu{background-color:white;color:blueviolet} QMenu::selected{color:white;background-color:blueviolet}");
    contextMenu->exec(this->mapToGlobal(pos));
}

void WebView::loadFinished(){
    QFile file("history.txt");
    file.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream out(&file);
    out << title().toLatin1()+">>>>>"+url().toString().toLatin1()+">>>>>"+QDate::currentDate().toString().toLatin1()+"\n";
    file.close();
}












