#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#ifdef _WIN32
    #define clearScreen() (system("cls"))
#else
    #define clearScreen() (system("clear"))
#endif

struct Date{
    int day;
    int month;
    int year;
};

struct Node{
    struct Node* prev;
    struct Node* next;
    struct Date date;
    char** notes;
    int note_count;
    int max_note_count;
};

int node_count = 0;

void freeAllNodes(struct Node* node){
    if(node->next != NULL){
        freeAllNodes(node->next);
    }
    for(int i = 0;i< node->note_count;i++){
        free(node->notes[i]);
    }
    free(node->notes);
    free(node);
}

void freeNode(struct Node* node){
    for(int i = 0;i< node->note_count;i++){
        free(node->notes[i]);
    }
    free(node->notes);
    free(node);
}

int isValidDate(char* date){
    if(strlen(date)!=11){
        return 0;
    }
    date[10]='\0'; //because the raw input contains \n
    if(!isdigit(date[0])) return 0;
    if(!isdigit(date[1])) return 0;
    if(date[2]!='.') return 0;
    if(!isdigit(date[3])) return 0;
    if(!isdigit(date[4])) return 0;
    if(date[5]!='.') return 0;
    if(!isdigit(date[6])) return 0;
    if(!isdigit(date[7])) return 0;
    if(!isdigit(date[8])) return 0;
    if(!isdigit(date[9])) return 0;

    int day;
    int month;
    int year;

    sscanf(date,"%d.%d.%d",&day,&month,&year);

    int is_leap = 0;

    if(month>12){
        return 0;
    }

    if((year % 4 == 0 && year % 100 != 0) || year % 400 == 0){
        is_leap = 1;
    }

    if(month == 2 && day<=28+is_leap){
        return 1;
    }
    if(month<=7){
        if(month % 2 == 1 && day<=31){
            return 1;
        }
        if(month % 2 == 0 && day<=30){
            return 1;
        }
    }
    else{
        if(month % 2 == 0 && day<=31){
            return 1;
        }
        if(month % 2 == 1 && day<=30){
            return 1;
        }
    }

    return 0;
}

struct Node* makeNode(struct Node* prev, struct Node* next, struct Date date){
    node_count++;
    struct Node *node = malloc(sizeof(struct Node));
    node->prev = prev;
    node->next = next;
    node->date = date;
    node->max_note_count = 10;
    node->notes = (char**)malloc(sizeof(char*) * node->max_note_count);
    node->note_count = 0;
    return node;
}

char* dateToString(struct Date date){
    static char date_string[10+1];
    sprintf(date_string,"%02d.%02d.%04d",date.day,date.month, date.year);
    return date_string;
}

struct Date stringToDate(char* date_string){ //Must be in the correct format
    struct Date date;

    sscanf(date_string,"%d.%d.%d",&date.day,&date.month,&date.year);
    return date;
}

int compareDates(struct Date date1,struct Date date2){ //compares two Date structs return -1 if first date is smaller 0 if dates are the same 1 if date two is smaller 
    if(date1.year < date2.year) return -1;
    if(date1.year > date2.year) return 1;

    if(date1.month < date2.month) return -1;
    if(date1.month > date2.month) return 1;

    if(date1.day < date2.day) return -1;
    if(date1.day > date2.day) return 1;

    return 0;
}

int saveNotes(char* file_name, struct Node* node){
    while(node->prev!=NULL){
        node=node->prev;
    }
    FILE * fptr = fopen(strcat(file_name,".notes"),"w");
    fprintf(fptr,"%d\n",node_count); //node count
    while(node != NULL){
        //prints the date, current note count and max note count
        fprintf(fptr,"%s %d %d\n",dateToString(node->date),node->note_count,node->max_note_count);
        for(int i = 0;i<node->note_count;i++){
            fprintf(fptr,"%s\n",node->notes[i]);
        }
        node = node -> next;
    }
    fclose(fptr);
    return 0;
}

struct Node* loadNotes(char* file_name){
    FILE* fptr = fopen(strcat(file_name,".notes"),"r");
    if(fptr == NULL){
        return NULL;
    }
    fscanf(fptr,"%d\n",&node_count);
    struct Node* node = malloc(sizeof(struct Node));
    node->prev = NULL;

    for(int i = 0;i<node_count;i++){
        char date_string[11];
        fscanf(fptr,"%s %d %d\n",date_string,&node->note_count,&node->max_note_count);
        node->date = stringToDate(date_string);
        node->notes = (char**)malloc(sizeof(char*)*node->max_note_count);

        for(int j = 0;j<node->note_count;j++){
            int max_note_size = 100;
            int note_index = 1; //the first character is already collected before the while loop
            char* note = (char*)malloc(sizeof(char)*max_note_size);
            char c = fgetc(fptr);
            note[0]= c;

            while(1){
                c=fgetc(fptr);
                if(note[note_index-1] == '\n' && c=='\n'){
                    break;
                }
                note[note_index++] = c;
                if(note_index > max_note_size - 2){ //2 to make sure it doesn't overflow
                    max_note_size*=2;
                    note = realloc(note, sizeof(char)*max_note_size);
                }
            }
            note[note_index] = '\0';
            node->notes[j] = note;
        }
        
        if(i==node_count-1){
            node->next = NULL;
        }
        else{
            struct Node* tmp_node = malloc(sizeof(struct Node));
            tmp_node->prev = node;
            node->next = tmp_node;

            node = node->next;
        }
    }

    fclose(fptr);
    return node;
}

int main(){ //* This uses C23 (Just so that i know for the MD file after I finish this)
    struct Node *node = NULL;
    while (1)
    {
        clearScreen();

        printf("You currently have %d days available\n\n",node_count);

        if(node && node->note_count>0){
            printf("The notes for the day of %s (%d)\n",dateToString(node->date),node->note_count);
            for(int i = 0;i<node->note_count;i++){
                printf("%d. %s\n",i+1,node->notes[i]);
            }
        }

        printf("Available commands (choose pomoc for command descriptions):\n");
        printf("predchozi, dalsi, zacatek, konec, novy, smaz, uloz, nacti, zavri, pomoc\n");
        printf("What would you like to do: ");

        char command[20];
        fgets(command,sizeof(command),stdin);
        clearScreen();
        if(strcmp(command,"zavri\n") == 0){ // I hate to do it this way but C doesn't support string switches
            break;
        }
        else if(strcmp(command,"pomoc\n")==0){
            printf("predchozi - move to day before\n");
            printf("dalsi - move to day after\n");
            printf("zacatek - move to the first day\n");
            printf("konec - move to the last day\n");
            printf("novy - add a new note to the day of your choice\n");
            printf("smaz - delete the chosen note from the current day\n");
            printf("uloz - save your notes to a file\n");
            printf("nacti - load your notes from a file\n");
            printf("zavri - close the app\n\n");
            printf("Press enter to continue: ");
            getchar();
        }
        else if(strcmp(command,"novy\n") == 0){
            char date[20];

            while(1){
                printf("What day should this note be added to (dd.MM.yyyy format): ");
                fgets(date,sizeof(date),stdin);
                clearScreen();
                if(isValidDate(date)){
                    break;
                }
                printf("Incorrect format or date, try again\n");
            }

            struct Date struct_date = stringToDate(date);

            printf("Type what you want to add to %s's notes (Empty line to end editing note):\n",date);
            int max_note_size = 100;
            int note_index = 1; //the first character is already collected before the while loop
            char* note = (char*)malloc(sizeof(char)*max_note_size);
            char c = getchar();
            note[0]= c;

            while(1){
                c=getchar();
                if(note[note_index-1] == '\n' && c=='\n'){
                    break;
                }
                note[note_index++] = c;
                if(note_index > max_note_size - 2){ //2 to make sure it doesn't overflow
                    max_note_size*=2;
                    note = realloc(note, sizeof(char)*max_note_size);
                }
            }
            note[note_index] = '\0';

            //* finding the correct node with the entered date
            if(!node){ 
                node = makeNode(node,NULL,struct_date);
            }
            else{
                while(node->prev!=NULL){ //getting back to the start of the list
                    node=node->prev;
                }
                
                while(node->next!=NULL){
                    int date_cmp_res = compareDates(struct_date,node->date);
                    if(date_cmp_res!=1){
                        break;
                    }
                    node=node->next;
                }

                int date_cmp_res = compareDates(struct_date,node->date);
                struct Node* temp_node;
                switch (date_cmp_res){
                    case -1:
                        if(node->prev){
                            temp_node = makeNode(node->prev,node,struct_date);
                            node->prev->next = temp_node;
                        }
                        else{
                            temp_node = makeNode(NULL,node,struct_date);
                        }
                        node->prev = temp_node;
                        node = node->prev;
                        break;
                    case 1:
                        if(node->next){
                            temp_node = makeNode(node,node->next,struct_date);
                            node->next->prev = temp_node;
                        }
                        else{
                            temp_node = makeNode(node,NULL,struct_date);
                        }
                        node->next = temp_node;
                        node = node->next;
                        break;
                }            
            }

            if(node->note_count >= node->max_note_count -1){
                node->max_note_count *=2;
                node->notes = realloc(node->notes,sizeof(char*) * node->max_note_count);
            }

            node->notes[node->note_count++] = note;
        }
        else if(strcmp(command,"smaz\n")==0){
            if(!(node && node->note_count>0)){
                printf("There is nothing to delete\n");
                printf("Press enter to continue: ");
                getchar();
                continue;
            }
            printf("The notes for the day of %s (%d)\n",dateToString(node->date),node->note_count);
            for(int i = 0;i<node->note_count;i++){
                printf("%d. %s\n",i+1,node->notes[i]);
            }

            int note_num=0;
            printf("Choose the note you want to delete (0 to cancel): ");
            scanf("%d",&note_num); getchar(); //gets rid of the \n still stuck
            if(note_num==0){
                continue;
            }
            clearScreen();
            if(note_num<0 || note_num>node->note_count){
                printf("A note with this number doesn't exist\n");
                printf("Press enter to continue: ");
                getchar();
                continue;
            }
            printf("%s\nDo you really want to delete this note (yes/No): ",node->notes[note_num-1]);
            char confirmation[5];
            fgets(confirmation,sizeof(confirmation),stdin);
            if(strcmp(confirmation,"yes\n")==0){
                free(node->notes[note_num-1]);
                for(int i = note_num; i< node->note_count;i++){
                    node->notes[i-1] = node->notes[i];
                }
                node->notes[node->note_count-1]=NULL;
                node->note_count--;
            }
            if(node->note_count==0){ //according to the assignment it should firstly go to the next one if it doesn't exist the one before and if this one doesn't exist home screen
                struct Node* temp_node = node;
                node_count--;
                if(node->prev){
                    node->prev->next = node->next;
                }
                if(node->next){
                    node->next->prev = node->prev;
                    node = node->next;
                    freeNode(temp_node);
                    continue;
                }
                if(node->prev){
                    node = node->prev;
                    freeNode(temp_node);
                    continue;
                }
                node = NULL;
                freeNode(temp_node);
            }
        }
        else if(strcmp(command,"zacatek\n")==0){
            if(!node){
                continue;
            }
            while(node->prev!=NULL){
                node=node->prev;
            }
        }
        else if(strcmp(command,"konec\n")==0){
            if(!node){
                continue;
            }
            while(node->next!=NULL){
                node=node->next;
            }
        }
        else if(strcmp(command,"dalsi\n")==0){
            if(node && node->next!=NULL){
                node=node->next;
            }
        }
        else if(strcmp(command,"predchozi\n")==0){
            if(node && node->prev!=NULL){
                node=node->prev;
            }
        }
        else if(strcmp(command,"uloz\n")==0){
            if(!node){
                printf("There is nothing to save\n");
                printf("Press enter to continue: ");
                getchar();
                continue;
            }
            printf("What file do you want to save these notes to (without the extension)\n(just press enter to cancel): ");
            int max_name_size = 100;
            int name_index = 0; //the first character is already collected before the while loop
            char* file_name = (char*)malloc(sizeof(char)*max_name_size);
            char c;

            while((c = getchar())!='\n'){
                file_name[name_index++] = c;
                if(name_index > max_name_size - 2){ //2 to make sure it doesn't overflow
                    max_name_size*=2;
                    file_name = realloc(file_name, sizeof(char)*max_name_size);
                }
            }
            file_name[name_index] = '\0';
            if(strcmp(file_name,"")==0){
                free(file_name);
                continue;
            }
            clearScreen();
            if(saveNotes(file_name,node)){
                printf("Your notes failed to save\n");
                printf("Press enter to continue: ");
                getchar();
            }
            else{
                printf("Your notes were saved successfully\n");
                printf("Press enter to continue: ");
                getchar();
            }
            free(file_name);
        }
        else if(strcmp(command,"nacti\n")==0){
            if(node){
                printf("You might have unsaved notes\nDo you still want to continue loading from a file (yes/No): ");
                char confirmation[5];
                fgets(confirmation,sizeof(confirmation),stdin);
                if(strcmp(confirmation,"yes\n")!=0){
                    continue;
                }
            }
            clearScreen();
            printf("What file do you want to load these notes from (without the extension)\n(just press enter to cancel): ");
            int max_name_size = 100;
            int name_index = 0; //the first character is already collected before the while loop
            char* file_name = (char*)malloc(sizeof(char)*max_name_size);
            char c;

            while((c = getchar())!='\n'){
                file_name[name_index++] = c;
                if(name_index > max_name_size - 2){ //2 to make sure it doesn't overflow
                    max_name_size*=2;
                    file_name = realloc(file_name, sizeof(char)*max_name_size);
                }
            }
            file_name[name_index] = '\0';
            
            if(strcmp(file_name,"")==0){
                free(file_name);
                continue;
            }

            clearScreen();
            struct Node* tmp_node = loadNotes(file_name);
            free(file_name);
            if(!tmp_node){
                printf("There was an error loading your notes\n");
                printf("Press enter to continue: ");
                getchar();
                continue;
            }
            printf("Your notes have been successfully loaded\n");
            printf("Press enter to continue: ");
            getchar();
            if(node){
                freeAllNodes(node);
            }
            node = tmp_node;
        }
    }

    if(node){
        while(node->prev!=NULL){
            node=node->prev;
        }
        freeAllNodes(node);
    }
    return 0;
}