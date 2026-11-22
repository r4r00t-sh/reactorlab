#include "engine/engine_controller.hpp"

#include "reactorlab/version.hpp"

#include <QDir>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QUrl>

int main(int argc, char* argv[]) {
  QGuiApplication app(argc, argv);
  QGuiApplication::setApplicationName(QStringLiteral("ReactorLab Control Centre"));
  QGuiApplication::setOrganizationName(QStringLiteral("ReactorLab"));
  QGuiApplication::setApplicationVersion(QString::fromUtf8(
      reactorlab::version_string().data(),
      static_cast<int>(reactorlab::version_string().size())));

  QQuickStyle::setStyle(QStringLiteral("Basic"));

  reactorlab::ui::EngineController engine;

  QQmlApplicationEngine qml;

  // Allow overriding import root (dev tree vs install).
  const QByteArray importEnv = qgetenv("REACTORLAB_QML_IMPORTS");
  const QString importRoot = importEnv.isEmpty()
                                 ? QStringLiteral(REACTORLAB_CC_IMPORTS)
                                 : QString::fromLocal8Bit(importEnv);
  qml.addImportPath(importRoot);

  qml.rootContext()->setContextProperty(QStringLiteral("Engine"), &engine);
  qml.rootContext()->setContextProperty(
      QStringLiteral("AppVersion"),
      QString::fromUtf8(reactorlab::version_string().data(),
                        static_cast<int>(reactorlab::version_string().size())));

  const QByteArray mainEnv = qgetenv("REACTORLAB_QML_MAIN");
  const QString mainQml = mainEnv.isEmpty() ? QStringLiteral(REACTORLAB_CC_MAIN_QML)
                                            : QString::fromLocal8Bit(mainEnv);

  QObject::connect(
      &qml, &QQmlApplicationEngine::objectCreationFailed, &app,
      []() { QCoreApplication::exit(1); }, Qt::QueuedConnection);
  qml.load(QUrl::fromLocalFile(mainQml));
  if (qml.rootObjects().isEmpty()) {
    return 1;
  }
  return app.exec();
}
