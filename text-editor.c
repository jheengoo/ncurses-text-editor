
#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define size 600000                     // total size of text which can be opened roughly around 0.6mb
int x=0, y=0;                           //x y current coordinates used in menu box portion only
bool search_marks=false;                // tells if search marks '[]' should be there or not


char* left(char *string, const int length);   //set length of string
void malloc_error(void);                      // see if memory is successfully allocated or not
//otext editor is the function which takes care of everything which happens in the text editor, all the key presses etc.,
int texteditor(char *text,int maxchars,const int startrow, const int startcol,int maxrows,const int maxcols,const int displayrows, const int returnhandler,const char *permitted, bool ins, const bool allowcr);
/*text stores the text at current instance, maxhars is the maximum number of characters text can store, start row and start col is the number of xy coordinates for editable area
maxrows is the maximum number of rows display area(upper box) will have maxcols is number of max columns display area can have,
 ins shows which mode(add or replace) is the writing text is in diplay area
 return handler is sued to select the behaviour when we press enter we have selected it to add a new line*/


void remove_search_marks(char* text);    //this function looks for search marks if present it'll remove them
int file_exist(char* path);              //this function will open a file in read mode at path to see if it exist it returns 1 or 0 based on file existence
void clr_bottom();                       // this function clears the areaa below menu so we can have interactive session with user to search, replace or save file etc.,
int save_file(char* text);               // this function saves the  text in file, it asks the user to give file path In case file at given path already exists it asks user if he want to over write and do so if user want to overwrite
int insert_string(int pos, char* string, char* data);  // this function inserts string in data  at given pos
int search_string(char *string, char* data);           // this function searches for string in the data variable and adds search marks([]) if it finds the text
int delete_string(int pos, int len, char *data);       // This functions delets the text of length len from data at given pos
int count_words_chars(char* data);                     // this funcition counts number of words and characters in the text variable at given time
int replace_string(char* data);                       // this function searches for a string in variable data and deletes it and then inserts the replacement string in that position

int main(int argc, char* argv[])
{
  /* This is the porition we ask user to run program with file path and mode to open*/
	FILE* fp;	//declare a file pointer to open file
	int  file_size;	//a variable to know file size, but it is useless so you may remove it (I have declared various such variables during program typing)
	int maxchars=size/*number of characters allowed to be opened*/,startrow=0/*cursor start row*/,startcol=0/*cursor start column*/;
	int maxrows=10000/*maximum number of rows allowed calculated by (maxchars/maxcols)*/,maxcols=60/*maximum number of rows allowed calculated by (maxchars/maxrows)*/;
	int displayrows=20/*number of rows displayed in text box(scroll added too)*/,returnhandler=1/*select behaviour for enter key press*/;
        char *message=malloc(maxchars*sizeof(char));  //allocate memory for our message string
        bool ins=false/*insert mode is initially false means text will be replaced in beginning*/,allowcr=true;
	if(argc<3)/*in case program is not given the necessary arguments*/
	{
		printf("usage: %s filename mode | modes can be \nopen new file with [w] open existing file with [r]\n\n",argv[0]);/* generate an error */
		return 0;/*and exit*/
	}
	if(strcmp(argv[2],"w")==0)/*if user asked toopen a new file*/
	{
		if(file_exist(argv[1]))/*check if file exist?*/
		{
			printf("file already exist : \n");/*in case file already exist, tell user about it*/
			return 0;/*and exit*/
		}
		else /*in case file didn't exist already*/
			fp=fopen(argv[1],"w+");/*create the file and assign it's address to fp*/
	}
	else if(strcmp(argv[2],"r")==0)/*in case user want to open an existing file*/
	{
		if(!file_exist(argv[1]))/*check if file exist*/
		{
			printf("file does not exist : \n");/*in case file didn't exist display the error*/
			return 0;/*and exit*/
		}
		else/*in case file exist*/
			fp=fopen(argv[1],"r+");/*open it*/
	}
	else/**in case user specified a mode other than r or w to open a file*/
	{
		printf("wrong mode specified\n");/*dsiplay error*/
		return 0;/*exit*/
	}
 /////////// WE checked above if user has done everythign right while runing the program
////////////////file work above

// if user did everythign right then we read the data from openend file here
	int i=0;/*just to count*/
	char c;/*just to read a character from file */
	if(fp!=NULL)/*if fp is pointing to a file*/
	{
		while((c=fgetc(fp))!=EOF)/*keep getting a charcetr from file unless it reaches end of file*/
			{message[i]=(char)c;i++;}/*add the charcetr to data structure message*/
		fclose(fp);/*close file when done traversing*/
	}
// read all the text from file in variable message
///////////////////////////////////reading
//// doing our text screen initialization
        initscr();    //initialize screen for text box
        noecho();	//disable echo so whatever we press shouldn't be randomly displayed on textbox
        cbreak();   //get all the key presses in our program except few specific signals
        keypad(stdscr,TRUE);//initialize our screen for keypad for example arrow keys and num keys
////////////////////////menu down
        move(20,0);      // down menu starts from y=20
        hline(ACS_CKBOARD,80);  // placing a horizontal line to separate menu from text area here
///////////////Now we print menu here
	mvaddstr(21,0,"Home = goto first character on current row");
	mvaddstr(21,50,"F3 to search and remove Text");
	mvaddstr(22,50,"F2 to delete all text");
	mvaddstr(22,0,"PgDn = goto last character in text string");
	mvaddstr(23,50,"F4 to replace Text");
	mvaddstr(23,0,"INS = insert mode on/off");
	mvaddstr(24,0,"PgUp = goto first character in text string");
        mvaddstr(24,50,"Press ESCAPE to quit");
	mvaddstr(25,0,"End = goto last character on current row");
        mvaddstr(25,50,"CTRL-Y to Delete Current Line");
        mvaddstr(26,0,"F6 to see number of words and characters");
	mvaddstr(26,50,"F5 = to save file");
	mvaddstr(27,0,"F7 to remove search marks");

///****************MENU above*/
////////////menu printed now call the most important function the text editor
        texteditor(message,maxchars,startrow,startcol,maxrows,maxcols,displayrows,returnhandler,NULL,ins,allowcr);
		/*with variables as explained above*/
// user exited text editor? clear and restrore original console windwo
        clear();//once returned from texteditor clear the screen and buffer
        endwin();//end window
//        printf("Message\n-------\n\"%s\"",message);
        free(message);    // free memory allocated for message
        return 0;//and exit
}

int texteditor(char *text, int maxchars, const int startrow, const int startcol,int maxrows,const int maxcols,const int displayrows,const int returnhandler,const char *permitted, bool ins, const bool allowcr)
{
	int ky,position=0,row,col,scrollstart=0, x=0, y=0;   //ky is most of the time the input user gives, position(it stays in sync with cursor) points to text row and col keep track of row or column being managed, scroll start keeps track of position from where scroll starts, x and y are xy coordinates on screen
	char *where, *original, *savetext,**display, *last_undo;  // where keeps the line of text for a while, original is the text as copied from text file
	bool exitflag=false;                                      // save text is used to keep a backup of our text so we may get previous text bac if something unusuall happens
                                                                  // last undo is useless // display is 2 dimensional array to keep enough text in it to fill the diplay window
	if (!maxchars)                                      // if user has not specified maxchars
		maxchars=maxrows*maxcols+1;

	if (!maxrows || maxrows > maxchars-1)              // if maxrows are not specified or maxchars are less then number of rows possible then make sure max rows are less then maxchars
		maxrows=maxchars-1;

	if (ins)                                         // if ins variable is set  
		curs_set(2);								//then set cursor to high visibility mode
	else											//else keep it normal
		curs_set(1);
     /* below we will allocate memory to our above declared arrays and assign data to them*/
	if ((display = malloc(maxrows * sizeof(char *))) ==NULL) /*assign memory equal to number of maxrows to display*/
		malloc_error();	//call the function to check if memory was successfully allocated
	for(ky = 0; ky < maxrows; ky++)		//now traverse all rows in display and allocated and assign memory equal to maxcols to each row of display
		if ((display[ky] = malloc((maxcols+1) * sizeof(char)))==NULL)
		        malloc_error();

	if ((original=malloc(maxchars*sizeof(char)))==NULL)	//allocated memory to original 
		malloc_error();
	strcpy(original,text);//copy text taken from file to original variable

	if ((savetext=malloc(maxchars*sizeof(char)))==NULL)// allocate memory to savetext 
		malloc_error();
	strcpy(savetext,text);//copy text to savetext
  /*finished assigning**/

	while (!exitflag) // our loop to stay in unless user presses a button to turn exit flag true initially it is set false
	{
		int counter; //counter to count
		do	//start of do while loop
		{
		        counter=0;
		        where=text;	//temporarily copy all text taken from file to where

		        for (row=0; row < maxrows; row++) // initialize display vriable making sure every row has a null in the start
		        {
		                display[row][0]=127; //assign a backspace character to first column of every row
		                display[row][1]='\0'; //assign a null character to second column of every row indicating that the row ends here
		        }

		        row=0;    // set row to zero
		        while ((strlen(where) > maxcols || strchr(where,'\n') != NULL) &&  (display[maxrows-1][0]==127 || display[maxrows-1][0]=='\n')) //while loop
				//keep traversing if both of following conditions are met successfully				
				//1. length of text(where) is more than number of columns which means there are more than one rows
				//   "OR" if a new row exist
				//2. if first column of last row has a backspace OR an end of line character(means we haev not reached last row yet)
		        {
		                strncpy(display[row],where,maxcols); //copy text from where to current row of display from where specifyint he maximum columns it can have
		                display[row][maxcols]='\0';          // insert a null at the end of a row indicating that the row ends here
		                if (strchr(display[row],'\n') != NULL)  // if there s a new line character in this row
		                        left(display[row],strchr(display[row],'\n')-display[row]);//end the row where a new line character is found
		                else									//in case there is no newline character in this row
		                        left(display[row],strrchr(display[row],' ')-display[row]);//mark end of row at next white space charcter
		                if (display[maxrows-1][0]==127 || display[maxrows-1][0]=='\n')//if first column of last row has a backspace OR an end of line character
		                {
		                        where+=strlen(display[row]);       // move where pointer to the address before which text is already dealt with
		                        if (where[0]=='\n' || where[0]==' ' || where[0]=='\0')    //in case where points to and of newline characters
		                                where++;	 //move one step forward
		                        row++;	 //current is dealt with now we move to next row
		                 }
		        }// while loop ends here text is assigned to display datas tructure properly
		        if (row == maxrows-1 && strlen(where) > maxcols) // if we have reached last row and remaining text is more than number of characters last row can have
		        {
		                strcpy(text,savetext);	//copy the savetext to text
		                if (ky==127/*KEY_BACKSPACE*/)	//if ky is KEY_BACKSPACE
		                        position++;	//increase position variable
		                counter=1;			//set counter to be one since there is text remaining to be copied
		        }
		}while (counter);//do while loop ends here
		strcpy(display[row],where);//now copy remaining text from where to display row

		ky=0;				//set ky to be 0
		if (strchr(display[maxrows-1],'\n') != NULL)	//if first character of last row is a newline character
		        if (strchr(display[maxrows-1],'\n')[1] != '\0')	//and second charcter is a newline character
		                ky=127/*KEY_BACKSPACE*/;						//then set ky to be equal to backspace

		col=position;		//set current column to be equal to position of cursor we are at
		row=0;				// set row to pont to fisrt row
		counter=0;			//set counter to be 0
		while (col > strlen(display[row]))	//keep traversing if column is greator than number of columsn in current row
		{
		        col-=strlen(display[row]);	//set column= column - number of columns of current row
		        counter+=strlen(display[row]);		//set counter to point at the end of current row
		        if (text[counter] ==' ' || text[counter]=='\n' || text[counter]=='\0')	//in case text at current cursor position is a newline character
		        {
		                col--;	//decrease col by one
		                counter++;	//point to text charcter in text
		        }
		        row++;	//increase row by one since current row has been properly taken care of
		}
		if (col > maxcols-1)	//in case col is greator than total number of columns a row can hev
		{
		        row++;	//move to next row means increment row variable
		        col=0;	//set col = 0 means point to first colunmn in next row
		}

		if (!ky) // otherwise  ky==KEY_BACKSPACE and we're getting rid of the last character we entered to avoid pushing text into the last line +1
		{
		        if (row < scrollstart)	// in case current row is lower than scroll start
		                scrollstart--;  // then decrease scroll start
		        if (row > scrollstart+displayrows-1) //in case current row is greator than scroll start plus number of displayrows
		                scrollstart++;	//then increase scroll start
		        for (counter=0;counter < displayrows; counter++)	//traverse all the display rows
		        {
		                mvhline(counter+startrow,startcol,' ',maxcols);	//move to position counter+startrow on y coordinate and startcol on x coordinate and print spaces on all columns onwards from counter of this row
		                if (display[counter+scrollstart][0] != 127)		//if current position has a backspace in it then
		                        mvprintw(counter+startrow,startcol,"%s",display[counter+scrollstart]);//move to position counter+startrow on y and startcol on x and display text in display area
		        }
		        move(row+startrow-scrollstart,col+startcol);     // move to end of text

		        ky=getch(); // text has been displayed  now wait for user to press a key
		}

		switch(ky)     // take actions based on pressed key
		{
			case 27: //escape
				exitflag=true;	//set exitflag to be true
				break;			
		        case KEY_F(2):  //f2 key to delete all file contents
				memset(text,'\0',maxchars);	//make all the contens of file to be null
		                position=0;	//set postion to be zero
		                scrollstart=0;	//set scroll start to be zero
		                break;
			case KEY_F(3):   //f3 key to search for strings
				getyx(stdscr,y,x);//get current coordinates of cursor
				remove_search_marks(text);//remove searchmarks from text
				search_string(NULL,text);	//goto search function
				clr_bottom();				//clear the interactive area below menu
				move(y,x);					//move back to textbox
				break;
 			case KEY_F(4):    //f4 to replace strings
				getyx(stdscr,y,x);	//get current coordinates of cursor
				replace_string(text);//call the function to replace string
				clr_bottom();	
				move(y,x);
				break;
			case KEY_F(5):  //f5 key to save file
				getyx(stdscr,y,x);
				save_file(text);	//call the function save file
				clr_bottom();
				move(y,x);
				break;
			case KEY_F(6):   //f6 key to count number of words
				clr_bottom();
				count_words_chars(text); //call the function to count words and chars in text
			case KEY_F(7):   //f7 key to remove search marks
				remove_search_marks(text); //remove search marks
		        case KEY_HOME:	//move to start of current row
		                if (col)	//if current column is not the first column
		                {
		                        position=0;//set position to be zero
		                        for (col=0; col < row; col++)//traverse until col is less then row
		                        {
		                                position += strlen(display[col]);//move cursor to end of current row in display array
		                                if ((strchr(display[row],'\n') != NULL) || (strchr(display[row],' ') != NULL))//if there is a escape character ' ' or '\n' in current row
		                                        position++;		// increment position and move to start of the row
		                        }
		                }
		                break;
		        case KEY_END:	//move to end of current row
		                if (col < strlen(display[row]))	//if col is less than number of columns in current row
		                {
		                        position=-1;	//set potiion to -1
		                        for (col=0; col <=row; col++)	//traverse until col is less than or equal to row (row is position of current row)
		                        {
									position+=strlen(display[col]);//set position to be the end of curent row
									if ((strchr(display[row],'\n') != NULL) || (strchr(display[row],' ') != NULL)) //if we haev reached enf of row
											position++;	//increment position so it may move to end of current row
		                        }
		                }
		                break;
		        case KEY_PPAGE:	//move to start of text
		                position=0;	//set position to be zero 
		                scrollstart=0;	//set scroll start to be zero
		                break;
		        case KEY_NPAGE:	//move to end of text
		                position=strlen(text);	//set postion to be end of text
		                for (counter=0; counter < maxrows; counter++)	//traverse until counter is less than total number of rows
		                        if(display[counter][0]==127)	//if the row at position counter's first character is backspace
		                                break;	//then break from loop
		                scrollstart=counter-displayrows;	//else set scroll start to equal counter-displayrows
		                if (scrollstart < 0)	//in case scroll start has decreased to become negative
		                        scrollstart=0;	//set it to be zero again
		                break;
		        case KEY_LEFT:	//move cursor left
		                if (position)	//if position is not zero
		                        position--;	//decrease by one
		                break;
		        case KEY_RIGHT:	//move cursor right
		                if (position < strlen(text) && (row != maxrows-1 || col < maxcols-1))//if cursor is not already at end or far from end
		                        position++;		//increase position by one
		                break;
		        case KEY_UP:	//move cursor up
		                if (row)	//in case we are not on first row
		                {
		                        if (col > strlen(display[row-1]))//if current column is greater then length of previous row
		                                position=strlen(display[row-1]);	//then set cursor at the end of above row
		                        else
		                                position=col;	//else set position to col
		                        ky=0;	//set ky=0
		                        for (col=0; col < row-1; col++)	//traverse for col less than row above current row
		                        {
		                                position+=strlen(display[col]);	//move cursor to end of column
		                                ky+=strlen(display[col]);	//move ky to next row
		                                if ((strlen(display[col]) < maxcols) || (text[ky]==' ' && strlen(display[col])==maxcols))//if we are not already at end of row
		                                {
		                                        position++;	//increment position
		                                        ky++;	//move to next row
		                                }
		                        }
		                }
		                break;
		        case KEY_DOWN:// move cursor to below row
		                if (row < maxrows-1)	//if it's not last row
		                        if (display[row+1][0] !=127)	//if next row is not empty
		                        {
		                                if (col < strlen(display[row+1]))	//if col is less than length of next row
		                                        position=col;	//set position to be col
		                                else
		                                        position=strlen(display[row+1]);//set position to be end of above row
		                                ky=0;//set ky=0;
		                                for (col=0; col <= row; col++)//traverse until we reach the current row
		                                {
		                                        position+=strlen(display[col]);//move to next row
		                                        ky+=strlen(display[col]);//move to next row
		                                        if ((strlen(display[col]) < maxcols) || (text[ky]==' ' && strlen(display[col])==maxcols))//if we are not already at end of row
		                                        {
		                                                position++;
		                                                ky++;
		                                        }
		                                }

		                        }
		                break;
		        case KEY_IC: // insert key
		                ins=!ins;//invert the insert mode, turn it on if it's on or vice versa
		                if (ins)
		                        curs_set(2);
		                else
		                        curs_set(1);
		                break;
		        case 330://KEY_DC:  // delete key
		                if (strlen(text))//if text exist
		                {
		                        strcpy(savetext,text);//save text 
		                        memmove(&text[position],&text[position+1],maxchars-position);//move text to left by one character meanwhile eating the one character, search google for function memove and read about it...
		                }
		                break;
		        case KEY_BACKSPACE:// remove text				
		                if (strlen(text) && position)//if text exist and cursor is already not at begiing of text
		                {
		                        strcpy(savetext,text);//save text 
		                        position--;//decrease poition by one basically making cursor go back
		                        memmove(&text[position],&text[position+1],maxchars-position);//move text to left by one character meanwhile eating the one character
		                }
		                break;
		        case 25: // ctrl-y delete entire line
				if (display[1][0] != 127)//if there's text on first row
		                {
		                        position-=col;//move cursor to start of first row
		                        ky=0;//set ky=0
		                        do
		                        {
		                                memmove(&text[position],&text[position+1],maxchars-position);//move text right by one
		                                ky++;//increase ky
		                        }
		                        while (ky < strlen(display[row]));//keep doing it until ky is less than number of columns in current row
		                }
		                else
		                        memset(text,'\0',maxchars);//other wise it's already empty
		                break;
		        case 10: // return        // insert a line by pressing return button
				switch (returnhandler)//see what do we want enter to do
		                {
		                        case 1:// in case return handler==1
		                                if (display[maxrows-1][0] == 127 || display[maxrows-1][0] == '\n')// if display area is not empty
		                                 {
		                                        memmove(&text[position+1],&text[position],maxchars-position);//move text to right by one, emptying the space for one charectre at potiions
		                                        text[position]='\n';	//insert a new row character at position in text
		                                        position++;//increase poition
		                                 }
		                                break;
		                        case 2:
		                                ky=' ';//insert a space
		                                ungetch(ky);//
		                                break;
		                        case 3:
		                                exitflag=true;//exit
		                }
		                break;
		        default: //in case of any other characters than specified above
					if (((permitted==NULL && ky > 31 && ky < 127) || (permitted != NULL && strchr(permitted,ky))) && strlen(text) < maxchars-1 && (row !=maxrows-1 || (strlen(display[maxrows-1]) < maxcols || (ins && (row!=maxrows-1 && col < maxcols )))))
					{
						if (ins || text[position+1]=='\n' || text[position]== '\n')    //in ase insert mode is on or there is no text after this in current row
								memmove(&text[position+1],&text[position],maxchars-position);//move text to right by one and add characters
						text[position]=ky;//insert current key in text
						if (row != maxrows-1 || col < maxcols-1)//in case insert mode is off and we're not at the end of row
								position++;//increment position(move cursor to right)
		            }
		        }
				if(!allowcr)//if isnert mode is allowed
		        if (text[0]=='\n') //if first char in text is a newline
		        {
		                memmove(&text[0],&text[1],maxchars-1);//movetext to left by one
		                if (position)//if position is not zero
		                        position--;//decrease position
		         }
		}//switch ends here
/*user decided to exit free all assigned memory    */
	free(original);//free memory of original
	free(savetext);//free memory of savetext
	free(last_undo);//free memory of last undo, it's not used at all so no need for it
	for(scrollstart = 0; scrollstart < maxrows; scrollstart++)//traverse all rows of siplay and free their memory
		free(display[scrollstart]);
	free(display);//free dispay
	/************************/

	
	return 0; 
}


char* left(char *string, const int length)
{
	if (strlen(string) > length)   // if length of string is more than length then add a null to string to shorten its length to length
		string[length]='\0';
	return string;
}

void malloc_error(void)      // in case of mallox error just exit
{
	endwin();//end window
	fprintf(stderr, "malloc error:out of memory\n");//print error
	exit(EXIT_FAILURE);//generate signal for exit and exit
}

/***/
int file_exist(char* path)
{
     FILE *fp = fopen(path,"r");   // open file in read mode
     if(fp==NULL)                   // couldn't open mean file not existing
        return 0;
     fclose(fp);   //close file if opened means file exists
     return 1;
}
void clr_bottom()                      // scroll from 29th row to max number of rows to set it to blank spaces,, it basically used to clear the text
{
	int i=0, j=0; //counters
	int max_y, max_x; // max coordinates we can haev on scren
	getmaxyx(stdscr,max_y, max_x);// get maximum coordinates
	for(i=29; i<max_y; i++) //start from 29 and traverse unless we rach end of screen
	{
		for(j=0; j<max_x; j++) //traverse all columns of each row
		{
			move(i,j); //move to right
			addch(' '); //insert a space 
		}
	}
}

int save_file(char* text)   //save file
{
     char path[200], ch_over, spath='a'; int sp; //path is the path to file, ch_over is just to input a character spath is a condition variablle
	int i=0;
     FILE* fps;	//pointer for file
     move(33, 0);//move cursor to lower area
     printw("Enter path to save the text: ");   // ask for path
     while(spath!='\n')       // getting path
     {
		spath=getch();	//get path
		sp=(int) spath; //get code verion of pressed key
	if((sp>=65 && sp<=90) || (sp>=48 && sp<=57) || (sp>=97 && sp<=122) || sp==95 || sp==46) 
	{
		addch(spath); //if it is an alphanumeric chaacter
		path[i]=spath;i++;//add to path variable
	}
	if(sp==27 )//escape key)        // press escape to abort entering path
	{
		move(33,0); deleteln();	//delete line fromm below area
		return 0;
	}
     }
     path[i]='\0';//add null to end of path variable
     if(file_exist(path))        // in case file is already there do this
     {
	move(33, 0);deleteln();   //deleet line form menu
	
	printw("\nFile with this name already exist, Press 'y' to over write?");
	ch_over=getch();
	addch(ch_over);
        if(ch_over=='y')//if user pressed y
        {
          fps=fopen(path,"w+");//open file
          fprintf(fps,"%s",text);//write text to file
          fclose(fps);//close file
	  move(33, 0);deleteln();//delete line in menu
	  printw("\nFile saved ");
          return 1;
        }
     	move(33, 0);deleteln();
     	printw("\nFile not saved                                                         ");	//if did not user press y then tell him it's not saved
	return 0;
     }
     fps=fopen(path,"w+");            // file wasn't there open a new file
     fprintf(fps,"%s",text); //save text to file
     move(33, 0);deleteln();	//delete line from menu
     printw("\nFile saved                                                         ");//tell user file saved
     fclose(fps);//close file
     return 1;
}
///////////////////

int insert_string(int pos, char* string, char* data)
{
    
    int i=pos, j=0;
    if(strlen(data)==0)                // if data has no charcters thendo this
    {
	for(i=0; i<strlen(string); i++) //traverse string which has to be inserted in data
	{
	   data[i]=string[i]; //append strng to data
	}
    }
	                            // if data has some text in it already   then make space to insert text
	else
	{
	    for(i=strlen(data); i>=pos; i--)//keep moving until given position arrives
	    {
	       data[i+strlen(string)]=data[i]; //move text to right making space at the required position for string
	    }
	    for(i=pos; i<pos+strlen(string); i++, j++)//above loop has made space no isert string in that position
	    {
		if(string[j]!='\0')
		  data[i]=string[j];
	    }
	}
    return 0;
}


int search_string(char *string, char* data)
{
    int i=0, j=0, start[100], end[100], n_o_occur=0, maxchars=size;
	char ch='a', s_a='n', st[2]="[", en[2]="]";
	move(32,0);	
    if(string==NULL)          // if user has not given any string to search then ask him to enter one
    {
		s_a='y';
	     string = malloc(500*sizeof(char));
	     printw("Enter String to search for: ");
	     while(ch!='\n')
	     {
		ch=getch();
		addch(ch);
		string[i]=ch;i++;
		if(ch==127) //bakspace
		{
			getyx(stdscr,y,x);
			move(y,x-1);	
		}		
		if(ch==27 )//escape key)
		{
			move(33,0); deleteln();	
			return 0;
		}
	     }
	     string[i]='\0';
    }
    int flag=0, len1, len2;    
    len1 = strlen(string);
    len2 = strlen(data);
    if (len1 > len2){       // if seach string is greator than data then it's wrong string so exit
        return 0;
    }
    for (i = 0; i <= len2; ++i){                  // start loop and traverse thorough data and match string
        if (string[0]==data[i]){                 // if first character of string is found in data
            flag = 1;                                          //set flag to be one
            for (j = 1; j<(len1-1) && (i+j)<strlen(data); ++j){   // see if remaning characters are also there then
                if (string[j]!=data[i+j]){
                    flag=0;               // all characters were not matched set flag to 0 and start searching in next part of data
                    break;
                }
            }
            if((flag==1 || (i+j)>strlen(data)) && s_a=='n')        // if user is sarching for a string then just tel positio of first rting
            {
	    	break;
	    }
            if((flag==1 || (i+j)>strlen(data)) && s_a=='y')              // if searching for a string in all data then keep going and insert the search marks [] in text
            {
		insert_string(i,st,data); insert_string(i+j+1,en,data);i=i+1;
		search_marks=true;
	    }
        }
    }
    if (flag==1){
	return i;
    }
    return -1;
}
int delete_string(int pos, int len, char *data)   // delet a string at pos of length len from data
{
    int i;

    i=pos;
    for(i=pos; i<strlen(data)-len; i++)   // taraverse here
    {
      data[i]=data[i+len];               // keep moving backwards
    }
	data[i]='\0';
    return 0;
    
}

int count_words_chars(char* data)
{
    int i=0, wrd_count=0;

    for(i=0; i<strlen(data); i++)
    {
      if(i==0)
              wrd_count++;
      if(data[i]==' ' || data[i]=='\n')    // saecg for new line or space and mark word++
      {
         wrd_count++;
      }
    }
    move(34,0);
    printw("Characters: %d, Words: %d",strlen(data), wrd_count);
    return 0;
}


int replace_string(char* data)
{
    int pos, i=0, j=0, start[100], end[100],n_o_occur=0, maxchars=size;
    char string[500], *replace_with, ch='a';
	replace_with=malloc(500*sizeof(char)); 
     move(32, 0);deleteln();
     printw("Enter String to replace: ");          // take the string to replace
     while(ch!='\n')
     {
	ch=getch();
	addch(ch);
	if(ch!='\n'){string[i]=ch;i++;}
	if(ch==27 )//escape key)
	{
		move(33,0); deleteln();	
		return 0;
	}
     }
     string[i]='\0';
	ch='a';
     move(33, 0);
     i=0;printw("Enter replacement string: ");           // take the string to replace with
     while(ch!='\n')
     {
	ch=getch();
	addch(ch);
	if(ch!='\n')
	{replace_with[i]=ch;i++;}
	if(ch==27 )//escape key)
	{
		move(33,0); deleteln();	
		return 0;
	}
     }
	deleteln();
     replace_with[i]='\0';
    while((pos=search_string(string, data))!=-1)  // now search the string to be replaced and get its position
    {		
	    delete_string(pos, strlen(string), data);	 // delete the searched string dfrom its position
            insert_string(pos, replace_with, data);	 // insert the replacement string at that position
    }
	free(replace_with);
    return 0;
}
void remove_search_marks(char* text)              // fnd [] marks and remove them
{
	int i;
	if(search_marks==true)                // if search marks exist then remove them and tell search marks don't exist anymore
	{
		search_marks=false;
		for(i=0; i<strlen(text); i++)
		{		
			if(text[i]=='[' || text[i]==']')
			{
				delete_string(i,1,text);
			}
		}
	}
}


/*****/

