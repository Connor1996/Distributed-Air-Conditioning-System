#ifndef ROTATIONLABEL_H
#define ROTATIONLABEL_H

#include <QLabel>
#include <QTimer>

class RotationLabel : public QLabel
{
public:
    RotationLabel(int width = 64, int rotateratio = 2, QWidget *parent = 0) :
        QLabel(parent),
        _width(width),
        _rotateratio(rotateratio),
        _angle(0),
        _timer(this),
        _pic(":/server/fan"),
        _isActive(false)
    {
        _pic = _pic.scaled(_width, _width, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        this->setPixmap(_pic);
        _timer.start(10);

        connect(&_timer, &QTimer::timeout, [this](){
            auto rotated = _pic.transformed(QTransform().rotate(_angle),Qt::SmoothTransformation);
            int xoffset = (rotated.width() - _pic.width()) / 2;
            int yoffset = (rotated.height() - _pic.height()) / 2;
            this->setPixmap(rotated.copy(xoffset, yoffset, _pic.width(), _pic.height()));
            if (_isActive)
                _angle += _rotateratio;
        });
    }

    bool isActive() { return _isActive; }
    void Start() { _isActive = true; }
    void Stop() { _isActive = false; }
    void AdjustRotateRatio(int rotateratio) { _rotateratio = rotateratio; }

private:
    int _width;
    int _rotateratio;
    qreal _angle;
    QTimer _timer;
    QPixmap _pic;
    bool _isActive;
};

#endif // ROTATIONLABEL_H
