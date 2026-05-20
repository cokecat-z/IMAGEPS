#ifndef PUBLICFUNCTIONS_H
#define PUBLICFUNCTIONS_H

#include <opencv2/opencv.hpp>
#include <QImage>
#include <QString>
#include <QFile>
#include <QSet>
#include <QTextCodec>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QDateTime>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QDomElement>
#include <QDebug>


class PublicFunctions
{
public:
	// 将OpenCV的Mat转换为QImage
	static QImage cvMatToQImage(const cv::Mat &mat);

	//读取文件
	static QList<QStringList> loadFile(QString filePath, QString splitFlag);

	//读取配置文件
	static QMap<QString, QList<QStringList>> loadConfigFile(QString filePath);

	static QList<QPair<QString, QList<QStringList>>> loadConfigFileL(QString filePath);

	//将工程名和工程目录写入.csv
	static bool writeToCsv(const QString &filePath, const QList<QStringList> &records);

	static QHash<QString, QString> loadFromCsv(const QString &filePath);

	static QString findFileDir(const QHash<QString, QString> &fileMap, const QString &filename);

	//文件标记读取
	static QStringList getNewlyCreatedFiles(const QString& dirPath, const QDateTime & markerFileTime);

	//static bool copyControlPointToPSTimestamp();
	static bool copyControlPointToPSTimestamp(const QString& projectDir);

	static void updateOrCreateElement(QDomDocument& doc, QDomElement& parent,
		const QString& tagName, const QString& text);

	//static bool writeTimestampToXml(const QString& nodeName);

	//static QDateTime readTimestampFromXml(const QString& nodeName);

	static bool writeTimestampToXml(const QString& nodeName, const QString& projectDir);

	static QDateTime readTimestampFromXml(const QString& nodeName, const QString& projectDir);

	static void restartApplication();

	static bool validateDogLicense();

	static QStringList parseCsvLine(const QString &line);
	static QString escapeCsvField(const QString &field);
	static QString unescapeCsvField(const QString &field);
};

#endif