#include "toolsmodel.h"
#include "tooltreeitem.h"
#include "utils/qtcassert.h"
#include "utils/stringutils.h"
#include <ExeWrappers/mesontools.h>

namespace MesonProjectManager {
namespace Internal {

ToolsModel::ToolsModel()
{
    setHeader({tr("Name"), tr("Location")});
    rootItem()->appendChild(new Utils::StaticTreeItem(tr("Auto-detected")));
    rootItem()->appendChild(new Utils::StaticTreeItem(tr("Manual")));
    for (const auto &tool : MesonTools::tools()) {
        addMesonTool(tool);
    }
}

ToolTreeItem *ToolsModel::mesoneToolTreeItem(const QModelIndex &index) const
{
    return itemForIndexAtLevel<2>(index);
}

void ToolsModel::updateItem(const Core::Id &itemId,
                                const QString &name,
                                const Utils::FilePath &exe)
{
    auto treeItem = findItemAtLevel<2>(
        [itemId](ToolTreeItem *n) { return n->id() == itemId; });
    QTC_ASSERT(treeItem, return );
    treeItem->update(name, exe);
}

void ToolsModel::addMesonTool()
{
    manualGroup()->appendChild(new ToolTreeItem{uniqueName(tr("New Meson or Ninja tool"))});
}

void ToolsModel::removeMesonTool(ToolTreeItem *item)
{
    QTC_ASSERT(item,return);
    destroyItem(item);
    m_itemsToRemove.enqueue(item->id());
}

ToolTreeItem * ToolsModel::cloneMesonTool(ToolTreeItem *item)
{
    QTC_ASSERT(item,return nullptr);
    auto newItem = new ToolTreeItem(*item);
    manualGroup()->appendChild(newItem);
    return item;
}

void ToolsModel::apply()
{
    forItemsAtLevel<2>([](ToolTreeItem *item) {
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

void ToolsModel::addMesonTool(const MesonTools::Tool_t &tool)
{
    if (tool->autoDetected())
        autoDetectedGroup()->appendChild(new ToolTreeItem(tool));
    else
        manualGroup()->appendChild(new ToolTreeItem(tool));
}

QString ToolsModel::uniqueName(const QString &baseName)
{
    QStringList names;
    forItemsAtLevel<2>([&names](auto *item) { names << item->name(); });
    return Utils::makeUniquelyNumbered(baseName, names);
}

Utils::TreeItem *ToolsModel::autoDetectedGroup() const
{
    return rootItem()->childAt(0);
}

Utils::TreeItem *ToolsModel::manualGroup() const
{
    return rootItem()->childAt(1);
}
} // namespace Internal
} // namespace MesonProjectManager
