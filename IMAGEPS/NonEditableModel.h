#pragma once
#include <qabstractitemmodel.h>

class NonEditableModel : public QAbstractTableModel {
	Q_OBJECT
public:
	explicit NonEditableModel(QObject *parent = nullptr);
	NonEditableModel(int rows, int columns, QObject *parent = nullptr);
	~NonEditableModel();

	// 重写 QAbstractTableModel 的纯虚函数 
	int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	int columnCount(const QModelIndex &parent = QModelIndex()) const override;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

	// 设置不可编辑 
	Qt::ItemFlags flags(const QModelIndex &index) const override;

	// 批量添加数据的公共方法 
	void appendRowsBatch(const QList<QList<QVariant>>& rows);
	void setHorizontalHeaderLabels(const QStringList &labels);
	void setDataBatch(const QList<QList<QVariant>>& data);

	// 清空数据 
	void clear();

	// 设置数据 
	bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

	// 获取指定单元格的数据 
	QVariant getData(int row, int column, int role = Qt::DisplayRole) const;


	void setRowCount(int rows);
	void setColumnCount(int columns);
	bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
	bool removeRowsBatch(const QVector<int>& rows);
	QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
	QVariant horizontalHeaderItem(int section) const;


private:
	int m_rows;
	int m_columns;
	QVector<QVector<QVariant>> m_data;
	QStringList m_headers;
};

