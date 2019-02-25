/*
    New blockt.c

    I need to figure out a way to handle errors for massive text
    files. Other than that, this code is better than the old code.
    2017.5.19 Korgan Rivera
*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>
#define DEFAULT 80

/* Words from the file are
   stored in a linked list. */
typedef struct _list{
    unsigned length;
    char *p;
    struct _list *next;
}list;

int onlywhitespace(char *str);
void add_node(list **head, list **end, unsigned length);
unsigned error_calc(int rem);

int main(int argc, char **argv){

    // Make sure user calls program correctly.---------------------------------
    if(argc != 2 && argc != 3){
        printf("\nUSAGE: %s [textfile] <max width>", argv[0]);

        exit(0);
    }

    // Open file.--------------------------------------------------------------
    FILE *fp = fopen(argv[1], "r");
    if(fp == NULL){
        printf("File \'%s\' doesn't exist.", argv[1]);
        exit(1);
    }

    // Get file size. Quit if empty.-------------------------------------------
    fseek(fp, 0L, SEEK_END);
    long int size = ftell(fp);
    if(size == 0){
        puts("\nFile is empty.");
        exit(0);
    }
    rewind(fp);

    // Make buffer to hold the file.-------------------------------------------
    char *buffer;
    if((buffer = malloc(size + 1)) == NULL){
        puts("\nCan't malloc() :/");
        exit(1);
    }

    // Read file into buffer.--------------------------------------------------
    fread(buffer, 1, size, fp);
    buffer[size] = '\0';
    fclose(fp);

    // Check that there's at least one non-whitespace character.---------------
    if(onlywhitespace(buffer)){
        puts("\nFile contains only whitespace.");
        exit(0);
    }

    // Build linked list of words from the buffer.-----------------------------
    unsigned i, j, k;
    i = j = 0;
    unsigned max_width;
    unsigned char_count = 0;
    unsigned word_count = 0;
    unsigned longest_word = 0;
    list *head, *end;
    head = end = NULL;

    do{
        // Skip over initial whitespace if any.
        while(buffer[i] && isspace(buffer[i])) i++;

        // Find length of word that begins at buffer[i].
        for(j = i; buffer[j] && !isspace(buffer[j]); j++, char_count++);
        unsigned length = j - i;

        if(length == 0) break; // not sure about this line yet.

        // Create a new node to hold this word.
        add_node(&head, &end, length);

        // Read string from the buffer into the node.
        for(k = 0; i < j; end->p[k++] = buffer[i++]);
        end->p[k] = '\0';

        // Keep track of the longest word found so far.
        if(length > longest_word) longest_word = length;

    }while(buffer[j]);
    end->next = NULL;

    // Find new character count ignoring multiple whitespace characters.-------
    size = char_count + (word_count - 1);

    // If user gave a width on the command line, use it if possible.-----------
    max_width = DEFAULT;
    if(argc == 3){
        unsigned user_width = strtoul(argv[2], 0, 10);
        if(user_width >= 0 && user_width <= 500) max_width = user_width;
    }

    // If max width is smaller than longest word, abort.-----------------------
    if(max_width < longest_word){
        printf("\nWidth given is less than the\nlongest word in the text (%u).", longest_word);
        exit(1);
    }

    // If max_width > entire text, adjust max_width.
    if(max_width > size)
        max_width = size;

    // Find which width, from longest_word to max_width, is the best.----------
    unsigned smallest_error = UINT_MAX; // This is shitty. Fix it.
    unsigned best_width = 0;
    unsigned remainder;
    char firstword;
    list *scout;

    for(i = longest_word; i <= max_width; i++){
        unsigned error = 0;
        scout = head;
        remainder = i;
        firstword = 1;

        while(scout){

            /* If this is the first word in the line, calculate space left
               on the line and move to the next word and the next line. */
            if(firstword){
                remainder -= scout->length;
                firstword = 0;
                scout = scout->next;
            }

            else{
                /* If this is not the first word, and this word
                   won't fit on this line, add the remainder to
                   the running error, and move to the next line. */
                if(remainder < (scout->length + 1)){
                    error += error_calc(remainder);
                    remainder = i;
                    firstword = 1;
                }

                else{
                    /* If this is not the first word,
                       and the word will fit, add the
                       remainder to the running error
                       and move to the next word. */
                    remainder -= (scout->length + 1);
                    scout = scout->next;
                }
            }
        }

        // Take care of error on the last line.
        error += remainder;
        // Log this width if it's the best so far.
        if(error < smallest_error){
            smallest_error = error;
            best_width = i;
        }

        // If perfect solution found, break: can't beat that.
        if(smallest_error == 0){
            break;
        }
    }

    // Display the test using the best width.----------------------------------
    remainder = best_width;
    firstword = 1;
    scout = head;

    while(scout){
        if(firstword){
            printf("%s", scout->p);
            remainder -= scout->length;
            firstword = 0;
            scout = scout->next;
        }
        else{
            /* If this is not the first word, and this word
               won't fit on this line, add the remainder to
               the running error, and move to the next line. */
            if(remainder < (scout->length + 1)){
                putchar('\n');
                remainder = best_width;
                firstword = 1;
            }

            else{
                /* If this is not the first word,
                   and the word will fit, add the
                   remainder to the running error
                   and move to the next word. */
                printf(" %s", scout->p);
                remainder -= (scout->length + 1);
                scout = scout->next;
            }
        }
    }

    putchar('\n');

    // Free the linked list.---------------------------------------------------
    list *temp = head;
    scout = head->next;
    while(temp){
       free(temp->p);
        free(temp);
        temp = scout;
        if(scout) scout = scout->next;
    }
}

int onlywhitespace(char *str){
    /* returns 0 if at least one non-whitespace character
       is in the buffer, otherwise returns 1. */
    unsigned i = 0;
    while(str[i] && isspace(str[i])) i++;
    return (!str[i]);
}

void add_node(list **head, list **end, unsigned length){

        // If first word in list, malloc the root.
        if(*head == NULL){
            if((*head = malloc(sizeof(list))) == NULL){
                puts("\nadd_node(): malloc() fail.");
                exit(1);
            }
            *end = *head;

        }

        // If not first word, malloc space on the end of list.
        else{
            if(((*end)->next = malloc(sizeof(list))) == NULL){
                printf("\nadd_node(): malloc() fail");
                exit(1);
            }
            *end = (*end)->next;
        }

        // Malloc space for the string.
        if(((*end)->p = malloc((length + 1))) == NULL){
                printf("\nadd_node(): malloc() fail");
                exit(1);
        }

        (*end)->length = length;
}

unsigned error_calc(int rem){
    unsigned i, error = 0;

    if (rem == 0) return 0;
    return (rem * rem + 3 * rem) / 2;
}
