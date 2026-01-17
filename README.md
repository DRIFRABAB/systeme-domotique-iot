# systeme-domotique-iot

# Vue d'ensemble
Ce projet propose une solution domotique complète intégrant :

- Surveillance en temps réel : Température, humidité, luminosité, détection de présence
- Automatisation intelligente : Contrôle automatique basé sur des règles prédéfinies
- Contrôle manuel : Interface web pour contrôler individuellement chaque dispositif
- Communication MQTT : Architecture distribuée et scalable

# Démonstration

 Capteurs          →  ESP32  →  MQTT Broker  →  Node-RED  →   Dashboard Web
 (DHT22, LDR, PIR)    WiFi      HiveMQ          Flows         Interface Utilisateur

# Fonctionnalités
***Mode Automatique***

- Gestion de la température : Ventilateur activé automatiquement si T > 25°C
- Gestion de l'éclairage : Lampe salon allumée si luminosité < 1500 lux
- Détection de présence : Activation automatique lampe + appareil lors de mouvement

***Mode Manuel***

- Contrôle individuel de chaque dispositif via boutons interactifs
- Interface accessible depuis n'importe quel appareil (PC, tablette, smartphone)
- Réactivité < 2 secondes entre commande et exécution

# Surveillance

- Jauges visuelles pour température (0-40°C) et luminosité (0-4095)
- Indicateur de présence en temps réel
- Mise à jour automatique toutes les 2 secondes
