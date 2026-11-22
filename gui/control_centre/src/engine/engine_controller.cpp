#include "engine/engine_controller.hpp"

#include "engine/engine_worker.hpp"

#include <QCoreApplication>

namespace reactorlab::ui {

EngineController::EngineController(QObject* parent) : QObject(parent) {
  qRegisterMetaType<ReactorSnapshot>("reactorlab::ui::ReactorSnapshot");
  qRegisterMetaType<QVector<double>>("QVector<double>");

  worker_ = new EngineWorker();
  worker_->moveToThread(&worker_thread_);

  connect(&worker_thread_, &QThread::finished, worker_, &QObject::deleteLater);
  connect(worker_, &EngineWorker::snapshotReady, this, &EngineController::onSnapshot,
          Qt::QueuedConnection);
  connect(worker_, &EngineWorker::trendsReady, this, &EngineController::onTrends,
          Qt::QueuedConnection);
  connect(worker_, &EngineWorker::exportFinished, this, &EngineController::onExportFinished,
          Qt::QueuedConnection);
  connect(&worker_thread_, &QThread::started, worker_, &EngineWorker::start);

  worker_thread_.start();
}

EngineController::~EngineController() {
  worker_thread_.quit();
  worker_thread_.wait(3000);
}

void EngineController::play() {
  QMetaObject::invokeMethod(worker_, "play", Qt::QueuedConnection);
}

void EngineController::pause() {
  QMetaObject::invokeMethod(worker_, "pause", Qt::QueuedConnection);
}

void EngineController::reset() {
  QMetaObject::invokeMethod(worker_, "reset", Qt::QueuedConnection);
}

void EngineController::scram() {
  QMetaObject::invokeMethod(worker_, "scram", Qt::QueuedConnection);
}

void EngineController::setRodZ(double z) {
  QMetaObject::invokeMethod(worker_, "setRodZ", Qt::QueuedConnection, Q_ARG(double, z));
}

void EngineController::setRhoExt(double rho) {
  QMetaObject::invokeMethod(worker_, "setRhoExt", Qt::QueuedConnection, Q_ARG(double, rho));
}

void EngineController::setSpeed(double speed) {
  QMetaObject::invokeMethod(worker_, "setSpeed", Qt::QueuedConnection, Q_ARG(double, speed));
}

void EngineController::setShieldCm(double cm) {
  QMetaObject::invokeMethod(worker_, "setShieldCm", Qt::QueuedConnection, Q_ARG(double, cm));
}

void EngineController::setScenario(const QString& id) {
  QMetaObject::invokeMethod(worker_, "setScenario", Qt::QueuedConnection, Q_ARG(QString, id));
}

void EngineController::acknowledgeAlarms() {
  QMetaObject::invokeMethod(worker_, "acknowledgeAlarms", Qt::QueuedConnection);
}

void EngineController::exportCsv(const QString& path) {
  QMetaObject::invokeMethod(worker_, "exportCsv", Qt::QueuedConnection, Q_ARG(QString, path));
}

QVariantList EngineController::scenarioIds() const {
  return {QStringLiteral("manual"), QStringLiteral("rod_step"), QStringLiteral("scram_xe"),
          QStringLiteral("doppler")};
}

QVariantList EngineController::scenarioLabels() const {
  return {QStringLiteral("Manual"), QStringLiteral("Rod step (power dip)"),
          QStringLiteral("Scram → Xe peak"), QStringLiteral("ρ step + Doppler")};
}

void EngineController::onExportFinished(bool ok, QString message) {
  export_status_ = ok ? message : QStringLiteral("Export failed: %1").arg(message);
  emit exportStatusChanged();
  emit snapshotChanged();
}

void EngineController::onSnapshot(ReactorSnapshot snap) {
  snap_ = std::move(snap);
  emit snapshotChanged();
}

void EngineController::onTrends(QVector<double> t,
                                QVector<double> P,
                                QVector<double> rho_pcm,
                                QVector<double> Tf,
                                QVector<double> Tc,
                                QVector<double> X) {
  trend_t_ = std::move(t);
  trend_P_ = std::move(P);
  trend_rho_ = std::move(rho_pcm);
  trend_Tf_ = std::move(Tf);
  trend_Tc_ = std::move(Tc);
  trend_X_ = std::move(X);
  emit trendsChanged();
}

QVariantList EngineController::toVariant(const QVector<double>& v) {
  QVariantList out;
  out.reserve(v.size());
  for (double x : v) {
    out.push_back(x);
  }
  return out;
}

QVariantList EngineController::toVariantStr(const QVector<QString>& v) {
  QVariantList out;
  out.reserve(v.size());
  for (const QString& x : v) {
    out.push_back(x);
  }
  return out;
}

QVariantList EngineController::spark(const QVector<double>& v) {
  constexpr int kSpark = 40;
  if (v.size() <= kSpark) {
    return toVariant(v);
  }
  QVariantList out;
  out.reserve(kSpark);
  const auto start = v.size() - kSpark;
  for (auto i = start; i < v.size(); ++i) {
    out.push_back(v[i]);
  }
  return out;
}

}  // namespace reactorlab::ui
