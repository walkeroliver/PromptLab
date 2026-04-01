#pragma once

#include <QWidget>
#include <QString>

namespace StarRatingText {
QString toDisplayString(double rating, int starCount = 5);
}

class StarRating : public QWidget {
    Q_OBJECT
    Q_PROPERTY(double rating READ rating WRITE setRating NOTIFY ratingChanged)
    Q_PROPERTY(bool editable READ isEditable WRITE setEditable)

public:
    explicit StarRating(QWidget* parent = nullptr);

    double rating() const { return m_rating; }
    void setRating(double rating);
    bool isEditable() const { return m_editable; }
    void setEditable(bool editable);
    void setStarSize(int size);
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

signals:
    void ratingChanged(double rating);
    void editingFinished();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    double m_rating = 0.0;
    bool m_editable = true;
    int m_starCount = 5;
    int m_hoverStar = 0;
    int m_starSize = 20;

    int starAtPosition(int x) const;
};
