#include "canvas.h"
#include <QPainter>

Canvas::Canvas(QWidget *parent)
    : QWidget{parent} {
    droneImg.load("../../media/drone.png");
    setMouseTracking(true);
}


void Canvas::paintEvent(QPaintEvent *) {
    QPainter painter(this);

    // Vérifier si le fond (les cellules de Voronoï) a besoin d'être recalculé
    if (!backgroundUpdated) {
        // Créer l'image de fond si elle n'a pas été dessinée ou si la taille du widget change
        voronoiImage = QImage(width(), height(), QImage::Format_RGB32);
        voronoiImage.fill(Qt::white);  // Remplir l'image avec du blanc

        QPainter imagePainter(&voronoiImage);

        // Dessiner les cellules de Voronoï dans l'image de fond
        if (!servers.isEmpty()) {
            for (int x = 0; x < width(); ++x) {
                for (int y = 0; y < height(); ++y) {
                    // Trouver le serveur le plus proche
                    int nearestIndex = -1;
                    double minDistance = std::numeric_limits<double>::max();

                    Vector2D currentPoint(x, y);
                    for (int i = 0; i < servers.size(); ++i) {
                        double distance = (servers[i].position - currentPoint).length();
                        if (distance < minDistance) {
                            minDistance = distance;
                            nearestIndex = i;
                        }
                    }

                    // Remplir le pixel avec la couleur du serveur le plus proche
                    if (nearestIndex != -1) {
                        imagePainter.setPen(QPen(servers[nearestIndex].color));
                        imagePainter.drawPoint(x, y);
                    }
                }
            }

            // Dessiner les serveurs avec leurs noms dans l'image de fond
            QPen pointPen(Qt::black);
            imagePainter.setPen(pointPen);
            for (const auto& server : servers) {
                // Dessiner le point du serveur
                imagePainter.setBrush(server.color);
                imagePainter.drawEllipse(QPointF(server.position.x, server.position.y), 5, 5);

                // Dessiner le nom du serveur à côté du point
                imagePainter.setPen(Qt::black);
                imagePainter.drawText(QPointF(server.position.x + 10, server.position.y - 10), server.name);

                // Remettre le pinceau à la couleur du serveur pour le point
                imagePainter.setPen(pointPen);
            }
        }

        // Le fond est maintenant mis à jour
        backgroundUpdated = true;
    }

    // Dessiner l'image du fond (Voronoï + serveurs)
    painter.drawImage(0, 0, voronoiImage);

    // Dessiner les drones par-dessus les cellules de Voronoï
    if (mapDrones) {
        Vector2D p;
        QRect rect(-droneIconSize / 2, -droneIconSize / 2, droneIconSize, droneIconSize);
        QRect rectCol(-droneCollisionDistance / 2, -droneCollisionDistance / 2, droneCollisionDistance, droneCollisionDistance);

        // Déclaration et configuration de penCol pour le détecteur de collision
        QPen penCol(Qt::DashDotDotLine);
        penCol.setColor(Qt::lightGray);
        penCol.setWidth(3);

        for (auto &drone : *mapDrones) {
            painter.save();
            // Placer et orienter le drone
            painter.translate(drone->getPosition().x, drone->getPosition().y);
            painter.rotate(drone->getAzimut());
            painter.drawImage(rect, droneImg);
            // Allumer les leds si le drone vole
            if (drone->getStatus() != Drone::landed) {
                painter.setPen(Qt::NoPen);
                painter.setBrush(Qt::red);
                painter.drawEllipse((-185.0 / 511.0) * droneIconSize, (-185.0 / 511.0) * droneIconSize, (65.0 / 511.0) * droneIconSize, (65.0 / 511.0) * droneIconSize);
                painter.drawEllipse((115.0 / 511.0) * droneIconSize, (-185.0 / 511.0) * droneIconSize, (65.0 / 511.0) * droneIconSize, (65.0 / 511.0) * droneIconSize);
                painter.setBrush(Qt::green);
                painter.drawEllipse((-185.0 / 511.0) * droneIconSize, (115.0 / 511.0) * droneIconSize, (70.0 / 511.0) * droneIconSize, (70.0 / 511.0) * droneIconSize);
                painter.drawEllipse((115.0 / 511.0) * droneIconSize, (115.0 / 511.0) * droneIconSize, (70.0 / 511.0) * droneIconSize, (70.0 / 511.0) * droneIconSize);
            }
            // Dessiner le détecteur de collision
            if (drone->hasCollision()) {
                painter.setPen(penCol);
                painter.setBrush(Qt::NoBrush);
                painter.drawEllipse(rectCol);
            }
            painter.restore();
        }
    }
}


void Canvas::mousePressEvent(QMouseEvent *event) {
    // search for a drone that is landed
    auto it = mapDrones->begin();
    while (it!=mapDrones->end() && (*it)->getStatus()!=Drone::landed) {
        it++;
    }
    // if found, ask for a motion to the mouse position
    if (it!=mapDrones->end()) {
        (*it)->setGoalPosition(Vector2D(event->pos().x(),event->pos().y()));
        (*it)->start();
    }
    repaint();
}
