/**
 * NakumiOS Design Tokens Implementation
 */

#include "designtokens.h"

namespace NakumiOS {

DesignTokens::DesignTokens(QObject *parent)
    : QObject(parent)
{
}

DesignTokens *DesignTokens::create(QQmlEngine *qmlEngine, QJSEngine *jsEngine)
{
    Q_UNUSED(jsEngine)
    
    auto *tokens = new DesignTokens();
    
    // Set ownership to QML engine for proper memory management
    QQmlEngine::setObjectOwnership(tokens, QQmlEngine::JavaScriptOwnership);
    
    return tokens;
}

} // namespace NakumiOS
