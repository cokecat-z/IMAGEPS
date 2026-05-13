#include "NonEditableModel.h"

NonEditableModel::NonEditableModel(QObject *parent)
	: QAbstractTableModel(parent), m_rows(0), m_columns(0)
{
}

NonEditableModel::NonEditableModel(int rows, int columns, QObject *parent)
	: QAbstractTableModel(parent), m_rows(rows), m_columns(columns)
{
	// 初始化数据容器 
	m_data.resize(m_rows);
	for (int i = 0; i < m_rows; ++i) {
		m_data[i].resize(m_columns);
	}
}

NonEditableModel::~NonEditableModel()
{
}

int NonEditableModel::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent)
		return m_rows;
}

int NonEditableModel::columnCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent)
		return m_columns;
}

QVariant NonEditableModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid() || role != Qt::DisplayRole)
		return QVariant();

	if (index.row() >= m_rows || index.column() >= m_columns)
		return QVariant();

	return m_data[index.row()][index.column()];
}

QVariant NonEditableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role != Qt::DisplayRole)
		return QVariant();

	if (orientation == Qt::Horizontal) {
		if (section >= 0 && section < m_headers.size())
			return m_headers[section];
		else
			return QString("Column %1").arg(section + 1);
	}
	else {
		return QString("Row %1").arg(section + 1);
	}
}

Qt::ItemFlags NonEditableModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return Qt::NoItemFlags;

	// 设置为不可编辑，但可以选择 
	return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

void NonEditableModel::appendRowsBatch(const QList<QList<QVariant>>& rows)
{
	if (rows.isEmpty())
		return;

	beginInsertRows(QModelIndex(), m_rows, m_rows + rows.size() - 1);

	for (const QList<QVariant>& row : rows) {
		QVector<QVariant> newRow;
		newRow.resize(m_columns);
		for (int i = 0; i < qMin(row.size(), m_columns); ++i) {
			newRow[i] = row[i];
		}
		m_data.append(newRow);
		m_rows++;
	}

	endInsertRows();
}

void NonEditableModel::setHorizontalHeaderLabels(const QStringList &labels)
{
	m_headers = labels;
	if (labels.size() > m_columns) {
		setColumnCount(labels.size());
	}
	emit headerDataChanged(Qt::Horizontal, 0, m_columns - 1);
}

void NonEditableModel::clear()
{
	beginResetModel();
	m_data.clear();
	m_rows = 0;
	endResetModel();
}

bool NonEditableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if (!index.isValid() || role != Qt::EditRole)
		return false;

	if (index.row() >= m_rows || index.column() >= m_columns)
		return false;

	// 确保数据容器足够大 
	if (m_data.size() <= index.row()) {
		m_data.resize(index.row() + 1);
	}
	if (m_data[index.row()].size() <= index.column()) {
		m_data[index.row()].resize(m_columns);
	}

	m_data[index.row()][index.column()] = value;
	emit dataChanged(index, index, { role });
	return true;
}

QVariant NonEditableModel::getData(int row, int column, int role) const
{
	if (role != Qt::DisplayRole && role != Qt::EditRole)
		return QVariant();

	if (row < 0 || row >= m_rows || column < 0 || column >= m_columns)
		return QVariant();

	return m_data[row][column];
}

// 新增方法的实现

void NonEditableModel::setRowCount(int rows)
{
	if (rows == m_rows)
		return;

	if (rows > m_rows) {
		// 增加行数
		beginInsertRows(QModelIndex(), m_rows, rows - 1);
		m_data.resize(rows);
		for (int i = m_rows; i < rows; ++i) {
			m_data[i].resize(m_columns);
		}
		m_rows = rows;
		endInsertRows();
	}
	else {
		// 减少行数 
		beginRemoveRows(QModelIndex(), rows, m_rows - 1);
		m_data.resize(rows);
		m_rows = rows;
		endRemoveRows();
	}
}

void NonEditableModel::setColumnCount(int columns)
{
	if (columns == m_columns)
		return;

	if (columns > m_columns) {
		// 增加列数 
		beginResetModel();
		for (int i = 0; i < m_rows; ++i) {
			m_data[i].resize(columns);
		}
		m_columns = columns;
		endResetModel();
	}
	else {
		// 减少列数 
		beginResetModel();
		for (int i = 0; i < m_rows; ++i) {
			m_data[i].resize(columns);
		}
		m_columns = columns;
		endResetModel();
	}
}

bool NonEditableModel::removeRows(int row, int count, const QModelIndex &parent)
{
	Q_UNUSED(parent)

		if (row < 0 || count <= 0 || row + count > m_rows)
			return false;

	beginRemoveRows(QModelIndex(), row, row + count - 1);
	for (int i = 0; i < count; ++i) {
		m_data.remove(row);
	}
	m_rows -= count;
	endRemoveRows();

	return true;
}

QModelIndex NonEditableModel::index(int row, int column, const QModelIndex &parent) const
{
	Q_UNUSED(parent)
		if (row < 0 || row >= m_rows || column < 0 || column >= m_columns)
			return QModelIndex();

	return createIndex(row, column);
}

QVariant NonEditableModel::horizontalHeaderItem(int section) const
{
	if (section >= 0 && section < m_headers.size())
		return m_headers[section];
	return QVariant();
}

//void NonEditableModel::setDataBatch(const QList<QList<QVariant>>& data)
//{
//	if (data.isEmpty()) {
//		setRowCount(0);
//		return;
//	}
//
//	beginResetModel(); // 使用 reset 模型以获得最佳性能
//
//	m_rows = data.size();
//	m_columns = data.isEmpty() ? 0 : data.first().size();
//
//	m_data.resize(m_rows);
//	for (int i = 0; i < m_rows; ++i) {
//		m_data[i].resize(m_columns);
//		for (int j = 0; j < qMin(data[i].size(), m_columns); ++j) {
//			m_data[i][j] = data[i][j];
//		}
//	}
//
//	endResetModel();
//}

void NonEditableModel::setDataBatch(const QList<QList<QVariant>>& data)
{
	beginResetModel();

	m_rows = data.size();
	m_columns = data.isEmpty() ? 0 : data.first().size();

	m_data.resize(m_rows);
	for (int i = 0; i < m_rows; ++i) {
		m_data[i].resize(m_columns);
		for (int j = 0; j < qMin(data[i].size(), m_columns); ++j) {
			m_data[i][j] = data[i][j];
		}
	}

	endResetModel();
}

bool NonEditableModel::removeRowsBatch(const QVector<int>& rows)
{
	if (rows.isEmpty())  return true;

	// 按从大到小排序 
	QVector<int> sortedRows = rows;
	std::sort(sortedRows.begin(), sortedRows.end(), std::greater<int>());

	// 批量删除行 
	for (int row : sortedRows) {
		if (row < 0 || row >= m_rows) continue;

		// 发出删除信号 
		beginRemoveRows(QModelIndex(), sortedRows.last(), sortedRows.first());

		// 批量删除数据 
		for (int row : sortedRows) {
			m_data.remove(row);
		}

		m_rows -= sortedRows.size();
		endRemoveRows();
	}
	return true;
}

