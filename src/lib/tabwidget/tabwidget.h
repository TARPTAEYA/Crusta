#ifndef CR_TAB_WIDGET_H
#define CR_TAB_WIDGET_H

#include <QTabWidget>

class Tab;
class TabBar;

class TabWidget : public QTabWidget
{
public:
    explicit TabWidget(QWidget *parent = nullptr);

    int addTab();
    int addTab(const QString &address);
    void closeTab(int index);

    Tab *currentTab();

private:
    TabBar *m_tabBar = nullptr;
};

#endif
