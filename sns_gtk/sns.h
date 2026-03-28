#ifndef SNS_H
#define SNS_H

#include <time.h>

#define MAX_NOM 50
#define MAX_EMAIL 100
#define MAX_MSG 200

/* Linked list node for relations (friends/subscriptions) */
typedef struct NoeudListe {
    int id;
    struct NoeudListe* suivant;
} NoeudListe;

/* Publication (linked list stack style) */
typedef struct Publication {
    int auteur_id;
    char texte[MAX_MSG];
    time_t timestamp;
    struct Publication* suivant;
} Publication;

/* User structure */
typedef struct Utilisateur {
    int id;
    char nom[MAX_NOM];
    char email[MAX_EMAIL];
    NoeudListe* amis;
    NoeudListe* abonnements;
    Publication* publications;
} Utilisateur;

/* AVL tree node to store Utilisateur */
typedef struct NoeudAVL {
    Utilisateur utilisateur;
    struct NoeudAVL* gauche;
    struct NoeudAVL* droite;
    int hauteur;
} NoeudAVL;

/* ---------- Prototypes (backend API) ---------- */
/* AVL / user management */
NoeudAVL* insererUtilisateur(NoeudAVL* racine, int id, const char* nom, const char* email);
Utilisateur* obtenirUtilisateurParId(NoeudAVL* racine, int id);
NoeudAVL* rechercherUtilisateur(NoeudAVL* racine, int id);   // <-- ADD THIS LINE
NoeudAVL* supprimerUtilisateur(NoeudAVL* racine, int id);
void afficherTousUtilisateurs(NoeudAVL* racine);
void libererArbre(NoeudAVL* racine);


/* relations (friends / subscriptions) */
int idExiste(NoeudListe* liste, int id);
void ajouterRelation(NoeudListe** liste, int id);
void supprimerRelation(NoeudListe** liste, int id);
void afficherListe(NoeudListe* liste);
void libererListe(NoeudListe* liste);

/* publications */
void ajouterPublication(Utilisateur* u, const char* texte);
void supprimerDernierePublication(Utilisateur* u);
void afficherPublicationsUtilisateur(Utilisateur* u);
Publication* copierPile(Publication* p);
Publication* fusionnerPiles(Publication* a, Publication* b);
void libererPublications(Publication* p);

/* timeline & suggestions */
Publication* construireTimeline(NoeudAVL* racine, int userId);
void afficherTimeline(Publication* timeline, NoeudAVL* racine);
void suggererAmis(NoeudAVL* racine, int userId);

/* persistence */
void sauvegarderDonnees(NoeudAVL* racine);
NoeudAVL* chargerUtilisateurs(void);

#endif /* SNS_H */
