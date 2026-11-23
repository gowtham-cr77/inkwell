#include "ThemeManager.h"

namespace Inkwell {

ThemeManager::ThemeManager(QObject *parent)
    : QObject(parent)
    , m_isDark(false)
{
}

void ThemeManager::setIsDark(bool dark)
{
    if (m_isDark != dark) {
        m_isDark = dark;
        emit themeChanged();
    }
}

void ThemeManager::toggleTheme()
{
    setIsDark(!m_isDark);
}

QColor ThemeManager::background() const
{
    return m_isDark ? QColor("#1E1E1E") : QColor("#FAFAFA");
}

QColor ThemeManager::surface() const
{
    return m_isDark ? QColor("#2D2D2D") : QColor("#FFFFFF");
}

QColor ThemeManager::surfaceVariant() const
{
    return m_isDark ? QColor("#383838") : QColor("#F0F0F0");
}

QColor ThemeManager::primary() const
{
    return m_isDark ? QColor("#64B5F6") : QColor("#4A90D9");
}

QColor ThemeManager::onPrimary() const
{
    return m_isDark ? QColor("#1E1E1E") : QColor("#FFFFFF");
}

QColor ThemeManager::onSurface() const
{
    return m_isDark ? QColor("#E0E0E0") : QColor("#1A1A1A");
}

QColor ThemeManager::onSurfaceVariant() const
{
    return m_isDark ? QColor("#AAAAAA") : QColor("#666666");
}

QColor ThemeManager::outline() const
{
    return m_isDark ? QColor("#444444") : QColor("#DDDDDD");
}

QColor ThemeManager::accent() const
{
    return m_isDark ? QColor("#81C784") : QColor("#2ECC71");
}

QColor ThemeManager::error() const
{
    return m_isDark ? QColor("#EF5350") : QColor("#E74C3C");
}

QColor ThemeManager::toolbarBg() const
{
    return m_isDark ? QColor("#252525") : QColor("#FFFFFF");
}

} // namespace Inkwell
