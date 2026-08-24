#include "mainwindow.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardItemModel>

#include "./ui_mainwindow.h"
#include "about.h"

extern "C" {
#include <db_log.h>
#include <db_service.h>
}

extern DB_SERVICE *g_dbService;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);
  connect(ui->loadDatabase, &QPushButton::clicked, this,
          &MainWindow::loadSoldaDatabase);
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::loadSoldaDatabase() {
  int err;
  if (g_dbService == nullptr) {
    // TODO remove. Testing Qt6 ...
    err = db_service_init(&g_dbService, const_cast<char *>("host=localhost "
                                                           "dbname=solda_mvp "
                                                           "user=fabio "
                                                           "password=1234 "));
  } else
    err = 0;

  if (err == 0) {
    char *json = nullptr;
    size_t json_sz;

    err = db_service_load_technicians_json(&json, &json_sz, g_dbService, 25, 1);
    if (err == 0) {
      QJsonDocument doc = QJsonDocument::fromJson(
          QString::fromUtf8(json, (int)json_sz).toUtf8());

      free(json);

      if (!doc.isArray()) {
        DB_ERROR("Was expected JSON array")
        return;
      }

      QJsonArray techArray = doc.array();
      QStandardItemModel *model =
          new QStandardItemModel(techArray.size(), 5, this);

      model->setHorizontalHeaderItem(0, new QStandardItem("ID"));
      model->setHorizontalHeaderItem(1, new QStandardItem("Nome"));
      model->setHorizontalHeaderItem(2, new QStandardItem("E-mail"));
      model->setHorizontalHeaderItem(3, new QStandardItem("Telefone"));
      model->setHorizontalHeaderItem(4, new QStandardItem("Regras (Rules)"));

      for (int i = 0; i < techArray.size(); ++i) {
        QJsonObject obj = techArray[i].toObject();

        QString id = QString::number(obj["id"].toInt());
        QString name = obj["name"].toString();
        QString email = obj["email"].toString();
        QString phone = obj["phone_number"].toString();
        QString rules = QString::number(obj["rules"].toInt());

        model->setItem(i, 0, new QStandardItem(id));
        model->setItem(i, 1, new QStandardItem(name));
        model->setItem(i, 2, new QStandardItem(email));
        model->setItem(i, 3, new QStandardItem(phone));
        model->setItem(i, 4, new QStandardItem(rules));
      }

      ui->tableView->setModel(model);

      ui->tableView->horizontalHeader()->setSectionResizeMode(
          QHeaderView::Stretch);  // Column stretch
      ui->tableView->setSelectionBehavior(
          QAbstractItemView::SelectRows);  // Select lines on click
      ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
      return;
    }

    DB_ERROR("db_service_load_technicians_json error %d", err)
    return;
  }

  DB_ERROR("Unable to load db_service_init %d", err)
}

// cppcheck-suppress unusedFunction
void MainWindow::on_action_About_Solda_triggered() {
  About about = About(this);
  about.exec();
}
