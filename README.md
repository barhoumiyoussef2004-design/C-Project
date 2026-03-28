# SNS - Système de Réseau Social en C

**Documentation complète du projet**

**Auteurs:** Youssef Barhoumi / Eya Azouzi / Zeineb Ayadi

---

## Introduction

Le projet **SNS (Social Network System)** est une implémentation complète d'un réseau social en langage C, intégrant une gestion avancée des utilisateurs, des relations sociales, des publications, et d'une timeline dynamique. Il constitue un travail démontrant la maîtrise des structures de données complexes, notamment les **arbres binaires (AVL)**, les **listes chaînées**, les **piles**, ainsi que la **gestion des fichiers**.

L'objectif du projet est de simuler les fonctionnalités essentielles d'une plateforme sociale moderne : **ajout d'utilisateurs, amitiés, abonnements, publications, affichage chronologique des flux**, et **sauvegarde des données**.

---

## Fonctionnalités Principales

### Gestion des Utilisateurs

Le système permet de :

- Ajouter un utilisateur avec un ID unique, un nom et un email
- Rechercher un utilisateur dans l'arbre binaire
- Supprimer un utilisateur, tout en libérant toute la mémoire associée
- Afficher des informations détaillées : nom, email, ID
- Afficher tous les utilisateurs triés automatiquement grâce à l'arbre binaire

### Amitiés et Abonnements

Le système gère deux types de relations sociales :

- Liste d'amis (relation réciproque)
- Liste d'abonnements (relation non réciproque)

Fonctionnalités :

- Ajouter ou supprimer un ami
- Suivre / ne plus suivre un utilisateur
- Afficher les amis et abonnements avec leur nom et email

### Publications

Chaque utilisateur peut :

- Publier un message (texte)
- Afficher toutes ses publications
- Supprimer sa dernière publication

Les publications sont stockées dans une pile, associée à un timestamp (`time_t`).

### Timeline (Flux)

La timeline d'un utilisateur est construite en fusionnant :

- Ses propres publications
- Celles de ses amis
- Celles des utilisateurs suivis

Caractéristiques :

- Tri chronologique décroissant (publications les plus récentes d'abord)

---

## Sauvegarde et Chargement des Données

Le système encode les données sous deux formats :

### utilisateurs.bin

- Stocke les utilisateurs en binaire
- Comprend : ID, nom, email
- Chargé automatiquement au lancement

### relations.txt

- Fichier texte lisible
- Contient pour chaque utilisateur :
  - amis
  - abonnements

### Sauvegarde manuelle ou automatique

À la fermeture (`choix 0`), tout est écrit sur disque.

---

## Architecture et Structures de Données

### Structures de Données Usuelles

#### Arbre binaire

L'arbre AVL est utilisé pour stocker les utilisateurs. Ses avantages :

- Insertion équilibrée
- Recherche et suppression rapides

Chaque nœud contient :

- Les données d'un utilisateur
- Un pointeur vers les enfants gauche et droit
- La hauteur du nœud

#### Exemple d'arbre binaire (AVL)

```
        [Utilisateur 52]
        /              \
[Utilisateur 21]    [Utilisateur 76]
```

#### Listes Chaînées

Utilisées pour :

- Gestion des amis
- Gestion des abonnements

Chaque élément contient un ID et un pointeur vers le prochain.

#### Piles

Utilisées pour :

- Gestion des publications

---

## Autres structures de Données non usuelles définies dans le programme

### Utilisateur

```
Utilisateur
├─ id
├─ nom
├─ email
├─ amis → Liste chaînée
├─ abonnements → Liste chaînée
└─ publications → Pile
```

---

## Menu complet du programme

1. Ajouter un utilisateur
2. Rechercher un utilisateur
3. Supprimer un utilisateur
4. Afficher tous les utilisateurs
5. Ajouter un ami
6. Supprimer un ami
7. Ajouter un abonnement
8. Supprimer un abonnement
9. Publier un message
10. Voir timeline
11. Suggestions d'amis
12. Sauvegarder les données
13. Supprimer la dernière publication
14. Voir mes publications
15. Quitter

---

## Exemple de Sortie Timeline

```
================ TIMELINE ================
------------------------------------------
- Alice (ID: 10)
- Tue Sep 10 14:12:54 2024
- Hello world!
------------------------------------------
```

---

## Gestion de la Mémoire

Chaque structure allouée dynamiquement est libérée :

- Listes d'amis
- Listes d'abonnements
- Publications
- Nœuds AVL

Des fonctions dédiées :

- `libererListe`
- `libererPublications`
- `libererArbre`

---

## Validation des Entrées

La fonction :

```c
int isInt(const char* id);
```

vérifie :

- chaîne non vide
- contient uniquement des chiffres

---

## Améliorations Potentielles

- Système d'authentification
- Implémentation d'une base de données et d'un SGBD
- Ajout des réactions (likes, commentaires)
- Pagination de la timeline

---
