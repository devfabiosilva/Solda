#include "about.h"

#include "ui_about.h"

About::About(QWidget *parent) : QDialog(parent), ui(new Ui::About) {
  ui->setupUi(this);

  adjustSize();
  setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
  // setFixedSize(640, 480);
  connect(ui->aboutOkButton, &QPushButton::clicked, this, &About::accept);
}

About::~About() { delete ui; }
