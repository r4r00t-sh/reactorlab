#include "mainwindow.hpp"
#include "reactorlab/version.hpp"

#include <QApplication>

int main(int argc, char* argv[]) {
  QApplication app(argc, argv);
  QApplication::setApplicationName("ReactorLab");
  QApplication::setApplicationVersion(QString::fromUtf8(
      reactorlab::version_string().data(), static_cast<int>(reactorlab::version_string().size())));
  QApplication::setOrganizationName("ReactorLab");

  MainWindow window;
  window.show();
  return app.exec();
}
