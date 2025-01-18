#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>
#include <QMap>
#include <QTimer>
#include <QElapsedTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFileDialog>
#include <QFile>
#include "drone.h"
#include "vector2d.h"
#include "server.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_actionQuit_triggered();
    void update();
    Vector2D findServerPositionByName(const QString& serverName);
    void on_actionLoad_triggered();  // Ajout de la méthode pour charger le fichier JSON

private:
    Ui::MainWindow *ui;
    QMap<QString, Drone*> mapDrones;
    QTimer *timer;
    QElapsedTimer elapsedTimer;
    QList<Server> servers;  // Liste pour stocker les serveurs après chargement
};

#endif // MAINWINDOW_H
