#pragma once

#include "CityMapWidget.h"
#include "GameTypes.h"

#include <QMainWindow>
#include <QPoint>
#include <QVector>
#include <QString>

class QHBoxLayout;
class QLayout;
class QLabel;
class QPlainTextEdit;
class QProgressBar;
class QPushButton;
class QTableWidget;
class QStackedWidget;
class QTimer;
class QVBoxLayout;
class QWidget;

class MainWindow : public QMainWindow {
public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    QWidget *createMenuPage();
    QWidget *createGamePage();
    QWidget *createStatPanel(const BotInfo &bot);
    QWidget *createLegendPanel();
    QWidget *createRealtorPanel();
    QWidget *createMiniMapPanel();
    void openSetupDialog();
    void startDemoGame(const QVector<BotSetup> &setups, int months, int gridSize);
    void openSettingsDialog();
    void advanceMonth();
    void toggleAutoplay();
    void updateGameUi();
    void rebuildBotPanels();
    void clearLayout(QLayout *layout);
    void addEvent(const QString &message);
    void showResults();
    void updateCountsFromBuildings();
    void recordDemoSales();
    void refreshSalesTable();

    QStackedWidget *pages_ = nullptr;
    QWidget *menuPage_ = nullptr;
    QWidget *gamePage_ = nullptr;
    CityMapWidget *cityMap_ = nullptr;
    QLabel *monthLabel_ = nullptr;
    QProgressBar *monthProgress_ = nullptr;
    QPushButton *autoplayButton_ = nullptr;
    QPushButton *pauseButton_ = nullptr;
    QPlainTextEdit *eventsLog_ = nullptr;
    QVBoxLayout *leftBotsLayout_ = nullptr;
    QWidget *leftBotsHost_ = nullptr;
    QWidget *miniMap_ = nullptr;
    QTableWidget *salesTable_ = nullptr;
    QTimer *autoplayTimer_ = nullptr;

    QVector<BotInfo> bots_;
    QVector<BuildingVisual> buildings_;
    QVector<QPoint> demoSites_;
    struct DemoSale {
        int month = 0;
        int botIndex = 0;
        QString houseType;
        int apartments = 0;
        double pricePerSqm = 0.0;
    };
    QVector<DemoSale> demoSales_;
    int currentMonth_ = 0;
    int totalMonths_ = 12;
    int nextDemoSite_ = 0;
    int preferredGridSize_ = 16;
    bool autoplayRunning_ = false;
};
