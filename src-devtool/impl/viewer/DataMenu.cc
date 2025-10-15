#include "DataMenu.h"

#include "internals/LandCacheViewer.h"

namespace devtool {

DataMenu::DataMenu() : IMenu("数据") { this->registerElement<internals::LandCacheViewer>(); }

} // namespace devtool