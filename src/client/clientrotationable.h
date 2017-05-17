//#ifndef CLIENTROTATIONABLE_H
//#define CLIENTROTATIONABLE_H

//#include "src/rotationlabel.h"

//const int DefaultRotateRatio = 2;

//class Panel;

//class ClientRotationable : public RotationLabel {
//private:
//    qreal _angle;
//    QTimer _timer;
//    QPixmap _pic;
//    bool _isActive;
//    int _rotateRatio;
//public:
//    ClientRotationable(QWidget *parent = 0) :
//    QLable(parent),
//    _angle(0),
//    _timer(this),
//    _pic(":/server/fan"),
//    _isActive(false),
//    _rotateRatio(DefaultRotateRatio)
//    {
//        _pic = _pic.scaled(128, 128, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
//        this->setPixmap(_pic);
//        _timer.start(10);

//        connect(&_timer, &QTimer::timeout, [this](){
//            auto rotated = _pic.transformed(QTransform().rotate(_angle),Qt::SmoothTransformation);
//            int xoffset = (rotated.width() - _pic.width()) / 2;
//            int yoffset = (rotated.height() - _pic.height()) / 2;
//            this->setPixmap(rotated.copy(xoffset, yoffset, _pic.width(), _pic.height()));
//            if (_isActive)
//                _angle += _rotateRatio;
//        });
//    }
//    void AdjustRotateRatio(int rotateratio) {
//        _rotateRatio = rotateratio;
//    }
//};

//#endif // CLIENTROTATIONABLE_H
