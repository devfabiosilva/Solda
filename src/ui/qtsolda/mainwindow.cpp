#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QStandardItemModel>
#include <QJsonObject>

extern "C" {
    #include <db_service.h>
    #include <db_log.h>
}

extern DB_SERVICE *g_dbService;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->pushButton, &QPushButton::clicked, this, &MainWindow::on_loadDatabase_clicked_a);
}

MainWindow::~MainWindow()
{
    delete ui;
}

/*
void MainWindow::on_loadDatabase_pressed()
{
    int err;
    if (g_dbService == nullptr) {
        err = db_service_init(
            &g_dbService,
                const_cast<char*>(
                    "host=localhost "
                    "dbname=solda_mvp "
                    "user=fabio "
                    "password=1234 "
                )
            );
    } else
        err = 0;

    if (err == 0) {
        char *json = nullptr;
        size_t json_sz;

        err = db_service_load_technicians_json(&json, &json_sz, g_dbService, 25, 1);
        if (err == 0) {

            QJsonDocument doc = QJsonDocument::fromJson(QString::fromUtf8(json, (int)json_sz).toUtf8());

            DB_INFO("\nPrinting JSON in app %.*s", (int)json_sz, json)
            free(json);

            if (!doc.isArray()) {
                return;
            }

            //TODO implement
            return;
        }

        DB_ERROR("db_service_load_technicians_json error %d", err)
        return;
    }

    DB_ERROR("Unable to load db_service_init %d", err)
}
*/

void MainWindow::on_loadDatabase_clicked_a()
{
    int err;
    if (g_dbService == nullptr) {
        err = db_service_init(
            &g_dbService,
            const_cast<char*>(
                "host=localhost "
                "dbname=solda_mvp "
                "user=fabio "
                "password=1234 "
                )
            );
    } else
        err = 0;

    if (err == 0) {
        char *json = nullptr;
        size_t json_sz;

        err = db_service_load_technicians_json(&json, &json_sz, g_dbService, 25, 1);
        if (err == 0) {

            QJsonDocument doc = QJsonDocument::fromJson(QString::fromUtf8(json, (int)json_sz).toUtf8());

            //DB_INFO("\nPrinting JSON in app %.*s", (int)json_sz, json)
            free(json);

            if (!doc.isArray()) {
                DB_ERROR("Was expected JSON array")
                return;
            }

            QJsonArray techArray = doc.array();
            QStandardItemModel *model = new QStandardItemModel(techArray.size(), 5, this);

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

            ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); // Expande as colunas
            ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);             // Seleciona a linha inteira ao clicar
            ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
            return;
        }

        DB_ERROR("db_service_load_technicians_json error %d", err)
        return;
    }

    DB_ERROR("Unable to load db_service_init %d", err)
}


void MainWindow::on_loadDatabase_clicked()
{

}

