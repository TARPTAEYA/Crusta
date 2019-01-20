#ifndef CR_TAB_BAR_H
#define CR_TAB_BAR_H

#include <QToolButton>
#include <QTabBar>

class TabBar : public QTabBar
{
public:
    explicit TabBar(QWidget *parent = nullptr);

    void loadSettings();

private:
    int getTabWidth(int index) const;

    QToolButton *m_newTabButton = nullptr;

    bool m_isInClosingState = false;
    int m_tabCountForClosingState = 0;

protected:
    QSize tabSizeHint(int index) const;
    void paintEvent(QPaintEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
};

#endif
