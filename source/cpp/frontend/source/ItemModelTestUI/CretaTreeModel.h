#ifndef CRETATREEMODEL_H
#define CRETATREEMODEL_H

#include <QAbstractItemModel>
class CretaTreeItem
{ public: explicit CretaTreeItem(QString const & caption, CretaTreeItem * parent = nullptr) :
        m_parentItem(parent)
    {
        m_itemData[Qt::DisplayRole] = caption;
    }
    ~CretaTreeItem() { clearChildren();  }
    void clearChildren()
    {
        qDeleteAll(m_childItems);
        m_childItems.clear();
    }

    int childCount() const { return m_childItems.count();  }
    QVariant data(int role) const { return m_itemData[role]; }
    CretaTreeItem * parent() { return m_parentItem; }

    CretaTreeItem * child(int number)
    {
        if (number < 0 || number >= m_childItems.size()) {
            return nullptr;
        }
        return m_childItems.at(number);
    }
    bool insertChildren(int position)
    {
        if (position < 0 || position > m_childItems.size()) {
            return false;
        }
        m_childItems.insert(position, new CretaTreeItem("", this));
        return true;
    }

    int childNumber() const
    {
        if (m_parentItem) {
            return m_parentItem->m_childItems.indexOf(const_cast<CretaTreeItem *>(this));
        }
        return 0;
    }
    void setData(int role, QVariant const & value) { m_itemData[role] = value; }
    QVector<CretaTreeItem *> & childItems() { return m_childItems; }

private:
    QVector<CretaTreeItem *> m_childItems;
    QMap<int, QVariant> m_itemData;
    CretaTreeItem * m_parentItem;
};

class CretaTreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit CretaTreeModel(QObject *parent = nullptr);

    // Header:
    QVariant headerData(int section,
                        Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    // Basic functionality:
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private:
};

#endif // CRETATREEMODEL_H
