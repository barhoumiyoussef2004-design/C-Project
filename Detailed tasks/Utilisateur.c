//Fonction de structure de l'utilisateur
typedef struct Utilisateur {
   int id;
   char nom[MAX_NOM];
   char email[MAX_EMAIL];
   NoeudListe* amis;
   NoeudListe* abonnements;
   Publication* publications;
} Utilisateur;

//Procédure pour ajouter une relation 
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

//Procédure pour supprimer une relation
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

//Procédure pour afficher liste des utilisateurs 
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

//Procédure pour liberer pointeur de la liste
void libererListe(NoeudListe* liste) {
   while (liste) {
       NoeudListe* temp = liste;
       liste = liste->suivant;
       free(temp);
   }
}

//Fonction pour créer un utilisateur 
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

//Fonction pour inserer l'utilisateur 
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

//Fonction pour rechercher l'utilisateur 
NoeudAVL* rechercherUtilisateur(NoeudAVL* racine, int id) {
   if (!racine || racine->utilisateur.id == id)
       return racine;
   if (id < racine->utilisateur.id)
       return rechercherUtilisateur(racine->gauche, id);
   return rechercherUtilisateur(racine->droite, id);
}

//Fonction pour obtenir un utilisateur par id
Utilisateur* obtenirUtilisateurParId(NoeudAVL* racine, int id) {
   NoeudAVL* noeud = rechercherUtilisateur(racine, id);
   if (noeud) {
       return &noeud->utilisateur;
   }
   return NULL;
}

//Fonction pour supprimer l'utilisateur 
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




