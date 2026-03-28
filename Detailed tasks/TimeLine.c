//Fonction pour construire TimeLine
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

//Fonction pour afficher le TimeLine
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

//Fonction pour supprimer le derniere publication
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



