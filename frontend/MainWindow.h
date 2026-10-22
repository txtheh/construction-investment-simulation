#pragma once

#include "CityMapWidget.h"
#include "GameTypes.h"

#include "../backend/core/GameEngine.h"

#include <QHash>
#include <QMainWindow>
#include <QPoint>
#include <QString>
#include <QVector>

#include <memory>

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
    void startGame(const QVector<BotSetup> &setups, int months, 
            int gridSize, int maxBuildings);
    void openSettingsDialog();
    void advanceMonth();
    void toggleAutoplay();
    void refreshFromEngine();
    void rebuildBuildingVisuals();
    void rebuildBotPanels();
    void clearLayout(QLayout *layout);
    void addEvent(const QString &message);
    void showResults();
    void refreshSalesTable();
    void buildAvailableSites(int gridSize);

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

    std::unique_ptr<GameEngine> engine_;

    QVector<BotInfo> bots_;
    QVector<BuildingVisual> buildings_;
    QVector<QPoint> availableSites_;
    QHash<int, QPoint> cellById_;

    int currentMonth_ = 0;
    int totalMonths_ = 12;
    int preferredGridSize_ = 16;
    bool autoplayRunning_ = false;
};
