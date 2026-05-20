#ifndef PUBLICSTRUCT_H
#define PUBLICSTRUCT_H
#include <QString>
#include <QList>

struct SatImageData {
	QString path;
	double imageX;
	double imageY;
	double imageVX;
	double imageVY;
};

// 加密点数据结构 
struct EncryptedPoint {
	QString id;
	int overlap;
	QString residual;
	QString state;
	QString pointX;
	QString pointY;
	QString pointZ;
	QList<SatImageData> satImages;
};

// 控制点数据结构
struct ControlPoint {
	QString id;
	QString type;
	int overlap;
	QString imageSide;
	QString objectSide;
	QString height;
	QString state;
	QString priority;
	QString pointX;
	QString pointY;
	QString pointZ;
	QList<SatImageData> satImages;
};

#endif