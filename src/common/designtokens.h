/**
 * NakumiOS Design Tokens
 * 
 * Centralized design system constants for consistent theming
 * across all NakumiOS components.
 * 
 * Theme: Deep Dark Mode
 */

#ifndef NAKUMIOS_DESIGNTOKENS_H
#define NAKUMIOS_DESIGNTOKENS_H

#include <QObject>
#include <QColor>
#include <QString>
#include <QFont>
#include <QQmlEngine>

namespace NakumiOS {

class DesignTokens : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    // Colors - Background
    Q_PROPERTY(QColor backgroundBase READ backgroundBase CONSTANT)
    Q_PROPERTY(QColor surfaceBase READ surfaceBase CONSTANT)
    Q_PROPERTY(QColor surfaceElevated READ surfaceElevated CONSTANT)
    
    // Colors - Accent
    Q_PROPERTY(QColor accentPrimary READ accentPrimary CONSTANT)
    Q_PROPERTY(QColor accentSecondary READ accentSecondary CONSTANT)
    Q_PROPERTY(QColor accentHover READ accentHover CONSTANT)
    Q_PROPERTY(QColor accentPressed READ accentPressed CONSTANT)
    
    // Colors - Text
    Q_PROPERTY(QColor textPrimary READ textPrimary CONSTANT)
    Q_PROPERTY(QColor textSecondary READ textSecondary CONSTANT)
    Q_PROPERTY(QColor textDisabled READ textDisabled CONSTANT)
    Q_PROPERTY(QColor textOnAccent READ textOnAccent CONSTANT)
    
    // Colors - Borders
    Q_PROPERTY(QColor borderDefault READ borderDefault CONSTANT)
    Q_PROPERTY(QColor borderFocused READ borderFocused CONSTANT)
    
    // Colors - Semantic
    Q_PROPERTY(QColor success READ success CONSTANT)
    Q_PROPERTY(QColor warning READ warning CONSTANT)
    Q_PROPERTY(QColor error READ error CONSTANT)
    Q_PROPERTY(QColor info READ info CONSTANT)
    
    // Dimensions
    Q_PROPERTY(int borderRadius READ borderRadius CONSTANT)
    Q_PROPERTY(int borderWidth READ borderWidth CONSTANT)
    Q_PROPERTY(int panelHeight READ panelHeight CONSTANT)
    Q_PROPERTY(int iconSize READ iconSize CONSTANT)
    Q_PROPERTY(int iconSizeLarge READ iconSizeLarge CONSTANT)
    Q_PROPERTY(int spacingSmall READ spacingSmall CONSTANT)
    Q_PROPERTY(int spacingMedium READ spacingMedium CONSTANT)
    Q_PROPERTY(int spacingLarge READ spacingLarge CONSTANT)
    
    // Effects
    Q_PROPERTY(qreal panelOpacity READ panelOpacity CONSTANT)
    Q_PROPERTY(int blurRadius READ blurRadius CONSTANT)
    Q_PROPERTY(int shadowRadius READ shadowRadius CONSTANT)
    
    // Animation
    Q_PROPERTY(int animationDurationFast READ animationDurationFast CONSTANT)
    Q_PROPERTY(int animationDurationNormal READ animationDurationNormal CONSTANT)
    Q_PROPERTY(int animationDurationSlow READ animationDurationSlow CONSTANT)
    
    // Typography
    Q_PROPERTY(QString fontFamily READ fontFamily CONSTANT)
    Q_PROPERTY(int fontSizeSmall READ fontSizeSmall CONSTANT)
    Q_PROPERTY(int fontSizeNormal READ fontSizeNormal CONSTANT)
    Q_PROPERTY(int fontSizeLarge READ fontSizeLarge CONSTANT)
    Q_PROPERTY(int fontSizeTitle READ fontSizeTitle CONSTANT)

public:
    explicit DesignTokens(QObject *parent = nullptr);
    ~DesignTokens() override = default;
    
    static DesignTokens *create(QQmlEngine *qmlEngine, QJSEngine *jsEngine);
    
    // Background colors
    QColor backgroundBase() const { return QColor(0x0F, 0x0F, 0x14); }
    QColor surfaceBase() const { return QColor(0x1A, 0x1A, 0x24); }
    QColor surfaceElevated() const { return QColor(0x24, 0x24, 0x30); }
    
    // Accent colors
    QColor accentPrimary() const { return QColor(0x6C, 0x5C, 0xE7); }
    QColor accentSecondary() const { return QColor(0x8B, 0x7B, 0xF7); }
    QColor accentHover() const { return QColor(0x7D, 0x6D, 0xF8); }
    QColor accentPressed() const { return QColor(0x5B, 0x4B, 0xD6); }
    
    // Text colors
    QColor textPrimary() const { return QColor(0xFF, 0xFF, 0xFF); }
    QColor textSecondary() const { return QColor(0xAA, 0xAA, 0xBB); }
    QColor textDisabled() const { return QColor(0x66, 0x66, 0x77); }
    QColor textOnAccent() const { return QColor(0xFF, 0xFF, 0xFF); }
    
    // Border colors
    QColor borderDefault() const { return QColor(0x2D, 0x2D, 0x3B); }
    QColor borderFocused() const { return accentPrimary(); }
    
    // Semantic colors
    QColor success() const { return QColor(0x00, 0xD9, 0x8B); }
    QColor warning() const { return QColor(0xFF, 0xA5, 0x02); }
    QColor error() const { return QColor(0xFF, 0x47, 0x57); }
    QColor info() const { return QColor(0x00, 0xB4, 0xD8); }
    
    // Dimensions
    int borderRadius() const { return 12; }
    int borderWidth() const { return 1; }
    int panelHeight() const { return 48; }
    int iconSize() const { return 24; }
    int iconSizeLarge() const { return 48; }
    int spacingSmall() const { return 8; }
    int spacingMedium() const { return 16; }
    int spacingLarge() const { return 24; }
    
    // Effects
    qreal panelOpacity() const { return 0.6; }
    int blurRadius() const { return 32; }
    int shadowRadius() const { return 16; }
    
    // Animation durations (ms)
    int animationDurationFast() const { return 150; }
    int animationDurationNormal() const { return 250; }
    int animationDurationSlow() const { return 400; }
    
    // Typography
    QString fontFamily() const { return QStringLiteral("Inter"); }
    int fontSizeSmall() const { return 11; }
    int fontSizeNormal() const { return 13; }
    int fontSizeLarge() const { return 16; }
    int fontSizeTitle() const { return 24; }
};

} // namespace NakumiOS

#endif // NAKUMIOS_DESIGNTOKENS_H
