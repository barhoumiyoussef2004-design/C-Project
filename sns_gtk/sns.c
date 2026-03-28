/* sns.c  - backend implementation (AVL, users, relations, publications, timeline, persistence)
   Matches sns.h and main.c provided here.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "sns.h"

int obtenirHauteur(NoeudAVL* n) {
    return n ? n->hauteur : 0;
}

int max(int a, int b) { return (a > b) ? a : b; }

int obtenirBalance(NoeudAVL* n) {
    if (!n) return 0;
    return obtenirHauteur(n->gauche) - obtenirHauteur(n->droite);
}

/* AVL rotations */
NoeudAVL* rotationDroite(NoeudAVL* y) {
    NoeudAVL* x = y->gauche;
    NoeudAVL* T2 = x->droite;
    x->droite = y;
    y->gauche = T2;
    y->hauteur = max(obtenirHauteur(y->gauche), obtenirHauteur(y->droite)) + 1;
    x->hauteur = max(obtenirHauteur(x->gauche), obtenirHauteur(x->droite)) + 1;
    return x;
}

NoeudAVL* rotationGauche(NoeudAVL* x) {
    NoeudAVL* y = x->droite;
    NoeudAVL* T2 = y->gauche;
    y->gauche = x;
    x->droite = T2;
    x->hauteur = max(obtenirHauteur(x->gauche), obtenirHauteur(x->droite)) + 1;
    y->hauteur = max(obtenirHauteur(y->gauche), obtenirHauteur(y->droite)) + 1;
    return y;
}

/* ---------- NoeudListe helpers ---------- */
int idExiste(NoeudListe* liste, int id) {
    while (liste) {
        if (liste->id == id) return 1;
        liste = liste->suivant;
    }
    return 0;
}

void ajouterRelation(NoeudListe** liste, int id) {
    if (idExiste(*liste, id)) {
        // already exists
        return;
    }
    NoeudListe* nouveau = malloc(sizeof(NoeudListe));
    if (!nouveau) return;
    nouveau->id = id;
    nouveau->suivant = *liste;
    *liste = nouveau;
}

void supprimerRelation(NoeudListe** liste, int id) {
    if (!*liste) return;
    if ((*liste)->id == id) {
        NoeudListe* temp = *liste;
        *liste = (*liste)->suivant;
        free(temp);
        return;
    }
    NoeudListe* courant = *liste;
    while (courant->suivant && courant->suivant->id != id) {
        courant = courant->suivant;
    }
    if (courant->suivant) {
        NoeudListe* temp = courant->suivant;
        courant->suivant = temp->suivant;
        free(temp);
    }
}

void afficherListe(NoeudListe* liste) {
    if (!liste) {
        printf("(vide)");
        return;
    }
    while (liste) {
        printf("%d ", liste->id);
        liste = liste->suivant;
    }
}

void libererListe(NoeudListe* liste) {
    while (liste) {
        NoeudListe* temp = liste;
        liste = liste->suivant;
        free(temp);
    }
}

/* ---------- Publications ---------- */
void ajouterPublication(Utilisateur* u, const char* texte) {
    if (!u) return;
    Publication* pub = malloc(sizeof(Publication));
    if (!pub) return;
    pub->auteur_id = u->id;
    strncpy(pub->texte, texte ? texte : "", MAX_MSG - 1);
    pub->texte[MAX_MSG - 1] = '\0';
    pub->timestamp = time(NULL);
    pub->suivant = u->publications;
    u->publications = pub;
}

Publication* copierPile(Publication* p) {
    if (!p) return NULL;
    Publication* copie = malloc(sizeof(Publication));
    if (!copie) return NULL;
    *copie = *p; /* shallow copy; fix links */
    copie->suivant = copierPile(p->suivant);
    return copie;
}

/* merge two sorted descending-by-timestamp linked lists */
Publication* fusionnerPiles(Publication* a, Publication* b) {
    if (!a) return b;
    if (!b) return a;
    if (a->timestamp >= b->timestamp) {
        a->suivant = fusionnerPiles(a->suivant, b);
        return a;
    } else {
        b->suivant = fusionnerPiles(a, b->suivant);
        return b;
    }
}

void libererPublications(Publication* p) {
    while (p) {
        Publication* temp = p;
        p = p->suivant;
        free(temp);
    }
}

void afficherPublicationsUtilisateur(Utilisateur* u) {
    if (!u || !u->publications) {
        printf("--- Aucune publication.\n");
        return;
    }
    printf("\n=== Publications de %s ===\n", u->nom);
    Publication* p = u->publications;
    while (p) {
        char tempsbuf[64];
        struct tm *tm = localtime(&p->timestamp);
        strftime(tempsbuf, sizeof(tempsbuf), "%Y-%m-%d %H:%M:%S", tm);
        printf("- %s : %s\n", tempsbuf, p->texte);
        p = p->suivant;
    }
}

/* ---------- AVL / user management ---------- */
NoeudAVL* creerUtilisateur(int id, const char* nom, const char* email) {
    NoeudAVL* n = malloc(sizeof(NoeudAVL));
    if (!n) return NULL;
    n->utilisateur.id = id;
    strncpy(n->utilisateur.nom, nom ? nom : "", MAX_NOM - 1);
    n->utilisateur.nom[MAX_NOM - 1] = '\0';
    strncpy(n->utilisateur.email, email ? email : "", MAX_EMAIL - 1);
    n->utilisateur.email[MAX_EMAIL - 1] = '\0';
    n->utilisateur.amis = NULL;
    n->utilisateur.abonnements = NULL;
    n->utilisateur.publications = NULL;
    n->gauche = n->droite = NULL;
    n->hauteur = 1;
    return n;
}

NoeudAVL* insererUtilisateur(NoeudAVL* racine, int id, const char* nom, const char* email) {
    if (!racine) return creerUtilisateur(id, nom, email);
    if (id < racine->utilisateur.id)
        racine->gauche = insererUtilisateur(racine->gauche, id, nom, email);
    else if (id > racine->utilisateur.id)
        racine->droite = insererUtilisateur(racine->droite, id, nom, email);
    else {
        /* id exists */
        return racine;
    }
    racine->hauteur = 1 + max(obtenirHauteur(racine->gauche), obtenirHauteur(racine->droite));
    int balance = obtenirBalance(racine);
    if (balance > 1 && id < racine->gauche->utilisateur.id)
        return rotationDroite(racine);
    if (balance < -1 && id > racine->droite->utilisateur.id)
        return rotationGauche(racine);
    if (balance > 1 && id > racine->gauche->utilisateur.id) {
        racine->gauche = rotationGauche(racine->gauche);
        return rotationDroite(racine);
    }
    if (balance < -1 && id < racine->droite->utilisateur.id) {
        racine->droite = rotationDroite(racine->droite);
        return rotationGauche(racine);
    }
    return racine;
}

NoeudAVL* rechercherUtilisateur(NoeudAVL* racine, int id) {
    if (!racine || racine->utilisateur.id == id)
        return racine;
    if (id < racine->utilisateur.id)
        return rechercherUtilisateur(racine->gauche, id);
    return rechercherUtilisateur(racine->droite, id);
}

Utilisateur* obtenirUtilisateurParId(NoeudAVL* racine, int id) {
    NoeudAVL* noeud = rechercherUtilisateur(racine, id);
    return noeud ? &noeud->utilisateur : NULL;
}

NoeudAVL* trouverMinimum(NoeudAVL* n) {
    while (n && n->gauche) n = n->gauche;
    return n;
}

NoeudAVL* supprimerUtilisateur(NoeudAVL* racine, int id) {
    if (!racine) return NULL;
    if (id < racine->utilisateur.id)
        racine->gauche = supprimerUtilisateur(racine->gauche, id);
    else if (id > racine->utilisateur.id)
        racine->droite = supprimerUtilisateur(racine->droite, id);
    else {
        if (!racine->gauche || !racine->droite) {
            NoeudAVL* temp = racine->gauche ? racine->gauche : racine->droite;
            libererListe(racine->utilisateur.amis);
            libererListe(racine->utilisateur.abonnements);
            libererPublications(racine->utilisateur.publications);
            free(racine);
            return temp;
        }
        NoeudAVL* temp = trouverMinimum(racine->droite);
        racine->utilisateur = temp->utilisateur;
        racine->droite = supprimerUtilisateur(racine->droite, temp->utilisateur.id);
    }
    if (!racine) return NULL;
    racine->hauteur = 1 + max(obtenirHauteur(racine->gauche), obtenirHauteur(racine->droite));
    int balance = obtenirBalance(racine);
    if (balance > 1 && obtenirBalance(racine->gauche) >= 0)
        return rotationDroite(racine);
    if (balance > 1 && obtenirBalance(racine->gauche) < 0) {
        racine->gauche = rotationGauche(racine->gauche);
        return rotationDroite(racine);
    }
    if (balance < -1 && obtenirBalance(racine->droite) <= 0)
        return rotationGauche(racine);
    if (balance < -1 && obtenirBalance(racine->droite) > 0) {
        racine->droite = rotationDroite(racine->droite);
        return rotationGauche(racine);
    }
    return racine;
}

/* ---------- Timeline ---------- */
Publication* construireTimeline(NoeudAVL* racine, int userId) {
    NoeudAVL* userNode = rechercherUtilisateur(racine, userId);
    if (!userNode) return NULL;

    Publication* timeline = copierPile(userNode->utilisateur.publications);

    NoeudListe* rel = userNode->utilisateur.amis;
    while (rel) {
        NoeudAVL* a = rechercherUtilisateur(racine, rel->id);
        if (a) {
            Publication* copy = copierPile(a->utilisateur.publications);
            timeline = fusionnerPiles(timeline, copy);
        }
        rel = rel->suivant;
    }

    rel = userNode->utilisateur.abonnements;
    while (rel) {
        NoeudAVL* a = rechercherUtilisateur(racine, rel->id);
        if (a) {
            Publication* copy = copierPile(a->utilisateur.publications);
            timeline = fusionnerPiles(timeline, copy);
        }
        rel = rel->suivant;
    }

    return timeline;
}

void afficherTimeline(Publication* timeline, NoeudAVL* racine) {
    if (!timeline) {
        printf("--- Timeline vide.\n");
        return;
    }
    printf("\n================ TIMELINE ================\n");
    while (timeline) {
        NoeudAVL* auteur = rechercherUtilisateur(racine, timeline->auteur_id);
        char tempsbuf[64];
        struct tm *tm = localtime(&timeline->timestamp);
        strftime(tempsbuf, sizeof(tempsbuf), "%Y-%m-%d %H:%M:%S", tm);
        printf("------------------------------------------\n");
        printf("- %s (ID: %d)\n", auteur ? auteur->utilisateur.nom : "Inconnu", timeline->auteur_id);
        printf("- %s\n", tempsbuf);
        printf("- %s\n", timeline->texte);
        printf("------------------------------------------\n\n");
        timeline = timeline->suivant;
    }
}

/* ---------- Suggestions ---------- */
void suggererAmis(NoeudAVL* racine, int userId) {
    NoeudAVL* userNode = rechercherUtilisateur(racine, userId);
    if (!userNode) {
        printf("--- Utilisateur introuvable.\n");
        return;
    }
    printf("\n--- Suggestions d'amis pour %s:\n", userNode->utilisateur.nom);
    int trouve = 0;
    NoeudListe* ami = userNode->utilisateur.amis;
    while (ami) {
        NoeudAVL* amiNode = rechercherUtilisateur(racine, ami->id);
        if (amiNode) {
            NoeudListe* amiDami = amiNode->utilisateur.amis;
            while (amiDami) {
                if (amiDami->id != userId &&
                    !idExiste(userNode->utilisateur.amis, amiDami->id) &&
                    !idExiste(userNode->utilisateur.abonnements, amiDami->id)) {
                    NoeudAVL* suggestion = rechercherUtilisateur(racine, amiDami->id);
                    if (suggestion) {
                        printf("   -> %s (ID: %d) - ami de %s\n",
                               suggestion->utilisateur.nom,
                               suggestion->utilisateur.id,
                               amiNode->utilisateur.nom);
                        trouve = 1;
                    }
                }
                amiDami = amiDami->suivant;
            }
        }
        ami = ami->suivant;
    }
    if (!trouve) printf("   Aucune suggestion disponible.\n");
}

/* ---------- Persistence (simple) ---------- */
/* Save users (binary) and relations (text) */
void sauvegarderRelations(NoeudAVL* racine, FILE* f) {
    if (!racine) return;
    sauvegarderRelations(racine->gauche, f);
    fprintf(f, "ID: %d\n", racine->utilisateur.id);
    fprintf(f, "amis: ");
    NoeudListe* l = racine->utilisateur.amis;
    while (l) {
        fprintf(f, "%d ", l->id);
        l = l->suivant;
    }
    fprintf(f, "\nabonnements: ");
    l = racine->utilisateur.abonnements;
    while (l) {
        fprintf(f, "%d ", l->id);
        l = l->suivant;
    }
    fprintf(f, "\n\n");
    sauvegarderRelations(racine->droite, f);
}

void sauvegarderUtilisateurs(NoeudAVL* racine, FILE* f) {
    if (!racine) return;
    sauvegarderUtilisateurs(racine->gauche, f);
    fwrite(&racine->utilisateur.id, sizeof(int), 1, f);
    fwrite(racine->utilisateur.nom, sizeof(char), MAX_NOM, f);
    fwrite(racine->utilisateur.email, sizeof(char), MAX_EMAIL, f);
    sauvegarderUtilisateurs(racine->droite, f);
}

void sauvegarderDonnees(NoeudAVL* racine) {
    FILE* f = fopen("utilisateurs.bin", "wb");
    if (f) {
        sauvegarderUtilisateurs(racine, f);
        fclose(f);
    }
    f = fopen("relations.txt", "w");
    if (f) {
        sauvegarderRelations(racine, f);
        fclose(f);
    }
}

NoeudAVL* chargerUtilisateurs(void) {
    FILE* f = fopen("utilisateurs.bin", "rb");
    if (!f) return NULL;
    NoeudAVL* racine = NULL;
    int id;
    char nom[MAX_NOM], email[MAX_EMAIL];
    while (fread(&id, sizeof(int), 1, f) == 1) {
        fread(nom, sizeof(char), MAX_NOM, f);
        fread(email, sizeof(char), MAX_EMAIL, f);
        racine = insererUtilisateur(racine, id, nom, email);
    }
    fclose(f);
    /* relations */
    FILE* r = fopen("relations.txt", "r");
    if (r) {
        char line[512];
        int currentId = -1;
        while (fgets(line, sizeof(line), r)) {
            if (sscanf(line, "ID: %d", &currentId) == 1) {
                char amisline[512], abline[512];
                if (fgets(amisline, sizeof(amisline), r) &&
                    fgets(abline, sizeof(abline), r)) {
                    char *p = strchr(amisline, ':');
                    if (p) {
                        p++;
                        char *tok = strtok(p, " \n");
                        while (tok) {
                            int aid = atoi(tok);
                            Utilisateur *u = obtenirUtilisateurParId(racine, currentId);
                            if (u) ajouterRelation(&u->amis, aid);
                            tok = strtok(NULL, " \n");
                        }
                    }
                    p = strchr(abline, ':');
                    if (p) {
                        p++;
                        char *tok = strtok(p, " \n");
                        while (tok) {
                            int aid = atoi(tok);
                            Utilisateur *u = obtenirUtilisateurParId(racine, currentId);
                            if (u) ajouterRelation(&u->abonnements, aid);
                            tok = strtok(NULL, " \n");
                        }
                    }
                }
            }
        }
        fclose(r);
    }
    return racine;
}

/* ---------- Display helpers ---------- */
void afficherUtilisateur(Utilisateur *u) {
    if (!u) {
        printf("Utilisateur introuvable.\n");
        return;
    }
    printf("\n========================================\n");
    printf("  Profil de %s\n", u->nom);
    printf("========================================\n");
    printf(" ID     : %d\n", u->id);
    printf(" Email  : %s\n", u->email);
}

void afficherTousUtilisateurs(NoeudAVL* racine) {
    if (!racine) return;
    afficherTousUtilisateurs(racine->gauche);
    printf("ID: %d | Nom: %s | Email: %s\n",
           racine->utilisateur.id,
           racine->utilisateur.nom,
           racine->utilisateur.email);
    afficherTousUtilisateurs(racine->droite);
}

void libererArbre(NoeudAVL* racine) {
    if (!racine) return;
    libererArbre(racine->gauche);
    libererArbre(racine->droite);
    libererListe(racine->utilisateur.amis);
    libererListe(racine->utilisateur.abonnements);
    libererPublications(racine->utilisateur.publications);
    free(racine);
}
void supprimerDernierePublication(Utilisateur *u)
{
    if (!u || !u->publications) {
        printf("--- Aucune publication à supprimer.\n");
        return;
    }

    Publication *temp = u->publications;
    u->publications = temp->suivant;
    free(temp);

    printf("--- Dernière publication supprimée.\n");
}
