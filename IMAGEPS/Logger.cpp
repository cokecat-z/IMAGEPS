#include "Logger.h"
#include <QDir>
#include <QCoreApplication>
#include <QStandardPaths>

#ifdef Q_OS_LINUX 
#include <syslog.h>
#endif 

Logger::Logger(){
}

void Logger::write(LogLevel level, const QString& message, const char* file, int line)
{
	QMutexLocker locker(&mutex_);

	QString timeStr = QDateTime::currentDateTime().toString("[yyyy-MM-dd hh:mm:ss.zzz]");

	QString levelStr;
	switch (level) {
	case DEBUG:    levelStr = "DEBUG"; break;
	case INFO:     levelStr = "INFO"; break;
	case WARNING:  levelStr = "WARN"; break;
	case PSERROR:    levelStr = "ERROR"; break;
	case CRITICAL: levelStr = "FATAL"; break;
	}

	QString fullMessage = QString::fromLocal8Bit("%1 [%2] [%3] %4")
		.arg(timeStr)
		.arg(levelStr)
		.arg(projectName_)
		.arg(message);

	if (file && line > 0) {
		//QString srcLocation = QFileInfo(file).fileName();
		fullMessage += QString::fromLocal8Bit(" (%1:%2)").arg(file).arg(line);
	}

	// 写入文件 
	writeToFile(fullMessage);

	// 控制台输出 
	if (config_.consoleOutput) {
		QTextStream out(stdout);
		switch (level) {
		case DEBUG:
		case INFO:
			out << fullMessage << Qt::endl; break;
		case WARNING:
			out << "\033[33m" << fullMessage << "\033[0m" << Qt::endl; break;
		case PSERROR:
		case CRITICAL:
			out << "\033[31m" << fullMessage << "\033[0m" << Qt::endl; break;
		}
	}

	// 系统日志
	if (config_.useSystemLog) {
		writeToSystemLog(level, fullMessage);
	}
}

void Logger::setConfig(const LogConfig& config)
{
	QMutexLocker locker(&mutex_);
	config_ = config;
	projectName_ = config_.logFilePrefix;
	initLogDir();
	cleanupOldLogs();
}

LogConfig Logger::getConfig() const
{
	QMutexLocker locker(&mutex_);
	return config_;
}

void Logger::initLogDir()
{
	QDir dir(config_.logDir);
	if (!dir.exists()) {
		dir.mkpath(".");
	}
}

void Logger::cleanupOldLogs()
{
	QDateTime now = QDateTime::currentDateTime();
	QDir dir(config_.logDir);

	QStringList logFiles = dir.entryList(
		QStringList() << config_.logFilePrefix + "_" + projectName_ + "_*.log",
		QDir::Files);

	foreach(const QString &logFile, logFiles) {
		QFileInfo fileInfo(dir.filePath(logFile));
		if (fileInfo.lastModified().daysTo(now) > config_.maxDays) {
			QFile::remove(fileInfo.absoluteFilePath());
		}
	}
}

QString Logger::getCurrentLogFilePath() const
{
	QString dateStr = QDate::currentDate().toString("yyyyMMdd");
	return QDir(config_.logDir).filePath(
		QString("%1_%2_%3.log")
		.arg(config_.logFilePrefix)
		.arg(projectName_)
		.arg(dateStr));
}

void Logger::writeToFile(const QString& message)
{
	QString logFilePath = getCurrentLogFilePath();
	QFile logFile(logFilePath);

	if (logFile.size() > config_.maxSizeMB * 1024 * 1024) {
		QString timeStr = QDateTime::currentDateTime().toString("yyyyMMddhhmmss");
		QString backupPath = logFilePath + "." + timeStr;
		logFile.rename(backupPath);
	}

	if (logFile.open(QIODevice::Append | QIODevice::Text)) {
		QTextStream out(&logFile);
		out.setCodec("UTF-8");

		out << message << "\n";
		logFile.close();
	}
}

void Logger::writeToSystemLog(LogLevel level, const QString& message) const
{
#ifdef Q_OS_LINUX
	int priority;
	switch (level) {
	case DEBUG: priority = LOG_DEBUG; break;
	case INFO: priority = LOG_INFO; break;
	case WARNING: priority = LOG_WARNING; break;
	case PSERROR: priority = LOG_ERR; break;
	case CRITICAL: priority = LOG_CRIT; break;
	default: priority = LOG_INFO;
	}
	syslog(priority, "[%s] %s",
		projectName_.toUtf8().constData(),
		message.toUtf8().constData());
#endif
}