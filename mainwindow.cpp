#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QListWidgetItem>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    /* preset initial positions of the drones */
    const QVector<Vector2D> tabPos={{60,80},{400,700},{50,250},{800,800},{700,50}};

    int n=0;
    for (auto &pos:tabPos) {
        QListWidgetItem *LWitems=new QListWidgetItem(ui->listDronesInfo);
        ui->listDronesInfo->addItem(LWitems);
        QString name="Drone"+QString::number(++n);
        mapDrones[name]=new Drone(name);
        mapDrones[name]->setInitialPosition(pos);
        ui->listDronesInfo->setItemWidget(LWitems,mapDrones[name]);
    }

    ui->widget->setMap(&mapDrones);
    timer = new QTimer(this);
    timer->setInterval(100);
    connect(timer,SIGNAL(timeout()),this,SLOT(update()));
    timer->start();

    elapsedTimer.start();
}


MainWindow::~MainWindow() {
    delete ui;
    delete timer;
}

void MainWindow::on_actionQuit_triggered()
{
    QApplication::quit();
}

void MainWindow::update() {
    static int last=elapsedTimer.elapsed();
    static int steps=5;
    int current=elapsedTimer.elapsed();
    double dt=(current-last)/(1000.0*steps);
    for (int step=0; step<steps; step++) {
        // update positions of drones
        for (auto &drone:mapDrones) {
            // detect collisions between drone and other flying drones
            if (drone->getStatus()!=Drone::landed) {
                drone->initCollision();
                for (auto &obs:mapDrones) {
                    if (obs->getStatus()!=Drone::landed && obs->getName()!=drone->getName()) {
                        Vector2D B=obs->getPosition();
                        drone->addCollision(B,ui->widget->droneCollisionDistance);
                    }
                }
            }
            drone->update(dt);
        }
    }
    int d = elapsedTimer.elapsed()-current;
    ui->statusbar->showMessage("duree:"+QString::number(d)+" steps="+QString::number(steps));
    if (d>90) {
        steps/=2;
    } else {

        if (steps<10) steps++;
    }
    last=current;
    ui->widget->repaint();
}

Vector2D MainWindow::findServerPositionByName(const QString& serverName) {
    for (const auto& server : servers) {
        if (server.name == serverName) {
            return server.position;  // Retourne la position du serveur trouvé
        }
    }
    qWarning() << "Server with name" << serverName << "not found!";
    return Vector2D(0, 0);  // Si le serveur n'est pas trouvé, retourner (0, 0)
}

void MainWindow::on_actionLoad_triggered() {
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Config File"), "", tr("JSON Files (*.json)"));
    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning("Couldn't open config file.");
        return;
    }

    QByteArray jsonData = file.readAll();
    file.close();

    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData);
    if (!jsonDoc.isObject()) {
        qWarning("Invalid JSON format.");
        return;
    }

    QJsonObject rootObject = jsonDoc.object();

    // Effacer les anciens drones
    for (auto& drone : mapDrones) {
        delete drone;  // Nettoyage mémoire
    }
    mapDrones.clear();  // Effacer la carte des drones
    ui->listDronesInfo->clear();  // Effacer la liste des drones dans l'UI

    // Effacer les anciens serveurs
    servers.clear();
    QJsonArray serversArray = rootObject["servers"].toArray();
    for (const QJsonValue& serverValue : serversArray) {
        QJsonObject serverObject = serverValue.toObject();
        QString name = serverObject["name"].toString();
        QStringList positionStr = serverObject["position"].toString().split(",");
        int x = positionStr[0].toInt();
        int y = positionStr[1].toInt();
        QColor color(serverObject["color"].toString());

        Server server = {name, Vector2D(x, y), color};
        servers.append(server);
    }

    ui->widget->setServers(servers);  // Envoyer la liste des serveurs au Canvas

    // Charger les drones depuis le JSON
    QJsonArray dronesArray = rootObject["drones"].toArray();
    for (const QJsonValue& droneValue : dronesArray) {
        QJsonObject droneObject = droneValue.toObject();
        QString name = droneObject["name"].toString();
        QStringList positionStr = droneObject["position"].toString().split(",");
        int x = positionStr[0].toInt();
        int y = positionStr[1].toInt();
        QString targetServer = droneObject["server"].toString();  // Serveur cible du drone

        // Créer le drone et l'ajouter à la carte des drones
        Drone *newDrone = new Drone(name);
        newDrone->setInitialPosition(Vector2D(x, y));  // Position initiale du drone

        // Stocker le nom du serveur cible dans le drone
        newDrone->targetServer = targetServer;  // Stocker le serveur cible

        mapDrones[name] = newDrone;

        // Ajouter le drone à l'interface utilisateur
        QListWidgetItem *LWitems = new QListWidgetItem(ui->listDronesInfo);
        ui->listDronesInfo->addItem(LWitems);
        ui->listDronesInfo->setItemWidget(LWitems, newDrone);
    }

    ui->widget->setMap(&mapDrones);  // Envoyer la liste des drones au Canvas
    ui->widget->repaint();  // Rafraîchir le Canvas pour afficher les nouveaux serveurs et drones

    // Faire démarrer chaque drone après le chargement
    for (auto &drone : mapDrones) {
        // Utiliser le serveur cible stocké dans le drone
        Vector2D goalPosition = findServerPositionByName(drone->targetServer);  // Trouver la position du serveur cible

        // Vérifier la position cible avec qDebug
        qDebug() << "Drone" << drone->getName() << "moving to server" << drone->targetServer << "at position" << goalPosition.x << goalPosition.y;

        drone->setGoalPosition(goalPosition);  // Aller vers la position du serveur associé
        drone->start();  // Faire démarrer le drone
    }
}





