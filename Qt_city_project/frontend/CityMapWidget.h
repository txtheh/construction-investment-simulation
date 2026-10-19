#pragma once

#include <QColor>
#include <QPointF>
#include <QVector>
#include <QWidget>

class QPainter;

struct BuildingVisual {
    enum class Type {
        House,
        Shop,
        Office
    };

    int owner = 0;
    int row = 0;
    int column = 0;
    Type type = Type::House;
    int stage = 4;
    int houseType = 0;
    int apartmentsSold = 0;
};

class CityMapWidget : public QWidget {
public:
    explicit CityMapWidget(QWidget *parent = nullptr);

    QSize sizeHint() const override;
    void setBuildings(const QVector<BuildingVisual> &buildings);
    void setOwnerColors(const QVector<QColor> &colors);
    void setGridSize(int size);
    int gridSize() const;
    void advanceConstruction();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QPointF tileCenter(int row, int column, qreal tileWidth, qreal tileHeight,
                       qreal originX, qreal originY) const;
    void drawTile(QPainter &painter, const QPointF &center, qreal tileWidth,
                  qreal tileHeight, bool road, int row, int column);
    void drawTree(QPainter &painter, const QPointF &center, qreal size);
    void drawBuilding(QPainter &painter, const QPointF &center, qreal tileWidth,
                      qreal tileHeight, const BuildingVisual &building);
    QColor ownerColor(int owner) const;

    QVector<BuildingVisual> buildings_;
    QVector<QColor> ownerColors_;
    int animationFrame_ = 0;
    int mapRows_ = 16;
    int mapColumns_ = 16;
};
