#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_NOM 50
#define MAX_EMAIL 100
#define MAX_MSG 200
typedef struct NoeudListe {
    int id;
    struct NoeudListe* suivant;
} NoeudListe;

typedef struct Publication {
    int auteur_id;
    char texte[MAX_MSG];
    time_t timestamp;
    struct Publication* suivant;
} Publication;

typedef struct Utilisateur {
    int id;
    char nom[MAX_NOM];
    char email[MAX_EMAIL];
    NoeudListe* amis;
    NoeudListe* abonnements;
    Publication* publications;
} Utilisateur;
typedef struct NoeudAVL {
    Utilisateur utilisateur;
    struct NoeudAVL* gauche;
    struct NoeudAVL* droite;
    int hauteur;
} NoeudAVL;

int obtenirHauteur(NoeudAVL* n) {
    if (n == NULL) {
        return 0;
    }
    else {
        return n->hauteur;
    }
}

int max(int a, int b) {
    if (a > b) {
        return a;
    }
    else{
        return b;

    }
}

int obtenirBalance(NoeudAVL* n) {
    if (n == NULL) {
        return 0;
    }
    else{
        return obtenirHauteur(n->gauche) - obtenirHauteur(n->droite);
    }
}
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

int isInt(const char *id) {
    if (id[0] == '\0') return 0;

    for (int i = 0; id[i] != '\0'; i++) {
        if (id[i] < '0' || id[i] > '9')
            return 0;
    }

    return 1;
}

int idExiste(NoeudListe* liste, int id) {
    while (liste) {
        if (liste->id == id) return 1;
        liste = liste->suivant;
    }
    return 0;
}

void ajouterRelation(NoeudListe** liste, int id) {
    if (idExiste(*liste, id)) {
        printf("Relation deja existante.\n");
        return;
    }
    NoeudListe* nouveau = malloc(sizeof(NoeudListe));
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
void ajouterPublication(Utilisateur* u, const char* texte) {
    if (!u) return;
    Publication* pub = malloc(sizeof(Publication));
    pub->auteur_id = u->id;
    strncpy(pub->texte, texte, MAX_MSG - 1);
    pub->texte[MAX_MSG - 1] = '\0';
    pub->timestamp = time(NULL);
    pub->suivant = u->publications;
    u->publications = pub;
    printf("--- Publication ajoutee!\n");
}

Publication* copierPile(Publication* p) {
    if (!p) return NULL;
    Publication* copie = malloc(sizeof(Publication));
    *copie = *p;
    copie->suivant = copierPile(p->suivant);
    return copie;
}

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
    int index = 0;

    while (p) {
        char* temps = ctime(&p->timestamp);
        temps[strlen(temps) - 1] = '\0';
        printf("[%d] %s - %s\n", index, temps, p->texte);
        p = p->suivant;
        index++;
    }
}

NoeudAVL* creerUtilisateur(int id, const char* nom, const char* email) {
    NoeudAVL* n = malloc(sizeof(NoeudAVL));
    n->utilisateur.id = id;
    strncpy(n->utilisateur.nom, nom, MAX_NOM - 1);
    strncpy(n->utilisateur.email, email, MAX_EMAIL - 1);
    n->utilisateur.nom[MAX_NOM - 1] = '\0';
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
        printf("--- ID %d existe deja!\n", id);
        return racine;
    }

    racine->hauteur = 1 + max(obtenirHauteur(racine->gauche), obtenirHauteur(racine->droite));
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

NoeudAVL* rechercherUtilisateur(NoeudAVL* racine, int id) {
    if (!racine || racine->utilisateur.id == id)
        return racine;
    if (id < racine->utilisateur.id)
        return rechercherUtilisateur(racine->gauche, id);
    return rechercherUtilisateur(racine->droite, id);
}


Utilisateur* obtenirUtilisateurParId(NoeudAVL* racine, int id) {
    NoeudAVL* noeud = rechercherUtilisateur(racine, id);
    if (noeud) {
        return &noeud->utilisateur;
    }
    return NULL;
}

NoeudAVL* trouverMinimum(NoeudAVL* n) {
    while (n->gauche) n = n->gauche;
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

// TIMELINE
Publication* construireTimeline(NoeudAVL* racine, int userId) {
    NoeudAVL* user = rechercherUtilisateur(racine, userId);
    if (!user) return NULL;

    Publication* timeline = copierPile(user->utilisateur.publications);

    NoeudListe* relation = user->utilisateur.amis;
    while (relation) {
        NoeudAVL* ami = rechercherUtilisateur(racine, relation->id);
        if (ami) {
            timeline = fusionnerPiles(timeline, copierPile(ami->utilisateur.publications));
        }
        relation = relation->suivant;
    }

    relation = user->utilisateur.abonnements;
    while (relation) {
        NoeudAVL* abonne = rechercherUtilisateur(racine, relation->id);
        if (abonne) {
            timeline = fusionnerPiles(timeline, copierPile(abonne->utilisateur.publications));
        }
        relation = relation->suivant;
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
        char* temps = ctime(&timeline->timestamp);
        temps[strlen(temps) - 1] = '\0';
        printf("------------------------------------------\n");
        printf("- %s (ID: %d)\n", auteur ? auteur->utilisateur.nom : "Inconnu", timeline->auteur_id);
        printf("- %s\n", temps);
        printf("- %s\n", timeline->texte);
        printf("------------------------------------------\n\n");
        timeline = timeline->suivant;
    }
}

void supprimerDernierePublication(Utilisateur* u) {
    if (!u || !u->publications) {
        printf("--- Aucune publication a supprimer.\n");
        return;
    }

    Publication* temp = u->publications;
    u->publications = u->publications->suivant;
    free(temp);

    printf("--- Derniere publication supprimee.\n");
}

void suggererAmis(NoeudAVL* racine, int userId) {
    NoeudAVL* user = rechercherUtilisateur(racine, userId);
    if (!user) {
        printf("--- Utilisateur introuvable.\n");
        return;
    }

    printf("\n--- Suggestions d'amis pour %s:\n", user->utilisateur.nom);
    int trouve = 0;

    NoeudListe* ami = user->utilisateur.amis;
    while (ami) {
        NoeudAVL* amiNode = rechercherUtilisateur(racine, ami->id);
        if (amiNode) {
            NoeudListe* amiDami = amiNode->utilisateur.amis;
            while (amiDami) {
                if (amiDami->id != userId &&
                    !idExiste(user->utilisateur.amis, amiDami->id) &&
                    !idExiste(user->utilisateur.abonnements, amiDami->id)) {
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

    if (!trouve) {
        printf("   Aucune suggestion disponible.\n");
    }
}

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
        printf("--- Utilisateurs sauvegardes.\n");
    }

    f = fopen("relations.txt", "w");
    if (f) {
        sauvegarderRelations(racine, f);
        fclose(f);
        printf("--- Relations sauvegardees.\n");
    }
}

NoeudAVL* chargerUtilisateurs() {
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
    return racine;
}

void afficherUtilisateur(Utilisateur* u) {
    printf("\n========================================\n");
    printf("  Profil de %s\n", u->nom);
    printf("========================================\n");
    printf(" ID     : %d\n", u->id);
    printf(" Email  : %s\n", u->email);
}
void afficherAmisParNom(NoeudAVL* racine, int userId) {
    NoeudAVL* user = rechercherUtilisateur(racine, userId);
    if (!user) {
        printf("Utilisateur introuvable.\n");
        return;
    }

    if (!user->utilisateur.amis) {
        printf("Aucun ami.\n");
        return;
    }

    NoeudListe* ami = user->utilisateur.amis;
    int count = 0;

    while (ami) {
        NoeudAVL* amiNode = rechercherUtilisateur(racine, ami->id);
        if (amiNode) {
            printf("  %d. %s (ID: %d) - %s\n",
                   ++count,
                   amiNode->utilisateur.nom,
                   amiNode->utilisateur.id,
                   amiNode->utilisateur.email);
        } else {
            printf("  %d. ID %d (utilisateur introuvable)\n", ++count, ami->id);
        }
        ami = ami->suivant;
    }

    printf("--> Total : %d ami(s)\n", count);
}

void afficherAbonnementsParNom(NoeudAVL* racine, int userId) {
    NoeudAVL* user = rechercherUtilisateur(racine, userId);
    if (!user) {
        printf("Utilisateur introuvable.\n");
        return;
    }

    if (!user->utilisateur.abonnements) {
        printf("Aucun abonnement.\n");
        return;
    }

    NoeudListe* abonne = user->utilisateur.abonnements;
    int count = 0;

    while (abonne) {
        NoeudAVL* abonneNode = rechercherUtilisateur(racine, abonne->id);
        if (abonneNode) {
            printf("  %d. %s (ID: %d) - %s\n",
                   ++count,
                   abonneNode->utilisateur.nom,
                   abonneNode->utilisateur.id,
                   abonneNode->utilisateur.email);
        } else {
            printf("  %d. ID %d (utilisateur introuvable)\n", ++count, abonne->id);
        }
        abonne = abonne->suivant;
    }

    printf("--> Total : %d abonnement(s)\n", count);
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

void afficherMenu() {
    printf("\n===========================================\n");
    printf("        SYSTEME SNS - MENU PRINCIPAL      \n");
    printf("===========================================\n");
    printf(" 1.  Ajouter un utilisateur               \n");
    printf(" 2.  Rechercher un utilisateur            \n");
    printf(" 3.  Supprimer un utilisateur             \n");
    printf(" 4.  Afficher tous les utilisateurs       \n");
    printf(" 5.  Ajouter un ami                       \n");
    printf(" 6.  Supprimer un ami                     \n");
    printf(" 7.  Ajouter un abonnement                \n");
    printf(" 8.  Supprimer un abonnement              \n");
    printf(" 9.  Publier un message                   \n");
    printf(" 10. Voir timeline                        \n");
    printf(" 11. Suggestions d'amis                   \n");
    printf(" 12. Sauvegarder les donnees              \n");
    printf(" 13. Supprimer la derniere publication    \n");
    printf(" 14. Voir mes publications                \n");
    printf(" 0.  Quitter                              \n");
    printf("===========================================\n");
    printf("Votre choix : ");
}

int main() {
    NoeudAVL* racine = chargerUtilisateurs();
    int choix, id, cible;
    char nom[MAX_NOM], email[MAX_EMAIL], message[MAX_MSG],aux[20];

    printf("===========================================\n");
    printf("   BIENVENUE DANS LE SYSTEME SNS    \n");
    printf("===========================================\n");

    while (1) {
        afficherMenu();
        scanf("%d", &choix);
        getchar();

        switch (choix) {
            case 0:
                printf("\n--- Sauvegarde avant fermeture...\n");
                sauvegarderDonnees(racine);
                libererArbre(racine);
                printf("--- Au revoir!\n");
                return 0;

            case 1:
                printf("ID : ");
                scanf("%19s", aux);
                getchar();
                while ((isInt(aux))==0) {
                    printf("L'id est un entier\n");
                    printf("ID : ");
                    scanf("%19s", aux);
                    getchar();

                }
                id = atoi(aux);
                printf("Nom : ");
                fgets(nom, MAX_NOM, stdin);
                nom[strcspn(nom, "\n")] = '\0';
                printf("Email : ");
                fgets(email, MAX_EMAIL, stdin);
                email[strcspn(email, "\n")] = '\0';
                racine = insererUtilisateur(racine, id, nom, email);
                printf("--- Utilisateur ajoute.\n");
                break;

            case 2:
                printf("ID a rechercher: ");
                scanf("%19s", aux);
                getchar();
                while (!(isInt(aux))) {
                    printf("L'id est un entier\n");
                    printf("ID a rechercher: ");
                    scanf("%19s", aux);
                    getchar();

                }
                id = atoi(aux);
                Utilisateur* user = obtenirUtilisateurParId(racine, id);
                if (user) {
                    afficherUtilisateur(user);
                    printf("Amis: ");
                    afficherAmisParNom(racine, id);
                    printf("Abonnements: ");
                    afficherAbonnementsParNom(racine, id);
                }
                else
                    printf("--- Utilisateur introuvable.\n");
                break;

            case 3:
                printf("ID a supprimer: ");
                scanf("%19s", aux);
                getchar();
                while (!(isInt(aux))) {
                    printf("L'id est un entier\n");
                    printf("ID a supprimer: ");
                    scanf("%19s", aux);
                    getchar();

                }
                id = atoi(aux);
                racine = supprimerUtilisateur(racine, id);
                printf("--- Suppression effectuee.\n");
                break;

            case 4:
                printf("\n=== LISTE DES UTILISATEURS ===\n");
                afficherTousUtilisateurs(racine);
                break;

            case 5:
                printf("Votre ID : ");
                scanf("%19s", aux);
                getchar();
                while (!(isInt(aux))) {
                    printf("L'id est un entier\n");
                    printf("Votre ID : ");
                    scanf("%19s", aux);
                    getchar();

                }
                id = atoi(aux);
                printf("ID ami a ajouter: ");
                scanf("%19s", aux);
                getchar();
                while (!(isInt(aux))) {
                    printf("L'id est un entier\n");
                    printf("ID ami a ajouter: ");
                    scanf("%19s", aux);
                    getchar();

                }
                cible = atoi(aux);
                Utilisateur* u1 = obtenirUtilisateurParId(racine, id);
                Utilisateur* u22 = obtenirUtilisateurParId(racine, cible);
                if ((u1) && (u22)) {
                    ajouterRelation(&u1->amis, cible);
                    ajouterRelation(&u22->amis, id);
                    printf("--- Ami ajoute.\n");
                }
                break;

            case 6:
                printf("Votre ID : ");
                scanf("%19s", aux);
                getchar();
                while (!(isInt(aux))) {
                    printf("L'id est un entier\n");
                    printf("Votre ID : ");
                    scanf("%19s", aux);
                    getchar();

                }
                id = atoi(aux);
                printf("ID ami a supprimer: ");
                scanf("%19s", aux);
                getchar();
                while (!(isInt(aux))) {
                    printf("L'id est un entier\n");
                    printf("ID ami a supprimer: : ");
                    scanf("%19s", aux);
                    getchar();

                }
                cible = atoi(aux);
                Utilisateur* u2 = obtenirUtilisateurParId(racine, id);
                if (u2) {
                    supprimerRelation(&u2->amis, cible);
                    printf("--- Ami supprime.\n");
                }
                break;

            case 7:
                printf("Votre ID : ");
                scanf("%19s", aux);
                getchar();
                while (!(isInt(aux))) {
                    printf("L'id est un entier\n");
                    printf("Votre ID : ");
                    scanf("%19s", aux);
                    getchar();

                }
                id = atoi(aux);
                printf("ID a suivre : ");
                scanf("%19s", aux);
                getchar();
                while (!(isInt(aux))) {
                    printf("L'id est un entier\n");
                    printf("ID a suivre: : ");
                    scanf("%19s", aux);
                    getchar();

                }
                cible = atoi(aux);
                Utilisateur* u3 = obtenirUtilisateurParId(racine, id);
                if (u3) {
                    ajouterRelation(&u3->abonnements, cible);
                    printf("--- Abonnement ajoute.\n");
                }
                break;

            case 8:
                printf("Votre ID : ");
                scanf("%19s", aux);
                getchar();
                while (!(isInt(aux))) {
                    printf("L'id est un entier\n");
                    printf("Votre ID : ");
                    scanf("%19s", aux);
                    getchar();

                }
                id = atoi(aux);
                printf("ID a ne plus suivre: ");
                scanf("%19s", aux);
                getchar();
                while (!(isInt(aux))) {
                    printf("L'id est un entier\n");
                    printf("ID a ne plus suivre: ");
                    scanf("%19s", aux);
                    getchar();

                }
                cible = atoi(aux);
                Utilisateur* u4 = obtenirUtilisateurParId(racine, id);
                if (u4) {
                    supprimerRelation(&u4->abonnements, cible);
                    printf("--- Abonnement supprime.\n");
                }
                break;

            case 9:
                printf("Votre ID : ");
                scanf("%19s", aux);
                getchar();
                while (!(isInt(aux))) {
                    printf("L'id est un entier\n");
                    printf("Votre ID : ");
                    scanf("%19s", aux);
                    getchar();

                }
                id = atoi(aux);
                Utilisateur* u5 = obtenirUtilisateurParId(racine, id);
                if (u5) {
                    printf("Message : ");
                    fgets(message, MAX_MSG, stdin);
                    message[strcspn(message, "\n")] = '\0';
                    ajouterPublication(u5, message);
                }
                break;

            case 10:
                printf("ID utilisateur: ");
                scanf("%19s", aux);
                getchar();
                while (!(isInt(aux))) {
                    printf("L'id est un entier\n");
                    printf("ID utilisateur: ");
                    scanf("%19s", aux);
                    getchar();

                }
                id = atoi(aux);
                Publication* timeline = construireTimeline(racine, id);
                afficherTimeline(timeline, racine);
                libererPublications(timeline);
                break;

            case 11:
                printf("ID utilisateur: ");
                scanf("%19s", aux);
                getchar();
                while (!(isInt(aux))) {
                    printf("L'id est un entier\n");
                    printf("ID utilisateur: ");
                    scanf("%19s", aux);
                    getchar();

                }
                id = atoi(aux);
                suggererAmis(racine, id);
                break;

            case 12:
                sauvegarderDonnees(racine);
                break;

            case 13:
                printf("ID utilisateur: ");
                scanf("%19s", aux);
                getchar();
                while (!(isInt(aux))) {
                    printf("L'id est un entier\n");
                    printf("ID utilisateur: ");
                    scanf("%19s", aux);
                    getchar();

                }
                id = atoi(aux);
                Utilisateur* u6 = obtenirUtilisateurParId(racine, id);
                if (u6) {
                    supprimerDernierePublication(u6);
                } else {
                    printf("--- Utilisateur introuvable.\n");
                }
                break;

            case 14:
                printf("ID utilisateur: ");
                scanf("%19s", aux);
                getchar();
                while (!(isInt(aux))) {
                    printf("L'id est un entier\n");
                    printf("ID utilisateur: ");
                    scanf("%19s", aux);
                    getchar();

                }
                id = atoi(aux);
                Utilisateur* u7 = obtenirUtilisateurParId(racine, id);
                if (u7) {
                    afficherPublicationsUtilisateur(u7);
                } else {
                    printf("--- Utilisateur introuvable.\n");
                }
                break;

            default:
                printf("--- Choix invalide.\n");
        }
    }

    return 0;
}
