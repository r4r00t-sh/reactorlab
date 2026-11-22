#include "engine/engine_worker.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSignalSpy>
#include <QTest>
#include <QThread>

#include <algorithm>
#include <cmath>

class EngineWorkerTest : public QObject {
  Q_OBJECT
private slots:
  void advancesWhenPlaying() {
    reactorlab::ui::EngineWorker worker;
    QSignalSpy spy(&worker, &reactorlab::ui::EngineWorker::snapshotReady);
    worker.start();
    QVERIFY(spy.wait(500));
    const auto first = spy.takeFirst().at(0).value<reactorlab::ui::ReactorSnapshot>();
    worker.play();
    // drain a few ticks
    QTest::qWait(300);
    QVERIFY(spy.count() >= 1);
    const auto last = spy.last().at(0).value<reactorlab::ui::ReactorSnapshot>();
    QVERIFY(last.t > first.t);
    QVERIFY(last.power_MW > 0.0);
  }

  void scramInsertsRods() {
    reactorlab::ui::EngineWorker worker;
    QSignalSpy spy(&worker, &reactorlab::ui::EngineWorker::snapshotReady);
    worker.start();
    QVERIFY(spy.wait(500));
    worker.scram();
    QTest::qWait(100);
    QVERIFY(spy.count() >= 1);
    const auto s = spy.last().at(0).value<reactorlab::ui::ReactorSnapshot>();
    QCOMPARE(s.rod_z, 1.0);
    QCOMPARE(s.state, QStringLiteral("Scram"));
  }

  void scenarioScramXeAndExport() {
    reactorlab::ui::EngineWorker worker;
    QSignalSpy spy(&worker, &reactorlab::ui::EngineWorker::snapshotReady);
    QSignalSpy exportSpy(&worker, &reactorlab::ui::EngineWorker::exportFinished);
    worker.start();
    QVERIFY(spy.wait(15000));
    worker.setScenario(QStringLiteral("scram_xe"));
    worker.play();
    // Wait until scenario inserts rods (t≥1 with substeps).
    bool rods = false;
    for (int i = 0; i < 40; ++i) {
      QTest::qWait(50);
      if (spy.isEmpty()) continue;
      const auto s = spy.last().at(0).value<reactorlab::ui::ReactorSnapshot>();
      if (s.t >= 1.0 && s.rod_z >= 0.999) {
        rods = true;
        QVERIFY(s.event_log.size() >= 1);
        break;
      }
    }
    QVERIFY2(rods, "scram_xe did not insert rods");

    const QString path = QDir::temp().filePath(QStringLiteral("reactorlab_ui7_test.csv"));
    worker.exportCsv(path);
    QVERIFY(exportSpy.count() >= 1 || exportSpy.wait(2000));
    QCOMPARE(exportSpy.last().at(0).toBool(), true);
    QVERIFY(QFileInfo::exists(path));
    QFile::remove(path);
  }

  void radiationSnapshotPresent() {
    reactorlab::ui::EngineWorker worker;
    QSignalSpy spy(&worker, &reactorlab::ui::EngineWorker::snapshotReady);
    worker.start();
    QVERIFY(spy.wait(15000));
    const auto s = spy.last().at(0).value<reactorlab::ui::ReactorSnapshot>();
    QVERIFY(s.rad_names.size() >= 3);
    QCOMPARE(s.rad_activity_Bq.size(), s.rad_names.size());
    QVERIFY(s.dose_r_m.size() >= 2);
    QCOMPARE(s.dose_gamma_uSvh.size(), s.dose_r_m.size());
    QVERIFY(s.rad_gamma_Bq > 0.0);
    worker.setShieldCm(0.0);
    QTest::qWait(80);
    const auto thin = spy.last().at(0).value<reactorlab::ui::ReactorSnapshot>();
    worker.setShieldCm(200.0);
    QTest::qWait(80);
    const auto thick = spy.last().at(0).value<reactorlab::ui::ReactorSnapshot>();
    QVERIFY(thin.dose_gamma_uSvh.size() > 0);
    QVERIFY(thick.dose_gamma_uSvh.size() > 0);
    QVERIFY(thin.dose_gamma_uSvh[0] >= thick.dose_gamma_uSvh[0]);
  }

  void heatmapFormFactorPresent() {
    reactorlab::ui::EngineWorker worker;
    QSignalSpy spy(&worker, &reactorlab::ui::EngineWorker::snapshotReady);
    worker.start();
    QVERIFY2(spy.wait(15000), "TWIGL form factor solve timed out");
    const auto s = spy.last().at(0).value<reactorlab::ui::ReactorSnapshot>();
    QVERIFY(s.map_nx > 0);
    QVERIFY(s.map_ny > 0);
    QCOMPARE(s.power_rel.size(), s.map_nx * s.map_ny);
    QCOMPARE(s.flux_rel.size(), s.map_nx * s.map_ny);
    QCOMPARE(s.tfuel_map.size(), s.map_nx * s.map_ny);
    QVERIFY(s.tcool_axial.size() >= 8);
    double mx = 0.0;
    for (double v : s.power_rel) {
      mx = std::max(mx, v);
    }
    QVERIFY(mx > 0.0);
  }
};

QTEST_MAIN(EngineWorkerTest)
#include "test_engine_worker.moc"
