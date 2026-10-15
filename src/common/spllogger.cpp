/**
 *  OSM
 *  Copyright (C) 2026  Pavel Smokotnin
 */
#include "spllogger.h"

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QLocale>
#include <QStandardPaths>
#include <QTextStream>
#include <QDebug>
#include <QUrl>

#include "model/metertablemodel.h"
#include "notifier.h"

SplLogger::SplLogger(QObject *parent)
    : QObject(parent),
      m_file(),
      m_directory(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation)),
      m_filePath(generatedFilePath())
{
    m_timer.setInterval(LoggingIntervalMs);
    connect(&m_timer, &QTimer::timeout, this, &SplLogger::writeSample);
}

bool SplLogger::running() const
{
    return m_timer.isActive();
}

QString SplLogger::filePath() const
{
    return m_filePath;
}

QString SplLogger::lastError() const
{
    return m_lastError;
}

QString SplLogger::generatedFilePath() const
{
    const QDateTime now = QDateTime::currentDateTime();
    const QString fileName = QStringLiteral("SPL_Log_%1_%2.csv")
                                 .arg(now.toString(QStringLiteral("yyyy-MM-dd")),
                                      now.toString(QStringLiteral("HH-mm-ss")));
    return QDir(m_directory).filePath(fileName);
}

void SplLogger::addModel(MeterTableModel *model)
{
    removeDestroyedModels();
    if (!model) {
        return;
    }

    for (const auto &registeredModel : m_models) {
        if (registeredModel == model) {
            return;
        }
    }

    if (running()) {
        stop();
    }
    m_models.append(QPointer<MeterTableModel>(model));
    connect(model, &QObject::destroyed, this, [this]() {
        removeDestroyedModels();
    });
}

void SplLogger::removeDestroyedModels()
{
    bool removed = false;
    for (auto it = m_models.begin(); it != m_models.end();) {
        if (it->isNull()) {
            it = m_models.erase(it);
            removed = true;
        } else {
            ++it;
        }
    }

    if (removed && running()) {
        stop();
    }
}

void SplLogger::removeModel(MeterTableModel *model)
{
    if (!model) {
        return;
    }

    bool removed = false;
    for (auto it = m_models.begin(); it != m_models.end();) {
        if (it->isNull() || it->data() == model) {
            it = m_models.erase(it);
            removed = true;
        } else {
            ++it;
        }
    }

    if (!removed) {
        return;
    }

    if (running()) {
        stop();
    }
}

void SplLogger::setDirectory(const QString &directory)
{
    if (directory.isEmpty()) {
        return;
    }

    QString localDirectory = directory;
    const QUrl url(directory);
    if (url.isLocalFile()) {
        localDirectory = url.toLocalFile();
    }

    const QString cleanDirectory = QDir::cleanPath(localDirectory);
    if (m_directory == cleanDirectory) {
        return;
    }

    if (running()) {
        stop();
    }

    m_directory = cleanDirectory;
    m_filePath = generatedFilePath();
    emit filePathChanged();
}

void SplLogger::start()
{
    if (running()) {
        return;
    }
    removeDestroyedModels();
    if (m_models.isEmpty()) {
        setError(QStringLiteral("The SPL meter model is not available."));
        return;
    }

    m_filePath = generatedFilePath();
    emit filePathChanged();

    const QFileInfo targetInfo(m_filePath);
    if (!QDir().mkpath(targetInfo.absolutePath())) {
        setError(QStringLiteral("Could not create the SPL log directory: %1").arg(targetInfo.absolutePath()));
        return;
    }

    m_file.setFileName(targetInfo.absoluteFilePath());
    if (!m_file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        setError(QStringLiteral("Could not open %1: %2").arg(targetInfo.absoluteFilePath(), m_file.errorString()));
        return;
    }

    QTextStream stream(&m_file);
    QStringList headers {QStringLiteral("timestamp")};
    for (const auto &registeredModel : m_models) {
        auto *model = registeredModel.data();
        for (auto *meter : model->meterPlots()) {
            headers.append(csvHeader(meter->sourceName() + QStringLiteral(" ") + meter->title()));
        }
    }
    stream << headers.join(QLatin1Char(',')) << Qt::endl;
    m_file.flush();
    m_lastError.clear();
    emit lastErrorChanged();
    m_timer.start();
    emit runningChanged();
    writeSample();
}

void SplLogger::stop()
{
    if (!running()) {
        return;
    }
    m_timer.stop();
    m_file.close();
    emit runningChanged();
}

void SplLogger::writeSample()
{
    if (m_models.isEmpty() || !m_file.isOpen()) {
        stop();
        return;
    }

    QTextStream stream(&m_file);
    QStringList values {QDateTime::currentDateTime().toString(Qt::ISODate)};
    for (const auto &registeredModel : m_models) {
        auto *model = registeredModel.data();
        for (auto *meter : model->meterPlots()) {
            bool ok = false;
            const QString displayedValue = meter->value();
            QString numericValue = displayedValue;
            if (numericValue.endsWith(QLatin1Char('%')) || numericValue.endsWith(QChar(0x2030))) {
                numericValue.chop(1);
            }
            const double value = QLocale::c().toDouble(numericValue, &ok);
            values.append(ok ? QLocale::c().toString(value, 'f', 3) : QString());
        }
    }
    stream << values.join(QLatin1Char(',')) << Qt::endl;
    if (stream.status() != QTextStream::Ok) {
        setError(QStringLiteral("Could not write to %1.").arg(m_filePath));
        stop();
    }
}

QString SplLogger::csvValue(const QString &value) const
{
    QString escaped = value;
    escaped.replace(QLatin1Char('"'), QStringLiteral("\"\""));
    return QStringLiteral("\"") + escaped + QStringLiteral("\"");
}

QString SplLogger::csvHeader(const QString &value) const
{
    return csvValue(value);
}

void SplLogger::setError(const QString &error)
{
    if (m_lastError == error) {
        return;
    }
    m_lastError = error;
    qWarning() << m_lastError;
    Notifier::getInstance()->newMessage(QStringLiteral("SPL logging"), m_lastError);
    emit lastErrorChanged();
}
