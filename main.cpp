#include "FEHLCD.h"
#include "FEHImages.h"
#include "FEHRandom.h"
#include "FEHUtility.h"
#include <math.h>
#include <string.h>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
using namespace std;


struct Position{
    float x; 
    float y;
};
struct Size{
    int width;
    int height;
};
enum spritePath{
    left1,
    left2,
    right1,
    right2
    
};

//Creates a class for a basic rectangular hitbox
class HitBox {
    public:
    //defines the hitbox's position and size
    Position position;
    Size size;
    //basic constructor which builds hitbox
    HitBox(int x, int y, int width, int height){
        position.x = x;
        position.y = y;
        size.width = width;
        size.height = height;
    }

    void updatePosition(int x, int y){
        position.x = x;
        position.y = y;
        //LCD.DrawRectangle(x,y,size.width, size.height);
    }
    //Checks if a xy coordinte is within the hitbox, if it is, return true, else, return false
    bool checkClick(int hitX, int hitY){
        if(hitX >= position.x && hitX <= position.x+size.width){
            if(hitY >= position.y && hitY <= position.y+size.height){
                return true;
            }
        }
        return false;
    } 
};

//Basic button with a border, text, and a hitbox
class Button {
    public:
    Position position;
    Size size;
    int id;
    //32 char-limit on text because I dont feel like implementing strings
    char text[33];
    //Initializes a basically nonexistent hitbox because C got mad at me for not including it. 
    HitBox hitbox = HitBox(0,0,0,0);
    

    //Constructor that assigns the position/size, as well as drawing the button
    Button(int x, int y, int width, int height, char text[], int id){
        position.x = x;
        position.y = y;
        size.width = width;
        size.height = height;
        this->id = id;
        hitbox = HitBox(x,y,width,height);

        strcpy(this->text, text);
        LCD.DrawRectangle(x,y,width,height);
        LCD.WriteAt(text,x,y);
        LCD.Update();
    }
    //Constructor that assigns the position/size. Made because we don't always want to draw a button right when it is made.
    Button(int x, int y, int width, int height, int id){
        position.x = x;
        position.y = y;
        size.width = width;
        size.height = height;
        this->id = id;
        hitbox = HitBox(x,y,width,height);
    }
    //Changes Button's text value to given value.
    void changeText(char change[33]){
        //fix bug where it draws on top of already existing text later
        strcpy(text, change);
    }
    //Draws the border/writes the text
    void drawButton(){
        LCD.DrawRectangle(position.x,position.y,size.width,size.height);
        LCD.WriteAt(text,position.x,position.y);
        LCD.Update();
    }

};

//Created by Connor McGuffey
class Duck{
    public:
    Position position;
    Size size;
    HitBox hitbox = HitBox(0,0,0,0);
    
    FEHImage sprite;
    spritePath path; 

    Position* flightpath;
    bool dead;
    int direction, progress, speed, flapRate; //either 0 or 1, 0 for spawning on left side going right, 1 for spawning on right side going left. 
    Duck(int x,int y,int width, int height, int speed){
        position.x = x;
        position.y = y;
        size.width = width;
        size.height = height;
        this-> speed = speed;

        hitbox = HitBox(x - width/4,y - height/4,width,height);     
        flapRate = (int)(speed/10);   
        changeDirection();
        dead = false;
        progress = 0;

    }
    //Generates N positions for the duck to follow on its path
    void generateRandomFlightPath(int n){
        Position startPosition;
        //Creates a starting position. 
        //Since direction is only ever 0 or 1, it can be used as a "multiplier" That makes the starting x direction either 0 or 320
        startPosition.x = 4*direction;
        //Creates the starting y position. Can be a value from 0 to the max screen value minus the height of the duck. This is to
        //prevent screen overlap
        startPosition.y = Random.RandInt() % (240-size.height);
        //Creates a linear coefficient that can be between 0 and 3
        float coefficient = (Random.RandInt() % 3) / (float)(Random.RandInt() % 6 + 1);
        //generates a new random position for the duck
        changeDirection();

        //Prevents ducks that spawn near the bottom of the screen from going straight down
        //Only kinda works????
        if(startPosition.y > 120){
            coefficient *= -1;
        }
        
        //C gave me errors when using a regular array here earlier, I fixed the error but it's too late to switch the syntax now
        flightpath = (Position*)malloc(n*sizeof(Position));
        //Creates a factor to keep the x and y values between 0 and their respecive max. Since parabolic flightpaths were never implemented
        float xFactor = n/300.0, yFactor = (coefficient*n + startPosition.y)/240.0;
        //If the maximum x value that can be caluculated is greater than the maximum y value of the screen, then 
        //Change the x factor to make x values ranging from 0 to whichever x value makes y=240
        if(coefficient*300 > 240){
            xFactor = n/(240/coefficient);
        }
        //Does the same thing but for the top half of the screen. Since y=mx+b, when the maximum y value is less than 0, change the 
        //xfactor such that the maximum x value is where y=0. 
        else if(coefficient*300+startPosition.y < 0){
            xFactor = (n/(-startPosition.y/coefficient));
        }
        //Generates positions for when the ducks go left
        if(direction){
            int counter = 0;
            for(int i = n; i > 0; i--){
                flightpath[counter].x = i/xFactor; //creates n values that range from 300-0
                flightpath[counter].y = (coefficient*i + startPosition.y); //creates n y values that range from 0-240 using y=mx+b 
                counter++;  
            }

        }
        else{
            //generates positions for when the ducks go right
            for(int i = 0; i < n; i++){
                flightpath[i].x = i/xFactor; //creates n values that range from 0-320
                flightpath[i].y = (coefficient*i + startPosition.y); //creates n y values that range from 0-24   
            }
        }

    }
    //Created by Cole Gibson
    void updatePosition(int x, int y){
        position.x = x;
        position.y = y;
        //Upades the hitbox position as the duck moves
        hitbox.updatePosition(x - size.width/4, y - size.height/4);
        if(progress % flapRate == 0){
            //Switch case handles the animation of the ducks flying
            //Changes images periodically based on path
            switch (path)
            {
            case 0: 
                path = left2;
                sprite.Open("PIC_files/duck3FEH.pic");
                break;
            case 1:
                path = left1;
                sprite.Open("PIC_files/duck1FEH.pic");
                break;
            case 2:
                path = right2;
                sprite.Open("PIC_files/duck4FEH.pic");
                break;
            case 3:
                path = right1;
                sprite.Open("PIC_files/duck2FEH.pic");
                break;
            }   
        }
        //Draw sprite and increase the ducks progress meter
        sprite.Draw(x,y);
        progress++;
    }
    void Kill(){
        //Free allocated memory
        free(flightpath);
        sprite.Close();
    }
    void increaseSpeed(){
        //Increases the duck's speed
        speed -= 10;
        //Increases the duck's flaprate
        flapRate = (int)(speed/10);
    }

    private: 
    void changeDirection(){
        //Generates a 50/50 shot of which screen side the duck will spawn in on (left/right)
        direction = Random.RandInt() % 2;
        //If/else handles which sprite needs to be displayed based on direction
        if(direction){
            path = left1;
            sprite.Open("PIC_files/duck1FEH.pic");
        }
        else{
            path = right1;
            sprite.Open("PIC_files/duck2FEH.pic");
        }
    }
};

void playMovie(char path[100]){
    //Read frame data from text file that contains paths to PIC images
    FILE* textFile = fopen(path,"r");
    FEHImage frame;
    char hold[100];
    
    //Draws images from text file in sequential order
    while(fscanf(textFile, "%s", hold) != EOF){
        frame.Open(hold);
        frame.Draw(0,0);
        Sleep(0.1);
    }
    frame.Close();  
}

Button* drawButtons(char loadFile[50]){
    //For ease of development/easier code, the menu buttons are loaded from a text file
    //This text file contains the buttons text, size, position, and ID
    //FILE pointer to Menu buttons load file
    FILE* loadButtons;

    //Since the amount of buttons can be dynamically altered from the text file, the size of the array that we store the 
    //button data in must be dynamically changeable. A pointer and allocation functions are used to accomplish this
    Button* buttons;
    loadButtons = fopen(loadFile, "r");
    
    //Allocates memory for single Button 
    buttons = (Button*)malloc(sizeof(Button));   
    //Counter used to determine how many Buttons are defined the file
    int count = 0;

    //Defines holding variables for the buttons, as they only get drawn when constructor is called
    //Find a better way to do this maybe?
    char name[33];
    int x,y,width,height, id;

    //The structure here for reading the file is a little wonky
    //I was running into issues with getting the correct data into the right variables
    //However, this works so I dont want to change it

    //Scans in the first Button's Name since the name gets scanned last in the structure
    fscanf(loadButtons, "%s", name);
    do{
        //If there is more than one button, reallocate enough memory for n amount of buttons
        //while retaining the data of the buttons already there
        if(count != 0){
            buttons = (Button*)realloc(buttons, (count+1)*sizeof(Button));
        }
        //gets width/height
        fscanf(loadButtons, "%d %d", &width, &height);
        //gets x/y positions
        fscanf(loadButtons, "%d %d" , &x, &y);
        //gets ID
        fscanf(loadButtons, "%d", &id);
        //Constructsthe button/draws the button on screen
        buttons[count] = Button(x,y,width,height,name,id);
        count++;
        //If another name exists, continue reading
    }while(fscanf(loadButtons, "%s", name) != EOF);

    return buttons;
}


void typeName(int score){
    LCD.Clear();
    
    //xMouse & yMouse keeps mouse position
    float xMouse, yMouse;
    char name[9] = "";
    int counter = 0, position=130, scores[5];
    bool flag = false, clicked = false;
    
    //Load in digital keyboard from text file
    Button* keyboard = drawButtons("load_files/Keyboard.txt");
    Button enterButton = Button(250,0,60,20, "Enter", 28);
    //Output file keeps all scores
    FILE* outputFile = fopen("load_files/Scores.txt", "a+");

    //Prompt user to input name
    LCD.WriteLine("Enter your name: ");

    //Creates infinite loop to wait for Enter button to pressed
    while(1){
        
        if(LCD.Touch(&xMouse,&yMouse)){
            //Checks each Alphabet letter in the keyboard 
            for(int i = 0; i < 26; i++){
                //Checks if one is clicked, and that the mouse isnt being held down, and that the current name does not exceed the limit 
                //of 8 characters
                if(keyboard[i].hitbox.checkClick(xMouse,yMouse) && !clicked && counter != 8){
                    clicked = true;
                    counter++;

                    //In order to create typing effect, a rectangle is drawn over the name and then the full name string is drawn
                    LCD.SetFontColor(BLUE);
                    LCD.FillRectangle(0,80,320,50);
                    strcat(name, keyboard[i].text);
                    LCD.SetFontColor(BLACK);
                    LCD.WriteAt(name,130-(counter*2), 80);
                }
            }
            //Checks if backspace is clicked
            if(keyboard[26].hitbox.checkClick(xMouse, yMouse) && !clicked){
                clicked = true;
                LCD.SetFontColor(BLUE);
                LCD.FillRectangle(0,80,320,50);
                //Removes latest character
                name[counter-1] = NULL;
                counter--;
                LCD.SetFontColor(BLACK);
                LCD.WriteAt(name,130-(counter*2), 80);
            }
            //Breaks out if Enter button is clicked
            if(enterButton.hitbox.checkClick(xMouse,yMouse)){
                break;
            }
        }
        else{
            clicked = false;
        }
        
        
    }
    //Print to output file
    if(counter > 0){
        fprintf(outputFile, "\n%s %d", name, score);
    }
    //Close text file and free memory allocation
    fclose(outputFile);
    free(keyboard);


}

void gameLoop(){
    int n = 110, duckCount = 0, rounds = 0, count = 0;
    float xMouse = 0.0, yMouse = 0.0;
    bool clicked = false;
    //Create duck object
    Duck duck = Duck(0,0,50,50, n);
    //Generate the duck's flight path
    duck.generateRandomFlightPath(duck.speed);
    LCD.SetFontColor(BLACK);
    
    //Initialize 2D array to keep the paths of all different images utilized
    char backgroundPaths[4][100] = {"PIC_files/Background5FEH.pic","PIC_files/BackgroundFEH.pic", "PIC_files/Background7FEH.pic", "PIC_files/Background9FEH.pic"};
    char backgroundHunterPaths[4][100] = {"PIC_files/Background4FEH.pic","PIC_files/Background3FEH.pic", "PIC_files/Background6FEH.pic", "PIC_files/Background8FEH.pic"};
    
    //Create image objects
    FEHImage background;
    FEHImage backgroundHunter;
    FEHImage crosshair;
    crosshair.Open("PIC_files/CrosshairFEH.pic");

    background.Open(backgroundPaths[count]);
    backgroundHunter.Open(backgroundHunterPaths[count]);
    
    background.Draw(0, 0);

    //Handles the round change screen display
    for(int i = 0; i < 3; i++){
        Sleep(0.5);
        LCD.WriteAt("Round 1", 120, 100);
        Sleep(0.5);
        background.Draw(0,0);   
    }
    count++;
    //While loops ends once the duck reaches the end of its progress
    while(duck.progress < duck.speed){
        
        //Draw background image if duck is dead and a new round is happening
        if(duck.dead && duckCount % 10 == 0){
            backgroundHunter.Draw(0,0);
            Sleep(1.0);
            //Resets count so that day cycle continues
            if(count > 3){
                count = 0;
            }
            //Draw background images
            background.Open(backgroundPaths[count]);
            backgroundHunter.Open(backgroundHunterPaths[count]);
            background.Draw(0,0);
            char base[16] = "Round ";
            char round[8];
            //Print round number to the screen
            sprintf(round, "%d", (duckCount / 10) + 1);
            strcat(base, round);
            //Flash the round number 3 times for aesthetic effect
            for(int i = 0; i < 3; i++){
                Sleep(0.5);
                LCD.WriteAt(base, 120, 100);
                Sleep(0.5);
                background.Draw(0,0);
            }
            backgroundHunter.Draw(0,0);
            duck.dead = false;
            //Puts a cap on the ducks speed
            if(duck.speed > 20){
                duck.increaseSpeed();
            }
            count++;
        }

        //Draw background image
        LCD.Clear();
        backgroundHunter.Draw(0, 0);
        LCD.WriteAt(duckCount, 220, 10);

        //Update the ducks position and draw crosshair according to position of mouse
        duck.updatePosition(duck.flightpath[duck.progress].x, duck.flightpath[duck.progress].y);
        crosshair.Draw(xMouse - 20, yMouse - 20);

        //Creates the muzzle flashing effect everytime the player clicks
        if(LCD.Touch(&xMouse,&yMouse)){
            if(!clicked){
                LCD.SetFontColor(YELLOW);
                LCD.FillCircle(120, 135, 10);
                //Sleeps 10 milliseconds to allow the flash to actually show on screen
                Sleep(10);
                if(duck.hitbox.checkClick(xMouse, yMouse)){
                    //If the duck is hit, reset the progress, increase the duck count, and create a new flightpath
                    LCD.Clear();
                    duck.progress = 0;
                    duckCount++;
                    duck.dead = true;
                    duck.generateRandomFlightPath(duck.speed);
                }
            }
            clicked=true;
        }
        else{
            clicked=false;
        }
        LCD.Update();   
    } 
    //Close files
    background.Close();
    backgroundHunter.Close();
    crosshair.Close();
    duck.Kill();
    
    //Holds the paths to the different movies that play
    char badEnds[2][100] = {"PIC_files/Movies/gun/list.txt","PIC_files/Movies/dance/list.txt"};
    char goodEnds[2][100] = {"PIC_files/Movies/explosion/list.txt","PIC_files/Movies/dinnerdinner/list.txt"};
    int choice = Random.RandInt() % 2;

    //Decides which movie plays based on how many ducks the player shot
    if(duckCount > 19){
        playMovie(goodEnds[choice]);
    }
    else{
        playMovie(badEnds[choice]);
    }
    typeName(duckCount);
}

void bubbleSort(int scores[], char names[][9], int n){
    int i, j;
    bool swap;

    //Creates a typical bubble sort that sorts scores and rearranges names to match their respective score
    for(int i = 0; i < n-1; i++){
        swap = false;
        for(j=0; j < n-i-1; j++){
            if(scores[j] > scores[j+1]){
                int tempScore = scores[j];
                char tempName[9];
                strcpy(tempName, names[j]);
                scores[j] = scores[j+1];
                scores[j+1] = tempScore;
                strcpy(names[j], names[j+1]);
                strcpy(names[j+1], tempName);
                swap = true;
            }
        }

        if(!swap){
            break;
        }
    }
}

void mainMenu(){   
    //Clears screen and loads image of grass
    LCD.Clear();
    //Opens and loads the background for the main menu screen
    FEHImage background;
    background.Open("PIC_files/Background5FEH.pic");
    background.Draw(0,0);
    background.Close();

    LCD.SetFontColor(BLACK);
    Button* buttons = drawButtons("load_files/MENU_buttons.txt");
    //variables for storing click position 
    float xMouse, yMouse;
    //Tracks whether a button has been clicked, and it it has, break out of loop and allow the main menu screen to be drawn again
    bool buttonClicked = false;
    //Constructs the button, but doesnt draw it. This is done because the exit button only needs to be drawn when a selection has been made
    //And constructing a new exit button for every selection would be messy 
    Button exitButton = Button(250,0,48,20,5);
    exitButton.changeText("Exit");

    
    while(1){
        //LCD.Update();
        //Checks if the lcd screen has been touched, and if it has, updates xMouse and yMouse with the position
        if(LCD.Touch(&xMouse,&yMouse)){
            //Checks if the "Start" button has been pressed and creates the screen for that
            if(buttons[1].hitbox.checkClick(xMouse, yMouse)){
                LCD.Clear();
                //While loop to make sure screen stay on until user clicks the exit button
                gameLoop();
                break;
            }
            //Mostly same code as before but for the "Top Scores" button
            if(buttons[2].hitbox.checkClick(xMouse, yMouse)){
                LCD.Clear();
                //Open the top scores text file for reading
                FILE* topScores = fopen("load_files/Scores.txt","r");
                char base[25], name[9], scoreBuff[8];
                int score, counter, lines;

                LCD.WriteLine("Top Scores:");
                exitButton.drawButton();

                //Lines represents the number of lines in the file, while counter is an indexing variable
                lines = 0;
                counter = 0;
                //Load data from text file and store it to array here
                while(fscanf(topScores, "%*s %*d", &name, &score) != EOF){
                    lines++;
                }
                //Creates variables to store names/scores for storage
                char names[lines][9];
                int scores[lines];
                //rewinds file to start from beginning
                rewind(topScores);
                while(fscanf(topScores, "%s %d", names[counter], &scores[counter]) != EOF){
                    counter++;
                }
                //Sorts the scores
                bubbleSort(scores, names, lines);
                //Limits the number of Scores shown on screen to 12
                int limit;
                if(lines > 12){
                    limit = lines - 12;
                }
                else{
                    limit = 0;
                }
                //"Character array manipulation" doesn't have the same ring to it :(
                //Conjoins the name and score and displays it on screen
                for(int i = lines-1; i >= limit; i--){
                    memset(base, 0, 25);
                    sprintf(scoreBuff, "%d", scores[i]);
                    strcat(base, names[i]);
                    strcat(base, " ");
                    strcat(base, scoreBuff);
                    LCD.WriteLine(base);
                }
                
                //Keeps the screen open while waiting for button to be pressed
                while(1){
                    LCD.Update();
                    if(LCD.Touch(&xMouse,&yMouse)){
                        if(exitButton.hitbox.checkClick(xMouse, yMouse)){
                            buttonClicked = true;
                            break;
                        }
                    }
                }

            }
            //Handles "Instructions" button
            if(buttons[3].hitbox.checkClick(xMouse, yMouse)){ 
                LCD.Clear();
                LCD.WriteLine("Instructions:");
                LCD.WriteLine("Shoot the duck. If you miss, you lose. As you rack up points, the speed of the ducks will increase.");
                exitButton.drawButton();
                while(1){
                    //Handles the use of exit button
                    LCD.Update();
                    if(LCD.Touch(&xMouse,&yMouse)){
                        if(exitButton.hitbox.checkClick(xMouse, yMouse)){
                            buttonClicked = true;
                            break;
                        }
                    }
                }
            }
            //Handles "Credits" button
            if(buttons[4].hitbox.checkClick(xMouse, yMouse)){
                //put a function here 
                LCD.Clear();
                //Writes our names to the credit tab
                LCD.WriteLine("Credits:");
                LCD.WriteLine("Connor Jackson McGuffey");
                LCD.WriteLine("Cole Christopher Gibson");
                exitButton.drawButton();
                while(1){
                    //Handles the use of exit button
                    LCD.Update();
                    if(LCD.Touch(&xMouse,&yMouse)){
                        if(exitButton.hitbox.checkClick(xMouse, yMouse)){
                            buttonClicked = true;
                            break;
                        }
                    }
                }
            }
            
        }
        //Breaks out of main loop if exit button in not the main menu was pressed
        if(buttonClicked){
            break;
        }
        
    }
    //Frees allocated memory
    free(buttons);        
}

int main()
{
    //Main loop
    LCD.Clear(BLUE);
    while(1){
        LCD.Update();
        mainMenu();
    }
    return 0;
}