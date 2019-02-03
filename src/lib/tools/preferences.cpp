#include "browserwindow.h"
#include "common-defs.h"
#include "mainapplication.h"
#include "preferences.h"
#include "tab.h"
#include "tabwidget.h"
#include "webpage.h"
#include "webview.h"

#include <QCheckBox>
#include <QComboBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QSettings>
#include <QSizePolicy>
#include <QVBoxLayout>

Preferences::Preferences(QWidget *parent)
    : QTabWidget (parent)
{

}

void Preferences::show()
{
    clear();

    createGeneralTab();
    createBrowsingTab();
    createSearchEngineTab();
    createOthersTab();

    activateWindow();
    QTabWidget::show();
}

void Preferences::createGeneralTab()
{
    QWidget *widget = new QWidget;
    addTab(widget, QSL("General"));

    QVBoxLayout *vboxLayout = new QVBoxLayout;

    QGroupBox *launch = new QGroupBox(QSL("On Launch"));
    QGridLayout *gridLayout1 = new QGridLayout;

    QLabel *startPage = new QLabel(QSL("Start Page"));
    QLineEdit *startPageUrl = new QLineEdit;
    QLabel *homePage = new QLabel(QSL("Home Page"));
    QLineEdit *homePageUrl = new QLineEdit;
    QLabel *newTab = new QLabel(QSL("New Tab"));
    QLineEdit *newTabUrl = new QLineEdit;

    gridLayout1->addWidget(startPage, 0, 0);
    gridLayout1->addWidget(startPageUrl, 0, 1);
    gridLayout1->addWidget(homePage, 1, 0);
    gridLayout1->addWidget(homePageUrl, 1, 1);
    gridLayout1->addWidget(newTab, 2, 0);
    gridLayout1->addWidget(newTabUrl, 2, 1);

    launch->setLayout(gridLayout1);

    QGroupBox *profile = new QGroupBox(QSL("Profile"));
    QGridLayout *gridLayout2 = new QGridLayout;

    QLabel *activeProfileLabel = new QLabel(QSL("Active Profile"));
    QLabel *activeProfileName = new QLabel(appManager->webEngineProfile()->storageName());
    QLabel *startUpProfile = new QLabel(QSL("Startup Profile"));
    QComboBox *profileCombo = new QComboBox;
    QPushButton *newProfile = new QPushButton(QSL("New"));
    QPushButton *delProfile = new QPushButton(QSL("Remove"));

    gridLayout2->addWidget(activeProfileLabel, 0, 0);
    gridLayout2->addWidget(activeProfileName, 0, 1);
    gridLayout2->addWidget(startUpProfile, 1, 0);
    gridLayout2->addWidget(profileCombo, 1, 1);

    QHBoxLayout *hboxLayout2 = new QHBoxLayout;
    hboxLayout2->addWidget(newProfile);
    hboxLayout2->addWidget(delProfile);
    gridLayout2->addLayout(hboxLayout2, 2, 1);

    profile->setLayout(gridLayout2);

    vboxLayout->addWidget(launch);
    vboxLayout->addWidget(profile);

    QHBoxLayout *hboxLayout = new QHBoxLayout;

    QPushButton *defaults = new QPushButton(QSL("Defaults"));
    QPushButton *apply = new QPushButton(QSL("Apply"));
    QPushButton *ok = new QPushButton(QSL("Ok"));

    hboxLayout->addWidget(defaults);
    hboxLayout->addWidget(new QWidget);
    hboxLayout->addWidget(apply);
    hboxLayout->addWidget(ok);

    vboxLayout->addWidget(new QWidget);
    vboxLayout->addLayout(hboxLayout);

    widget->setLayout(vboxLayout);
    widget->setMinimumSize(widget->sizeHint());

    startPageUrl->setText(appManager->settings()->value(QSL("webView/startPageUrl"), QSL("crusta:speeddial")).toString());
    homePageUrl->setText(appManager->settings()->value(QSL("webView/homePageUrl"), QSL("crusta:speeddial")).toString());
    newTabUrl->setText(appManager->settings()->value(QSL("webView/newTabUrl"), QSL("crusta:speeddial")).toString());

    QStringList profiles = appManager->settings()->value(QSL("profile/profiles"), QStringList() << QSL("default")).toStringList();
    profileCombo->addItems(profiles);
    profileCombo->setCurrentText(appManager->settings()->value(QSL("profile/default"), QSL("default")).toString());

    connect(newProfile, &QPushButton::clicked, this, [profileCombo] {
        const QString text = QInputDialog::getText(profileCombo, QSL("New Profile"), QSL("New Profile Name"));

        QStringList profiles = appManager->settings()->value(QSL("profile/profiles"), QStringList() << QSL("default")).toStringList();
        if (profiles.contains(text)) {
            return ;
        }

        profileCombo->addItem(text);

        profiles << text;
        appManager->settings()->setValue(QSL("profile/profiles"), profiles);
    });

    connect(delProfile, &QPushButton::clicked, this, [profileCombo] {
        const QString text = profileCombo->currentText();
        if (text == appManager->webEngineProfile()->storageName()) {
            QMessageBox::critical(profileCombo, QSL("Info"), QSL("Active profile cannot be removed"));
            return;
        }

        profileCombo->removeItem(profileCombo->findText(text, Qt::MatchExactly));
        profileCombo->setCurrentText(appManager->settings()->value(QSL("profile/default"), QSL("default")).toString());

        QStringList profiles = appManager->settings()->value(QSL("profile/profiles"), QStringList() << QSL("default")).toStringList();
        profiles.removeOne(text);
        appManager->settings()->setValue(QSL("profile/profiles"), profiles);
    });

    connect(defaults, &QPushButton::clicked, this, [startPageUrl, homePageUrl, newTabUrl, profileCombo] {
        startPageUrl->setText(QSL("crusta:speeddial"));
        homePageUrl->setText(QSL("crusta:speeddial"));
        newTabUrl->setText(QSL("crusta:speeddial"));
        profileCombo->setCurrentText(QSL("default"));
    });

    connect(apply, &QPushButton::clicked, this, [startPageUrl, homePageUrl, newTabUrl, profileCombo] {
        appManager->settings()->setValue(QSL("webView/startPageUrl"), startPageUrl->text());
        appManager->settings()->setValue(QSL("webView/homePageUrl"), homePageUrl->text());
        appManager->settings()->setValue(QSL("webView/newTabUrl"), newTabUrl->text());

        appManager->settings()->setValue(QSL("profile/default"), profileCombo->currentText());
    });

    connect(ok, &QPushButton::clicked, this, [apply, this] {
        apply->click();
        setVisible(false);
    });
}

void Preferences::createBrowsingTab()
{
    QWidget *root = new QWidget;
    QVBoxLayout *vboxLayout = new QVBoxLayout;

    QGroupBox *webEngineSettings = new QGroupBox(QSL("QWebEngineSettings"));
    webEngineSettings->setMaximumHeight(250);
    QVBoxLayout *webEngineSettingsLayout = new QVBoxLayout;

    QScrollArea *area = new QScrollArea;
    area->setWidgetResizable(true);

    QWidget *widget = new QWidget;

    QVBoxLayout *vboxLayout1 = new QVBoxLayout;

    QCheckBox *autoLoadImages = new QCheckBox(QSL("Auto-Load Images"));
    QCheckBox *javaScriptEnabled = new QCheckBox(QSL("JavaScript Enabled"));
    QCheckBox *javaScriptCanOpenWindows = new QCheckBox(QSL("JavaScript Can Open Windows"));
    QCheckBox *javaScriptCanAccessClipboard = new QCheckBox(QSL("JavaScript Can Access Clipboard"));
    QCheckBox *linkIncludedInFocusChain = new QCheckBox(QSL("Links Included In Focus Chain"));
    QCheckBox *localStorageEnabled = new QCheckBox(QSL("Local Storage Enabled"));
    QCheckBox *localContentCanAccessRemoteUrls = new QCheckBox(QSL("Local Content Can Access Remote Urls"));
    QCheckBox *xssAuditingEnabled = new QCheckBox(QSL("XSS Auditing Enabled"));
    QCheckBox *spatialNavigationEnabled = new QCheckBox(QSL("Spatial Navigation Enabled"));
    QCheckBox *localContentCanAccessFileUrls = new QCheckBox(QSL("Local Content Can Access File Urls"));
    QCheckBox *hyperLinkAuditingEnabled = new QCheckBox(QSL("Hyperlink Auditing Enabled"));
    QCheckBox *scrollAnimatorEnabled = new QCheckBox(QSL("Scroll Animator Enabled"));
    QCheckBox *errorPageEnabled = new QCheckBox(QSL("Error Page Enabled"));
    QCheckBox *pluginsEnabled = new QCheckBox(QSL("Plugins Enabled"));
    QCheckBox *fullScreenSupportEnabled = new QCheckBox(QSL("Full Screen Support Enabled"));
    QCheckBox *screenCaptureEnabled = new QCheckBox(QSL("Screen Capture Enabled"));
    QCheckBox *webGLEnabled = new QCheckBox(QSL("WebGL Enabled"));
    QCheckBox *accelerated2dCanvasEnabled = new QCheckBox(QSL("Accelerated 2d Canvas Enabled"));
    QCheckBox *autoLoadIconsForPage = new QCheckBox(QSL("Auto-Load Icons For Page"));
    QCheckBox *touchIconsEnabled = new QCheckBox(QSL("Touch Icons Enabled"));
    QCheckBox *focusOnNavigationEnabled = new QCheckBox(QSL("Focus On Navigation Enabled"));
    QCheckBox *printElementBackgrounds = new QCheckBox(QSL("Print Element Backgrounds"));
    QCheckBox *allowRunningInsecureContent = new QCheckBox(QSL("Allow Running Insecure Content"));
    QCheckBox *allowGeolocationOnInsecureOrigins = new QCheckBox(QSL("Allow Geolocation on Insecure Origins"));
    QCheckBox *allowWindowActivationFromJavaScript = new QCheckBox(QSL("Allow Window Activation From JavaScript"));
    QCheckBox *showScrollBars = new QCheckBox(QSL("Show Scroll Bars"));
    QCheckBox *playbackRequiresUserGesture = new QCheckBox(QSL("Playback Requires User Gesture"));
    QCheckBox *javaScriptCanPaste = new QCheckBox(QSL("JavaScript Can Paste"));
    QCheckBox *webRTCPublicInterfacesOnly = new QCheckBox(QSL("WebRTC Public Interfaces Only"));
    QCheckBox *dnsPrefetchEnabled = new QCheckBox(QSL("DNS Prefetch Enabled"));

    vboxLayout1->addWidget(autoLoadImages);
    vboxLayout1->addWidget(javaScriptEnabled);
    vboxLayout1->addWidget(javaScriptCanOpenWindows);
    vboxLayout1->addWidget(javaScriptCanAccessClipboard);
    vboxLayout1->addWidget(linkIncludedInFocusChain);
    vboxLayout1->addWidget(localStorageEnabled);
    vboxLayout1->addWidget(localContentCanAccessRemoteUrls);
    vboxLayout1->addWidget(xssAuditingEnabled);
    vboxLayout1->addWidget(spatialNavigationEnabled);
    vboxLayout1->addWidget(localContentCanAccessFileUrls);
    vboxLayout1->addWidget(hyperLinkAuditingEnabled);
    vboxLayout1->addWidget(scrollAnimatorEnabled);
    vboxLayout1->addWidget(errorPageEnabled);
    vboxLayout1->addWidget(pluginsEnabled);
    vboxLayout1->addWidget(fullScreenSupportEnabled);
    vboxLayout1->addWidget(screenCaptureEnabled);
    vboxLayout1->addWidget(webGLEnabled);
    vboxLayout1->addWidget(accelerated2dCanvasEnabled);
    vboxLayout1->addWidget(autoLoadIconsForPage);
    vboxLayout1->addWidget(touchIconsEnabled);
    vboxLayout1->addWidget(focusOnNavigationEnabled);
    vboxLayout1->addWidget(printElementBackgrounds);
    vboxLayout1->addWidget(allowRunningInsecureContent);
    vboxLayout1->addWidget(allowGeolocationOnInsecureOrigins);
    vboxLayout1->addWidget(allowWindowActivationFromJavaScript);
    vboxLayout1->addWidget(showScrollBars);
    vboxLayout1->addWidget(playbackRequiresUserGesture);
    vboxLayout1->addWidget(javaScriptCanPaste);
    vboxLayout1->addWidget(webRTCPublicInterfacesOnly);
    vboxLayout1->addWidget(dnsPrefetchEnabled);

    widget->setLayout(vboxLayout1);
    area->setWidget(widget);

    webEngineSettingsLayout->addWidget(area);
    webEngineSettings->setLayout(webEngineSettingsLayout);
    vboxLayout->addWidget(webEngineSettings);

    QGridLayout *gridLayout = new QGridLayout;

    QLabel *defaultZoom = new QLabel(QSL("Default Zoom On Pages (%)"));
    QComboBox *defaultZoomCombo = new QComboBox();
    defaultZoomCombo->setEditable(true);
    defaultZoomCombo->addItems(QStringList()
                               << QSL("25") << QSL("50") << QSL("75") << QSL("100")
                               << QSL("125") << QSL("150") << QSL("175") << QSL("200")
                               << QSL("225") << QSL("250") << QSL("275") << QSL("300"));
    gridLayout->addWidget(defaultZoom, 0, 0);
    gridLayout->addWidget(defaultZoomCombo, 0, 1);

    QLabel *userAgentLabel = new QLabel(QSL("User Agent"));
    QLineEdit *userAgent = new QLineEdit;

    gridLayout->addWidget(userAgentLabel, 1, 0);
    gridLayout->addWidget(userAgent, 1, 1);

    vboxLayout->addLayout(gridLayout);

    QHBoxLayout *hboxLayout = new QHBoxLayout;

    QPushButton *defaults = new QPushButton(QSL("Defaults"));
    QPushButton *apply = new QPushButton(QSL("Apply"));
    QPushButton *ok = new QPushButton(QSL("Ok"));

    hboxLayout->addWidget(defaults);
    hboxLayout->addWidget(new QWidget);
    hboxLayout->addWidget(apply);
    hboxLayout->addWidget(ok);

    vboxLayout->addWidget(new QWidget);
    vboxLayout->addLayout(hboxLayout);

    root->setLayout(vboxLayout);
    addTab(root, QSL("Browsing"));

    autoLoadImages->setChecked(appManager->settings()->value(QSL("webPage/autoLoadImages"), true).toBool());
    javaScriptEnabled->setChecked(appManager->settings()->value(QSL("webPage/javascriptEnabled"), true).toBool());
    javaScriptCanOpenWindows->setChecked(appManager->settings()->value(QSL("webPage/javascriptCanOpenWindows"), true).toBool());
    javaScriptCanAccessClipboard->setChecked(appManager->settings()->value(QSL("webPage/javascriptCanAccessClipboard"), false).toBool());
    linkIncludedInFocusChain->setChecked(appManager->settings()->value(QSL("webPage/linksIncludedInFocusChain"), true).toBool());
    localStorageEnabled->setChecked(appManager->settings()->value(QSL("webPage/localStorageEnabled"), true).toBool());
    localContentCanAccessRemoteUrls->setChecked(appManager->settings()->value(QSL("webPage/localContentCanAccessRemoteUrls"), false).toBool());
    xssAuditingEnabled->setChecked(appManager->settings()->value(QSL("webPage/xssAuditingEnabled"), true).toBool());
    spatialNavigationEnabled->setChecked(appManager->settings()->value(QSL("webPage/spatialNavigationEnabled"), false).toBool());
    localContentCanAccessFileUrls->setChecked(appManager->settings()->value(QSL("webPage/localContentCanAccessFileUrls"), true).toBool());
    hyperLinkAuditingEnabled->setChecked(appManager->settings()->value(QSL("webPage/hyperLinkAuditingEnabled"), false).toBool());
    scrollAnimatorEnabled->setChecked(appManager->settings()->value(QSL("webPage/scrollAnimatorEnabled"), false).toBool());
    errorPageEnabled->setChecked(appManager->settings()->value(QSL("webPage/errorPageEnabled"), true).toBool());
    pluginsEnabled->setChecked(appManager->settings()->value(QSL("webPage/pluginsEnabled"), true).toBool());
    fullScreenSupportEnabled->setChecked(appManager->settings()->value(QSL("webPage/fullScreenSupportEnabled"), true).toBool());
    screenCaptureEnabled->setChecked(appManager->settings()->value(QSL("webPage/screenCaptureEnabled"), true).toBool());
    webGLEnabled->setChecked(appManager->settings()->value(QSL("webPage/webGLEnabled"), true).toBool());
    accelerated2dCanvasEnabled->setChecked(appManager->settings()->value(QSL("webPage/accelerated2dCanvasEnabled"), true).toBool());
    autoLoadIconsForPage->setChecked(appManager->settings()->value(QSL("webPage/autoLoadIconsForPage"), true).toBool());
    touchIconsEnabled->setChecked(appManager->settings()->value(QSL("webPage/touchIconsEnabled"), true).toBool());
    focusOnNavigationEnabled->setChecked(appManager->settings()->value(QSL("webPage/focusOnNavigationEnabled"), true).toBool());
    printElementBackgrounds->setChecked(appManager->settings()->value(QSL("webPage/printElementBackground"), true).toBool());
    allowGeolocationOnInsecureOrigins->setChecked(appManager->settings()->value(QSL("webPage/allowGeoLocationOnInsecureOrigins"), false).toBool());
    allowWindowActivationFromJavaScript->setChecked(appManager->settings()->value(QSL("allowWindowActivationFromJavaScript"), false).toBool());
    showScrollBars->setChecked(appManager->settings()->value(QSL("webPage/showScrollBars"), true).toBool());
    playbackRequiresUserGesture->setChecked(appManager->settings()->value(QSL("webPage/playbackRequiresUserGesture"), true).toBool());
    javaScriptCanPaste->setChecked(appManager->settings()->value(QSL("webPage/javascriptCanPaste"), false).toBool());
    webRTCPublicInterfacesOnly->setChecked(appManager->settings()->value(QSL("webPage/webRTCPublicInterfacesOnly"), false).toBool());
    dnsPrefetchEnabled->setChecked(appManager->settings()->value(QSL("webPage/dnsPrefetchEnabled"), true).toBool());

    defaultZoomCombo->setCurrentText(appManager->settings()->value(QSL("webView/defaultZoom"), QSL("100")).toString());
    userAgent->setText(appManager->settings()->value(QSL("profile/userAgent"), appManager->webEngineProfile()->httpUserAgent()).toString());

    connect(defaults, &QPushButton::clicked, this, [autoLoadImages, javaScriptEnabled, javaScriptCanOpenWindows,
            javaScriptCanAccessClipboard, linkIncludedInFocusChain, localStorageEnabled, localContentCanAccessRemoteUrls,
            xssAuditingEnabled, spatialNavigationEnabled, localContentCanAccessFileUrls, hyperLinkAuditingEnabled,
            scrollAnimatorEnabled, errorPageEnabled, pluginsEnabled, fullScreenSupportEnabled, screenCaptureEnabled,
            webGLEnabled, accelerated2dCanvasEnabled, autoLoadIconsForPage, touchIconsEnabled, focusOnNavigationEnabled,
            printElementBackgrounds, allowGeolocationOnInsecureOrigins, allowWindowActivationFromJavaScript, showScrollBars,
            playbackRequiresUserGesture, javaScriptCanPaste, webRTCPublicInterfacesOnly, dnsPrefetchEnabled,
            defaultZoomCombo, userAgent] {
        autoLoadImages->setChecked(true);
        javaScriptEnabled->setChecked(true);
        javaScriptCanOpenWindows->setChecked(true);
        javaScriptCanAccessClipboard->setChecked(false);
        linkIncludedInFocusChain->setChecked(true);
        localStorageEnabled->setChecked(true);
        localContentCanAccessRemoteUrls->setChecked(false);
        xssAuditingEnabled->setChecked(true);
        spatialNavigationEnabled->setChecked(false);
        localContentCanAccessFileUrls->setChecked(true);
        hyperLinkAuditingEnabled->setChecked(false);
        scrollAnimatorEnabled->setChecked(false);
        errorPageEnabled->setChecked(true);
        pluginsEnabled->setChecked(true);
        fullScreenSupportEnabled->setChecked(true);
        screenCaptureEnabled->setChecked(true);
        webGLEnabled->setChecked(true);
        accelerated2dCanvasEnabled->setChecked(true);
        autoLoadIconsForPage->setChecked(true);
        touchIconsEnabled->setChecked(true);
        focusOnNavigationEnabled->setChecked(true);
        printElementBackgrounds->setChecked(true);
        allowGeolocationOnInsecureOrigins->setChecked(false);
        allowWindowActivationFromJavaScript->setChecked(false);
        showScrollBars->setChecked(true);
        playbackRequiresUserGesture->setChecked(true);
        javaScriptCanPaste->setChecked(false);
        webRTCPublicInterfacesOnly->setChecked(false);
        dnsPrefetchEnabled->setChecked(true);

        defaultZoomCombo->setCurrentText(QSL("100"));
        userAgent->setText(appManager->defaultUserAgent());
    });

    connect(apply, &QPushButton::clicked, this, [autoLoadImages, javaScriptEnabled, javaScriptCanOpenWindows,
            javaScriptCanAccessClipboard, linkIncludedInFocusChain, localStorageEnabled, localContentCanAccessRemoteUrls,
            xssAuditingEnabled, spatialNavigationEnabled, localContentCanAccessFileUrls, hyperLinkAuditingEnabled,
            scrollAnimatorEnabled, errorPageEnabled, pluginsEnabled, fullScreenSupportEnabled, screenCaptureEnabled,
            webGLEnabled, accelerated2dCanvasEnabled, autoLoadIconsForPage, touchIconsEnabled, focusOnNavigationEnabled,
            printElementBackgrounds, allowGeolocationOnInsecureOrigins, allowWindowActivationFromJavaScript, showScrollBars,
            playbackRequiresUserGesture, javaScriptCanPaste, webRTCPublicInterfacesOnly, dnsPrefetchEnabled,
            defaultZoomCombo, userAgent] {
        appManager->settings()->setValue(QSL("webPage/autoLoadImages"), autoLoadImages->isChecked());
        appManager->settings()->setValue(QSL("webPage/javascriptEnabled"), javaScriptEnabled->isChecked());
        appManager->settings()->setValue(QSL("webPage/javascriptCanOpenWindows"), javaScriptCanOpenWindows->isChecked());
        appManager->settings()->setValue(QSL("webPage/javascriptCanAccessClipboard"), javaScriptCanAccessClipboard->isChecked());
        appManager->settings()->setValue(QSL("webPage/linksIncludedInFocusChain"), linkIncludedInFocusChain->isChecked());
        appManager->settings()->setValue(QSL("webPage/localStorageEnabled"), localStorageEnabled->isChecked());
        appManager->settings()->setValue(QSL("webPage/localContentCanAccessRemoteUrls"), localContentCanAccessRemoteUrls->isChecked());
        appManager->settings()->setValue(QSL("webPage/xssAuditingEnabled"), xssAuditingEnabled->isChecked());
        appManager->settings()->setValue(QSL("webPage/spatialNavigationEnabled"), spatialNavigationEnabled->isChecked());
        appManager->settings()->setValue(QSL("webPage/localContentCanAccessFileUrls"), localContentCanAccessFileUrls->isChecked());
        appManager->settings()->setValue(QSL("webPage/hyperLinkAuditingEnabled"), hyperLinkAuditingEnabled->isChecked());
        appManager->settings()->setValue(QSL("webPage/scrollAnimatorEnabled"), scrollAnimatorEnabled->isChecked());
        appManager->settings()->setValue(QSL("webPage/errorPageEnabled"), errorPageEnabled->isChecked());
        appManager->settings()->setValue(QSL("webPage/pluginsEnabled"), pluginsEnabled->isChecked());
        appManager->settings()->setValue(QSL("webPage/fullScreenSupportEnabled"), fullScreenSupportEnabled->isChecked());
        appManager->settings()->setValue(QSL("webPage/screenCaptureEnabled"), screenCaptureEnabled->isChecked());
        appManager->settings()->setValue(QSL("webPage/webGLEnabled"), webGLEnabled->isChecked());
        appManager->settings()->setValue(QSL("webPage/accelerated2dCanvasEnabled"), accelerated2dCanvasEnabled->isChecked());
        appManager->settings()->setValue(QSL("webPage/autoLoadIconsForPage"), autoLoadIconsForPage->isChecked());
        appManager->settings()->setValue(QSL("webPage/touchIconsEnabled"), touchIconsEnabled->isChecked());
        appManager->settings()->setValue(QSL("webPage/focusOnNavigationEnabled"), focusOnNavigationEnabled->isChecked());
        appManager->settings()->setValue(QSL("webPage/printElementBackground"), printElementBackgrounds->isChecked());
        appManager->settings()->setValue(QSL("webPage/allowGeoLocationOnInsecureOrigins"), allowGeolocationOnInsecureOrigins->isChecked());
        appManager->settings()->setValue(QSL("allowWindowActivationFromJavaScript"), allowWindowActivationFromJavaScript->isChecked());
        appManager->settings()->setValue(QSL("webPage/showScrollBars"), showScrollBars->isChecked());
        appManager->settings()->setValue(QSL("webPage/playbackRequiresUserGesture"), playbackRequiresUserGesture->isChecked());
        appManager->settings()->setValue(QSL("webPage/javascriptCanPaste"), javaScriptCanPaste->isChecked());
        appManager->settings()->setValue(QSL("webPage/webRTCPublicInterfacesOnly"), webRTCPublicInterfacesOnly->isChecked());
        appManager->settings()->setValue(QSL("webPage/dnsPrefetchEnabled"), dnsPrefetchEnabled->isChecked());

        appManager->settings()->setValue(QSL("webView/defaultZoom"), defaultZoomCombo->currentText());
        appManager->settings()->setValue(QSL("profile/userAgent"), userAgent->text());

        QList<BrowserWindow *> windows = appManager->windows();
        for (BrowserWindow *window : windows) {
            int count = window->tabWidget()->count();
            for (int i = 0; i < count; i++) {
                window->tabWidget()->tabAt(i)->webView()->page()->loadSettings();
            }
        }
    });

    connect(ok, &QPushButton::clicked, this, [apply, this] {
        apply->click();
        setVisible(false);
    });
}

void Preferences::createSearchEngineTab()
{
    QWidget *widget = new QWidget;
    addTab(widget, QSL("Search Engine"));
}

void Preferences::createOthersTab()
{
    QWidget *widget = new QWidget;
    addTab(widget, QSL("Others"));
}