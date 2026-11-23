#pragma once

#include <QObject>
#include <QColor>

namespace Inkwell {

class ThemeManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool isDark READ isDark WRITE setIsDark NOTIFY themeChanged)
    Q_PROPERTY(QColor background READ background NOTIFY themeChanged)
    Q_PROPERTY(QColor surface READ surface NOTIFY themeChanged)
    Q_PROPERTY(QColor surfaceVariant READ surfaceVariant NOTIFY themeChanged)
    Q_PROPERTY(QColor primary READ primary NOTIFY themeChanged)
    Q_PROPERTY(QColor onPrimary READ onPrimary NOTIFY themeChanged)
    Q_PROPERTY(QColor onSurface READ onSurface NOTIFY themeChanged)
    Q_PROPERTY(QColor onSurfaceVariant READ onSurfaceVariant NOTIFY themeChanged)
    Q_PROPERTY(QColor outline READ outline NOTIFY themeChanged)
    Q_PROPERTY(QColor accent READ accent NOTIFY themeChanged)
    Q_PROPERTY(QColor error READ error NOTIFY themeChanged)
    Q_PROPERTY(QColor toolbarBg READ toolbarBg NOTIFY themeChanged)
    
public:
    explicit ThemeManager(QObject *parent = nullptr);
    
    bool isDark() const { return m_isDark; }
    void setIsDark(bool dark);
    
    QColor background() const;
    QColor surface() const;
    QColor surfaceVariant() const;
    QColor primary() const;
    QColor onPrimary() const;
    QColor onSurface() const;
    QColor onSurfaceVariant() const;
    QColor outline() const;
    QColor accent() const;
    QColor error() const;
    QColor toolbarBg() const;
    
    Q_INVOKABLE void toggleTheme();

signals:
    void themeChanged();

private:
    bool m_isDark = false;
};

} // namespace Inkwell
