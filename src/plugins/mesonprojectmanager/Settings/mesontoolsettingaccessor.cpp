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
    setBaseFilePath(Utils::FilePath::fromString(Core::ICore::userResourcePath())
                        .pathAppended(Constants::Settings::FILENAME));
}

void MesonToolSettingAccessor::saveMesonTools(const std::vector<MesonTools::Tool_t> &tools,
                                              QWidget *parent)
{
    using namespace Constants;
    QVariantMap data;
    int entry_count = 0;
    std::for_each(std::cbegin(tools),
                  std::cend(tools),
                  [&data, &entry_count](const MesonTools::Tool_t &tool) {
                      auto asMeson = std::dynamic_pointer_cast<MesonWrapper>(tool);
                      if (asMeson)
                          data.insert(entryName(entry_count), toVariantMap<MesonWrapper>(*asMeson));
                      else {
                          auto asNinja = std::dynamic_pointer_cast<NinjaWrapper>(tool);
                          if (asNinja)
                              data.insert(entryName(entry_count),
                                          toVariantMap<NinjaWrapper>(*asNinja));
                      }
                      entry_count++;
                  });
    data.insert(Settings::ENTRY_COUNT, entry_count);
    saveSettings(data, parent);
}

std::vector<MesonTools::Tool_t> MesonToolSettingAccessor::loadMesonTools(QWidget *parent)
{
    using namespace Constants;
    auto data = restoreSettings(parent);
    auto entry_count = data.value(Settings::ENTRY_COUNT, 0).toInt();
    std::vector<MesonTools::Tool_t> result;
    for (auto toolIndex = 0; toolIndex < entry_count; toolIndex++) {
        auto name = entryName(toolIndex);
        if (data.contains(name)) {
            const auto map = data[name].toMap();
            auto type = map.value(Settings::TOOL_TYPE_KEY, Settings::TOOL_TYPE_MESON);
            if (type == Settings::TOOL_TYPE_NINJA)
                result.emplace_back(fromVariantMap<NinjaWrapper>(data[name].toMap()));
            else
                result.emplace_back(fromVariantMap<MesonWrapper>(data[name].toMap()));
        }
    }
    return result;
}
} // namespace Internal
} // namespace MesonProjectManager
