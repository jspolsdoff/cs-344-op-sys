#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>


// all programs function prototypes
char *createDir(int processID);
void shuffle(char **array, size_t n);
struct Rooms generate(char *rooms_dir); 
void adventureFunc(struct Rooms pos);

struct Rooms {
	char *start;
	char *end;
	char *path;
};

int main(void) {
	
	// need a random number, here it is
	srand(time(NULL));
	
	// going to need the pid to generate rooms is specified format
	int pid = getpid();
	
	// time to make the room folder
	char *rooms_folder = createDir(pid); 
	
	// time to make the rooms
	struct Rooms pos = generate(rooms_folder);
	
	// function to handle actual game functionality
	adventureFunc(pos);
	
	// time to free the memory
	free(rooms_folder);
	
	return 0;
}

char *createDirectory(int processID) {
	int buffer_size = 20;
	char *dir_name = malloc(buffer_size);
	char *file_prefix = "spolsdoj.rooms.";
	
	snprintf(dir_name, buffer_size, "%s%d", file_prefix, processID);
	
	struct stat st = {0};
	if(stat(dir_name, &st) == -1) {
		mkdir(dir_name, 0755);
	}
	
	return dir_name;
}

// generate and link all of the rooms
struct Rooms generate(char *rooms_dir) {
	struct Rooms pos;
	
	char *connection;
	int connection_count; 
	int idx; 
	char currentRoom[100]; 
	
	pos.path = rooms_dir;
    char *room_names[10];
    
		// here are my semi cheesy room names
		room_names[0] = "Meadow";
    room_names[1] = "Dungeon";
    room_names[2] = "Bridge";
    room_names[3] = "Grassland";
    room_names[4] = "Town";
    room_names[5] = "City";
    room_names[6] = "Forest";
    room_names[7] = "Mountains";
    room_names[8] = "Graveyard";
    room_names[9] = "Mine";

    int bufSize = 128; 
    char *current_file = malloc(bufSize);
    int i; 
		int j;

		// make sure that rooms are different for each game
    shuffle(room_names, 10); 

		// loop through rooms and pick seven rooms and create files
    for(i = 0; i < 7; i++) { 
			snprintf(current_file, bufSize, "%s/%s", rooms_dir, room_names[i]); 
			FILE *f = fopen(current_file, "w"); 
			if (f == NULL) {
				perror ("Error opening file.\n");
			}
			else {
				fprintf(f, "ROOM NAME: %s\n", room_names[i]); 
			}
			fclose(f);
    }

		// place the seven rooms in an array
    char *new_names[7];
    for(i = 0; i < 7; i++) { 
    	new_names[i] = room_names[i];
    }

		// randomly pick the start and end room
    int startPos = rand() % 7;
    int endPos = rand () % 7;
    while (startPos == endPos) endPos = rand() % 7;

    for(i = 0; i < 7; i++) { 
    	shuffle(new_names, 7); 
        snprintf(current_file, bufSize, "%s/%s", rooms_dir, room_names[i]); 

        FILE *f = fopen(current_file, "a"); 
        if (f == NULL) {
        	perror ("Error opening file.\n");
        } else {
            connection_count = rand() % 4 + 3; 

            idx = 0;
            
						for (j = 0; j < connection_count; j++) { 
            	connection = new_names[idx];
            	
							if (connection == room_names[i]) { 
            		idx++;
            		connection = new_names[idx];
            	}
            	fprintf(f, "CONNECTION %d: %s\n", j+1, connection);
            	idx++;
            }
            if (i == startPos) {
            	fprintf(f, "ROOM TYPE: START_ROOM\n");
            	pos.start = room_names[i];
            }
            else if (i == endPos ) {
            	fprintf(f, "ROOM TYPE: END_ROOM\n");
            	pos.end = room_names[i];
            } 
						else {
							fprintf(f, "ROOM TYPE: MID_ROOM\n");
						}
        }
        fclose(f);
    }
    free(current_file); 
    return pos;
}

// function to make the directory with process id in correct format
char *createDir(int processID) {
	int buffer_size = 20;
	char *dir_name = malloc(buffer_size);
	char *prefix = "spolsdoj.rooms.";
	
	snprintf(dir_name, buffer_size, "%s%d", prefix, processID);
	
	struct stat st = {0};
	if (stat(dir_name, &st) == -1) {
		mkdir(dir_name, 0755);
	}
	return dir_name;
}

// array shuffle function adapted from Stack Overflow thread
void shuffle(char **array, size_t n) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    int usec = tv.tv_usec;
    srand48(usec);

    if (n > 1) {
        size_t i;
        for (i = n - 1; i > 0; i--) {
            size_t j = (unsigned int) (drand48()*(i+1));
            char* t = array[j];
            array[j] = array[i];
            array[i] = t;
        }
    }
}

// game loop start
void adventureFunc(struct Rooms pos) {
	
	// set up some rooms
	char *current_room = pos.start; 
	char *end_room = pos.end; 
	char *dir = pos.path; 
	int step_count = 0; 
	
	int i;
	int valid; 
	int buffer_size = 128; 
	
	int c; 
	int nl_char; 
	valid = 0;
	
	// need variable to hold rooms names
	char str[20]; 
	
	// variables to store game loop information
	char (*steps)[15] = malloc(sizeof *steps * 8); 
	char (*contents)[15] = malloc(sizeof *contents * 8); 
	char destination[15]; 
  char *current_file = malloc(buffer_size); 

	while (!(strcmp(current_room, end_room)) == 0) {
		snprintf(current_file, buffer_size, "%s/%s", dir, current_room); 
        // need to open the file
				FILE *f = fopen(current_file, "r"); 

        nl_char = 0; 

        // loop through and count the new lines
        if (f) {
		    while ((c = getc(f)) != EOF){
		        if (c == '\n') 
		        	nl_char++;
		    }
		}
		nl_char = nl_char - 2; 

		// make sure the pointer is set right after room name
		fseek(f, 11, SEEK_SET); 
		
		// get the room name
		fgets(str, 20, f); 

		int len = strlen(str);
		if(str[len-1] == '\n') 
		    str[len-1] = 0;
		strcpy(contents[0], str);

		// get the room connections
		for(i = 1; i <= nl_char; i++){
			fseek(f, 14, SEEK_CUR); 
			fgets(str, 20, f); 
			
			// remove newline
			len = strlen(str);
			if(str[len-1] == '\n')
			    str[len-1] = 0;
			strcpy(contents[i], str);
		}

		while(valid == 0) {
			printf("CURRENT LOCATION: %s\n", contents[0]);
			printf("POSSIBLE CONNECTIONS: ");
			
			for(i = 1; i <= nl_char; i++){
				if (i == nl_char) {
					printf("%s.\n", contents[i]);
				}		
				else {
					printf("%s, ", contents[i]);
				}	
			}

			// ask the user where to go
			printf("WHERE TO? >");
			
			// use the user input
			scanf("%s", destination); 
			
			for(i = 1; i <= nl_char; i++){ 
				// if true than the room is a connection
				if (strcmp(destination, contents[i]) == 0) { 
					valid = 1;
					current_room = destination;
				}
			}
			// hit the user with the error message in the hw if they don't enter the a valid room
			if (valid != 1) 
				printf("\nHUH? I DON’T UNDERSTAND THAT ROOM. TRY AGAIN.\n\n");
		}
		printf("\n");
		strcpy(steps[step_count], current_room); 
		
		step_count++; 
		fclose(f);
	}

	// if the player is out of the loop than they have won
	printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");
	printf("YOU TOOK %d STEP(S). YOUR PATH TO VICTORY WAS:\n", step_count);
	
	// let the user know how many steps it took
	for (i = 0; i < step_count; i++) {
		printf("%s\n", steps[i]);
	}

	// free up memory
	free(steps);
	free(contents);
	free(current_file); 
}
