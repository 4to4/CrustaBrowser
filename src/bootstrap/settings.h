#pragma once

#include <QObject>
#include <QSettings>

#define APPEARANCE QStringLiteral("appearance")
#define WEBENGINE QStringLiteral("webengine")

class Settings : public QSettings
{
public:
    explicit Settings(QObject *parent = nullptr);
};