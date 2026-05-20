#ifndef LOGGER_H
#define LOGGER_H 

#include <QString>
#include <QMutex>
#include <QFile> 
#include <QTextStream>
#include <QDateTime>

// 日志级别 
enum LogLevel {
	DEBUG,
	INFO,
	WARNING,
	PSERROR,  // 修改这里避免与Windows宏冲突 
	CRITICAL
};

// 日志配置
struct LogConfig {
	QString logDir;                  // 日志目录
	QString logFilePrefix = "log";   // 日志文件前缀
	int maxSizeMB = 5;               // 单个日志文件最大大小(MB)
	int maxDays = 7;                 // 日志保留天数 
	bool consoleOutput = true;       // 是否输出到控制台 
	bool useSystemLog = false;       // 是否使用系统日志(如syslog)
};

class Logger
{
public:
	// 构造函数 
	explicit Logger();

	// 记录日志
	void write(LogLevel level, const QString& message,
		const char* file = nullptr, int line = 0);

	// 设置新配置
	void setConfig(const LogConfig& config);

	// 获取当前配置
	LogConfig getConfig() const;

private:
	// 初始化日志目录 
	void initLogDir();

	// 清理过期日志 
	void cleanupOldLogs();

	// 获取当前日志文件路径
	QString getCurrentLogFilePath() const;

	// 写入日志到文件
	void writeToFile(const QString& message);

	// 写入系统日志 
	void writeToSystemLog(LogLevel level, const QString& message) const;

	QString projectName_;    // 工程名称(用于区分不同工程)
	LogConfig config_;       // 日志配置
	mutable QMutex mutex_;   // 互斥锁(保证线程安全)
};

// 宏定义简化调用(每个工程有自己的宏)
#define PROJECT_LOG_DEBUG(logger, msg)    logger.write(DEBUG, msg, __FILE__, __LINE__)
#define PROJECT_LOG_INFO(logger, msg)     logger.write(INFO, msg, __FILE__, __LINE__)
#define PROJECT_LOG_WARNING(logger, msg)  logger.write(WARNING, msg, __FILE__, __LINE__)
#define PROJECT_LOG_ERROR(logger, msg)    logger.write(PSERROR, msg, __FILE__, __LINE__)
#define PROJECT_LOG_CRITICAL(logger, msg) logger.write(CRITICAL, msg, __FILE__, __LINE__)

#endif // LOGGER_H