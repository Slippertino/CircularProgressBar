#ifndef CIRCULARPROGRESSBAR_H
#define CIRCULARPROGRESSBAR_H

#include <QWidget>
#include <QPainter>
#include <QPaintEvent>
#include <QPainterPath>
#include <QTimer>
#include <QLinearGradient>
#include <algorithm>
#include <cmath>
#include <stdexcept>

class CircularProgressBar final : public QWidget
{
    Q_OBJECT

public:
    CircularProgressBar(QWidget *parent = nullptr);

    int minimum() const noexcept;
    int maximum() const noexcept;
    int value() const noexcept;

    void setDisplayState(bool flag);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

    ~CircularProgressBar();

public slots:
    void reset();
    void setValue(int nval);
    void setMinimum(int nmin);
    void setMaximum(int nmax);
    void setRange(int nmin, int nmax);

signals:
    void valueChanged(int nval);

protected:
    void paintEvent(QPaintEvent *pe) override;

private:
    void validateRange() const;

    QRect getExternalRect() const;
    QRect getInternalRect(const QRect& external) const;
    QRect getTextRect(const QRect& external) const;
    QRect getInnerRect(const QRect &external, qreal factor = 1.0) const;

    QPoint rotateCircular(const QRect& rect, const QPoint& target, qreal angle);
    QPainterPath markUpAnnularSpace(
        const QRect& exRect,
        const QRect& inRect,
        qreal offsetAngle,
        qreal spanAngle
    );

    QBrush createMovieBrush(const QRect &rect, qreal angle);

    QFont adjustFontToRect(
        const QString &text,
        const QFont &src,
        const QRect &rect
    );

private:
    int min_, max_;
    int value_;
    bool displayState_{ true };

    bool animationEnabled_{false};
    bool animationMovieToDraw_;
    QTimer *animationTimer_, *animationMovieTimer_;
    qreal animationOffsetAngle_{0};
};
#endif // CIRCULARPROGRESSBAR_H
