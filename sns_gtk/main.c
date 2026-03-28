/* main.c - GTK3 multi-window UI for SNS (each feature in its own dialog/window) */

#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include "sns.h"

/* Global root of SNS data */
static NoeudAVL *racine = NULL;

/* Utility: show message dialog */
static void show_message(GtkWindow *parent, const char *title, const char *msg) {
    GtkWidget *d = gtk_message_dialog_new(parent, GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "%s", msg);
    gtk_window_set_title(GTK_WINDOW(d), title);
    gtk_dialog_run(GTK_DIALOG(d));
    gtk_widget_destroy(d);
}

static void show_error(GtkWindow *parent, const char *title, const char *msg) {
    GtkWidget *d = gtk_message_dialog_new(parent, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "%s", msg);
    gtk_window_set_title(GTK_WINDOW(d), title);
    gtk_dialog_run(GTK_DIALOG(d));
    gtk_widget_destroy(d);
}

/* Ask for text */
static char *ask_for_text(GtkWindow *parent, const char *title, const char *label) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons(title, parent, GTK_DIALOG_MODAL,
                                                    "_OK", GTK_RESPONSE_OK, "_Cancel", GTK_RESPONSE_CANCEL, NULL);
    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    GtkWidget *lbl = gtk_label_new(label);
    GtkWidget *entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(box), lbl, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), entry, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(content), box);
    gtk_widget_show_all(dialog);

    char *res = NULL;
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
        const char *txt = gtk_entry_get_text(GTK_ENTRY(entry));
        if (txt && *txt) res = g_strdup(txt);
    }
    gtk_widget_destroy(dialog);
    return res;
}

/* Ask for integer */
static int ask_for_int(GtkWindow *parent, const char *title, const char *label) {
    char *s = ask_for_text(parent, title, label);
    if (!s) return -1;
    char *end;
    long v = strtol(s, &end, 10);
    g_free(s);
    if (*end != '\0') return -1;
    return (int)v;
}

/* Show text in a scrollable dialog (read-only) */
static void show_text_dialog(GtkWindow *parent, const char *title, const char *text) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons(title, parent, GTK_DIALOG_MODAL,
                                                    "_Close", GTK_RESPONSE_CLOSE, NULL);
    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *sc = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_size_request(sc, 700, 400);
    GtkWidget *tv = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(tv), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(tv), GTK_WRAP_WORD_CHAR);
    gtk_container_add(GTK_CONTAINER(sc), tv);
    gtk_box_pack_start(GTK_BOX(content), sc, TRUE, TRUE, 0);

    GtkTextBuffer *buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(tv));
    gtk_text_buffer_set_text(buf, text ? text : "(vide)", -1);

    gtk_widget_show_all(dialog);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

/* Build a string with all users (inorder traversal) */
static void build_all_users_string(NoeudAVL *node, GString *out) {
    if (!node) return;
    build_all_users_string(node->gauche, out);
    g_string_append_printf(out, "ID: %d | Nom: %s | Email: %s\n",
                           node->utilisateur.id, node->utilisateur.nom, node->utilisateur.email);
    build_all_users_string(node->droite, out);
}

/* Build profile string for a user (friends & subscriptions) */
static void build_user_profile_string(Utilisateur *u, GString *out) {
    if (!u) {
        g_string_append(out, "Utilisateur introuvable.\n");
        return;
    }
    g_string_append_printf(out, "Nom: %s\nID: %d\nEmail: %s\n\n", u->nom, u->id, u->email);
    g_string_append(out, "Amis: ");
    NoeudListe *l = u->amis;
    if (!l) g_string_append(out, "(aucun)");
    while (l) {
        g_string_append_printf(out, "%d ", l->id);
        l = l->suivant;
    }
    g_string_append(out, "\nAbonnements: ");
    l = u->abonnements;
    if (!l) g_string_append(out, "(aucun)");
    while (l) {
        g_string_append_printf(out, "%d ", l->id);
        l = l->suivant;
    }
    g_string_append(out, "\n\nPublications:\n");
    Publication *p = u->publications;
    if (!p) g_string_append(out, "(aucune)\n");
    while (p) {
        char tbuf[64];
        struct tm *tm = localtime(&p->timestamp);
        strftime(tbuf, sizeof(tbuf), "%Y-%m-%d %H:%M:%S", tm);
        g_string_append_printf(out, "- %s : %s\n", tbuf, p->texte);
        p = p->suivant;
    }
}

/* Build timeline string for userId using construireTimeline */
static void build_timeline_string(NoeudAVL *racine, int userId, GString *out) {
    Publication *timeline = construireTimeline(racine, userId);
    if (!timeline) {
        g_string_append(out, "Timeline vide.\n");
        return;
    }
    Publication *p = timeline;
    while (p) {
        NoeudAVL *author = rechercherUtilisateur(racine, p->auteur_id);
        char tbuf[64];
        struct tm *tm = localtime(&p->timestamp);
        strftime(tbuf, sizeof(tbuf), "%Y-%m-%d %H:%M:%S", tm);
        g_string_append_printf(out, "%s (ID: %d) — %s\n%s\n\n",
                               author ? author->utilisateur.nom : "Inconnu",
                               p->auteur_id, tbuf, p->texte);
        p = p->suivant;
    }
    libererPublications(timeline);
}

/* Build suggestions string (same logic as suggererAmis but returns text) */
static void build_suggestions_string(NoeudAVL *racine, int userId, GString *out) {
    NoeudAVL *userNode = rechercherUtilisateur(racine, userId);
    if (!userNode) {
        g_string_append(out, "Utilisateur introuvable.\n");
        return;
    }
    int found = 0;
    NoeudListe *ami = userNode->utilisateur.amis;
    while (ami) {
        NoeudAVL *amiNode = rechercherUtilisateur(racine, ami->id);
        if (amiNode) {
            NoeudListe *amiDami = amiNode->utilisateur.amis;
            while (amiDami) {
                int cand = amiDami->id;
                if (cand != userId &&
                    !idExiste(userNode->utilisateur.amis, cand) &&
                    !idExiste(userNode->utilisateur.abonnements, cand)) {
                    NoeudAVL *sugg = rechercherUtilisateur(racine, cand);
                    if (sugg) {
                        g_string_append_printf(out, "%s (ID: %d) — ami de %s\n",
                                               sugg->utilisateur.nom, sugg->utilisateur.id, amiNode->utilisateur.nom);
                        found = 1;
                    }
                }
                amiDami = amiDami->suivant;
            }
        }
        ami = ami->suivant;
    }
    if (!found) g_string_append(out, "Aucune suggestion disponible.\n");
}

/* ---------- UI callbacks (each opens a window/dialog) ---------- */

/* Add User */
static void on_add_user(GtkButton *b, gpointer data) {
    GtkWindow *parent = GTK_WINDOW(data);
    int id = ask_for_int(parent, "Ajouter utilisateur", "ID (entier) :");
    if (id < 0) return;
    char *nom = ask_for_text(parent, "Ajouter utilisateur", "Nom :");
    if (!nom) return;
    char *email = ask_for_text(parent, "Ajouter utilisateur", "Email :");
    if (!email) { g_free(nom); return; }
    racine = insererUtilisateur(racine, id, nom, email);
    g_free(nom); g_free(email);
    show_message(parent, "Utilisateur", "Utilisateur ajoute.");
}

/* Search user (profile dialog) */
static void on_search_user(GtkButton *b, gpointer data) {
    GtkWindow *parent = GTK_WINDOW(data);
    int id = ask_for_int(parent, "Rechercher utilisateur", "ID :");
    if (id < 0) return;
    Utilisateur *u = obtenirUtilisateurParId(racine, id);
    GString *out = g_string_new(NULL);
    build_user_profile_string(u, out);
    show_text_dialog(parent, "Profil utilisateur", out->str);
    g_string_free(out, TRUE);
}

/* Delete user */
static void on_delete_user(GtkButton *b, gpointer data) {
    GtkWindow *parent = GTK_WINDOW(data);
    int id = ask_for_int(parent, "Supprimer utilisateur", "ID :");
    if (id < 0) return;
    racine = supprimerUtilisateur(racine, id);
    show_message(parent, "Supprimer", "Suppression effectuee.");
}

/* List all users */
static void on_list_users(GtkButton *b, gpointer data) {
    GtkWindow *parent = GTK_WINDOW(data);
    GString *out = g_string_new(NULL);
    build_all_users_string(racine, out);
    if (out->len == 0) g_string_append(out, "(aucun utilisateur)\n");
    show_text_dialog(parent, "Liste des utilisateurs", out->str);
    g_string_free(out, TRUE);
}

/* Add friend */
static void on_add_friend(GtkButton *b, gpointer data) {
    GtkWindow *parent = GTK_WINDOW(data);
    int id = ask_for_int(parent, "Ajouter ami", "Votre ID :");
    if (id < 0) return;
    int cible = ask_for_int(parent, "Ajouter ami", "ID ami a ajouter :");
    if (cible < 0) return;
    Utilisateur *u = obtenirUtilisateurParId(racine, id);
    Utilisateur *v = obtenirUtilisateurParId(racine, cible);
    if (!u || !v) { show_error(parent, "Erreur", "Utilisateur introuvable."); return; }
    ajouterRelation(&u->amis, cible);
    ajouterRelation(&v->amis, id);
    show_message(parent, "Ami", "Ami ajoute.");
}

/* Remove friend */
static void on_remove_friend(GtkButton *b, gpointer data) {
    GtkWindow *parent = GTK_WINDOW(data);
    int id = ask_for_int(parent, "Supprimer ami", "Votre ID :");
    if (id < 0) return;
    int cible = ask_for_int(parent, "Supprimer ami", "ID ami a supprimer :");
    if (cible < 0) return;
    Utilisateur *u = obtenirUtilisateurParId(racine, id);
    if (!u) { show_error(parent, "Erreur", "Utilisateur introuvable."); return; }
    supprimerRelation(&u->amis, cible);
    show_message(parent, "Ami", "Ami supprime.");
}

/* Follow / Unfollow (subscriptions) */
static void on_follow(GtkButton *b, gpointer data) {
    GtkWindow *parent = GTK_WINDOW(data);
    int id = ask_for_int(parent, "Ajouter abonnement", "Votre ID :");
    if (id < 0) return;
    int cible = ask_for_int(parent, "Ajouter abonnement", "ID a suivre :");
    if (cible < 0) return;
    Utilisateur *u = obtenirUtilisateurParId(racine, id);
    if (!u) { show_error(parent, "Erreur", "Utilisateur introuvable."); return; }
    ajouterRelation(&u->abonnements, cible);
    show_message(parent, "Abonnement", "Abonnement ajoute.");
}

static void on_unfollow(GtkButton *b, gpointer data) {
    GtkWindow *parent = GTK_WINDOW(data);
    int id = ask_for_int(parent, "Supprimer abonnement", "Votre ID :");
    if (id < 0) return;
    int cible = ask_for_int(parent, "Supprimer abonnement", "ID a ne plus suivre :");
    if (cible < 0) return;
    Utilisateur *u = obtenirUtilisateurParId(racine, id);
    if (!u) { show_error(parent, "Erreur", "Utilisateur introuvable."); return; }
    supprimerRelation(&u->abonnements, cible);
    show_message(parent, "Abonnement", "Abonnement supprime.");
}

/* Publish */
static void on_publish(GtkButton *b, gpointer data) {
    GtkWindow *parent = GTK_WINDOW(data);
    int id = ask_for_int(parent, "Publier", "Votre ID :");
    if (id < 0) return;
    char *msg = ask_for_text(parent, "Publier", "Message:");
    if (!msg) return;
    Utilisateur *u = obtenirUtilisateurParId(racine, id);
    if (!u) { show_error(parent, "Erreur", "Utilisateur introuvable."); g_free(msg); return; }
    ajouterPublication(u, msg);
    g_free(msg);
    show_message(parent, "Publier", "Publication ajoutee.");
}

/* Delete last post */
static void on_delete_last_post(GtkButton *b, gpointer data) {
    GtkWindow *parent = GTK_WINDOW(data);
    int id = ask_for_int(parent, "Supprimer derniere publication", "ID utilisateur:");
    if (id < 0) return;
    Utilisateur *u = obtenirUtilisateurParId(racine, id);
    if (!u) { show_error(parent, "Erreur", "Utilisateur introuvable."); return; }
    supprimerDernierePublication(u);
    show_message(parent, "Publier", "Derniere publication supprimee.");
}

/* Show my posts */
static void on_show_my_posts(GtkButton *b, gpointer data) {
    GtkWindow *parent = GTK_WINDOW(data);
    int id = ask_for_int(parent, "Mes publications", "ID utilisateur:");
    if (id < 0) return;
    Utilisateur *u = obtenirUtilisateurParId(racine, id);
    GString *out = g_string_new(NULL);
    if (!u) g_string_append(out, "Utilisateur introuvable.\n");
    else build_user_profile_string(u, out);
    show_text_dialog(parent, "Mes publications & profil", out->str);
    g_string_free(out, TRUE);
}

/* Show timeline */
static void on_show_timeline(GtkButton *b, gpointer data) {
    GtkWindow *parent = GTK_WINDOW(data);
    int id = ask_for_int(parent, "Voir timeline", "ID utilisateur:");
    if (id < 0) return;
    GString *out = g_string_new(NULL);
    build_timeline_string(racine, id, out);
    show_text_dialog(parent, "Timeline", out->str);
    g_string_free(out, TRUE);
}

/* Show suggestions */
static void on_suggest(GtkButton *b, gpointer data) {
    GtkWindow *parent = GTK_WINDOW(data);
    int id = ask_for_int(parent, "Suggestions d'amis", "ID utilisateur:");
    if (id < 0) return;
    GString *out = g_string_new(NULL);
    build_suggestions_string(racine, id, out);
    show_text_dialog(parent, "Suggestions d'amis", out->str);
    g_string_free(out, TRUE);
}

/* Save & Load */
static void on_save(GtkButton *b, gpointer data) {
    sauvegarderDonnees(racine);
    show_message(GTK_WINDOW(data), "Sauvegarde", "Donnees sauvegardees.");
}

static void on_load(GtkButton *b, gpointer data) {
    libererArbre(racine);
    racine = chargerUtilisateurs();
    show_message(GTK_WINDOW(data), "Chargement", "Donnees chargees (si presentes).");
}

/* Build main window with buttons for each feature */
int main(int argc, char **argv) {
    gtk_init(&argc, &argv);

    /* Load data */
    racine = chargerUtilisateurs();

    GtkWidget *win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(win), "SNS - GTK3 Multi-window UI");
    gtk_window_set_default_size(GTK_WINDOW(win), 600, 400);
    g_signal_connect(win, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *v = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_container_set_border_width(GTK_CONTAINER(v), 10);
    gtk_container_add(GTK_CONTAINER(win), v);

    /* Buttons grouped by feature */
    GtkWidget *frame_users = gtk_frame_new("User Management");
    GtkWidget *box_users = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_container_add(GTK_CONTAINER(frame_users), box_users);
    GtkWidget *btn_add_user = gtk_button_new_with_label("Add User");
    GtkWidget *btn_search_user = gtk_button_new_with_label("Search User");
    GtkWidget *btn_delete_user = gtk_button_new_with_label("Delete User");
    GtkWidget *btn_list_users = gtk_button_new_with_label("List Users");
    gtk_box_pack_start(GTK_BOX(box_users), btn_add_user, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(box_users), btn_search_user, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(box_users), btn_delete_user, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(box_users), btn_list_users, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(v), frame_users, FALSE, FALSE, 0);

    GtkWidget *frame_friends = gtk_frame_new("Friends / Subscriptions");
    GtkWidget *box_friends = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_container_add(GTK_CONTAINER(frame_friends), box_friends);
    GtkWidget *btn_add_friend = gtk_button_new_with_label("Add Friend");
    GtkWidget *btn_remove_friend = gtk_button_new_with_label("Remove Friend");
    GtkWidget *btn_follow = gtk_button_new_with_label("Follow");
    GtkWidget *btn_unfollow = gtk_button_new_with_label("Unfollow");
    gtk_box_pack_start(GTK_BOX(box_friends), btn_add_friend, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(box_friends), btn_remove_friend, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(box_friends), btn_follow, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(box_friends), btn_unfollow, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(v), frame_friends, FALSE, FALSE, 0);

    GtkWidget *frame_posts = gtk_frame_new("Posts");
    GtkWidget *box_posts = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_container_add(GTK_CONTAINER(frame_posts), box_posts);
    GtkWidget *btn_publish = gtk_button_new_with_label("Publish");
    GtkWidget *btn_delete_last = gtk_button_new_with_label("Delete Last Post");
    GtkWidget *btn_show_my_posts = gtk_button_new_with_label("Show My Posts");
    gtk_box_pack_start(GTK_BOX(box_posts), btn_publish, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(box_posts), btn_delete_last, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(box_posts), btn_show_my_posts, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(v), frame_posts, FALSE, FALSE, 0);

    GtkWidget *frame_timeline = gtk_frame_new("Timeline & Suggestions");
    GtkWidget *box_timeline = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_container_add(GTK_CONTAINER(frame_timeline), box_timeline);
    GtkWidget *btn_show_timeline = gtk_button_new_with_label("Show Timeline");
    GtkWidget *btn_suggest = gtk_button_new_with_label("Suggest Friends");
    gtk_box_pack_start(GTK_BOX(box_timeline), btn_show_timeline, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(box_timeline), btn_suggest, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(v), frame_timeline, FALSE, FALSE, 0);

    GtkWidget *frame_io = gtk_frame_new("Save / Load");
    GtkWidget *box_io = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_container_add(GTK_CONTAINER(frame_io), box_io);
    GtkWidget *btn_save = gtk_button_new_with_label("Save Data");
    GtkWidget *btn_load = gtk_button_new_with_label("Load Data");
    gtk_box_pack_start(GTK_BOX(box_io), btn_save, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(box_io), btn_load, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(v), frame_io, FALSE, FALSE, 0);

    /* Connect signals: pass window as parent */
    g_signal_connect(btn_add_user, "clicked", G_CALLBACK(on_add_user), win);
    g_signal_connect(btn_search_user, "clicked", G_CALLBACK(on_search_user), win);
    g_signal_connect(btn_delete_user, "clicked", G_CALLBACK(on_delete_user), win);
    g_signal_connect(btn_list_users, "clicked", G_CALLBACK(on_list_users), win);

    g_signal_connect(btn_add_friend, "clicked", G_CALLBACK(on_add_friend), win);
    g_signal_connect(btn_remove_friend, "clicked", G_CALLBACK(on_remove_friend), win);
    g_signal_connect(btn_follow, "clicked", G_CALLBACK(on_follow), win);
    g_signal_connect(btn_unfollow, "clicked", G_CALLBACK(on_unfollow), win);

    g_signal_connect(btn_publish, "clicked", G_CALLBACK(on_publish), win);
    g_signal_connect(btn_delete_last, "clicked", G_CALLBACK(on_delete_last_post), win);
    g_signal_connect(btn_show_my_posts, "clicked", G_CALLBACK(on_show_my_posts), win);

    g_signal_connect(btn_show_timeline, "clicked", G_CALLBACK(on_show_timeline), win);
    g_signal_connect(btn_suggest, "clicked", G_CALLBACK(on_suggest), win);

    g_signal_connect(btn_save, "clicked", G_CALLBACK(on_save), win);
    g_signal_connect(btn_load, "clicked", G_CALLBACK(on_load), win);

    gtk_widget_show_all(win);
    gtk_main();

    /* On exit: save and free */
    sauvegarderDonnees(racine);
    libererArbre(racine);
    return 0;
}
