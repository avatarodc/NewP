#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <termios.h>
#include <ctype.h>
#define MAX_CLASSES 3
#define MAX_APPRENANTS 300
#define MAX_LENGTH 100
#define MAX_LINE_LENGTH 100 // Longueur maximale d'une ligne dans le fichier
#define FILENAME "etudiant.txt"
#define COUNTER_FILENAME "nombre_apprenants.txt"
//-------------------------------------------
#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>

char getch()
{
    struct termios oldt, newt;
    char ch;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
}
#endif

#define LONGUEUR_MAX_LOGIN 50
#define LONGUEUR_MAX_MDP 50
#define MAX_STUDENTS_PER_CLASS 50
#define MAX_DATES 100
#define MAX_DAYS 7
#define MAX_STRING_LENGTH 20
typedef struct
{
    int jour;
    int mois;
    int annee;
} Date;

typedef struct
{
    char login[LONGUEUR_MAX_LOGIN];
    char motDePasse[LONGUEUR_MAX_MDP];
} Identifiants;

typedef struct
{
    char matricule[10];
    char motdepasse[10];
    char prenom[20];
    char nom[20];
    char classe[10];
    int statut;
    int nombre_absences;
    int nombre_retards;
} Apprenant;


typedef struct
{
    int nbhr;
    int nba;
} Quota;


typedef struct
{
    int id;
    int status;
    char matricule[10];
    char dateHeur[20];
    char contenu[200];
} Message;

Identifiants identifiantsAdmin;
int nombreIdentifiantsAdmin = 1;

// Fonction pour bloquer un apprenant en fonction des quotas
void bloquerApprenants() {
    FILE *quotaFile = fopen("quota.txt", "r");
    if (quotaFile == NULL) {
        printf("Erreur lors de l'ouverture du fichier quota.txt\n");
        return;
    }

    Quota quotas;
    fscanf(quotaFile, "nbhr %d\n", &quotas.nbhr);
    fscanf(quotaFile, "nba %d\n", &quotas.nba);
    fclose(quotaFile);

    FILE *apprenantFile = fopen("etudiant.txt", "r");
    if (apprenantFile == NULL) {
        printf("Erreur lors de l'ouverture du fichier apprenants.txt\n");
        return;
    }

    Apprenant apprenants[MAX_APPRENANTS];
    int nbApprenants = 0;

    // Lecture des apprenants depuis le fichier
    while (fscanf(apprenantFile, "%s %s %s %s %s %d %d %d",
                  apprenants[nbApprenants].matricule,
                  apprenants[nbApprenants].motdepasse,
                  apprenants[nbApprenants].prenom,
                  apprenants[nbApprenants].nom,
                  apprenants[nbApprenants].classe,
                  &apprenants[nbApprenants].statut,
                  &apprenants[nbApprenants].nombre_absences,
                  &apprenants[nbApprenants].nombre_retards) != EOF) {
        nbApprenants++;
    }

    fclose(apprenantFile);

    // Bloquer les apprenants en fonction des quotas
    for (int i = 0; i < nbApprenants; i++) {
        if (apprenants[i].nombre_absences > quotas.nba || apprenants[i].nombre_retards > quotas.nbhr) {
            apprenants[i].statut = 0; // Bloquer l'apprenant
        }
    }

    // Écriture des apprenants bloqués dans un nouveau fichier
    FILE *blockedApprenantsFile = fopen("blocked_apprenants.txt", "w");
    if (blockedApprenantsFile == NULL) {
        printf("Erreur lors de l'ouverture du fichier blocked_apprenants.txt\n");
        return;
    }

    for (int i = 0; i < nbApprenants; i++) {
        fprintf(blockedApprenantsFile, "%s %s %s %s %s %d %d %d\n",
                apprenants[i].matricule,
                apprenants[i].motdepasse,
                apprenants[i].prenom,
                apprenants[i].nom,
                apprenants[i].classe,
                apprenants[i].statut,
                apprenants[i].nombre_absences,
                apprenants[i].nombre_retards);
    }

    fclose(blockedApprenantsFile);
    
    printf("Les apprenants ont été bloqués selon les quotas spécifiés.\n");
}


/* void generer_statistiques(Apprenant apprenants[], int num_apprenants)
{
    FILE *statistiques_file = fopen("statistiques.txt", "w");
    if (statistiques_file == NULL)
    {
        perror("Erreur lors de l'ouverture du fichier statistiques.txt");
        exit(EXIT_FAILURE);
    }
    fprintf(statistiques_file, "+------------+----------------------------------+\n");
    fprintf(statistiques_file, "| Matricule  | Présences / Absences sur 7 jours|\n");
    fprintf(statistiques_file, "+------------+----------------------------------+\n");
    for (int i = 0; i < num_apprenants; i++)
    {
        int presences = compter_presences(apprenants[i].matricule);
        fprintf(statistiques_file, "| %-10s |", apprenants[i].matricule);
        // Affichage des présences
        for (int j = 0; j < presences && j < MAX_DAYS; j++)
        {
            fprintf(statistiques_file, "*");
        }
        // Affichage des absences
        for (int j = presences; j < MAX_DAYS; j++)
        {
            fprintf(statistiques_file, ".");
        }
        fprintf(statistiques_file, " |\n");
    }
    fprintf(statistiques_file, "+------------+----------------------------------+\n");
    fclose(statistiques_file);
}
 */
int compter_presences(char *matricule, char dates[][20], int num_dates)
    {
        FILE *presence_file = fopen("presence.txt", "r");
        if (presence_file == NULL)
        {
            perror("Erreur lors de l'ouverture du fichier presence.txt");
            exit(EXIT_FAILURE);
        }
        int presences = 0;
        char matricule_presence[10], date[20], heure[10];
        while (fscanf(presence_file, "%s %s %s", matricule_presence, date, heure) == 3)
        {
            if (strcmp(matricule_presence, matricule) == 0)
            {
                presences++;
            }
        }
        fclose(presence_file);
        return num_dates - presences;
    }

void charger_etudiants_et_compter_absences(Apprenant apprenants[], int *num_apprenants)
{
    FILE *etudiant_file = fopen("etudiant.txt", "r");
    if (etudiant_file == NULL)
    {
        perror("Erreur lors de l'ouverture du fichier etudiant.txt");
        exit(EXIT_FAILURE);
    }
    int num_dates = 0;
    char dates[MAX_DATES][20];
    char matricule_presence[10], date[20], heure[10];
    while (fscanf(etudiant_file, "%s %s %s %s %s %d", apprenants[*num_apprenants].matricule,
                  apprenants[*num_apprenants].motdepasse,
                  apprenants[*num_apprenants].prenom,
                  apprenants[*num_apprenants].nom,
                  apprenants[*num_apprenants].classe,
                  &apprenants[*num_apprenants].statut) == 6)
    {
        FILE *presence_file = fopen("presence.txt", "r");
        if (presence_file == NULL)
        {
            perror("Erreur lors de l'ouverture du fichier presence.txt");
            exit(EXIT_FAILURE);
        }
        while (fscanf(presence_file, "%s %s %s", matricule_presence, date, heure) == 3)
        {
            int found = 0;
            for (int i = 0; i < num_dates; i++)
            {
                if (strcmp(dates[i], date) == 0)
                {
                    found = 1;
                    break;
                }
            }
            if (!found)
            {
                strcpy(dates[num_dates], date);
                num_dates++;
            }
        }
        fclose(presence_file);
        int absences = compter_presences(apprenants[*num_apprenants].matricule, dates, num_dates);
        printf("Matricule  %s : %d\n", apprenants[*num_apprenants].matricule, absences);
        (*num_apprenants)++;
    }
    fclose(etudiant_file);
}

int calculer_retard(char *heure_presence)
{
    int heure, minute;
    sscanf(heure_presence, "%d:%d", &heure, &minute);
    if ((heure == 8 && minute >= 15) || (heure > 8 && heure < 16))
    {
        return (heure - 8) * 60 + minute - 15;
    }
    return 0;
}

void charger_etudiants(Apprenant apprenants[], int *num_apprenants)
{
    FILE *etudiant_file = fopen("etudiant.txt", "r");
    if (etudiant_file == NULL)
    {
        perror("Erreur lors de l'ouverture du fichier etudiant.txt");
        exit(EXIT_FAILURE);
    }
    while (fscanf(etudiant_file, "%s %s %s %s %s %d", apprenants[*num_apprenants].matricule,
                  apprenants[*num_apprenants].motdepasse,
                  apprenants[*num_apprenants].prenom,
                  apprenants[*num_apprenants].nom,
                  apprenants[*num_apprenants].classe,
                  &apprenants[*num_apprenants].statut) == 6)
    {
        (*num_apprenants)++;
    }
    fclose(etudiant_file);
}

void lire_presence(Apprenant apprenants[], int num_apprenants)
{
    FILE *presence_file = fopen("presence.txt", "r");
    if (presence_file == NULL)
    {
        perror("Erreur lors de l'ouverture du fichier presence.txt");
        exit(EXIT_FAILURE);
    }
    char matricule[10], date[20], heure[10];
    while (fscanf(presence_file, "%s %s %s", matricule, date, heure) == 3)
    {
        int retard = calculer_retard(heure);
        for (int i = 0; i < num_apprenants; i++)
        {
            if (strcmp(apprenants[i].matricule, matricule) == 0)
            {
                apprenants[i].statut += retard;
                break;
            }
        }
    }
    fclose(presence_file);
}
void afficher_retards(Apprenant apprenants[], int num_apprenants) {
    FILE *fichier = fopen("retards.txt", "w");
    if (fichier == NULL) {
        printf("Erreur lors de l'ouverture du fichier pour écrire.\n");
        return;
    }

    fprintf(fichier, "+------------+----------------------+-------------+\n");
    fprintf(fichier, "| Matricule  | Nom Complet          | Retard (min)|\n");
    fprintf(fichier, "+------------+----------------------+-------------+\n");
    for (int i = 0; i < num_apprenants; i++) {
        fprintf(fichier, "| %-10s | %-20s | %-11d|\n", apprenants[i].matricule,
                strcat(apprenants[i].nom, apprenants[i].prenom), apprenants[i].statut);
    }
    fprintf(fichier, "+------------+----------------------+-------------+\n");

    fclose(fichier);
    printf("Les retards ont été enregistrés dans le fichier 'retards.txt'.\n");
}


// Fonction pour écrire le nombre d'apprenants dans un fichier
void ecrireNombreApprenants(int nombreApprenants)
{
    FILE *fichier = fopen(COUNTER_FILENAME, "w");
    if (fichier == NULL)
    {
        printf("Erreur lors de l'ouverture du fichier.\n");
        return;
    }

    fprintf(fichier, "%d", nombreApprenants);

    fclose(fichier);
}

// Fonction pour lire le nombre d'apprenants à partir d'un fichier
int lireNombreApprenants()
{
    FILE *fichier = fopen(COUNTER_FILENAME, "r");
    if (fichier == NULL)
    {
        return 0; // Retourne 0 si le fichier n'existe pas ou s'il y a une erreur lors de la lecture
    }

    int nombreApprenants;
    fscanf(fichier, "%d", &nombreApprenants);

    fclose(fichier);

    return nombreApprenants;
}

// Fonction pour créer un nouvel apprenant
void creerApprenant()
{
    Apprenant nouvelApprenant;

    // Ouverture du fichier en mode ajout
    FILE *fichier = fopen(FILENAME, "a");
    if (fichier == NULL)
    {
        printf("Erreur lors de l'ouverture du fichier.\n");
        return;
    }

    // Saisie du mot de passe (à remplacer par un mécanisme sécurisé)
    printf("Saisir le mot de passe pour l'apprenant: ");
    scanf("%9s", nouvelApprenant.motdepasse);

    // Saisie du prénom, nom et classe
    printf("Saisir le prénom de l'apprenant: ");
    scanf("%19s", nouvelApprenant.prenom);
    printf("Saisir le nom de l'apprenant: ");
    scanf("%19s", nouvelApprenant.nom);
    printf("Saisir la classe de l'apprenant: ");
    scanf("%9s", nouvelApprenant.classe);

    // Récupération du nombre d'apprenants déjà présents
    int nombreApprenants = lireNombreApprenants();

    // Calcul du matricule en fonction du nombre d'apprenants déjà présents dans le fichier
    snprintf(nouvelApprenant.matricule, sizeof(nouvelApprenant.matricule), "mat%d", nombreApprenants + 1);

    // Saisie de la donnée supplémentaire (toujours 0 dans cet exemple)
    nouvelApprenant.statut = 0;

    // Écriture des informations de l'apprenant dans le fichier
    fprintf(fichier, "%s %s %s %s %s %d\n", nouvelApprenant.matricule, nouvelApprenant.motdepasse,
            nouvelApprenant.prenom, nouvelApprenant.nom, nouvelApprenant.classe, nouvelApprenant.statut);

    // Fermeture du fichier
    fclose(fichier);

    // Mettre à jour le nombre d'apprenants dans le fichier
    ecrireNombreApprenants(nombreApprenants + 1);

    printf("L'apprenant a été ajouté avec succès dans le fichier %s.\n", FILENAME);
}

// Fonction pour modifier un apprenant
void modifierApprenant(char *matricule)
{
    // Ouvrir le fichier en mode lecture et écriture
    FILE *fichier = fopen("etudiant.txt", "r+");
    if (fichier == NULL)
    {
        printf("Erreur lors de l'ouverture du fichier.\n");
        return;
    }

    // Rechercher l'apprenant par son matricule
    Apprenant apprenant;
    int trouve = 0;
    while (fscanf(fichier, "%s %s %s %s %s %d", apprenant.matricule, apprenant.motdepasse,
                  apprenant.prenom, apprenant.nom, apprenant.classe, &apprenant.statut) == 6)
    {
        if (strcmp(apprenant.matricule, matricule) == 0)
        {
            trouve = 1;
            break;
        }
    }

    if (!trouve)
    {
        printf("Aucun apprenant trouvé avec le matricule fourni.\n");
        fclose(fichier);
        return;
    }

    // Afficher les informations actuelles de l'apprenant
    printf("Informations actuelles de l'apprenant:\n");
    printf("Matricule: %s\n", apprenant.matricule);
    printf("Mot de passe: %s\n", apprenant.motdepasse);
    printf("Prénom: %s\n", apprenant.prenom);
    printf("Nom: %s\n", apprenant.nom);
    printf("Classe: %s\n", apprenant.classe);
    printf("Statut: %d\n", apprenant.statut);
    char input[MAX_STRING_LENGTH];
    viderTampon();
    // Saisir les nouvelles informations (à compléter selon vos besoins)
   printf("Saisir le nouveau mot de passe pour l'apprenant (appuyez sur Entrée pour conserver l'ancienne valeur): ");
    fgets(input, MAX_STRING_LENGTH, stdin);
    if (input[0] != '\n' && input[0] != '\0') {
        strncpy(apprenant.motdepasse, input, sizeof(input));
        apprenant.motdepasse[strcspn(apprenant.motdepasse, "\n")] = 0; // Supprimer le saut de ligne s'il est présent
    }
    viderTampon();
    printf("Saisir le nouveau prénom de l'apprenant (appuyez sur Entrée pour conserver l'ancienne valeur): ");
    fgets(input, MAX_STRING_LENGTH, stdin);
    if (input[0] != '\n' && input[0] != '\0') {
        strncpy(apprenant.prenom, input, sizeof(input));
        apprenant.prenom[strcspn(apprenant.prenom, "\n")] = 0;
    }

    printf("Saisir le nouveau nom de l'apprenant (appuyez sur Entrée pour conserver l'ancienne valeur): ");
    fgets(input, MAX_STRING_LENGTH, stdin);
    if (input[0] != '\n' && input[0] != '\0') {
        strncpy(apprenant.nom, input, sizeof(input));
        apprenant.nom[strcspn(apprenant.nom, "\n")] = 0;
    }
    viderTampon();
    printf("Saisir la nouvelle classe de l'apprenant (appuyez sur Entrée pour conserver l'ancienne valeur): ");
    fgets(input, MAX_STRING_LENGTH, stdin);
    if (input[0] != '\n' && input[0] != '\0') {
        strncpy(apprenant.classe, input, sizeof(input));
        apprenant.classe[strcspn(apprenant.classe, "\n")] = 0;
    }

    // Replacer le curseur au début de la ligne de l'apprenant
    fseek(fichier, -1 * (strlen(apprenant.matricule) + strlen(apprenant.motdepasse) + strlen(apprenant.prenom) + strlen(apprenant.nom) + strlen(apprenant.classe) + sizeof(apprenant.statut) + 5),
          SEEK_CUR);

    // Écrire les nouvelles informations dans le fichier
    fprintf(fichier, "%s %s %s %s %s %d\n", apprenant.matricule, apprenant.motdepasse,
            apprenant.prenom, apprenant.nom, apprenant.classe, apprenant.statut);

    // Fermer le fichier
    fclose(fichier);

    printf("Les informations de l'apprenant ont été modifiées avec succès.\n");
}

// Fonction pour supprimer un apprenant
void supprimerApprenant(char *matricule)
{
    // Ouvrir le fichier en mode lecture et écriture
    FILE *fichier = fopen("etudiant.txt", "r+");
    if (fichier == NULL)
    {
        printf("Erreur lors de l'ouverture du fichier.\n");
        return;
    }

    // Créer un fichier temporaire
    FILE *fichier_temp = fopen("temp.txt", "w");
    if (fichier_temp == NULL)
    {
        printf("Erreur lors de la création du fichier temporaire.\n");
        fclose(fichier);
        return;
    }

    // Copier tous les apprenants sauf celui à supprimer dans le fichier temporaire
    Apprenant apprenant;
    while (fscanf(fichier, "%s %s %s %s %s %d", apprenant.matricule, apprenant.motdepasse,
                  apprenant.prenom, apprenant.nom, apprenant.classe, &apprenant.statut) == 6)
    {
        if (strcmp(apprenant.matricule, matricule) != 0)
        {
            fprintf(fichier_temp, "%s %s %s %s %s %d\n", apprenant.matricule, apprenant.motdepasse,
                    apprenant.prenom, apprenant.nom, apprenant.classe, apprenant.statut);
        }
    }

    // Fermer les fichiers
    fclose(fichier);
    fclose(fichier_temp);

    // Supprimer le fichier original
    if (remove("etudiant.txt") != 0)
    {
        printf("Erreur lors de la suppression du fichier original.\n");
        return;
    }

    // Renommer le fichier temporaire en fichier original
    if (rename("temp.txt", "etudiant.txt") != 0)
    {
        printf("Erreur lors du renommage du fichier temporaire.\n");
        return;
    }

    printf("L'apprenant a été supprimé avec succès.\n");
}

void viderTampon()
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;
}

void enregistrerPresence(char *matricule)
{
    FILE *fichierPresence = fopen("presence.txt", "r");
    if (fichierPresence == NULL)
    {
        printf("Erreur lors de l'ouverture du fichier de présence.\n");
        return;
    }

    // Récupérer la date actuelle
    time_t now = time(NULL);
    struct tm *timeinfo = localtime(&now);
    int jour = timeinfo->tm_mday;
    int mois = timeinfo->tm_mon + 1;      // Le mois commence à 0, donc on ajoute 1
    int annee = timeinfo->tm_year + 1900; // L'année est le nombre d'années depuis 1900
    fclose(fichierPresence);

    // Vérifier si l'étudiant est déjà marqué présent à la date actuelle
    fichierPresence = fopen("presence.txt", "r");
    if (fichierPresence == NULL)
    {
        printf("Erreur lors de l'ouverture du fichier de présence.\n");
        return;
    }
    char matricule_presence[10];
    int jour_presence, mois_presence, annee_presence;
    int present = 0;

    while (fscanf(fichierPresence, "%s %d/%d/%d", matricule_presence, &jour_presence, &mois_presence, &annee_presence) != EOF)
    {
        if (strcmp(matricule_presence, matricule) == 0 && jour_presence == jour && mois_presence == mois && annee_presence == annee)
        {
            printf("\n--- ❌ L'étudiant de matricule %s est déjà marqué présent aujourd'hui.\n", matricule);
            present = 1;
            break;
        }
    }
    fclose(fichierPresence);

    if (!present)
    {
        FILE *fichier = fopen("presence.txt", "a");
        if (fichier == NULL)
        {
            printf("Erreur lors de l'ouverture du fichier de présence.\n");
            return;
        }

        // Récupérer l'heure actuelle
        int heure = timeinfo->tm_hour;
        int minute = timeinfo->tm_min;
        int seconde = timeinfo->tm_sec;

        // Écrire dans le fichier la date et l'heure actuelles
        fprintf(fichier, "%s %d/%d/%d %02d:%02d:%02d\n", matricule, jour, mois, annee, heure, minute, seconde);
        fclose(fichier);
        printf("\n--- ✅ Presence marquee pour l'etudiant de matricule %s\n", matricule);
    }
}

void generer_fichier_par_date(int jour, int mois, int annee)
{
    FILE *fichier_presence = fopen("presence.txt", "r");
    if (fichier_presence == NULL)
    {
        printf("Erreur lors de l'ouverture du fichier de présence.\n");
        return;
    }

    char nom_fichier[20];
    sprintf(nom_fichier, "%d-%02d-%02d.txt", annee, mois, jour);

    FILE *fichier_apprenants = fopen(nom_fichier, "w");
    if (fichier_apprenants == NULL)
    {
        printf("Erreur lors de la création du fichier d'apprenants pour la date spécifiée.\n");
        fclose(fichier_presence);
        return;
    }

    // En-tête du tableau
    fprintf(fichier_apprenants, "+------------+-------------+----------+------------+---------+\n");
    fprintf(fichier_apprenants, "| Matricule  | Prénom      | Nom      | Classe     | Heure   |\n");
    fprintf(fichier_apprenants, "+------------+-------------+----------+------------+---------+\n");

    char matricule_presence[10];
    int jour_presence, mois_presence, annee_presence, heure, minute, seconde;
    int aucun_present = 1; // Indicateur pour vérifier s'il y a au moins un étudiant présent

    // Lire tous les étudiants présents à cette date
    while (fscanf(fichier_presence, "%s %d/%d/%d %dh%dmn%ds", matricule_presence, &jour_presence, &mois_presence, &annee_presence, &heure, &minute, &seconde) != EOF)
    {
        if (jour_presence == jour && mois_presence == mois && annee_presence == annee)
        {
            // Lire les détails de l'apprenant
            FILE *fichier_etudiant = fopen("etudiant.txt", "r");
            if (fichier_etudiant == NULL)
            {
                printf("Erreur lors de l'ouverture du fichier des étudiants.\n");
                fclose(fichier_presence);
                fclose(fichier_apprenants);
                return;
            }

            char matricule_etudiant[10];
            char nom[20], prenom[20], classe[10];
            // Parcourir tous les étudiants
            while (fscanf(fichier_etudiant, "%s %*s %s %s %s %*d %*s", matricule_etudiant, nom, prenom, classe) != EOF)
            {
                if (strcmp(matricule_presence, matricule_etudiant) == 0)
                {
                    // Écrire les détails de l'apprenant dans le fichier
                    fprintf(fichier_apprenants, "| %-10s | %-10s | %-8s | %-10s | %02d:%02d:%02d |\n", matricule_presence, prenom, nom, classe, heure, minute, seconde);
                    aucun_present = 0; // Indiquer qu'au moins un étudiant est présent
                }
            }
            fclose(fichier_etudiant);
        }
    }

    // Pied de tableau
    fprintf(fichier_apprenants, "+------------+-------------+----------+------------+---------+\n");

    fclose(fichier_presence);
    fclose(fichier_apprenants);

    if (aucun_present)
    {
        remove(nom_fichier); // Supprimer le fichier si aucun étudiant n'a été enregistré
        printf("Aucun apprenant présent à la date spécifiée.\n");
    }
    else
    {
        printf("Fichier généré avec succès : %s\n", nom_fichier);
    }
}

// Fonction pour vérifier si une année est bissextile
int est_bissextile(int annee)
{
    if ((annee % 4 == 0 && annee % 100 != 0) || (annee % 400 == 0))
    {
        return 1; //
    }
    else
    {
        return 0;
    }
}

int est_date_valide(int jour, int mois, int annee)
{
    // Vérification de la validité de la date
    if (mois < 1 || mois > 12)
        return 0; // Mois invalide

    if (jour < 1)
        return 0; // Jour invalide

    if ((mois == 2 && est_bissextile(annee) && jour > 29) || (mois == 2 && !est_bissextile(annee) && jour > 28))
        return 0; // Février avec jour invalide

    if ((mois == 4 || mois == 6 || mois == 9 || mois == 11) && jour > 30)
        return 0; // Mois avec 30 jours et jour invalide

    if (jour > 31)
        return 0;

    return 1;
}

void saisir_et_verifier_date()
{
    int jour, mois, annee;
    int result;

    do
    {
        printf("Entrez une date au format jj/mm/aaaa : ");
        result = scanf("%d/%d/%d", &jour, &mois, &annee);

        // Vérification de la saisie
        if (result != 3)
        {
            printf("Format de date invalide ou saisie incorrecte.\n");
            // Nettoyage du buffer d'entrée en cas de saisie incorrecte
            while (getchar() != '\n')
                ;
            continue; // Reprend la boucle pour une nouvelle saisie
        }

        // Vérification de la validité de la date
        if (!est_date_valide(jour, mois, annee))
        {
            printf("Date invalide.\n");
            continue; // Reprend la boucle pour une nouvelle saisie
        }

        // Sortir de la boucle si la date est valide
        break;

    } while (1); // Boucle infinie jusqu'à ce qu'une date valide soit saisie

    // Une fois que la date valide est saisie, appeler la fonction pour générer le fichier
    generer_fichier_par_date(jour, mois, annee);
}

void verifier_presence_et_generer_fichier()
{
    FILE *fichier_presence = fopen("presence.txt", "r");
    if (fichier_presence == NULL)
    {
        printf("Erreur lors de l'ouverture du fichier de présence.\n");
        return;
    }

    FILE *fichier_etudiants = fopen("etudiant.txt", "r");
    if (fichier_etudiants == NULL)
    {
        printf("Erreur lors de l'ouverture du fichier des étudiants.\n");
        fclose(fichier_presence);
        return;
    }

    // Obtenir la date actuelle
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    char date[20];
    strftime(date, sizeof(date), "%d-%m-%Y", tm); // Format de la date : JJ-MM-AAAA

    char nom_fichier_sortie[50];
    sprintf(nom_fichier_sortie, "presence_liste_%s.txt", date);

    FILE *fichier_sortie = fopen(nom_fichier_sortie, "w");
    if (fichier_sortie == NULL)
    {
        printf("Erreur lors de l'ouverture du fichier de sortie.\n");
        fclose(fichier_presence);
        fclose(fichier_etudiants);
        return;
    }
    printf("\n");
    char matricule_presence[10], matricule_etudiant[10], nom[20], prenom[20], classe[10];
    int jour, mois, annee, heure, minute, seconde;

    char date_precedente[20] = ""; // Stockage de la date précédente
    int premier_tableau = 1;       // Indicateur pour vérifier si c'est le premier tableau

    while (fscanf(fichier_presence, "%s %d/%d/%d %dh%dmn%ds", matricule_presence, &jour, &mois, &annee, &heure, &minute, &seconde) != EOF)
    {
        // Vérifier si la date actuelle est différente de la date précédente
        char date_actuelle[20];
        sprintf(date_actuelle, "%02d/%02d/%04d", jour, mois, annee);
        if (strcmp(date_actuelle, date_precedente) != 0)
        {
            // Si ce n'est pas le premier tableau, fermer le précédent
            if (!premier_tableau)
            {
                fprintf(fichier_sortie, "+------------+-------------+----------+------------+------------+\n\n");
            }

            // Commencer un nouveau tableau avec la date comme titre
            fprintf(fichier_sortie, "Liste de présence : %s\n", date_actuelle);
            fprintf(fichier_sortie, "+------------+-------------+----------+------------+------------+\n");
            fprintf(fichier_sortie, "| Matricule  | Heure       | Prénom   | Nom      | Classe       |\n");
            fprintf(fichier_sortie, "+------------+-------------+----------+------------+------------+\n");

            // Mettre à jour la date précédente
            strcpy(date_precedente, date_actuelle);
            premier_tableau = 0; // Marquer que ce n'est plus le premier tableau
        }

        // Recherche de l'étudiant correspondant au matricule de présence
        rewind(fichier_etudiants);
        while (fscanf(fichier_etudiants, "%s %*s %s %s %s %*d %*s", matricule_etudiant, nom, prenom, classe) != EOF)
        {
            if (strcmp(matricule_presence, matricule_etudiant) == 0)
            {
                fprintf(fichier_sortie, "| %-10s | %02d:%02d:%02d    | %-7s  | %-8s   | %-6s     |\n", matricule_presence, heure, minute, seconde, nom, prenom, classe);
                break;
            }
        }
    }

    fprintf(fichier_sortie, "+------------+-------------+----------+------------+------------+\n");

    fclose(fichier_sortie);
    fclose(fichier_presence);
    fclose(fichier_etudiants);
    printf("\n");
    printf("\nLe fichier de liste de présence a été généré avec succès.\n");
    printf("\n");
    printf("Appuyez sur une touche pour continuer...\n");
    getchar();
}

void marquerPresence()
{
    char choix[10];
    printf("\nEntrez le matricule de l'etudiant à marquer present ('Q' pour quitter) : ");
    scanf("%s", choix);
    while (strcmp(choix, "Q") != 0 && strcmp(choix, "q") != 0)
    {
        // Obtenir l'heure actuelle
        time_t t = time(NULL);
        struct tm *tm = localtime(&t);
        int heure_actuelle = tm->tm_hour;

        // Vérifier si l'heure est entre 8h et 16h
        if (heure_actuelle >= 8 && heure_actuelle < 16)
        {
            FILE *fichier = fopen("etudiant.txt", "r");
            if (fichier == NULL)
            {
                printf("Erreur lors de l'ouverture du fichier d'etudiants.\n");
                return;
            }

            char matricule[10];
            int found = 0;
            while (fscanf(fichier, "%s", matricule) != EOF)
            {
                if (strcmp(matricule, choix) == 0)
                {
                    found = 1;
                    break;
                }
            }
            fclose(fichier);

            if (!found)
            {
                printf("--- ❌ Matricule invalide. Veuillez reessayer ('Q' pour quitter) : ");
            }
            else
            {
                enregistrerPresence(choix);
            }
        }
        else
        {
            printf("--- ❌ Vous ne pouvez marquer la présence qu'entre 8h et 16h.\n");
        }

        printf("\n--- Entrez le matricule de l'etudiant à marquer present ('Q' pour quitter) : ");
        scanf("%s", choix);
    }
}

int recupNbApprenant(Apprenant *apprenants)
{
    FILE *fichier = fopen("etudiant.txt", "r");
    char ligne[50];

    if (fichier == NULL)
    {
        printf("Erreur lors de l'ouverture du fichier d'etudiants.\n");
        return -1;
    }
    int i = 0;
    while (fgets(ligne, sizeof(ligne), fichier) != NULL && i < MAX_APPRENANTS)
    {
        sscanf(ligne, "%s %s %s %s %s ", apprenants[i].matricule, apprenants[i].motdepasse, apprenants[i].prenom, apprenants[i].nom, apprenants[i].classe);
        i++;
    }

    fclose(fichier);

    return i;
}

int recupNbmessage(Message *messages)
{
    FILE *fichier = fopen("message.bin", "rb");
    char ligne[100];

    if (fichier == NULL)
    {
        fichier = fopen("message.bin", "wb");
    }

    char date[10], heure[10];
    int i = 0;
    // while (fgets(&ligne, sizeof(ligne), fichier) != NULL){
    //     sscanf(ligne, "%d | %d | %s | %s | %s | %s", &messages[i].id, &messages[i].status, messages[i].matricule, messages[i].dateHeur, messages[i].contenu);
    //     sprintf(messages[i].dateHeur, "%s %s", date, heure);
    //     i++;
    // }
    while (fread(&messages[i], sizeof(Message), 1, fichier) == 1)
        i++;

    fclose(fichier);

    return i;
}

int recupClasse(char classe[], Apprenant *etudiants)
{
    Apprenant apprenant[10];
    int nbApprenant = recupNbApprenant(apprenant), j = 0;
    for (int i = 0; i < nbApprenant; i++)
    {
        if (strcmp(apprenant[i].classe, classe) == 0)
        {
            etudiants[j++] = apprenant[i];
        }
    }
    return j;
}

// Fonction pour vérifier les identifiants de connexion
int verifierIdentifiants(Identifiants *identifiants, int nombreIdentifiants, char *login, char *motDePasse)
{
    for (int i = 0; i < nombreIdentifiants; i++)
    {
        if (strcmp(identifiants[i].login, login) == 0 && strcmp(identifiants[i].motDePasse, motDePasse) == 0)
        {
            return 1; // Identifiants valides
        }
    }
    return 0; // Identifiants invalides
}

int ajouterMessage(Message msg)
{
    FILE *fichier = fopen("message.bin", "ab");
    int rst = fwrite(&msg, sizeof(Message), 1, fichier);
    fclose(fichier);
    return rst;
}

int classeExiste(char *classeSaisie)
{
    char classes[3][7] = {"DEVWEB", "DATA", "REFDIG"};

    int valide = 0;
    while (!valide)
    {
        printf("Veuillez saisir une classe : ");
        scanf("%s", classeSaisie);

        for (int i = 0; i < MAX_CLASSES; i++)
        {
            if (strcmp(classes[i], classeSaisie) == 0)
            {
                return 1; // La classe existe
            }
        }

        if (!valide)
        {
            printf("La classe saisie n'est pas valide. Veuillez réessayer.\n");
        }
    }

    return 1; // La classe est valide
}

int recupMessageApprenant(char matricule[], Message *mesg)
{
    Message messages[100];
    int size = recupNbmessage(messages);
    int nbM = 0;
    for (int i = 0; i < size; i++)
    {
        if (strcmp(messages[i].matricule, matricule) == 0 && messages[i].status == 1)
        {
            mesg[nbM++] = messages[i];
        }
    }
    return nbM;
}

void afficher_message(Apprenant apprenant, char message[200], char date[20])
{
    printf("=============================================\n");
    printf("Message pour l'apprenant %s %s (Matricule: %s):\n", apprenant.prenom, apprenant.nom, apprenant.matricule);
    printf("Date et heure: %s\n", date);
    printf("Contenu:\n%s\n", message);
    printf("=============================================\n");
}

// Fonction pour envoyer un message à un ou plusieurs destinataires
void envoyer_message()
{
    Apprenant apprenants[50]; // Tableau d'apprenants
    int nbE = recupNbApprenant(apprenants);
    char matricules[100]; // pour stocker les matricules des étudiants
    char message[200];    // pour stocker le message
    char date[20];        // pour stocker la date et l'heure du message

    // Obtenir la date et l'heure actuelles
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(date, 20, "%d/%m/%Y %H:%M:%S", tm_info);

    printf("Entrez le(s) matricule(s) de(s) étudiant(s) destinataire(s) séparés par des virgules : ");
    scanf("%s", matricules); // lire les matricules entrés par l'utilisateur
    getchar();               // Pour absorber le saut de ligne
    printf("Taper votre message : \n");
    fgets(message, sizeof(message), stdin); // lire le message de l'utilisateur

    // Diviser les matricules entrés par l'utilisateur
    char *token = strtok(matricules, ",");
    while (token != NULL)
    {
        // Rechercher l'apprenant correspondant au matricule donné
        int matricule_valide = 0;
        for (int i = 0; i < nbE; i++)
        {
            if (strcmp(token, apprenants[i].matricule) == 0)
            {
                // Afficher le message pour cet apprenant
                afficher_message(apprenants[i], message, date);
                matricule_valide = 1; // Matricule valide
                break;
            }
        }
        // Vérifier si le matricule est valide et afficher un message approprié
        if (!matricule_valide)
            printf("Matricule invalide, message non envoyé pour le matricule %s.\n", token);

        token = strtok(NULL, ",");
    }
}

// fonction main
int main()
{

    Apprenant apprenants[MAX_APPRENANTS];
    int num_apprenants = 0;

    charger_etudiants(apprenants, &num_apprenants);

    /*
       generer_statistiques(apprenants, num_apprenants);
       return 0;
       viderTampon();
    */
    /* Message msg = {1, 1, "mat3", "9/3/2024 16h21mn50s", "Salut les dev"};
    ajouterMessage(msg);
    return 0; */

    // Création des fichiers pour stocker les identifiants
    FILE *fichierAdmin = fopen("admin.txt", "r");
    FILE *fichierEtudiant = fopen("etudiant.txt", "r");

    if (fichierAdmin == NULL || fichierEtudiant == NULL)
    {
        printf("Erreur lors de l'ouverture des fichiers.\n");
        return 1;
    }

    // Variables pour stocker les identifiants
    Identifiants identifiantsAdmin[100];    // Pour stocker jusqu'à 100 identifiants d'administrateur
    Identifiants identifiantsEtudiant[100]; // Pour stocker jusqu'à 100 identifiants d'étudiant

    int nombreIdentifiantsAdmin = 0;
    int nombreIdentifiantsEtudiant = 0;

    // Lecture des identifiants de l'admin
    while (fscanf(fichierAdmin, "%s %s", identifiantsAdmin[nombreIdentifiantsAdmin].login, identifiantsAdmin[nombreIdentifiantsAdmin].motDePasse) == 2)
    {
        nombreIdentifiantsAdmin++;
    }
    fclose(fichierAdmin);

    // Lecture des identifiants de l'étudiant
    while (fscanf(fichierEtudiant, "%s %s", identifiantsEtudiant[nombreIdentifiantsEtudiant].login, identifiantsEtudiant[nombreIdentifiantsEtudiant].motDePasse) == 2)
    {
        nombreIdentifiantsEtudiant++;
    }
    fclose(fichierEtudiant);

    int choix = 0;
    int choixMenu;
    char saisieLogin[LONGUEUR_MAX_LOGIN];
    char *saisieMotDePasse = malloc(LONGUEUR_MAX_MDP * sizeof(char)); // Allocation mémoire

    // Authentification
    do
    {

        printf("\nConnexion\n\n");
        saisieLogin[LONGUEUR_MAX_LOGIN] = '\0';
        printf(" login : ");
        fgets(saisieLogin, LONGUEUR_MAX_LOGIN, stdin);
        saisieLogin[strcspn(saisieLogin, "\n")] = 0; // Supprime le caractère de nouvelle ligne
        if (strlen(saisieLogin) == 0)
        {
            printf("\nVous avez laissé le champ vide. Veuillez rentrer votre login.\n");
            continue;
        }

        printf(" Mot de passe : ");

        int i = 0, c;
        while (i < LONGUEUR_MAX_MDP - 1 && (c = getch()) != '\n')
        {
            if (c == 127)
            { // ASCII value for backspace
                if (i > 0)
                {
                    printf("\b \b"); // Effacer le caractère précédent
                    i--;
                }
            }
            else
            {
                saisieMotDePasse[i++] = c;
                printf("*");
            }
        }
        saisieMotDePasse[i] = '\0';

        if (strlen(saisieMotDePasse) == 0)
        {
            printf("\nVous avez laissé le champ vide. Veuillez entrer votre mot de passe.\n");
            continue;
        }

        if (!(verifierIdentifiants(identifiantsAdmin, nombreIdentifiantsAdmin, saisieLogin, saisieMotDePasse)) && !(verifierIdentifiants(identifiantsEtudiant, nombreIdentifiantsEtudiant, saisieLogin, saisieMotDePasse)))
        {
            printf("\nLogin ou mot de passe invalides.\n");
        }
        if ((verifierIdentifiants(identifiantsAdmin, nombreIdentifiantsAdmin, saisieLogin, saisieMotDePasse)))
        {
            do
            {
                printf("*******************************************************************************\n");
                printf("*\t\t\tBienvenue dans le menu de l'administrateur:\t\t\t*\n");
                printf("*******************************************************************************\n");
                printf("*\t1. Gestion des étudiants\t\t\t\t\t\t*\n");
                printf("*\t2. Génération de fichiers\t\t\t\t\t*\n");
                printf("*\t3. Marquer les présences\t\t\t\t\t\t*\n");
                printf("*\t4. Envoyer un message\t\t\t\t\t\t*\n");
                printf("*\t5. Paramètres\t\t\t\t\t\t\t\t*\n");
                printf("*\t6. Deconnexion\t\t\t\t\t\t\t\t*\n");
                printf("*******************************************************************************\n");
                printf("\n Entrez votre choix : ");
                scanf("%d", &choix);
                if (choix == 1)
                {
                    printf("\n");
                    int choixG;
                    printf(" 1 Creer un etudiant\n");
                    printf(" 2 Supprimer un etudiant\n");
                    printf(" 3 Modifier un etudiant\n");
                    printf(" 4 Lister retards\n");
                    printf(" 5 Lister absences\n");
                    printf("Faites votre choix : ");
                    scanf("%d", &choixG);
                    if (choixG == 1)
                    {
                        creerApprenant();
                    }
                    if (choixG == 2)
                    {
                        char mat[10];
                        printf("Saisissez le matricule de l'apprenant a supprimer : ");
                        scanf("%s", mat);
                        //supprimerApprenant(mat);

                          bloquerApprenants();
                        // generer_statistiques(apprenants, num_apprenants);
                    }
                    if (choixG == 3)
                    {
                        char mat[10];
                        printf("Saisissez le matricule de l'apprenant a modifier : ");
                        scanf("%s", mat);
                        modifierApprenant(mat);
                    }
                    if (choixG == 4)
                    {
                        system("clear");
                        printf("Liste des retards : \n");
                        lire_presence(apprenants, num_apprenants);
                        afficher_retards(apprenants, num_apprenants);
                    }
                    if (choixG == 5)
                    {
                        system("clear");
                        printf("Liste des absences : \n");
                        charger_etudiants_et_compter_absences(apprenants, &num_apprenants);
                    }
                }

                if (choix == 3)
                {
                    marquerPresence();
                    do
                    {
                        getchar();
                        printf(" Mot de passe : ");

                        int i = 0, c;
                        while (i < LONGUEUR_MAX_MDP - 1 && (c = getch()) != '\n')
                        {
                            if (c == 127)
                            { // ASCII value for backspace
                                if (i > 0)
                                {
                                    printf("\b \b"); // Effacer le caractère précédent
                                    i--;
                                }
                            }
                            else
                            {
                                saisieMotDePasse[i++] = c;
                                printf("*");
                            }
                        }
                        saisieMotDePasse[i] = '\0';
                        if (!(verifierIdentifiants(identifiantsAdmin, nombreIdentifiantsAdmin, saisieLogin, saisieMotDePasse)))
                        {
                            printf("\n");
                            printf("\nVous n'êtes pas l'admin \n");
                            printf("Taper entrer pour continuer \n");
                            getchar();
                            system("clear");
                            marquerPresence();
                            printf("\n");
                        }
                    } while (!(verifierIdentifiants(identifiantsAdmin, nombreIdentifiantsAdmin, saisieLogin, saisieMotDePasse)));
                }
                if (choix == 4)
                {
                    printf("\n");
                    int choixmes;
                    char message[100];
                    time_t t = time(NULL);
                    struct tm tm = *localtime(&t);
                    do
                    {
                        char rep;
                        do
                        {

                            memset(message, '\0', sizeof(message));
                            printf("*******************************************************\n");
                            printf("*              Envoyer un message                     *\n");
                            printf("*-----------------------------------------------------*\n");
                            printf("*   1. Envoyer un message à tout le monde             *\n");
                            printf("*   2. Envoyer un message à une classe                *\n");
                            printf("*   3. Envoyer un message à un ou plusieurs étudiants *\n");
                            printf("*******************************************************\n");
                            scanf("%d", &choixmes);
                            if (choixmes == 1)
                            {
                                char m;
                                int i = 0;
                                getchar();
                                while (1)
                                {
                                    printf("Taper votre message : ");
                                    m = getchar();
                                    while (m != '\n')
                                    {
                                        message[i++] = m;
                                        m = getchar();
                                    }

                                    if (i != 0)
                                        break;
                                }

                                // Envoi du message à tous les étudiants
                                Message messages[50], msg;
                                Apprenant apprenants[50];
                                char date[20];
                                sprintf(date, "%02d/%02d/%04d %02d:%02d:%02d", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec);
                                int nbM = recupNbmessage(messages);
                                int nbE = recupNbApprenant(apprenants);

                                strcpy(msg.contenu, message);
                                strcpy(msg.dateHeur, date);
                                msg.status = 1;

                                for (int i = 0; i < nbE; i++)
                                {
                                    strcpy(msg.matricule, apprenants[i].matricule);
                                    msg.id = nbM + 1 + i;
                                    ajouterMessage(msg);
                                }
                            }
                            if (choixmes == 2)
                            {
                                char m;
                                int i = 0;
                                getchar();
                                while (1)
                                {
                                    printf("Taper votre message : ");
                                    m = getchar();
                                    while (m != '\n')
                                    {
                                        message[i++] = m;
                                        m = getchar();
                                    }

                                    if (i != 0)
                                        break;
                                }
                                char classeSaisie[MAX_LENGTH];
                                classeExiste(classeSaisie);
                                Apprenant etudiants[50];
                                char date[20];
                                Message messages[50], msg;

                                int nbE = recupClasse(classeSaisie, etudiants);
                                int nbM = recupNbmessage(messages);

                                sprintf(date, "%02d/%02d/%04d", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900);

                                strcpy(msg.contenu, message);
                                strcpy(msg.dateHeur, date);
                                msg.status = 1;

                                for (int i = 0; i < nbE; i++)
                                {
                                    strcpy(msg.matricule, etudiants[i].matricule);
                                    msg.id = nbM + 1 + i;
                                    ajouterMessage(msg);
                                }
                            }
                            if (choixmes == 3)
                            {
                                Message messages[50];
                                Apprenant apprenants[50];
                                char matricules[100]; // pour stocker les matricules des étudiants
                                printf("Entrez le(s) matricule(s) de(s) étudiant(s) destinataire(s) séparés par des virgules : ");
                                scanf("%s", matricules); // lire les matricules entrés par l'utilisateur
                                getchar();               // Pour absorber le saut de ligne
                                printf("Taper votre message : \n");
                                fgets(message, sizeof(message), stdin); // lire le message de l'utilisateur

                                int nbM = recupNbmessage(messages);
                                int nbE = recupNbApprenant(apprenants);

                                char date[20];
                                sprintf(date, "%02d/%02d/%04d %02d:%02d:%02d", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec);

                                // Diviser les matricules entrés par l'utilisateur
                                char *token = strtok(matricules, ",");
                                while (token != NULL)
                                {
                                    // Rechercher l'étudiant correspondant au matricule donné
                                    int matricule_valide = 0;
                                    for (int i = 0; i < nbE; i++)
                                    {
                                        if (strcmp(token, apprenants[i].matricule) == 0)
                                        {
                                            // Ajouter le message pour cet étudiant
                                            Message msg;
                                            strcpy(msg.contenu, message);
                                            strcpy(msg.dateHeur, date);
                                            msg.status = 1;
                                            strcpy(msg.matricule, token);
                                            msg.id = nbM + 1; // Identifiant unique pour chaque message
                                            ajouterMessage(msg);
                                            matricule_valide = 1; // Matricule valide
                                            break;
                                        }
                                    }
                                    // Vérifier si le matricule est valide et afficher un message approprié
                                    if (matricule_valide)
                                        printf("Message envoyé avec succès pour l'étudiant de matricule %s.\n", token);
                                    else
                                        printf("Matricule invalide, message non envoyé pour le matricule %s.\n", token);

                                    token = strtok(NULL, ",");
                                }
                            }

                            printf("Taper entrer pour continuer \n");
                            printf("Voulez-vous envoyer un autre message ? (o/n) ");
                            scanf("%c", &rep);
                        } while (rep == 'o' || rep == 'O');
                        getchar();
                        system("clear");
                        if (rep == 'n' || rep == 'N')
                            break;
                    } while (choix <= 0 || choix > 3);
                    system("clear");
                }
                if (choix == 6)
                {
                    printf("Vous êtes déconnecté !\n");
                }
                if (choix == 2)
                {
                    int choixP;
                    printf("***************************************************\n");
                    printf("*          Génération de fichiers                  *\n");
                    printf("*-------------------------------------------------*\n");
                    printf("*   1. Toutes les présences par date              *\n");
                    printf("*   2. Présences par date                          *\n");
                    printf("*   3. Quitter                                     *\n");
                    printf("***************************************************\n");
                    printf("Faites votre choix : ");
                    scanf("%d", &choixP);
                    if (choixP == 1)
                    {
                        verifier_presence_et_generer_fichier();
                    }
                    if (choixP == 2)
                    {
                        printf("Date : ");
                        saisir_et_verifier_date();
                    }
                    if (choixP == 3)
                    {
                        system("clear");
                        break;
                    }
                }
                if (choix < 1 || choix > 6)
                {
                    printf("Choix invalide. Veuillez entrer un choix entre 1 et 2.\n");
                }
            } while (choix != 6);
        }
        if ((verifierIdentifiants(identifiantsEtudiant, nombreIdentifiantsEtudiant, saisieLogin, saisieMotDePasse)))
        {
            int choix = 0, nb;
            Message messages[50];
            system("clear");
            do
            {
                nb = recupMessageApprenant(saisieLogin, messages);
                printf("*\t\t\tBienvenue dans le menu de l'apprenant :\t\t\t *\n");
                printf("**************************************************************************\n");
                printf("*  1. GESTION DES ÉTUDIANTS                                               *\n");
                printf("*  2. GÉNÉRATION DE FICHIERS                                               *\n");
                printf("*  3. MARQUER SA PRÉSENCE                                                  *\n");
                printf("*  4. Message (%d)\n", nb);
                printf("*  5. Déconnexion                                                           *\n");
                printf("**************************************************************************\n");
                printf("\n Entrez votre choix : ");
                scanf("%d", &choix);
                if (choix < 1 || choix > 5)
                {
                    printf("Choix invalide. Veuillez entrer un choix entre  1 et 5.\n");
                }
                if (choix == 1)
                {
                }
                if (choix == 3)
                {
                    system("clear");

                    //----------------------- Doublons & Présence ------------------------------------
                    FILE *fichierPresence = fopen("presence.txt", "r");
                    if (fichierPresence == NULL)
                    {
                        printf("Erreur lors de l'ouverture du fichier de présence.\n");
                        return 1;
                    }

                    int present = 0;
                    char matricule[10];
                    char dateLu[MAX_LENGTH];
                    while (fscanf(fichierPresence, "%s %s", matricule, dateLu) != EOF)
                    {
                        if (strcmp(matricule, matricule) == 0)
                        {
                            // Vérifier si la date correspond à la date actuelle
                            time_t t = time(NULL);
                            struct tm *tm = localtime(&t);
                            char dateActuelle[MAX_LENGTH];
                            strftime(dateActuelle, sizeof(dateActuelle), "%d/%m/%Y", tm);
                            if (strcmp(dateLu, dateActuelle) == 0)
                            {
                                fclose(fichierPresence);
                                present = 1;
                                return 1; // L'étudiant est déjà marqué présent aujourd'hui
                            }
                        }
                    }

                    fclose(fichierPresence);

                    if (!present)
                    {
                        FILE *fichier = fopen("etudiant.txt", "r+");
                        if (fichier == NULL)
                        {
                            printf("Erreur lors de l'ouverture du fichier d'etudiants.\n");
                            return 1;
                        }

                        while (fscanf(fichier, "%s", matricule) != EOF)
                        {
                            if (strcmp(matricule, matricule) == 0)
                            {
                                // Enregistrer la présence dans le fichier
                                enregistrerPresence(matricule);
                                // printf("\n--- ✅ Presence marquee avec succes");
                                present = 1;
                                break;
                            }
                        }

                        fclose(fichier);
                    }
                    //----------------------------------------  Fin --------------------------------------------------
                }
                if (choix == 5)
                {
                    printf("Vous êtes déconnecté !\n");
                    saisieLogin[LONGUEUR_MAX_LOGIN] = 'a';
                }
                if (choix == 4)
                {
                    for (int i = 0; i < nb; i++)
                    {
                        printf("[%d] => %s\n", i + 1, messages[i].contenu);
                    }

                    Message toutMessage[50], msg;
                    int nbMessage = recupNbmessage(toutMessage);
                    FILE *fichier = fopen("message.bin", "wb+");
                    for (int i = 0; i < nbMessage; i++)
                    {
                        msg = toutMessage[i];
                        for (int j = 0; j < nb; j++)
                        {
                            if (msg.id == messages[j].id)
                            {
                                msg.status = 0;
                                break;
                            }
                        }
                        fwrite(&msg, sizeof(Message), 1, fichier);
                    }

                    fclose(fichier);
                }
            } while (choix != 5);
        }
    } while (!(verifierIdentifiants(identifiantsAdmin, nombreIdentifiantsAdmin, saisieLogin, saisieMotDePasse)) || !(verifierIdentifiants(identifiantsEtudiant, nombreIdentifiantsEtudiant, saisieLogin, saisieMotDePasse)));

    return 0;
}