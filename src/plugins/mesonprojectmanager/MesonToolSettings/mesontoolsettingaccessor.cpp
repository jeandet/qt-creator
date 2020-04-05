#include "mesontoolsettingaccessor.h"
#include "../mesonpluginconstants.h"
#include "app/app_version.h"
#include "coreplugin/icore.h"
#include "utils/fileutils.h"
#include <iterator>
#include <vector>
#include <QCoreApplication>
#include <QVariantMap>

namespace MesonProjectManager {
namespace Internal {
namespace {
inline QString entryName(int index)
{
    using namespace Constants;
    return QString("%1%2").arg(Settings::ENTRY_KEY).arg(index);
}
} // namespace

MesonToolSettingAccessor::MesonToolSettingAccessor()
    : UpgradingSettingsAccessor("QtCreatorMesonTools",
                                QCoreApplication::translate("MesonProjectManager::MesonToolManager",
                                                            "Meson"),
                                Core::Constants::IDE_DISPLAY_NAME)
{
    setBaseFilePath(Utils::FilePath::fromString(
        QString("%1/%2").arg(Core::ICore::userResourcePath()).arg(Constants::Settings::FILENAME)));
}

void MesonToolSettingAccessor::saveMesonTools(const std::vector<MesonWrapper> &tools,
                                              QWidget *parent)
{
    using namespace Constants;
    QVariantMap data;
    int entry_count = 0;
    std::for_each(std::cbegin(tools),
                  std::cend(tools),
                  [&data, &entry_count](const MesonWrapper &tool) {
                      data.insert(entryName(entry_count), toVariantMap(tool));
                      entry_count++;
                  });
    data.insert(Settings::ENTRY_COUNT, entry_count);
    saveSettings(data, parent);
}

std::vector<MesonWrapper> MesonToolSettingAccessor::loadMesonTools(QWidget *parent)
{
    using namespace Constants;
    auto data = restoreSettings(parent);
    auto entry_count = data.value(Settings::ENTRY_COUNT, 0).toInt();
    std::vector<MesonWrapper> result;
    for (auto toolIndex = 0; toolIndex < entry_count; toolIndex++) {
        auto name = entryName(toolIndex);
        if (data.contains(name)) {
            result.emplace_back(fromVariantMap(data[name].toMap()));
        }
    }
    return result;
}
} // namespace Internal
} // namespace MesonProjectManager
