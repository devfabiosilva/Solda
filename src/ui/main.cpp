#include <QApplication>
#include <QLocale>
#include <QTranslator>

#include "mainwindow.h"

extern "C" {
#include <db.h>
#include <db_log.h>
#include <db_service.h>
}

DB_SERVICE *g_dbService = nullptr;

int main(int argc, char *argv[]) {
  QApplication a(argc, argv);

  QTranslator translator;
  const QStringList uiLanguages = QLocale::system().uiLanguages();
  for (const QString &locale : uiLanguages) {
    const QString baseName = "qtsolda_" + QLocale(locale).name();
    if (translator.load(":/i18n/" + baseName)) {
      a.installTranslator(&translator);
      break;
    }
  }
  MainWindow w;
  w.show();
  int res = QApplication::exec();

  db_service_free(&g_dbService);

  return res;
}
