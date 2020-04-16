#include "mesontoolmodel.h"
#include "mesonetooltreeitem.h"
#include "utils/qtcassert.h"
#include "utils/stringutils.h"

namespace MesonProjectManager {
namespace Internal {

MesonToolModel::MesonToolModel()
{
    setHeader({tr("Name"), tr("Location")});
    rootItem()->appendChild(new Utils::StaticTreeItem(tr("Auto-detected")));
    rootItem()->appendChild(new Utils::StaticTreeItem(tr("Manual")));
    for (const auto &tool : MesonTools::tools()) {
        addMesonTool(tool);
    }
}

MesoneToolTreeItem *MesonToolModel::mesoneToolTreeItem(const QModelIndex &index) const
{
    return itemForIndexAtLevel<2>(index);
}

void MesonToolModel::updateItem(const Core::Id &itemId,
                                const QString &name,
                                const Utils::FilePath &exe)
{
    auto treeItem = findItemAtLevel<2>(
        [itemId](MesoneToolTreeItem *n) { return n->id() == itemId; });
    QTC_ASSERT(treeItem, return );
    treeItem->update(name, exe);
}

void MesonToolModel::addMesonTool()
{
    manualGroup()->appendChild(new MesoneToolTreeItem{uniqueName(tr("New Meson"))});
}

void MesonToolModel::removeMesonTool(MesoneToolTreeItem *item)
{
    QTC_ASSERT(item,return);
    destroyItem(item);
    m_itemsToRemove.enqueue(item->id());
}

MesoneToolTreeItem * MesonToolModel::cloneMesonTool(MesoneToolTreeItem *item)
{
    QTC_ASSERT(item,return nullptr);
    auto newItem = new MesoneToolTreeItem(*item);
    manualGroup()->appendChild(newItem);
    return item;
}

void MesonToolModel::apply()
{
    forItemsAtLevel<2>([](MesoneToolTreeItem *item) {
        if (item->hasUnsavedChanges())
        {
            MesonTools::updateTool(item->id(), item->name(), item->executable());
            item->setSaved();
        }
    });
    while (!m_itemsToRemove.isEmpty()) {
        MesonTools::removeTool(m_itemsToRemove.dequeue());
    }
}

void MesonToolModel::addMesonTool(const MesonWrapper &tool)
{
    if (tool.autoDetected())
        autoDetectedGroup()->appendChild(new MesoneToolTreeItem(tool));
    else
        manualGroup()->appendChild(new MesoneToolTreeItem(tool));
}

QString MesonToolModel::uniqueName(const QString &baseName)
{
    QStringList names;
    forItemsAtLevel<2>([&names](auto *item) { names << item->name(); });
    return Utils::makeUniquelyNumbered(baseName, names);
}

Utils::TreeItem *MesonToolModel::autoDetectedGroup() const
{
    return rootItem()->childAt(0);
}

Utils::TreeItem *MesonToolModel::manualGroup() const
{
    return rootItem()->childAt(1);
}
} // namespace Internal
} // namespace MesonProjectManager
