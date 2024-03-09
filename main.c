#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <termios.h>
#include <ctype.h>

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

#define LONGUEUR_MAX_LOGIN 10
#define LONGUEUR_MAX_MDP 10
#define MAX_STUDENTS_PER_CLASS 50

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
    char classe[6];
    Date dateNaiss;
    int etat;
} Apprenant;

Identifiants identifiantsAdmin;
int nombreIdentifiantsAdmin = 1;

void enregistrerPresence(char *matricule)
{
    FILE *fichier = fopen("presence.txt", "a");
    if (fichier == NULL)
    {
        printf("Erreur lors de l'ouverture du fichier de présence.\n");
        return;
    }

    // Récupérer la date et l'heure  actuelle
    time_t now = time(NULL);
    struct tm *timeinfo = localtime(&now);
    // Écrire dans le fichier la date et l'heure
    fprintf(fichier, "%s %d/%d/%d %dh%dmn%ds\n", matricule, timeinfo->tm_mday, timeinfo->tm_mon + 1,
            timeinfo->tm_year + 1900, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);

    fclose(fichier);
}

// Fonction pour vérifier si une année est bissextile
int est_bissextile(int annee)
{
    if ((annee % 4 == 0 && annee % 100 != 0) || (annee % 400 == 0))
    {
        return 1; // Si l'année est bissextile, retourne 1
    }
    else
    {
        return 0; // Sinon, retourne 0
    }
}

// Fonction pour saisir et vérifier une date
void saisir_et_verifier_date()
{
    int jour, mois, annee;

    do
    {
        printf("Entrez le jour : ");
        scanf("%d", &jour);
        if (jour <= 0 || jour > 31)
        {
            printf("Veuillez entrer un jour valide\n");
        }
    } while (jour <= 0 || jour > 31);

    do
    {
        printf("Entrez le mois : ");
        scanf("%d", &mois);
        if (mois <= 0 || mois > 12)
        {
            printf("Veuillez entrer un mois valide\n");
        }
    } while (mois <= 0 || mois > 12);

    do
    {
        printf("Entrez l'annee : ");
        scanf("%d", &annee);
        if (annee <= 0 || annee > 2024)
        {
            printf("Veuillez entrer une annee valide\n");
        }
    } while (annee <= 0 || annee > 9999);

    // Vérification de la validité de la date
    if (mois == 2)
    {
        if (est_bissextile(annee))
        {
            if (jour > 29)
            {
                printf("La date est invalide\n");
                return;
            }
        }
        else
        {
            if (jour > 28)
            {
                printf("La date est invalide\n");
                return;
            }
        }
    }
    else if ((mois == 4 || mois == 6 || mois == 9 || mois == 11) && jour > 30)
    {
        printf("La date est invalide\n");
        return;
    }

    printf("Date: %d/%d/%d\n", jour, mois, annee);
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
    int premier_tableau = 1; // Indicateur pour vérifier si c'est le premier tableau

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
            fprintf(fichier_sortie, "| Matricule  | Heure       |  Nom     | Prénom     | Classe     |\n");
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
        FILE *fichierPresence = fopen("presence.txt", "r");
        if (fichierPresence == NULL)
        {
            printf("Erreur lors de l'ouverture du fichier de présence.\n");
            return;
        }
        int present = 0;
        char matricule[10];
        while (fscanf(fichierPresence, "%s", matricule) != EOF)
        {
            if (strcmp(matricule, choix) == 0)
            {
                printf("\n--- ❌ L'étudiant de matricule %s est déjà marqué présent.\n", choix);
                present = 1;
                break;
            }
        }
        fclose(fichierPresence);

        if (!present)
        {
            FILE *fichier = fopen("etudiant.txt", "r+");
            if (fichier == NULL)
            {
                printf("Erreur lors de l'ouverture du fichier d'etudiants.\n");
                return;
            }

            while (fscanf(fichier, "%s", matricule) != EOF)
            {
                if (strcmp(matricule, choix) == 0)
                {
                    // Enregistrer la présence dans le fichier
                    enregistrerPresence(choix);
                    printf("\n--- ✅ Presence marquee pour l'etudiant de matricule %s\n", choix);
                    present = 1;
                    break;
                }
            }
            fclose(fichier);
        }

        if (!present)
        {
            printf("--- ❌ Matricule invalide. Veuillez reessayer ('Q' pour quitter) : ");
        }
        else
        {
            printf("\n--- Entrez le matricule de l'etudiant à marquer present ('Q' pour quitter) : ");
        }

        scanf("%s", choix);
    }
}

int menuAdmin()
{
    int choix = 0;
    do
    {
        printf("--------------------------------------------------------------------------\n");
        printf("\t\t\tBienvenue dans le menu de l'administrateur:\n");
        printf("--------------------------------------------------------------------------\n");
        printf("1  Gestion des étudiants\n");
        printf("2  Génération de fichiers\n");
        printf("3  Marquer les présences\n");
        printf("4  Envoyer un message\n");
        printf("5  Paramètres\n");
        printf("6  Deconnexion\n");
        printf("\n--- Entrez votre choix : ");
        scanf("%d", &choix);
        if (choix < 1 || choix > 6)
        {
            printf("Choix invalide. Veuillez entrer un choix entre 1 et 2.\n");
        }
    } while (choix != 6);
    return choix;
}




int menuEtudiant()
{
    // Définition du menu de l'étudiant
    int choix = 0;
    do
    {

        printf("\t\t\tBienvenue dans le menu de l'apprenant :\n");

        printf("1 GESTION DES ÉTUDIANTS\n");
        printf("2 GÉNÉRATION DE FICHIERS\n");
        printf("3 MARQUER SA PRÉSENCE\n");
        printf("4 Message (0)\n");
        printf("5 Déconnexion\n");
        printf("\ Entrez votre choix : ");
        scanf("%d", &choix);
        if (choix < 1 || choix > 5)
        {
            printf("Choix invalide. Veuillez entrer un choix entre  1 et 5.\n");
        }
    } while (choix < 1 || choix > 5);
    return choix;
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

void generer_fichier_par_date(int jour, int mois, int annee)
{
    char nom_fichier[20];
    sprintf(nom_fichier, "%d-%02d-%02d.txt", annee, mois, jour);

    FILE *fichier_presence = fopen("presence.txt", "r");
    if (fichier_presence == NULL)
    {
        printf("Erreur lors de l'ouverture du fichier de présence.\n");
        return;
    }

    FILE *fichier_apprenants = fopen(nom_fichier, "w");
    if (fichier_apprenants == NULL)
    {
        printf("Erreur lors de la création du fichier d'apprenants pour la date spécifiée.\n");
        fclose(fichier_presence);
        return;
    }

    fprintf(fichier_apprenants, "Apprenants présents le %02d/%02d/%d :\n\n", jour, mois, annee);

    char matricule[10];
    while (fscanf(fichier_presence, "%s", matricule) != EOF)
    {
        int jour_present, mois_present, annee_present, heure, minute, seconde;
        fscanf(fichier_presence, "%d/%d/%d %dh%dmn%ds", &jour_present, &mois_present, &annee_present, &heure, &minute, &seconde);

        if (jour_present == jour && mois_present == mois && annee_present == annee)
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
            char nom[20], prenom[20], classe[6];
            while (fscanf(fichier_etudiant, "%s %s %s %s", matricule_etudiant, prenom, nom, classe) != EOF)
            {
                if (strcmp(matricule_etudiant, matricule) == 0)
                {
                    // Écrire les détails de l'apprenant dans le fichier
                    fprintf(fichier_apprenants, "Nom : %s\nPrénom : %s\nClasse : %s\nHeure : %02d:%02d:%02d\n\n", nom, prenom, classe, heure, minute, seconde);
                    break;
                }
            }
            fclose(fichier_etudiant);
        }
    }

    fclose(fichier_presence);
    fclose(fichier_apprenants);

    printf("Fichier généré avec succès : %s\n", nom_fichier);
}

// fonction main
int main()
{
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

                printf("\t\t\tBienvenue dans le menu de l'administrateur:\n");
                printf("--------------------------------------------------------------------------\n");
                printf("1  Gestion des étudiants\n");
                printf("2  Génération de fichiers\n");
                printf("3  Marquer les présences\n");
                printf("4  Envoyer un message\n");
                printf("5  Paramètres\n");
                printf("6  Deconnexion\n");
                printf("\n Entrez votre choix : ");
                scanf("%d", &choix);
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
                if (choix == 6)
                {
                    printf("Vous êtes déconnecté !\n");
                }
                if (choix==2)
                {
                    int choixP;
                    printf("Génération de fichiers \n");
                    printf("1 Toutes les presences par date \n");
                    printf("2 Presence par date \n");
                    printf("3 Quitter \n");
                    printf("Faite votre choix : ");
                    scanf("%d",&choixP);
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
                       
                        system("clear");break;
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
            int choix = 0;
            do
            {

                printf("\t\t\tBienvenue dans le menu de l'apprenant :\n");
                printf("--------------------------------------------------------------------------\n");
                printf("1  GESTION DES ÉTUDIANTS\n");
                printf("2  GÉNÉRATION DE FICHIERS\n");
                printf("3  MARQUER SA PRÉSENCE\n");
                printf("4  Message (0)\n");
                printf("5  Déconnexion\n");
                printf("\n Entrez votre choix : ");
                scanf("%d", &choix);
                if (choix < 1 || choix > 5)
                {
                    printf("Choix invalide. Veuillez entrer un choix entre  1 et 5.\n");
                }
                if (choix == 3)
                {

                    //----------------------- Doublons & Présence ----------------------------------------------------
                    FILE *fichierPresence = fopen("presence.txt", "r");
                    if (fichierPresence == NULL)
                    {
                        printf("Erreur lors de l'ouverture du fichier de présence.\n");
                        return 1;
                    }

                    int present = 0;
                    char matricule[10];
                    while (fscanf(fichierPresence, "%s", matricule) != EOF)
                    {
                        if (strcmp(matricule, matricule) == 0)
                        {
                            printf("\n--- ❌ L'étudiant dest deja marqué présent.\n", matricule);
                            present = 1;
                            break;
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
                                printf("\n--- ✅ Presence marquee avec succes");
                                present = 1;
                                break;
                            }
                        }

                        fclose(fichier);
                    }
                    //----------------------------------------  Fin -----------------------------------------------------
                }
                if (choix == 5)
                {
                    printf("Vous êtes déconnecté !\n");
                    saisieLogin[LONGUEUR_MAX_LOGIN] = 'a';
                }
            } while (choix != 5);
        }
    } while (!(verifierIdentifiants(identifiantsAdmin, nombreIdentifiantsAdmin, saisieLogin, saisieMotDePasse)) || !(verifierIdentifiants(identifiantsEtudiant, nombreIdentifiantsEtudiant, saisieLogin, saisieMotDePasse)));

    return 0;
}
