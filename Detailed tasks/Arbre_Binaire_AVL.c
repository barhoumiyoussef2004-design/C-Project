#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//structure de NoeudListe
typedef struct NoeudListe {
   int id;
   struct NoeudListe* suivant;
} NoeudListe; 

//faire une structure pour utilisateur
typedef struct Utilisateur {
   int id;
   char nom[MAX_NOM];
   char email[MAX_EMAIL];
   NoeudListe* amis;
   NoeudListe* abonnements;
   Publication* publications;
} Utilisateur;

//faire une structure pour l’arbre (noeud)
typedef struct NoeudAVL {
   Utilisateur utilisateur;
   struct NoeudAVL* gauche;
   struct NoeudAVL* droite;
   int hauteur;
} NoeudAVL;

//fonction qui retourne l’hauteur de l’arbre
int obtenirHauteur(NoeudAVL* n) {
   if (n == NULL) {
       return 0;
   }
   else {
       return n->hauteur;
   }
}

//fonction qui retourne le maximum 
int max(int a, int b) {
   if (a > b) {
       return a;
   }
   else{
       return b;
   }
}

// créer un noeud contenant les informations de l’utilisateur
 struct Noeud* creer_Noeud(int id, char* nom, char* email) {
                               (Allouer la mémoire pour un nouveau noeud)
                               struct Noeud* nouveau_Noeud = (struct Noeud*)malloc(sizeof(struct Noeud));

                               nouveauNoeud->utilisateur.id = id;
                               strcpy(nouveau_Noeud->utilisateur.nom, nom);
                               strcpy(nouveau_Noeud->utilisateur.email, email);
                               nouveau_Noeud->gauche = NULL;
                               nouveau_Noeud->droite = NULL;
                               nouveau_Noeud->hauteur = 1;

                               return nouveau_Noeud;
                            }

//Faire une rotation à droite pour faire l'équilibre
 struct Noeud* rotationDroite(struct Noeud* y) {
                                  struct Noeud* x = y->gauche;
                                  struct Noeud* T2 = x->droite;
                                  x->droite =y
                                  y->gauche = T2;
                                  y->hauteur = max(obtenir_Hauteur(y->gauche), obtenir_Hauteur(y->droite)) + 1;
                                  x->hauteur = max(obtenir_Hauteur(x->gauche), obtenir_Hauteur(x->droite)) + 1;
                                  return x;
                      }

//Faire une rotation à gauche pour faire l'équilibre
struct Noeud* rotationGauche(struct Noeud* x) {
                                  struct Noeud* y = x->droite;
                                  struct Noeud* T2 = y->gauche;
                                  y->gauche = x;
                                  x->droite = T2;
                                  x->hauteur = max(obtenir_Hauteur(x->gauche), obtenir_Hauteur(x->droite)) + 1;
                                  y->hauteur = max(obtenir_Hauteur(y->gauche), obtenir_Hauteur(y->droite)) + 1;
                                  return y;
                       }

//Obtenir le facteur de l’équilibre pour un noeud
 int obtenir_Balance(struct Noeud* noeud) {
                           if (noeud == NULL) {
                                   return 0;
                           }
                           return obtenir_Hauteur(noeud->gauche) - obtenir_Hauteur(noeud->droite);
                       }

//Fonction principale : insérer un utilisateur dans l’arbre 
struct Noeud* inserer(struct Noeud* noeud, int id, char* nom, char* email) {
                 if (noeud == NULL) {
                         return creer_Noeud(id, nom, email);
                 }
                 if (id < noeud->utilisateur.id) {
                             noeud->gauche = inserer(noeud->gauche, id, nom, email);
                 }
                 else if (id > noeud->utilisateur.id) {
                             noeud->droite = inserer(noeud->droite, id, nom, email);
                 }
                 else {
                             printf("Erreur: L'utilisateur avec ID %d existe déjà!\n", id);
                             return noeud;

//Mettre à jour à l’hauteur de ce noeud
noeud->hauteur = 1 + max(obtenir_Hauteur(noeud->gauche), obtenir_Hauteur(noeud->droite));

//Obtenir le facteur d'équilibre pour vérifier si l'arbre est déséquilibré
int balance = obtenir_Balance(noeud);

//Faire équilibrer l’arbre
if (balance > 1 && id < noeud->gauche->utilisateur.id) {
                         return rotationDroite(noeud);
                     }
                     if (balance < -1 && id > noeud->droite->utilisateur.id) {
                         return rotationGauche(noeud);
                     }
                     if (balance > 1 && id > noeud->gauche->utilisateur.id) {
                         noeud->gauche = rotationGauche(noeud->gauche);
                         return rotationDroite(noeud);
                     }
                     if (balance < -1 && id < noeud->droite->utilisateur.id) {
                         noeud->droite = rotationDroite(noeud->droite);
                         return rotationGauche(noeud);
                     }
                     return noeud;
               }

//fonction principale de rechercher un utilisateur par son ID
struct Noeud* rechercher(struct Noeud* noeud, int id) {
                              if (noeud == NULL || noeud->utilisateur.id == id) {
                                       return noeud;
                               }
                              if (id < noeud->utilisateur.id) {
                                       return rechercher(noeud->gauche, id);
                               }
                               return rechercher(noeud->droite, id);
                           }

//Fonction qui détermine l'hauteur de chaque noeud 
obtenirHauteur(racine->droite));
   int balance = obtenirBalance(racine);
   // Rotation gauche-gauche
   if (balance > 1 && id < racine->gauche->utilisateur.id)
       return rotationDroite(racine);
   // Rotation droite-droite
   if (balance < -1 && id > racine->droite->utilisateur.id)
       return rotationGauche(racine);
   // Rotation gauche-droite
   if (balance > 1 && id > racine->gauche->utilisateur.id) {
       racine->gauche = rotationGauche(racine->gauche);
       return rotationDroite(racine);
   }
   // Rotation droite-gauche
   if (balance < -1 && id < racine->droite->utilisateur.id) {
       racine->droite = rotationDroite(racine->droite);
       return rotationGauche(racine);
   }
   return racine;
}

