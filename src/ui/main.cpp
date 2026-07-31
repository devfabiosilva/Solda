#include <QApplication>
#include <QMainWindow>
#include <QLabel>
#include <QVBoxLayout>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // OBS style
    app.setStyle("Fusion");
    QPalette dark;
    dark.setColor(QPalette::Window,          QColor(30, 30, 30));
    dark.setColor(QPalette::WindowText,      QColor(220, 220, 220));
    dark.setColor(QPalette::Base,            QColor(20, 20, 20));
    dark.setColor(QPalette::AlternateBase,   QColor(40, 40, 40));
    dark.setColor(QPalette::Text,            QColor(220, 220, 220));
    dark.setColor(QPalette::Button,          QColor(50, 50, 50));
    dark.setColor(QPalette::ButtonText,      QColor(220, 220, 220));
    dark.setColor(QPalette::Highlight,       QColor(0, 120, 215));
    dark.setColor(QPalette::HighlightedText, QColor(255, 255, 255));
    app.setPalette(dark);

    QMainWindow window;
    window.setWindowTitle("Solda — Service Order Management");
    window.resize(1280, 720);

    QLabel *label = new QLabel("Solda MVP", &window);
    label->setAlignment(Qt::AlignCenter);
    label->setStyleSheet("font-size: 32px; color: #00BFFF;");
    window.setCentralWidget(label);

    window.show();
    return app.exec();
}


